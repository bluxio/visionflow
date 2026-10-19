//! Immutable init-only world caches and fresh host-backed map/vision/session operations.
//! Cached reads need the cache that `cambc::init` loads before `main`; getters never populate the cache.
use crate::{
    check, query, sys, Bitboard, Camp, CampGroupHandle, CampHandle, CampType, Champion,
    ChampionDraft, ChampionHandle, ChampionType, Constants, Entity, Error, ErrorCode, Result,
    Symmetry, Team, Vec2,
};
use std::sync::OnceLock;

struct State {
    constants: Constants,
    self_champion: Option<Champion>,
    champion_designs: [Champion; 12],
    memory: (SharedMemory, DraftMemory),
    symmetry: Symmetry,
    terrain: [Bitboard; 4],
    spawns: [Bitboard; 2],
}
const _: () = assert!(ChampionType::ALL.len() == 12);
#[cfg(not(test))]
static STATE: OnceLock<Box<State>> = OnceLock::new();
#[cfg(test)]
thread_local! {
    static STATE: std::cell::RefCell<OnceLock<&'static State>> = const {
        std::cell::RefCell::new(OnceLock::new())
    };
}
/// Read the initialised cache without lazy initialisation or host calls.
fn initialized() -> Option<&'static State> {
    #[cfg(not(test))]
    {
        STATE.get().map(|state| &**state)
    }
    #[cfg(test)]
    {
        STATE.with(|state| state.borrow().get().copied())
    }
}
/// Read the initialised cache or return an explicit init precondition error.
fn state() -> Result<&'static State> {
    initialized().ok_or_else(|| {
        Error::invalid_argument("SDK initialization failed; call cambc::init() for the error")
    })
}
/// Reset the thread-local test cache; leaked fixture states preserve existing static borrows.
#[cfg(test)]
pub(crate) fn reset_for_tests() {
    STATE.with(|state| {
        state.borrow_mut().take();
    });
}
/// Initialise immutable SDK data. A static constructor runs this before `main` on the first
/// turn's fuel, so calling it is only needed to retry or inspect a failed initialisation.
/// Successful repeat calls do nothing; failures publish nothing and can be retried.
/// No map layout is cached.
pub fn init() -> Result<()> {
    if initialized().is_some() {
        return Ok(());
    }
    let state = load_state()?;
    #[cfg(not(test))]
    {
        let _ = STATE.set(state);
    }
    #[cfg(test)]
    STATE.with(|cache| {
        let _ = cache.borrow().set(Box::leak(state));
    });
    Ok(())
}
/// Run [`init`] from wasi-libc's constructors in `_start`, before `main`; errors resurface from
/// cached accessors and from explicit `init` calls.
#[cfg(all(target_arch = "wasm32", not(test)))]
#[used]
#[link_section = ".init_array"]
static AUTO_INIT: extern "C" fn() = {
    extern "C" fn auto_init() {
        let _ = init();
    }
    auto_init
};
/// Read shared-memory capacity and the draft-memory snapshot.
fn load_memory() -> Result<(SharedMemory, DraftMemory)> {
    let capacity = query(|p| unsafe { sys::shared_memory_size(p) })? as usize;
    let size = query(|p| unsafe { sys::draft_memory_size(p) })?;
    let mut draft_memory = vec![0; size as usize];
    if size != 0 {
        check(unsafe { sys::read_draft_memory(0, draft_memory.as_mut_ptr().cast(), size) })?;
    }
    Ok((SharedMemory { capacity }, DraftMemory { draft_memory }))
}
/// Build all immutable snapshots in place before `init` publishes a single complete cache.
fn load_state() -> Result<Box<State>> {
    let layout = crate::board::layout()?;
    let h = unsafe { sys::my_champion() };
    let self_champion = if h == -1 {
        None
    } else if h >= 0 {
        Some(Champion::new(ChampionHandle(h as u32))?)
    } else {
        return Err(Error::invalid_value("self champion", h as i64));
    };
    let mut terrain = Vec::with_capacity(4);
    for kind in 0..4 {
        // SAFETY: bc_map_mask copies every chunk of the terrain board into the buffer.
        let mask = unsafe {
            Bitboard::from_host(
                layout.board_bytes as usize / 16,
                layout.width,
                layout.height,
                |p| sys::bc_map_mask(kind, p),
            )
        }?;
        terrain.push(mask);
    }
    let terrain = terrain
        .try_into()
        .map_err(|_| Error::invalid_argument("invalid terrain count"))?;
    let mut spawns = Vec::with_capacity(2);
    for team in [Team::Ally, Team::Enemy] {
        let team = team.absolute()?;
        let bounds = query(|p| unsafe { sys::spawn_region(team, p) })?;
        let mut region = Bitboard::from_layout(&layout)?;
        for y in bounds.min_y..bounds.max_y {
            for x in bounds.min_x..bounds.max_x {
                region.set(Vec2::new(x, y), true)?;
            }
        }
        spawns.push(region);
    }
    let spawns = spawns
        .try_into()
        .map_err(|_| Error::invalid_argument("invalid spawn count"))?;
    let memory = load_memory()?;
    let mut state = Box::<State>::new_uninit();
    let s = state.as_mut_ptr();
    // SAFETY: only fields without owned heap data are written before the last fallible
    // call, so an error frees the box without leaking; every field is written before
    // `assume_init`.
    unsafe {
        core::ptr::addr_of_mut!((*s).constants).write(crate::constants::read_constants()?);
        let designs = core::ptr::addr_of_mut!((*s).champion_designs).cast::<Champion>();
        for (i, kind) in ChampionType::ALL.iter().enumerate() {
            Champion::write_design(*kind, designs.add(i))?;
        }
        let symmetry = query(|p| sys::bc_map_symmetry(p))?;
        core::ptr::addr_of_mut!((*s).symmetry).write(Symmetry::try_from(symmetry)?);
        core::ptr::addr_of_mut!((*s).self_champion).write(self_champion);
        core::ptr::addr_of_mut!((*s).memory).write(memory);
        core::ptr::addr_of_mut!((*s).terrain).write(terrain);
        core::ptr::addr_of_mut!((*s).spawns).write(spawns);
        Ok(state.assume_init())
    }
}
/// Borrow immutable game constants loaded by [`cambc::init`](crate::init), without host calls.
///
/// # Panics
/// Panics if SDK initialisation failed.
///
/// # Examples
/// ```no_run
/// let max_rounds = cambc::constants().max_rounds;
/// # Ok::<(), cambc::Error>(())
/// ```
#[must_use]
pub fn constants() -> &'static Constants {
    &initialized()
        .expect("SDK initialization failed; call cambc::init() for the error")
        .constants
}
/// Borrow self's initialised wrapper, or `None` in draft; errors if SDK initialisation failed.
pub fn self_champion() -> Result<Option<&'static Champion>> {
    Ok(state()?.self_champion.as_ref())
}
/// Check whether the guest is running in the draft phase, where no self champion exists.
#[must_use]
pub fn is_draft() -> bool {
    unsafe { sys::my_champion() < 0 }
}
/// Borrow all initialised champion designs, with no live cooldown/visibility queries.
pub fn champion_designs() -> Result<&'static [Champion; 12]> {
    Ok(&state()?.champion_designs)
}
/// Borrow this team's shared-memory interface, initialised by [`cambc::init`](crate::init).
/// The accessor never queries the host or initialises the cache.
///
/// # Panics
/// Panics if SDK initialisation failed.
///
/// # Examples
/// ```no_run
/// let memory = cambc::shared_memory();
/// memory.set(0, 42)?;
/// assert_eq!(memory.get(0)?, 42);
/// memory.write(1, &[1, 2])?;
/// let mut bytes = [0; 2];
/// memory.read(1, &mut bytes)?;
/// # Ok::<(), cambc::Error>(())
/// ```
#[must_use]
pub fn shared_memory() -> &'static SharedMemory {
    &initialized()
        .map(|state| &state.memory)
        .expect("SDK initialization failed; call cambc::init() for the error")
        .0
}
/// Borrow the initialised draft-memory interface; submission never changes its snapshot.
///
/// # Panics
/// Panics if SDK initialisation failed.
#[must_use]
pub fn draft_memory() -> &'static DraftMemory {
    &initialized()
        .map(|state| &state.memory)
        .expect("SDK initialization failed; call cambc::init() for the error")
        .1
}
pub(crate) fn flush_output() {
    use std::io::Write;
    let _ = std::io::stdout().flush();
    let _ = std::io::stderr().flush();
}
/// Flush buffered output and yield to end the current guest turn; resumed execution continues after this call.
pub fn end_turn() {
    flush_output();
    unsafe { sys::r#yield() };
}
/// Resign the current team through the engine, preserving validation errors.
/// A successful resign ends the game without returning, so buffered output is flushed first.
pub fn resign() -> Result<()> {
    flush_output();
    check(unsafe { sys::resign() })
}
/// Return remaining guest fuel (10x on a team's first draft turn, 2x on each champion's first turn); invalid negative host values return an error.
pub fn fuel_remaining() -> Result<u64> {
    let value = unsafe { sys::fuel_remaining() };
    u64::try_from(value).map_err(|_| Error::invalid_value("fuel", value))
}
/// All allied/enemy roster members, including hidden champions with cached visibility errors.
///
/// # Arguments
/// * `team` - `Some(team)` for one side, or `None` for both.
pub fn champions(team: Option<Team>) -> Result<Vec<Champion>> {
    let mut result = Vec::new();
    for t in [Team::Ally, Team::Enemy] {
        if team.is_some_and(|s| s != t) {
            continue;
        }
        let count = query(|p| unsafe {
            match t {
                Team::Ally => sys::num_allies(p),
                Team::Enemy => sys::num_enemies(p),
            }
        })?;
        let handles = crate::counted_list(count, |p, n, out| unsafe {
            match t {
                Team::Ally => sys::allies(p, n, out),
                Team::Enemy => sys::enemies(p, n, out),
            }
        })?;
        for h in handles {
            result.push(Champion::new(ChampionHandle(h))?);
        }
    }
    Ok(result)
}
/// Query the current engine round number.
pub fn round_number() -> Result<u32> {
    query(|p| unsafe { sys::bc_round_number(p) })
}
/// Live visible camp-group state; respawn countdown is present only while respawning.
#[derive(Clone, Copy, Debug, PartialEq, Eq, Hash)]
pub enum CampGroupState {
    /// The group has living camps.
    Alive,
    /// The defeated group's reward is available.
    RewardAvailable,
    /// The group is respawning with a remaining engine countdown.
    Respawning(i32),
}
/// Camp membership and rewards queried from the host; live state is queried separately.
#[derive(Clone, Debug)]
pub struct CampGroup {
    handle: CampGroupHandle,
    gold: u32,
    gems: u32,
    respawn: i32,
    camps: Vec<Camp>,
}
impl PartialEq for CampGroup {
    fn eq(&self, b: &Self) -> bool {
        self.handle == b.handle
    }
}
impl Eq for CampGroup {}
impl core::hash::Hash for CampGroup {
    fn hash<H: core::hash::Hasher>(&self, state: &mut H) {
        self.handle.hash(state);
    }
}
impl CampGroup {
    /// Return the stored engine handle.
    #[cfg(test)]
    pub(crate) fn handle(&self) -> CampGroupHandle {
        self.handle
    }
    /// Return the captured camp-group gold reward.
    #[must_use]
    pub fn gold_value(&self) -> u32 {
        self.gold
    }
    /// Return the captured camp-group gem reward.
    #[must_use]
    pub fn gem_value(&self) -> u32 {
        self.gems
    }
    /// Return the captured maximum group respawn timer.
    #[must_use]
    pub fn max_respawn_timer(&self) -> i32 {
        self.respawn
    }
    /// Borrow the immutable camp membership, including symmetry-seam merged groups.
    #[must_use]
    pub fn camps(&self) -> &[Camp] {
        &self.camps
    }
    /// Borrow an iterator over immutable camp membership.
    pub fn iter(&self) -> core::slice::Iter<'_, Camp> {
        self.camps.iter()
    }
    /// Build the camp-spawn tiles; computed on every call.
    pub fn spawn_tiles(&self) -> Result<Bitboard> {
        let mut mask = Bitboard::new()?;
        self.spawn_tiles_into(&mut mask)?;
        Ok(mask)
    }
    /// Like [`Self::spawn_tiles`], but writes into `out`.
    ///
    /// # Arguments
    /// * `out` - Destination mask; its previous contents are overwritten.
    pub fn spawn_tiles_into(&self, out: &mut Bitboard) -> Result<()> {
        out.clear();
        for camp in &self.camps {
            out.set(camp.spawn_position(), true)?;
        }
        Ok(())
    }
    /// Whether any camp spawn tile is set in the given vision mask.
    fn visible_in(&self, visible: &Bitboard) -> Result<bool> {
        for camp in &self.camps {
            if visible.is_set(camp.spawn_position())? {
                return Ok(true);
            }
        }
        Ok(false)
    }
    /// Query group state only when its spawn mask intersects current vision.
    /// Hidden/unknown groups return `ApiNotInVision`; invalid runtime tags remain errors.
    pub fn state(&self) -> Result<CampGroupState> {
        let visible = Bitboard::query(|p| unsafe { sys::vision(p) })?;
        if !self.visible_in(&visible)? {
            return Err(Error::local(
                ErrorCode::ApiNotInVision,
                "camp group is not visible",
            ));
        }
        match query(|p| unsafe { sys::camp_group_state(self.handle.0, p) })? {
            0 => Err(Error::local(
                ErrorCode::ApiNotInVision,
                "camp group state is unknown",
            )),
            1 => Ok(CampGroupState::Alive),
            2 => Ok(CampGroupState::RewardAvailable),
            3 => Ok(CampGroupState::Respawning(query(|p| unsafe {
                sys::camp_group_cooldown(self.handle.0, p)
            })?)),
            v => Err(Error::invalid_value("camp group state", v as i64)),
        }
    }
}
impl<'a> IntoIterator for &'a CampGroup {
    type Item = &'a Camp;
    type IntoIter = core::slice::Iter<'a, Camp>;
    fn into_iter(self) -> Self::IntoIter {
        self.iter()
    }
}

/// Read initialised map width without querying or initialising the host cache.
pub fn width() -> Result<i32> {
    Ok(state()?.terrain[0].width())
}
/// Read initialised map height without querying or initialising the host cache.
pub fn height() -> Result<i32> {
    Ok(state()?.terrain[0].height())
}
/// Check a position against initialised map dimensions; errors if SDK initialisation failed.
///
/// # Arguments
/// * `p` - Position to check.
pub fn is_in_bounds(p: Vec2) -> Result<bool> {
    let board = &state()?.terrain[0];
    Ok(p.x >= 0 && p.y >= 0 && p.x < board.width() && p.y < board.height())
}
/// Read the initialised map symmetry without a host query.
pub fn symmetry() -> Result<Symmetry> {
    Ok(state()?.symmetry)
}
/// Reflect a map position horizontally, reversing tile x coordinates.
///
/// # Arguments
/// * `position` - Map position to reflect.
pub fn reflect_horizontal(position: Vec2) -> Result<Vec2> {
    let mut result = Vec2::default();
    check(unsafe {
        sys::Bitboard_ReflectHorizontalPosition(
            position.x,
            position.y,
            &mut result.x,
            &mut result.y,
        )
    })?;
    Ok(result)
}
/// Reflect a map position vertically, reversing tile y coordinates.
///
/// # Arguments
/// * `position` - Map position to reflect.
pub fn reflect_vertical(position: Vec2) -> Result<Vec2> {
    let mut result = Vec2::default();
    check(unsafe {
        sys::Bitboard_ReflectVerticalPosition(position.x, position.y, &mut result.x, &mut result.y)
    })?;
    Ok(result)
}
/// Rotate a map position 180 degrees within the map dimensions.
///
/// # Arguments
/// * `position` - Map position to rotate.
pub fn rotate_180(position: Vec2) -> Result<Vec2> {
    let mut result = Vec2::default();
    check(unsafe {
        sys::Bitboard_Rotate180Position(position.x, position.y, &mut result.x, &mut result.y)
    })?;
    Ok(result)
}
/// Borrow the initialised wall mask; errors if SDK initialisation failed.
pub fn walls() -> Result<&'static Bitboard> {
    Ok(&state()?.terrain[0])
}
/// Borrow the initialised bush mask; errors if SDK initialisation failed.
pub fn bushes() -> Result<&'static Bitboard> {
    Ok(&state()?.terrain[1])
}
/// Borrow the initialised hole mask; errors if SDK initialisation failed.
pub fn holes() -> Result<&'static Bitboard> {
    Ok(&state()?.terrain[2])
}
/// Borrow the initialised altar mask; errors if SDK initialisation failed.
pub fn altars() -> Result<&'static Bitboard> {
    Ok(&state()?.terrain[3])
}
/// Borrow the initialised spawn-region mask for the relative team.
///
/// # Arguments
/// * `team` - Team whose spawn region to return.
pub fn spawn_region(team: Team) -> Result<&'static Bitboard> {
    Ok(&state()?.spawns[if team == Team::Ally { 0 } else { 1 }])
}
/// Query engine groups on every call, including merged symmetry-seam clusters; also available in draft.
pub fn camp_groups() -> Result<Vec<CampGroup>> {
    state()?;
    load_camp_groups()
}
/// Query one group's rewards and membership from the host.
pub(crate) fn camp_group(handle: CampGroupHandle) -> Result<CampGroup> {
    state()?;
    load_camp_group(handle.0)
}
/// Load authoritative host group membership, including merged symmetry-seam groups.
fn load_camp_groups() -> Result<Vec<CampGroup>> {
    let mut words = vec![0u32; 256];
    let mut count = 0u32;
    crate::check(unsafe { sys::map_camp_layout(words.as_mut_ptr(), words.len() as u32, &mut count) })?;
    if count as usize > words.len() {
        words.resize(count as usize, 0);
        crate::check(unsafe { sys::map_camp_layout(words.as_mut_ptr(), count, &mut count) })?;
    }
    words.truncate(count as usize);
    let malformed = || Error::local(ErrorCode::InvalidArgument, "malformed camp layout");
    let mut groups = Vec::new();
    let mut rest = &words[..];
    while let [h, gold, gems, respawn, n, tail @ ..] = rest {
        let members = tail.get(..*n as usize * 4).ok_or_else(malformed)?;
        let camps = members
            .chunks_exact(4)
            .map(|c| {
                Ok(Camp::new(
                    CampHandle(c[0]),
                    CampGroupHandle(*h),
                    CampType::try_from(c[1] as i32)?,
                    Vec2::new(c[2] as i32, c[3] as i32),
                ))
            })
            .collect::<Result<Vec<_>>>()?;
        groups.push(CampGroup {
            handle: CampGroupHandle(*h),
            gold: *gold,
            gems: *gems,
            respawn: *respawn as i32,
            camps,
        });
        rest = &tail[members.len()..];
    }
    if !rest.is_empty() {
        return Err(malformed());
    }
    Ok(groups)
}
fn load_camp_group(h: u32) -> Result<CampGroup> {
    let rewards = query(|p| unsafe { sys::map_camp_group(h, p) })?;
    let respawn = query(|p| unsafe { sys::bc_camp_group_max_respawn(h, p) })?;
    let mut camps = Vec::new();
    let count = query(|p| unsafe { sys::map_camp_group_num_camps(h, p) })?;
    for member in crate::counted_list(count, |p, n, out| unsafe {
        sys::map_camp_group_camps(h, p, n, out)
    })? {
        let info = query(|p| unsafe { sys::map_camp_info(member, p) })?;
        camps.push(Camp::new(
            CampHandle(member),
            CampGroupHandle(h),
            CampType::try_from(info.kind)?,
            Vec2::new(info.x, info.y),
        ));
    }
    Ok(CampGroup {
        handle: CampGroupHandle(h),
        gold: rewards.gold,
        gems: rewards.gems,
        respawn,
        camps,
    })
}
/// Find the camp at an immutable spawn position, querying the host on every call.
/// No spawn returns `ApiNotFound`; this is not a live-position/visibility lookup.
///
/// # Arguments
/// * `position` - Spawn tile of the camp.
pub fn camp(position: Vec2) -> Result<Camp> {
    state()?;
    load_camp(query(|p| unsafe { sys::map_camp_at(position.x, position.y, p) })?)
}
fn load_camp(h: u32) -> Result<Camp> {
    let info = query(|p| unsafe { sys::map_camp_info(h, p) })?;
    let group = query(|p| unsafe { sys::map_camp_group_of(h, p) })?;
    Ok(Camp::new(
        CampHandle(h),
        CampGroupHandle(group),
        CampType::try_from(info.kind)?,
        Vec2::new(info.x, info.y),
    ))
}
/// An immutable snapshot of a visible ordinary ward.
#[derive(Clone, Copy, Debug, PartialEq, Eq, Hash)]
pub struct Ward {
    pub(crate) position: Vec2,
    pub(crate) team: Team,
    pub(crate) remaining_time: Option<i32>,
}
impl Ward {
    /// Decode an engine ward record.
    pub(crate) fn from_raw(v: &sys::WardInfo) -> Result<Self> {
        Ok(Self {
            position: Vec2::new(v.x, v.y),
            team: Team::from_absolute(v.team as i32)?,
            remaining_time: if v.timer < 0 { None } else { Some(v.timer) },
        })
    }
    /// Query a fresh hypothetical ordinary-ward vision mask at `position`.
    ///
    /// # Arguments
    /// * `position` - Hypothetical ward tile.
    pub fn visible_tiles_from(position: Vec2) -> Result<Bitboard> {
        Bitboard::query(|p| unsafe { sys::ward_vision(position.x, position.y, p) })
    }
    /// Query hypothetical vision at the ward's position.
    pub fn visible_tiles(&self) -> Result<Bitboard> {
        Self::visible_tiles_from(self.position)
    }
    /// Like [`Self::visible_tiles_from`], but writes into `out`.
    ///
    /// # Arguments
    /// * `position` - Hypothetical ward tile.
    /// * `out` - Destination mask; its previous contents are overwritten.
    pub fn visible_tiles_from_into(position: Vec2, out: &mut Bitboard) -> Result<()> {
        crate::check(unsafe { sys::ward_vision(position.x, position.y, out.ptr_mut()) })
    }
    /// Like [`Self::visible_tiles`], but writes into `out`.
    ///
    /// # Arguments
    /// * `out` - Destination mask; its previous contents are overwritten.
    pub fn visible_tiles_into(&self, out: &mut Bitboard) -> Result<()> {
        Self::visible_tiles_from_into(self.position, out)
    }
    /// Return the ward's tile.
    #[must_use]
    pub fn position(&self) -> Vec2 {
        self.position
    }
    /// Return the owning team relative to the observing guest.
    #[must_use]
    pub fn team(&self) -> Team {
        self.team
    }
    /// Return whether the ward belongs to the observing team.
    #[must_use]
    pub fn is_ally(&self) -> bool {
        self.team() == Team::Ally
    }
    /// Return whether the ward belongs to the opposing team.
    #[must_use]
    pub fn is_enemy(&self) -> bool {
        self.team() == Team::Enemy
    }
    /// Return the remaining lifetime, or `None` for an unlimited lifetime.
    #[must_use]
    pub fn remaining_time(&self) -> Option<i32> {
        self.remaining_time
    }
}
/// An owning visible teleport reservation, including the observer's own channel.
/// The caster type is internal metadata used by the accessor and hypothetical vision query.
#[derive(Clone, Copy, Debug, PartialEq, Eq, Hash)]
pub struct Teleport {
    /// Caster champion handle, not a copied live champion.
    champion: ChampionHandle,
    kind: ChampionType,
    /// Team relative to the observing guest.
    team: Team,
    /// Reserved teleport destination.
    destination: Vec2,
    /// Captured remaining teleport-channel countdown.
    remaining_time: i32,
}
impl Teleport {
    /// Construct a live wrapper for the teleport caster, preserving host visibility errors.
    pub fn champion(&self) -> Result<Champion> {
        Champion::new(self.champion)
    }
    /// Return the captured caster team relative to the observer.
    #[must_use]
    pub fn team(&self) -> Team {
        self.team
    }
    /// Return whether the teleport belongs to the observing team.
    #[must_use]
    pub fn is_ally(&self) -> bool {
        self.team() == Team::Ally
    }
    /// Return whether the teleport belongs to the opposing team.
    #[must_use]
    pub fn is_enemy(&self) -> bool {
        self.team() == Team::Enemy
    }
    /// Return the captured reserved destination, not the caster's current position.
    #[must_use]
    pub fn position(&self) -> Vec2 {
        self.destination
    }
    /// Return the captured teleport-channel countdown.
    #[must_use]
    pub fn remaining_time(&self) -> i32 {
        self.remaining_time
    }
    /// Query hypothetical caster vision after arrival at the reserved destination.
    /// This is not a mask of tiles currently visible to the caster.
    pub fn visible_tiles(&self) -> Result<Bitboard> {
        Bitboard::query(|p| unsafe {
            sys::hypothetical_vision(self.kind as i32, self.destination.x, self.destination.y, p)
        })
    }
    /// Like [`Self::visible_tiles`], but writes into `out`.
    ///
    /// # Arguments
    /// * `out` - Destination mask; its previous contents are overwritten.
    pub fn visible_tiles_into(&self, out: &mut Bitboard) -> Result<()> {
        crate::check(unsafe {
            sys::hypothetical_vision(
                self.kind as i32,
                self.destination.x,
                self.destination.y,
                out.ptr_mut(),
            )
        })
    }
}
/// An owning visible altar-lock snapshot with a targetable engine handle.
#[derive(Clone, Copy, Debug, PartialEq, Eq, Hash)]
pub struct AltarLock {
    /// Engine identity, not a visibility or ownership guarantee.
    handle: u32,
    /// Team relative to the observing guest.
    team: Team,
    /// Observed map position.
    position: Vec2,
    /// Hits still required to destroy this altar lock.
    hits_remaining: u32,
}
impl AltarLock {
    fn decode(i: sys::AltarLockInfo) -> Result<Self> {
        if i.kind != crate::EntityKind::AltarLock as u32 {
            return Err(Error::invalid_value("altar lock kind", i.kind as i64));
        }
        Ok(Self {
            handle: i.handle,
            team: Team::from_absolute(i.team as i32)?,
            position: Vec2::new(i.x, i.y),
            hits_remaining: i.hits_remaining,
        })
    }
    /// Return this handle-backed altar lock as an action target.
    pub(crate) fn entity(&self) -> Entity {
        Entity(crate::EntityTarget::AltarLock(self.handle))
    }
    /// Return the captured relative team.
    #[must_use]
    pub fn team(&self) -> Team {
        self.team
    }
    /// Return whether the altar lock belongs to the observing team.
    #[must_use]
    pub fn is_ally(&self) -> bool {
        self.team() == Team::Ally
    }
    /// Return whether the altar lock belongs to the opposing team.
    #[must_use]
    pub fn is_enemy(&self) -> bool {
        self.team() == Team::Enemy
    }
    /// Return the captured altar-lock position.
    #[must_use]
    pub fn position(&self) -> Vec2 {
        self.position
    }
    /// Return the captured hits still required to break the altar lock.
    #[must_use]
    pub fn hits_remaining(&self) -> u32 {
        self.hits_remaining
    }
}
impl crate::detail::Sealed for AltarLock {
    fn entity(&self) -> Result<Entity> {
        Ok(AltarLock::entity(self))
    }
}
impl crate::AttackTarget for AltarLock {}
/// Convert missing or hidden observations into absence, preserving other failures.
fn optional<T>(r: Result<T>) -> Result<Option<T>> {
    match r {
        Ok(v) => Ok(Some(v)),
        Err(e)
            if matches!(
                e.code(),
                Some(ErrorCode::ApiNotFound | ErrorCode::ApiNotInVision)
            ) =>
        {
            Ok(None)
        }
        Err(e) => Err(e),
    }
}

/// Query a fresh owning mask of current visible tiles.
pub fn visible_tiles() -> Result<Bitboard> {
    Bitboard::query(|p| unsafe { sys::vision(p) })
}
/// Query the current visible camp occupancy mask.
pub fn camp_tiles() -> Result<Bitboard> {
    Bitboard::query(|p| unsafe { sys::camp_occupancy(p) })
}
/// Query current visible teleport-destination occupancy.
pub fn teleport_tiles() -> Result<Bitboard> {
    Bitboard::query(|p| unsafe { sys::teleport_occupancy(p) })
}
/// Query visible champion occupancy for `team`, or both sides when `None`.
///
/// # Arguments
/// * `team` - `Some(team)` for one side, or `None` for both.
pub fn champion_tiles(team: Option<Team>) -> Result<Bitboard> {
    match team {
        Some(t) => Bitboard::query(|p| unsafe {
            if t == Team::Ally {
                sys::ally_occupancy(p)
            } else {
                sys::enemy_occupancy(p)
            }
        }),
        None => champion_tiles(Some(Team::Ally))?.union(&champion_tiles(Some(Team::Enemy))?),
    }
}
/// Query visible ordinary-ward occupancy for `team`, or both sides when `None`.
///
/// # Arguments
/// * `team` - `Some(team)` for one side, or `None` for both.
pub fn ward_tiles(team: Option<Team>) -> Result<Bitboard> {
    match team {
        Some(Team::Ally) => Bitboard::query(|p| unsafe { sys::ally_ward_occupancy(p) }),
        Some(Team::Enemy) => Bitboard::query(|p| unsafe { sys::enemy_ward_occupancy(p) }),
        None => ward_tiles(Some(Team::Ally))?.union(&ward_tiles(Some(Team::Enemy))?),
    }
}
/// Query visible altar-lock occupancy for `team`, or both sides when `None`.
///
/// # Arguments
/// * `team` - `Some(team)` for one side, or `None` for both.
pub fn altar_lock_tiles(team: Option<Team>) -> Result<Bitboard> {
    match team {
        Some(Team::Ally) => Bitboard::query(|p| unsafe { sys::ally_altar_lock_occupancy(p) }),
        Some(Team::Enemy) => Bitboard::query(|p| unsafe { sys::enemy_altar_lock_occupancy(p) }),
        None => altar_lock_tiles(Some(Team::Ally))?.union(&altar_lock_tiles(Some(Team::Enemy))?),
    }
}
/// Like [`visible_tiles`], but writes into `out`.
///
/// # Arguments
/// * `out` - Destination mask; its previous contents are overwritten.
pub fn visible_tiles_into(out: &mut Bitboard) -> Result<()> {
    crate::check(unsafe { sys::vision(out.ptr_mut()) })
}
/// Like [`camp_tiles`], but writes into `out`.
///
/// # Arguments
/// * `out` - Destination mask; its previous contents are overwritten.
pub fn camp_tiles_into(out: &mut Bitboard) -> Result<()> {
    crate::check(unsafe { sys::camp_occupancy(out.ptr_mut()) })
}
/// Like [`teleport_tiles`], but writes into `out`.
///
/// # Arguments
/// * `out` - Destination mask; its previous contents are overwritten.
pub fn teleport_tiles_into(out: &mut Bitboard) -> Result<()> {
    crate::check(unsafe { sys::teleport_occupancy(out.ptr_mut()) })
}
/// Like [`champion_tiles`], but writes into `out`.
///
/// # Arguments
/// * `team` - `Some(team)` for one side, or `None` for both.
/// * `out` - Destination mask; its previous contents are overwritten.
pub fn champion_tiles_into(team: Option<Team>, out: &mut Bitboard) -> Result<()> {
    match team {
        Some(Team::Ally) => crate::check(unsafe { sys::ally_occupancy(out.ptr_mut()) }),
        Some(Team::Enemy) => crate::check(unsafe { sys::enemy_occupancy(out.ptr_mut()) }),
        None => {
            champion_tiles_into(Some(Team::Ally), out)?;
            out.union_assign(&champion_tiles(Some(Team::Enemy))?)
        }
    }
}
/// Like [`ward_tiles`], but writes into `out`.
///
/// # Arguments
/// * `team` - `Some(team)` for one side, or `None` for both.
/// * `out` - Destination mask; its previous contents are overwritten.
pub fn ward_tiles_into(team: Option<Team>, out: &mut Bitboard) -> Result<()> {
    match team {
        Some(Team::Ally) => crate::check(unsafe { sys::ally_ward_occupancy(out.ptr_mut()) }),
        Some(Team::Enemy) => crate::check(unsafe { sys::enemy_ward_occupancy(out.ptr_mut()) }),
        None => {
            ward_tiles_into(Some(Team::Ally), out)?;
            out.union_assign(&ward_tiles(Some(Team::Enemy))?)
        }
    }
}
/// Like [`altar_lock_tiles`], but writes into `out`.
///
/// # Arguments
/// * `team` - `Some(team)` for one side, or `None` for both.
/// * `out` - Destination mask; its previous contents are overwritten.
pub fn altar_lock_tiles_into(team: Option<Team>, out: &mut Bitboard) -> Result<()> {
    match team {
        Some(Team::Ally) => crate::check(unsafe { sys::ally_altar_lock_occupancy(out.ptr_mut()) }),
        Some(Team::Enemy) => {
            crate::check(unsafe { sys::enemy_altar_lock_occupancy(out.ptr_mut()) })
        }
        None => {
            altar_lock_tiles_into(Some(Team::Ally), out)?;
            out.union_assign(&altar_lock_tiles(Some(Team::Enemy))?)
        }
    }
}
/// Return owning ordinary-ward observations, optionally filtered by team.
/// Overlapping observations remain separate entries.
///
/// # Arguments
/// * `team` - `Some(team)` for one side, or `None` for both.
pub fn wards(team: Option<Team>) -> Result<Vec<Ward>> {
    let mut wards = Vec::new();
    for v in crate::list(|p, n, out| unsafe { sys::wards(p, n, out) })? {
        let w = Ward::from_raw(&v)?;
        if team.is_none_or(|t| w.team == t) {
            wards.push(w);
        }
    }
    Ok(wards)
}
fn find_ward_handle(position: Vec2, team: Option<Team>) -> Result<u32> {
    let handle = query(|p| unsafe {
        match team {
            None => sys::ward_at(position.x, position.y, p),
            Some(Team::Ally) => sys::ally_ward_at(position.x, position.y, p),
            Some(Team::Enemy) => sys::enemy_ward_at(position.x, position.y, p),
        }
    })?;
    Ok(handle)
}
/// Return a visible ordinary ward for `team`.
///
/// # Arguments
/// * `position` - Tile to query.
/// * `team` - `Some(team)` for one side, or `None` for both.
pub fn ward_at(position: Vec2, team: Option<Team>) -> Result<Ward> {
    let h = find_ward_handle(position, team)?;
    Ward::from_raw(&query(|p| unsafe { sys::ward(h, p) })?)
}
/// Test for a visible ward belonging to `team`; missing or hidden wards return `false`.
///
/// # Arguments
/// * `position` - Tile to query.
/// * `team` - `Some(team)` for one side, or `None` for both.
pub fn has_ward(position: Vec2, team: Option<Team>) -> Result<bool> {
    Ok(optional(find_ward_handle(position, team))?.is_some())
}
/// Query whether a visible teleport reservation exists at `position`.
///
/// # Arguments
/// * `position` - Tile to query.
pub fn has_teleport(position: Vec2) -> Result<bool> {
    Ok(optional(query(|p| unsafe {
        sys::has_teleport(position.x, position.y, p)
    }))?
    .is_some_and(|b| b != 0))
}
/// Return owning visible teleport reservations with checked caster types and teams.
pub fn teleports() -> Result<Vec<Teleport>> {
    crate::list(|p, n, out| unsafe { sys::teleports(p, n, out) })?
        .into_iter()
        .map(|i| {
            Ok(Teleport {
                champion: ChampionHandle(i.champion),
                kind: ChampionType::try_from(i.kind)?,
                team: Team::from_absolute(i.team as i32)?,
                destination: i.destination,
                remaining_time: i.timer,
            })
        })
        .collect()
}
/// Return the visible teleport reservation at `position`.
///
/// # Arguments
/// * `position` - Tile to query.
pub fn teleport_at(position: Vec2) -> Result<Teleport> {
    if has_teleport(position)? {
        if let Some(t) = teleports()?.into_iter().find(|t| t.position() == position) {
            return Ok(t);
        }
    }
    Err(Error::local(
        ErrorCode::ApiNotFound,
        "no visible teleport at position",
    ))
}
/// Return an owning visible altar lock at `position`, preferring ally then enemy.
///
/// # Arguments
/// * `position` - Tile to query.
pub fn altar_lock_at(position: Vec2) -> Result<AltarLock> {
    match altar_lock_for_team(position, Team::Ally) {
        Err(e) if e.code() == Some(ErrorCode::ApiNotFound) => {
            altar_lock_for_team(position, Team::Enemy)
        }
        result => result,
    }
}
fn altar_lock_for_team(position: Vec2, team: Team) -> Result<AltarLock> {
    let team = team.absolute()?;
    AltarLock::decode(query(|p| unsafe {
        sys::altar_lock(position.x, position.y, team, p)
    })?)
}
/// Test for a visible altar lock for either team; missing or hidden locks return `false`.
///
/// # Arguments
/// * `position` - Tile to query.
pub fn has_altar_lock(position: Vec2) -> Result<bool> {
    if optional(altar_lock_for_team(position, Team::Ally))?.is_some() {
        return Ok(true);
    }
    Ok(optional(altar_lock_for_team(position, Team::Enemy))?.is_some())
}
/// Return owning visible altar-lock snapshots, optionally filtered by team.
///
/// # Arguments
/// * `team` - `Some(team)` for one side, or `None` for both.
pub fn altar_locks(team: Option<Team>) -> Result<Vec<AltarLock>> {
    let values: Result<Vec<_>> = crate::list(|p, n, out| unsafe { sys::altar_locks(p, n, out) })?
        .into_iter()
        .map(AltarLock::decode)
        .collect();
    Ok(values?
        .into_iter()
        .filter(|i| team.is_none_or(|t| t == i.team))
        .collect())
}
/// Look up a visible camp at its current position and borrow initialised metadata.
/// Absence and hidden tiles are errors; unknown handles and uninitialised caches remain errors.
///
/// # Arguments
/// * `position` - Current tile of the camp.
pub fn camp_at(position: Vec2) -> Result<Camp> {
    load_camp(query(|p| unsafe { sys::camp_at(position.x, position.y, p) })?)
}
/// Return a live wrapper for the visible champion at `position`; absence and hidden tiles are errors.
///
/// # Arguments
/// * `position` - Tile to query.
pub fn champion_at(position: Vec2) -> Result<Champion> {
    Champion::new(ChampionHandle(query(|p| unsafe {
        sys::champion_at(position.x, position.y, p)
    })?))
}
/// Query visible camp presence at `position`; invalid positions remain errors.
///
/// # Arguments
/// * `position` - Tile to query.
pub fn has_camp(position: Vec2) -> Result<bool> {
    Ok(optional(query(|p| unsafe {
        sys::has_camp(position.x, position.y, p)
    }))?
    .is_some_and(|i| i != 0))
}
/// Query visible champion presence at `position`; invalid positions remain errors.
///
/// # Arguments
/// * `position` - Tile to query.
pub fn has_champion(position: Vec2) -> Result<bool> {
    Ok(optional(query(|p| unsafe {
        sys::has_champion(position.x, position.y, p)
    }))?
    .is_some_and(|i| i != 0))
}
/// Return owning wrappers for currently visible champions, optionally filtered by team.
///
/// # Arguments
/// * `team` - `Some(team)` for one side, or `None` for both.
pub fn vision_champions(team: Option<Team>) -> Result<Vec<Champion>> {
    let positions = champion_tiles(team)?.positions()?;
    let mut v = Vec::with_capacity(positions.len());
    for p in positions {
        v.push(champion_at(p)?);
    }
    Ok(v)
}
/// Return owning wrappers for currently visible camps, using initialised map metadata.
pub fn vision_camps() -> Result<Vec<Camp>> {
    let positions = camp_tiles()?.positions()?;
    let mut v = Vec::with_capacity(positions.len());
    for p in positions {
        v.push(camp_at(p)?);
    }
    Ok(v)
}
/// Query groups with any camp spawn tile in current vision.
pub fn vision_camp_groups() -> Result<Vec<CampGroup>> {
    let visible = visible_tiles()?;
    let mut v = Vec::new();
    for g in camp_groups()? {
        if g.visible_in(&visible)? {
            v.push(g);
        }
    }
    Ok(v)
}

/// Bounds-checked team shared memory with capacity cached by `cambc::init`.
/// Byte values are always read from or written to the host.
#[derive(Debug)]
pub struct SharedMemory {
    capacity: usize,
}
impl SharedMemory {
    /// Return the shared-memory capacity in bytes, loaded by `cambc::init`.
    #[must_use]
    pub fn size(&self) -> usize {
        self.capacity
    }
    fn check_range(&self, offset: usize, len: usize) -> Result<i32> {
        if offset > self.capacity || len > self.capacity - offset || offset > i32::MAX as usize {
            return Err(Error::invalid_argument(
                "shared memory access outside buffer",
            ));
        }
        Ok(offset as i32)
    }
    /// Read bytes at a byte offset, preserving engine errors.
    /// The entire range must fit; an empty read at capacity is allowed and makes no host call.
    ///
    /// # Arguments
    /// * `offset` - Byte offset into shared memory.
    /// * `bytes` - Receives the bytes; its length sets how many are read.
    pub fn read(&self, offset: usize, bytes: &mut [u8]) -> Result<()> {
        let offset = self.check_range(offset, bytes.len())?;
        if !bytes.is_empty() {
            check(unsafe {
                sys::read_shared_memory(offset, bytes.as_mut_ptr().cast(), bytes.len() as u32)
            })?;
        }
        Ok(())
    }
    /// Write bytes at a byte offset, preserving engine errors.
    /// The entire range must fit; an empty write at capacity is allowed and makes no host call.
    ///
    /// # Arguments
    /// * `offset` - Byte offset into shared memory.
    /// * `bytes` - Bytes to write.
    pub fn write(&self, offset: usize, bytes: &[u8]) -> Result<()> {
        let offset = self.check_range(offset, bytes.len())?;
        if !bytes.is_empty() {
            check(unsafe {
                sys::write_shared_memory(offset, bytes.as_ptr().cast(), bytes.len() as u32)
            })?;
        }
        Ok(())
    }
    /// Read one bounds-checked byte, preserving engine errors.
    ///
    /// # Arguments
    /// * `offset` - Byte offset into shared memory.
    pub fn get(&self, offset: usize) -> Result<u8> {
        let mut byte = [0];
        self.read(offset, &mut byte)?;
        Ok(byte[0])
    }
    /// Write one bounds-checked byte, preserving engine errors.
    ///
    /// # Arguments
    /// * `offset` - Byte offset into shared memory.
    /// * `value` - Byte to store.
    pub fn set(&self, offset: usize, value: u8) -> Result<()> {
        self.write(offset, &[value])
    }
    /// Read a typed slice at a byte offset using native layout and byte order, like C++ spans.
    /// Only plain-data types with no padding and valid arbitrary bit patterns may be read.
    ///
    /// # Arguments
    /// * `offset` - Byte offset into shared memory.
    /// * `values` - Receives the values; its length sets how many are read.
    pub fn read_values<T: bytemuck::Pod>(&self, offset: usize, values: &mut [T]) -> Result<()> {
        self.read(offset, bytemuck::cast_slice_mut(values))
    }
    /// Write a typed slice at a byte offset using native layout and byte order, like C++ spans.
    /// Values must have no uninitialised padding bytes.
    ///
    /// # Arguments
    /// * `offset` - Byte offset into shared memory.
    /// * `values` - Values to write.
    pub fn write_values<T: bytemuck::NoUninit>(&self, offset: usize, values: &[T]) -> Result<()> {
        self.write(offset, bytemuck::cast_slice(values))
    }
}

/// Draft-memory startup snapshot and draft-only submission interface.
#[derive(Debug)]
pub struct DraftMemory {
    draft_memory: Vec<u8>,
}
#[allow(clippy::len_without_is_empty)]
impl DraftMemory {
    /// Borrow bytes captured by `cambc::init`; submissions never change this snapshot.
    #[must_use]
    pub fn bytes(&self) -> &[u8] {
        &self.draft_memory
    }
    /// Return the startup snapshot length in bytes.
    #[must_use]
    pub fn len(&self) -> usize {
        self.draft_memory.len()
    }
    /// Read one byte from the startup snapshot.
    ///
    /// # Arguments
    /// * `offset` - Byte offset into the draft memory snapshot.
    pub fn get(&self, offset: usize) -> Result<u8> {
        self.draft_memory
            .get(offset)
            .copied()
            .ok_or_else(|| Error::invalid_argument("draft memory access outside buffer"))
    }
    /// Submit caller bytes during draft for transfer to gameplay guests.
    /// Gameplay submissions return `InvalidAction`; the startup snapshot is unchanged.
    ///
    /// # Arguments
    /// * `bytes` - Bytes to pass to this team's gameplay bots.
    pub fn submit(&self, bytes: &[u8]) -> Result<()> {
        if self_champion()?.is_some() {
            return Err(Error::local(
                ErrorCode::InvalidAction,
                "draft memory can only be submitted during the draft",
            ));
        }
        check(unsafe { sys::bc_submit_draft_memory(bytes.as_ptr().cast(), bytes.len()) })
    }
}

/// Query the number of champions per team.
pub fn draft_team_size() -> Result<usize> {
    Ok(query(|p| unsafe { sys::draft_num_rounds(p) })? as usize)
}
/// Return this team's completed draft picks in round order, skipping unavailable rounds.
///
/// # Arguments
/// * `team` - Team whose picks to return.
pub fn current_draft(team: Team) -> Result<Vec<ChampionDraft>> {
    let team = team.absolute()?;
    let rounds = query(|p| unsafe { sys::draft_num_rounds(p) })? as u32;
    let mut picks = Vec::new();
    for round in 0..rounds {
        match query(|p| unsafe { sys::draft_get(team, round, p) }) {
            Ok(pick) => picks.push(ChampionDraft::try_from(pick)?),
            Err(error) if error.code() == Some(ErrorCode::DraftUnavailable) => {}
            Err(error) => return Err(error),
        }
    }
    Ok(picks)
}
/// Return the total number of gems submitted by a relative team.
///
/// # Arguments
/// * `team` - Team to count.
pub fn global_submitted_gems(team: Team) -> Result<u32> {
    let team = team.absolute()?;
    query(|p| unsafe { sys::bc_submitted_gems(team as u32, p) })
}
/// Submit one draft selection; phase and availability failures propagate engine errors.
///
/// # Arguments
/// * `kind` - Champion to pick.
pub fn draft_pick(kind: ChampionDraft) -> Result<()> {
    check(unsafe { sys::draft_champion(kind as i32) })
}
/// Draw a replay debug dot at `position` using packed RGB colour.
///
/// # Arguments
/// * `position` - Tile to mark.
/// * `rgb` - Packed `0xRRGGBB` colour.
pub fn debug_draw_dot(position: Vec2, rgb: u32) -> Result<()> {
    check(unsafe { sys::debug_dot(position.x, position.y, rgb) })
}
/// Draw a replay debug line from `from` to `to` using packed RGB colour.
///
/// # Arguments
/// * `from` - Start tile.
/// * `to` - End tile.
/// * `rgb` - Packed `0xRRGGBB` colour.
pub fn debug_draw_line(from: Vec2, to: Vec2, rgb: u32) -> Result<()> {
    check(unsafe { sys::debug_line(from.x, from.y, to.x, to.y, rgb) })
}
