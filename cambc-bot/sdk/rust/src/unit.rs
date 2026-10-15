//! Champion/camp functionality through composition, with explicit live/design identities.
//! Live state is queried on demand. Only the self champion may perform actions or private-resource queries.
use crate::{
    check, query, require_self, sys, Ability, Bitboard, BuffCollection, BuffRestrictions,
    CampGroup, CampGroupHandle, CampHandle, CampStats, CampType, ChampionHandle, ChampionStats,
    ChampionType, ConsumableType, Entity, Error, ErrorCode, ItemCounts, ItemType, PassiveType,
    Result, Team, Vec2,
};
use core::ops::Deref;
/// A live unit handle or a draft-only champion design with no action identity.
#[derive(Clone, Copy, Debug, PartialEq, Eq, Hash)]
pub(crate) enum UnitIdentity {
    /// A live champion handle; visibility is queried separately.
    Champion(ChampionHandle),
    /// A live camp handle; type metadata is not part of identity.
    Camp(CampHandle),
    /// An immutable champion design without a live handle.
    Design(ChampionType),
}
impl UnitIdentity {
    /// Return the action identity; draft designs and handle-free observations return an error.
    pub(crate) fn entity(self) -> Result<Entity> {
        match self {
            Self::Champion(h) => Ok(h.into()),
            Self::Camp(handle) => Ok(handle.into()),
            Self::Design(_) => Err(Error::MissingIdentity),
        }
    }
}
/// Shared live functionality for champion and camp views.
#[derive(Clone, Debug)]
pub struct Unit {
    identity: UnitIdentity,
    camp_kind: Option<CampType>,
}
impl PartialEq for Unit {
    fn eq(&self, b: &Self) -> bool {
        self.identity == b.identity
    }
}
impl Eq for Unit {}
impl core::hash::Hash for Unit {
    fn hash<H: core::hash::Hasher>(&self, state: &mut H) {
        self.identity.hash(state);
    }
}
impl Unit {
    /// Construct shared non-camp unit behaviour without a host query.
    fn new(identity: UnitIdentity) -> Self {
        Self {
            identity,
            camp_kind: None,
        }
    }
    /// Read private camp type metadata needed for immutable design-stat queries.
    fn camp_kind(&self) -> Result<CampType> {
        self.camp_kind
            .ok_or_else(|| Error::invalid_argument("missing camp type"))
    }
    /// Return the action identity; draft designs and handle-free observations return an error.
    pub(crate) fn entity(&self) -> Result<Entity> {
        self.identity.entity()
    }
    /// Construct a fresh owning buff collection; designs and hidden units return an error.
    pub fn buffs(&self) -> Result<BuffCollection> {
        BuffCollection::new(self.identity.entity().ok())
    }
    /// Query current health points.
    pub fn health(&self) -> Result<f32> {
        match self.identity {
            UnitIdentity::Design(_) => Err(design_error()),
            UnitIdentity::Champion(h) => Ok(query(|p| unsafe { sys::champion(h.0, p) })?.health),
            UnitIdentity::Camp(h) => Ok(query(|p| unsafe { sys::camp(h.0, p) })?.health),
        }
    }
    /// Query current maximum health points.
    pub fn max_health(&self) -> Result<f32> {
        match self.identity {
            UnitIdentity::Design(_) => Err(design_error()),
            UnitIdentity::Champion(h) => {
                Ok(query(|p| unsafe { sys::champion(h.0, p) })?.max_health)
            }
            UnitIdentity::Camp(h) => Ok(query(|p| unsafe { sys::camp(h.0, p) })?.max_health),
        }
    }
    /// Query current strength.
    pub fn strength(&self) -> Result<f32> {
        match self.identity {
            UnitIdentity::Design(_) => Err(design_error()),
            UnitIdentity::Champion(h) => Ok(query(|p| unsafe { sys::champion(h.0, p) })?.strength),
            UnitIdentity::Camp(h) => Ok(query(|p| unsafe { sys::camp(h.0, p) })?.strength),
        }
    }
    /// Query current focus.
    pub fn focus(&self) -> Result<f32> {
        match self.identity {
            UnitIdentity::Design(_) => Err(design_error()),
            UnitIdentity::Champion(h) => Ok(query(|p| unsafe { sys::champion(h.0, p) })?.focus),
            UnitIdentity::Camp(h) => Ok(query(|p| unsafe { sys::camp(h.0, p) })?.focus),
        }
    }
    /// Query attack range; camps use their engine design range.
    pub fn attack_range(&self) -> Result<i32> {
        match self.identity {
            UnitIdentity::Camp(_) => {
                let kind = self.camp_kind()?;
                Ok(query(|p| unsafe { sys::camp_design(kind as i32, p) })?
                    .base_stats
                    .base_attack_range)
            }
            UnitIdentity::Champion(h) => {
                Ok(query(|p| unsafe { sys::champion(h.0, p) })?.attack_range)
            }
            UnitIdentity::Design(_) => Err(design_error()),
        }
    }
    /// Build the live unit's Chebyshev attack-range mask without filtering targets or attack availability.
    /// Hidden units and design-only champions propagate position/range query errors.
    pub fn attack_tiles(&self) -> Result<Bitboard> {
        let mut mask = Bitboard::new()?;
        self.attack_tiles_into(&mut mask)?;
        Ok(mask)
    }
    /// Like [`Self::attack_tiles`], but writes into `out`.
    ///
    /// # Arguments
    /// * `out` - Destination mask; its previous contents are overwritten.
    pub fn attack_tiles_into(&self, out: &mut Bitboard) -> Result<()> {
        let position = self.position()?;
        let range = self.attack_range()?;
        out.clear();
        out.draw_chebyshev(position, range as f32)
    }
    /// Query the current map position; hidden/design units propagate unavailability.
    pub fn position(&self) -> Result<Vec2> {
        match self.identity {
            UnitIdentity::Design(_) => Err(design_error()),
            UnitIdentity::Champion(h) => {
                let i = query(|p| unsafe { sys::champion(h.0, p) })?;
                Ok(Vec2::new(i.x, i.y))
            }
            UnitIdentity::Camp(handle) => {
                let i = query(|p| unsafe { sys::camp(handle.0, p) })?;
                Ok(Vec2::new(i.x, i.y))
            }
        }
    }
    /// Query visibility; designs and known hidden/not-found units return `false`.
    /// Other engine errors are preserved rather than converted to `false`.
    pub fn is_visible(&self) -> Result<bool> {
        match self.identity {
            UnitIdentity::Design(_) => Ok(false),
            UnitIdentity::Champion(h) => match query(|p| unsafe { sys::champion(h.0, p) }) {
                Ok(i) => Ok(i.x >= 0 && i.y >= 0),
                Err(e) if e.code() == Some(ErrorCode::ApiNotInVision) => Ok(false),
                Err(e) => Err(e),
            },
            UnitIdentity::Camp(handle) => match query(|p| unsafe { sys::camp(handle.0, p) }) {
                Ok(_) => Ok(true),
                Err(e)
                    if matches!(
                        e.code(),
                        Some(ErrorCode::ApiNotInVision | ErrorCode::ApiNotFound)
                    ) =>
                {
                    Ok(false)
                }
                Err(e) => Err(e),
            },
        }
    }
}
/// Report a design-only wrapper being used for a live-state operation.
fn design_error() -> Error {
    Error::local(
        ErrorCode::ApiNotInVision,
        "design champion has no live state",
    )
}
bitflags::bitflags! {
    /// Champion state and turn-event flags with the engine's bit values.
    #[repr(transparent)]
    #[derive(Clone, Copy, Debug, Default, PartialEq, Eq, Hash)]
    pub struct ChampionState: u32 {
        /// Champion is alive.
        const ALIVE = 1 << 0;
        /// A recall channel is active.
        const RECALLING = 1 << 1;
        /// A gem-submission channel is active.
        const SUBMITTING_GEMS = 1 << 2;
        /// Champion may move according to engine state.
        const CAN_MOVE = 1 << 3;
        /// Champion may attack according to engine state.
        const CAN_ATTACK = 1 << 4;
        /// Champion has completed the turn.
        const COMPLETED_TURN = 1 << 5;
        /// Recall was started in the engine turn state.
        const RECALL_STARTED = 1 << 6;
        /// Gem submission was started in the engine turn state.
        const SUBMIT_GEMS_STARTED = 1 << 7;
        /// Recall was cancelled in the engine turn state.
        const RECALL_CANCELLED = 1 << 8;
        /// Gem submission was cancelled in the engine turn state.
        const SUBMIT_GEMS_CANCELLED = 1 << 9;
        /// Champion may cast according to engine state.
        const CAN_CAST = 1 << 11;
        /// A teleport channel is active.
        const TELEPORTING = 1 << 12;
    }
}
impl ChampionState {
    /// Test whether `ALIVE` is absent from these captured state flags.
    #[must_use]
    pub fn is_dead(&self) -> bool {
        !self.contains(Self::ALIVE)
    }
}
/// A champion handle or draft design, composed with `Unit` and cached slot metadata.
/// Live stats, inventories and buffs are queried separately; actions require the current champion.
#[derive(Clone, Debug)]
pub struct Champion {
    unit: Unit,
    /// Two immutable slot wrappers; cooldowns and actions remain live queries.
    ability_slots: [Ability; 2],
    /// Two owner-bound consumable wrappers; inventory counts are not cached.
    consumables: Consumables,
}
impl PartialEq for Champion {
    fn eq(&self, b: &Self) -> bool {
        self.unit.identity == b.unit.identity
    }
}
impl Eq for Champion {}
impl core::hash::Hash for Champion {
    fn hash<H: core::hash::Hasher>(&self, state: &mut H) {
        self.unit.identity.hash(state);
    }
}
impl Champion {
    /// Construct a live champion wrapper and capture its two ability slots.
    /// Hidden slot metadata is retained as an error; other initialisation failures propagate.
    pub(crate) fn new(handle: ChampionHandle) -> Result<Self> {
        Ok(Self {
            unit: Unit::new(UnitIdentity::Champion(handle)),
            ability_slots: [Ability::live(handle, 0)?, Ability::live(handle, 1)?],
            consumables: Consumables(core::array::from_fn(|i| Consumable {
                owner: Some(handle),
                kind: ConsumableType::ALL[i],
            })),
        })
    }
    /// Write a draft-safe champion design into `dst` field by field, with no live owner.
    /// Queries the engine definition at runtime and validates ability tags.
    ///
    /// # Safety
    /// `dst` must be valid for writes. On error it may be partly written, but holds no owned data.
    pub(crate) unsafe fn write_design(kind: ChampionType, dst: *mut Self) -> Result<()> {
        let mut d = core::mem::MaybeUninit::<sys::ChampionDesign>::uninit();
        // SAFETY: the host assigns the whole design record on success.
        check(unsafe { sys::champion_design(kind as i32, d.as_mut_ptr()) })?;
        let d = unsafe { d.assume_init_ref() };
        let slots = unsafe { core::ptr::addr_of_mut!((*dst).ability_slots) }.cast::<Ability>();
        for (i, raw) in d.definition.abilities.iter().enumerate() {
            unsafe { Ability::write_design(raw, i as u32, slots.add(i)) }?;
        }
        unsafe {
            core::ptr::addr_of_mut!((*dst).unit).write(Unit::new(UnitIdentity::Design(kind)));
            core::ptr::addr_of_mut!((*dst).consumables).write(Consumables(core::array::from_fn(
                |i| Consumable {
                    owner: None,
                    kind: ConsumableType::ALL[i],
                },
            )));
        }
        Ok(())
    }
    /// Return the stored engine handle.
    pub(crate) fn handle(&self) -> Result<ChampionHandle> {
        if let UnitIdentity::Champion(h) = self.unit.identity {
            Ok(h)
        } else {
            Err(design_error())
        }
    }
    /// Return the action identity; draft designs and handle-free observations return an error.
    pub(crate) fn entity(&self) -> Result<Entity> {
        self.unit.entity()
    }
    fn own(&self) -> Result<ChampionHandle> {
        let h = self.handle()?;
        require_self(h)?;
        Ok(h)
    }
    fn info(&self) -> Result<sys::ChampionInfo> {
        let h = self.handle()?;
        query(|p| unsafe { sys::champion(h.0, p) })
    }
    /// Return this champion's type; live champions require a host query.
    pub fn champion_name(&self) -> Result<ChampionType> {
        if let UnitIdentity::Design(t) = self.unit.identity {
            Ok(t)
        } else {
            ChampionType::try_from(self.info()?.kind)
        }
    }
    /// Query the checked champion passive from its engine design.
    pub fn passive(&self) -> Result<PassiveType> {
        let kind = self.champion_name()?;
        let d = query(|p| unsafe { sys::champion_design(kind as i32, p) })?;
        PassiveType::try_from(d.definition.passive_type)
    }
    /// Query the champion's team relative to the guest perspective.
    pub fn team(&self) -> Result<Team> {
        Team::from_absolute(self.info()?.team)
    }
    /// Check whether this champion belongs to the observing team.
    pub fn is_ally(&self) -> Result<bool> {
        Ok(self.team()? == Team::Ally)
    }
    /// Check whether this champion belongs to the opposing team.
    pub fn is_enemy(&self) -> Result<bool> {
        Ok(self.team()? == Team::Enemy)
    }
    /// Query immutable engine design stats, not current modified combat stats.
    pub fn base_stats(&self) -> Result<ChampionStats> {
        let kind = self.champion_name()?;
        Ok(query(|p| unsafe { sys::champion_design(kind as i32, p) })?.base_stats)
    }
    /// Query whether this live champion is alive; draft designs return an error.
    pub fn is_alive(&self) -> Result<bool> {
        let h = self.handle()?;
        Ok(query(|p| unsafe { sys::champion_is_alive(h.0, p) })? != 0)
    }
    /// Query champion state flags, preserving all engine bits including unknown future bits.
    pub fn state(&self) -> Result<ChampionState> {
        Ok(ChampionState::from_bits_retain(self.info()?.state as u32))
    }
    /// Check whether this champion can move.
    pub fn can_move(&self) -> Result<bool> {
        Ok(self.state()?.contains(ChampionState::CAN_MOVE))
    }
    /// Check whether this champion can attack.
    pub fn can_attack(&self) -> Result<bool> {
        Ok(self.state()?.contains(ChampionState::CAN_ATTACK))
    }
    /// Check whether this champion can cast.
    pub fn can_cast(&self) -> Result<bool> {
        Ok(self.state()?.contains(ChampionState::CAN_CAST))
    }
    /// Check whether this champion is recalling.
    pub fn is_recalling(&self) -> Result<bool> {
        Ok(self.state()?.contains(ChampionState::RECALLING))
    }
    /// Check whether this champion is submitting gems.
    pub fn is_submitting_gems(&self) -> Result<bool> {
        Ok(self.state()?.contains(ChampionState::SUBMITTING_GEMS))
    }
    /// Check whether this champion is teleporting.
    pub fn is_teleporting(&self) -> Result<bool> {
        Ok(self.state()?.contains(ChampionState::TELEPORTING))
    }
    /// Check whether this champion has completed its turn.
    pub fn has_completed_turn(&self) -> Result<bool> {
        Ok(self.state()?.contains(ChampionState::COMPLETED_TURN))
    }
    /// Query current action-restriction bits; distinct from active buff-kind flags.
    pub fn restrictions(&self) -> Result<BuffRestrictions> {
        let h = self.handle()?;
        Ok(BuffRestrictions::from_bits_retain(query(|p| unsafe {
            sys::champion_restrictions(h.0, p)
        })?))
    }
    /// Query a fresh owning vision mask for this live champion.
    pub fn visible_tiles(&self) -> Result<Bitboard> {
        let h = self.handle()?;
        Bitboard::query(|p| unsafe { sys::bc_champion_vision(h.0, p) })
    }
    /// Query an owning hypothetical vision mask at `position`, not current visibility.
    ///
    /// # Arguments
    /// * `position` - Hypothetical tile to see from.
    pub fn visible_tiles_from(&self, position: Vec2) -> Result<Bitboard> {
        let kind = self.champion_name()?;
        Bitboard::query(|p| unsafe {
            sys::hypothetical_vision(kind as i32, position.x, position.y, p)
        })
    }
    /// Like [`Self::visible_tiles`], but writes into `out`.
    ///
    /// # Arguments
    /// * `out` - Destination mask; its previous contents are overwritten.
    pub fn visible_tiles_into(&self, out: &mut Bitboard) -> Result<()> {
        let h = self.handle()?;
        crate::check(unsafe { sys::bc_champion_vision(h.0, out.ptr_mut()) })
    }
    /// Like [`Self::visible_tiles_from`], but writes into `out`.
    ///
    /// # Arguments
    /// * `position` - Hypothetical tile to see from.
    /// * `out` - Destination mask; its previous contents are overwritten.
    pub fn visible_tiles_from_into(&self, position: Vec2, out: &mut Bitboard) -> Result<()> {
        let kind = self.champion_name()?;
        crate::check(unsafe {
            sys::hypothetical_vision(kind as i32, position.x, position.y, out.ptr_mut())
        })
    }
    /// Query the current channel countdown for a visible live champion.
    pub fn remaining_channel_time(&self) -> Result<i32> {
        self.info()?;
        let h = self.handle()?;
        query(|p| unsafe { sys::champion_channel_time(h.0, p) })
    }
    /// Query owning positions of the current champion's ordinary wards; requires self.
    pub fn ward_positions(&self) -> Result<Vec<Vec2>> {
        let h = self.own()?;
        crate::list(|p, n, out| unsafe { sys::champion_ward_positions(h.0, p, n, out) })
    }
    /// Query the current champion's ward-resource cooldown; requires self.
    pub fn ward_cooldown(&self) -> Result<i32> {
        let h = self.own()?;
        Ok(query(|p| unsafe { sys::champion_wards(h.0, p) })?.cooldown)
    }
    /// Build the self champion's placement-range mask using initialised constants.
    /// This geometric mask alone does not guarantee that ward placement is legal.
    ///
    /// # Panics
    /// Panics if SDK initialisation failed.
    pub fn ward_placement_tiles(&self) -> Result<Bitboard> {
        let mut b = Bitboard::new()?;
        self.ward_placement_tiles_into(&mut b)?;
        Ok(b)
    }
    /// Like [`Self::ward_placement_tiles`], but writes into `out`.
    ///
    /// # Panics
    /// Panics if SDK initialisation failed.
    ///
    /// # Arguments
    /// * `out` - Destination mask; its previous contents are overwritten.
    pub fn ward_placement_tiles_into(&self, out: &mut Bitboard) -> Result<()> {
        self.own()?;
        out.clear();
        out.draw_chebyshev(self.position()?, crate::constants().ward_placement_radius)
    }
    fn shop_allowed(&self, consumable: bool) -> Result<bool> {
        let h = self.own()?;
        Ok(query(|p| unsafe { sys::champion_can_shop(h.0, i32::from(consumable), p) })? != 0)
    }
    /// Check whether an item can be bought in the current state.
    ///
    /// # Arguments
    /// * `item` - Item to buy.
    /// * `count` - Number of copies to buy.
    pub fn can_buy(&self, item: ItemType, count: u32) -> Result<bool> {
        if !self.shop_allowed(false)? || count == 0 {
            return Ok(false);
        }
        let info = item.info()?;
        Ok(info.purchasable
            && u64::from(self.gold_count()?) >= u64::from(info.cost) * u64::from(count))
    }
    /// Check whether a consumable can be bought in the current state.
    ///
    /// # Arguments
    /// * `consumable` - Consumable to buy.
    pub fn can_buy_consumable(&self, consumable: ConsumableType) -> Result<bool> {
        if !self.shop_allowed(true)? {
            return Ok(false);
        }
        let info = consumable.info()?;
        Ok(self.gold_count()? >= info.cost
            && self.consumables()[consumable].count()? < info.carry_limit)
    }
    /// Query self's current selling permission.
    pub fn can_sell(&self) -> Result<bool> {
        let h = self.own()?;
        Ok(query(|p| unsafe { sys::champion_can_sell(h.0, p) })? != 0)
    }
    /// Attack a champion, camp, unit or altar lock; requires self and engine-valid targeting.
    ///
    /// # Arguments
    /// * `target` - Visible champion, camp or altar lock within attack range.
    pub fn attack<T: crate::AttackTarget + ?Sized>(&self, target: &T) -> Result<()> {
        self.own()?;
        let t = crate::detail::Sealed::entity(target)?;
        check(unsafe { sys::attack(t.kind() as u32, t.handle()) })
    }
    /// Borrow the two cached ability wrappers; live timers are not included in this cache.
    #[must_use]
    pub fn abilities(&self) -> &[Ability; 2] {
        &self.ability_slots
    }
    /// Return the champion's owner-bound consumable wrappers.
    #[must_use]
    pub fn consumables(&self) -> &Consumables {
        &self.consumables
    }
    /// Query and return an independent owning item-count snapshot.
    pub fn item_count(&self) -> Result<ItemCounts> {
        let h = self.handle()?;
        let mut result = [0; 42];
        check(unsafe { sys::champion_items(h.0, result.as_mut_ptr()) })?;
        Ok(ItemCounts(result))
    }
    /// Query the carried gem count of a live champion.
    pub fn gem_count(&self) -> Result<u32> {
        let h = self.handle()?;
        query(|p| unsafe { sys::champion_gems(h.0, p) })
    }
    /// Query the self champion's current gold; other champions return an ownership error.
    pub fn gold_count(&self) -> Result<u32> {
        let h = self.own()?;
        query(|p| unsafe { sys::champion_gold(h.0, p) })
    }
    /// Buy one item for self; engine validates location, gold and recipe.
    ///
    /// # Arguments
    /// * `kind` - Item to buy.
    pub fn buy(&self, kind: ItemType) -> Result<()> {
        self.buy_count(kind, 1)
    }
    /// Buy `count` items for self in one engine request.
    ///
    /// # Arguments
    /// * `kind` - Item to buy.
    /// * `count` - Number of copies to buy.
    pub fn buy_count(&self, kind: ItemType, count: u32) -> Result<()> {
        self.own()?;
        check(unsafe { sys::buy_item(kind as u32, count) })
    }
    /// Buy one consumable for self; engine validates gold and carry limit.
    ///
    /// # Arguments
    /// * `kind` - Consumable to buy.
    pub fn buy_consumable(&self, kind: ConsumableType) -> Result<()> {
        self.own()?;
        check(unsafe { sys::buy_consumable(kind.raw()) })
    }
    /// Sell one item for self; engine validates inventory and location.
    ///
    /// # Arguments
    /// * `kind` - Item to sell.
    pub fn sell(&self, kind: ItemType) -> Result<()> {
        self.sell_count(kind, 1)
    }
    /// Sell sequentially `count` times, stopping at the first failure.
    /// Earlier successful sales are not rolled back.
    ///
    /// # Arguments
    /// * `kind` - Item to sell.
    /// * `count` - Number of copies to sell.
    pub fn sell_count(&self, kind: ItemType, count: u32) -> Result<()> {
        self.own()?;
        for _ in 0..count {
            check(unsafe { sys::sell(kind as u32) })?;
        }
        Ok(())
    }
    /// Cancel self's recall; returns an error when no recall is active.
    pub fn cancel_recall(&self) -> Result<()> {
        self.own()?;
        if !self.state()?.contains(ChampionState::RECALLING) {
            return Err(Error::invalid_argument("champion is not recalling"));
        }
        check(unsafe { sys::cancel_channel() })
    }
    /// Cancel self's gem submission; returns an error when none is active.
    pub fn cancel_submit_gems(&self) -> Result<()> {
        self.own()?;
        if !self.state()?.contains(ChampionState::SUBMITTING_GEMS) {
            return Err(Error::invalid_argument("champion is not submitting gems"));
        }
        check(unsafe { sys::cancel_channel() })
    }
    /// Move self by a displacement from the freshly queried current position.
    /// Returns `Ok(false)` for newly revealed obstructions; known invalid destinations return `Err`.
    ///
    /// # Arguments
    /// * `delta` - Displacement from the current position.
    pub fn r#move(&self, delta: Vec2) -> Result<bool> {
        self.move_absolute(self.position()? + delta)
    }
    /// Query the previous scheduling priority for this live champion.
    pub fn prev_priority(&self) -> Result<i32> {
        self.query_priority(false)
    }
    /// Query the current/next scheduling priority for this live champion.
    pub fn priority(&self) -> Result<i32> {
        self.query_priority(true)
    }
    /// Query scheduling priority; false selects previous, true selects current/next.
    fn query_priority(&self, next: bool) -> Result<i32> {
        let h = self.handle()?;
        query(|p| unsafe { sys::bc_champion_priority(h.0, i32::from(next), p) })
    }
    /// End self's current turn and yield; other/design champions return an ownership error.
    pub fn end_turn(&self) -> Result<()> {
        self.own()?;
        crate::world::end_turn();
        Ok(())
    }
    /// Begin recalling self; the engine validates channel and action restrictions.
    pub fn recall(&self) -> Result<()> {
        self.own()?;
        check(unsafe { sys::recall() })
    }
    /// Place an ordinary ward for self at `destination`; engine validates all placement rules.
    /// Returns `Ok(false)` for newly revealed obstructions; known invalid destinations return `Err`.
    ///
    /// # Arguments
    /// * `destination` - Tile to place the ward on.
    pub fn place_ward(&self, destination: Vec2) -> Result<bool> {
        self.own()?;
        query(|out| unsafe { sys::place_ward(destination.x, destination.y, out) }).map(|successful| successful != 0)
    }
    /// Begin submitting self's carried gems to `altar`; engine validates range and channel state.
    ///
    /// # Arguments
    /// * `altar` - Altar tile to tribute at.
    pub fn submit_gems(&self, altar: Vec2) -> Result<()> {
        self.own()?;
        check(unsafe { sys::submit_gems(altar.x, altar.y) })
    }
    /// Move self to an absolute map destination; engine validates movement legality.
    /// Returns `Ok(false)` for newly revealed obstructions; known invalid destinations return `Err`.
    ///
    /// # Arguments
    /// * `destination` - Map tile to move to.
    pub fn move_absolute(&self, destination: Vec2) -> Result<bool> {
        self.own()?;
        query(|out| unsafe { sys::move_to(destination.x, destination.y, out) }).map(|successful| successful != 0)
    }
    /// Move self to the reachable tile on a shortest path to `target`, avoiding walls, holes, altars,
    /// the enemy spawn and visible units. Returns `Ok(false)` without moving when already there,
    /// unable to move, or `target` is blocked or unreachable, and for a newly revealed obstruction.
    ///
    /// # Arguments
    /// * `target` - Map tile to approach.
    pub fn move_towards(&self, target: Vec2) -> Result<bool> {
        self.own()?;
        let origin = self.position()?;
        if origin == target || !self.state()?.contains(ChampionState::CAN_MOVE) {
            return Ok(false);
        }
        let mut blocked = crate::world::walls()?.clone();
        blocked.union_assign(crate::world::holes()?)?;
        blocked.union_assign(crate::world::altars()?)?;
        blocked.union_assign(crate::world::spawn_region(Team::Enemy)?)?;
        blocked.union_assign(&crate::world::champion_tiles(None)?)?;
        blocked.union_assign(&crate::world::camp_tiles()?)?;
        blocked.union_assign(&crate::world::teleport_tiles()?)?;
        blocked.set(origin, false)?;
        if blocked.is_set(target)? {
            return Ok(false);
        }
        let mut roots = Bitboard::new()?;
        roots.set(target, true)?;
        let mut destinations = Bitboard::new()?;
        destinations.set(origin, true)?;
        let mut distances = crate::DistanceField::new()?;
        distances.compute_to(
            &blocked,
            &roots,
            &destinations,
            &mut Bitboard::new()?,
            u16::MAX,
        )?;
        let (mut best, mut best_distance) = (origin, distances.get(origin)?);
        let speed = self.base_stats()?.base_move_speed as u16;
        for tile in blocked.reachable_tiles(origin, speed)?.positions()? {
            let distance = distances.get(tile)?;
            if distance < best_distance {
                (best, best_distance) = (tile, distance);
            }
        }
        if best == origin {
            return Ok(false);
        }
        self.move_absolute(best)
    }
    /// Set self's scheduling priority through the engine.
    ///
    /// # Arguments
    /// * `priority` - New scheduling priority; takes effect from the next round and persists.
    pub fn set_priority(&self, priority: i32) -> Result<()> {
        self.own()?;
        check(unsafe { sys::set_priority(priority) })
    }
}
impl Deref for Champion {
    type Target = Unit;
    fn deref(&self) -> &Self::Target {
        &self.unit
    }
}
/// A consumable type bound to a champion; counts are live and use requires self.
#[derive(Clone, Copy, Debug, PartialEq, Eq, Hash)]
pub struct Consumable {
    owner: Option<ChampionHandle>,
    kind: ConsumableType,
}
impl Consumable {
    /// Return this owner-bound consumable type without querying its count.
    #[must_use]
    pub fn kind(&self) -> ConsumableType {
        self.kind
    }
    /// Query the owning live champion's count of this consumable type.
    pub fn count(&self) -> Result<u32> {
        let h = self.owner.ok_or_else(design_error)?;
        let mut counts = [0; 4];
        check(unsafe { sys::champion_consumables(h.0, counts.as_mut_ptr()) })?;
        Ok(counts[self.kind.raw() as usize])
    }
    /// Use this consumable at `destination`; requires the self champion.
    /// Returns `Ok(false)` for newly revealed obstructions; known invalid destinations return `Err`.
    ///
    /// # Arguments
    /// * `destination` - Tile to use the consumable at.
    pub fn use_at(&self, destination: Vec2) -> Result<bool> {
        let h = self.owner.ok_or_else(design_error)?;
        require_self(h)?;
        query(|out| unsafe { sys::use_consumable(self.kind.raw(), destination.x, destination.y, out) }).map(|successful| successful != 0)
    }
    /// Use this consumable without a destination; requires the self champion.
    /// Returns `Ok(false)` for newly revealed obstructions; known invalid destinations return `Err`.
    pub fn use_untargeted(&self) -> Result<bool> {
        self.use_at(Vec2::new(-1, -1))
    }
}
/// Type-indexed owner-bound consumables for a champion.
#[derive(Clone, Copy, Debug, PartialEq, Eq, Hash)]
pub struct Consumables([Consumable; 2]);
impl Consumables {
    /// Iterate consumables in SDK order.
    pub fn iter(&self) -> impl Iterator<Item = &Consumable> {
        self.0.iter()
    }
}
impl core::ops::Index<ConsumableType> for Consumables {
    type Output = Consumable;
    fn index(&self, kind: ConsumableType) -> &Self::Output {
        &self.0[kind as usize]
    }
}
/// A camp with immutable spawn/type/group metadata and shared live `Unit` queries.
#[derive(Clone, Debug)]
pub struct Camp {
    unit: Unit,
    handle: CampHandle,
    group: CampGroupHandle,
    kind: CampType,
    spawn_position: Vec2,
}
impl PartialEq for Camp {
    fn eq(&self, b: &Self) -> bool {
        self.handle == b.handle
    }
}
impl Eq for Camp {}
impl core::hash::Hash for Camp {
    fn hash<H: core::hash::Hasher>(&self, state: &mut H) {
        self.handle.hash(state);
    }
}
impl Camp {
    /// Compose a camp from initialised immutable metadata, keeping its type out of identity.
    pub(crate) fn new(
        handle: CampHandle,
        group: CampGroupHandle,
        kind: CampType,
        spawn_position: Vec2,
    ) -> Self {
        Self {
            unit: Unit {
                identity: UnitIdentity::Camp(handle),
                camp_kind: Some(kind),
            },
            handle,
            group,
            kind,
            spawn_position,
        }
    }
    /// Return the captured engine camp type.
    #[must_use]
    pub fn kind(&self) -> CampType {
        self.kind
    }
    /// Return whether this camp is a wolf.
    #[must_use]
    pub fn is_wolf(&self) -> bool {
        self.kind() == CampType::Wolf
    }
    /// Return whether this camp is an elk.
    #[must_use]
    pub fn is_elk(&self) -> bool {
        self.kind() == CampType::Elk
    }
    /// Return whether this camp is a gem cluster.
    #[must_use]
    pub fn is_gem_cluster(&self) -> bool {
        self.kind() == CampType::GemCluster
    }
    /// Return whether this camp is a gold cluster.
    #[must_use]
    pub fn is_gold_cluster(&self) -> bool {
        self.kind() == CampType::GoldCluster
    }
    /// Return whether this camp is a monolith.
    #[must_use]
    pub fn is_monolith(&self) -> bool {
        self.kind() == CampType::Monolith
    }
    /// Return the immutable engine spawn position, not a current live position.
    #[must_use]
    pub fn spawn_position(&self) -> Vec2 {
        self.spawn_position
    }
    /// Query this camp's group from the host; requires successful SDK initialisation and gameplay perspective.
    pub fn camp_group(&self) -> Result<CampGroup> {
        crate::world::camp_group(self.group)
    }
    /// Query immutable engine design stats, not current modified combat stats.
    pub fn base_stats(&self) -> Result<CampStats> {
        Ok(query(|p| unsafe { sys::camp_design(self.kind as i32, p) })?.base_stats)
    }
}
impl Deref for Camp {
    type Target = Unit;
    fn deref(&self) -> &Self::Target {
        &self.unit
    }
}
impl crate::detail::Sealed for Camp {
    fn entity(&self) -> Result<Entity> {
        Ok(Entity(crate::EntityTarget::Camp(self.handle)))
    }
}
impl crate::detail::Sealed for Unit {
    fn entity(&self) -> Result<Entity> {
        Unit::entity(self)
    }
}
impl crate::detail::Sealed for Champion {
    fn entity(&self) -> Result<Entity> {
        Champion::entity(self)
    }
}
impl crate::CastTarget for Camp {}
impl crate::CastTarget for Unit {}
impl crate::CastTarget for Champion {}
impl crate::AttackTarget for Camp {}
impl crate::AttackTarget for Unit {}
impl crate::AttackTarget for Champion {}
