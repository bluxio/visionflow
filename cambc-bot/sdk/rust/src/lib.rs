//! Bot-facing API. `cambc::init()` runs before `main`; live queries return owning
//! snapshots or Results. Cached design data never contains live cooldowns.
mod ability;
mod board;
mod constants;
mod shop;
mod sys;
mod types;
mod unit;
mod world;
pub use crate::world::{
    champion_designs, champions, constants, draft_memory, end_turn, fuel_remaining,
    global_submitted_gems, init, is_draft, resign, round_number, self_champion, shared_memory,
};
pub use ability::*;
pub use board::*;
pub use constants::Constants;
pub use shop::{ConsumableInfo, ItemInfo, ItemPassive};
pub use sys::{CampStats, ChampionStats};
pub(crate) use types::detail::{self, Entity, EntityKind, EntityTarget};
pub use types::*;
pub use unit::*;
pub use world::{AltarLock, CampGroup, CampGroupState, DraftMemory, SharedMemory, Teleport, Ward};
/// Map geometry and immutable terrain/spawn/camp snapshots.
pub mod map {
    pub use crate::world::{
        altars, bushes, camp, camp_groups, height, holes, is_in_bounds, reflect_horizontal,
        reflect_vertical, rotate_180, spawn_region, symmetry, walls, width, CampGroup,
        CampGroupState,
    };
}
/// Live visibility, occupancy and visible-entity queries.
pub mod vision {
    pub use crate::world::{
        altar_lock_at, altar_lock_tiles, altar_lock_tiles_into, altar_locks, camp_at, camp_tiles,
        camp_tiles_into, champion_at, champion_tiles, champion_tiles_into, has_altar_lock, has_camp,
        has_champion, has_teleport, has_ward, teleport_at, teleport_tiles, teleport_tiles_into,
        teleports, vision_camp_groups as camp_groups, vision_camps as camps,
        vision_champions as champions, visible_tiles, visible_tiles_into, ward_at, ward_tiles,
        ward_tiles_into, wards,
    };
}
/// Draft selection and team size.
pub mod draft {
    pub use crate::world::{current_draft, draft_pick as pick, draft_team_size as team_size};
}
/// Replay debug drawing.
pub mod debug {
    pub use crate::world::{debug_draw_dot as draw_dot, debug_draw_line as draw_line};
}
/// Typed ability metadata and owning payloads.
pub mod ability_types {
    pub use crate::ability::{
        AbilityAoeDamage as AoeDamage, AbilityAttackReset as AttackReset, AbilityCc as Cc,
        AbilityDamage as Damage, AbilityDash as Dash, AbilityHeal as Heal, AbilityInfo,
        AbilityMetadata, AbilityProperties, AbilityShield as Shield,
    };
}
#[cfg(test)]
mod fake;
#[cfg(test)]
mod tests;

/// SDK result preserving host diagnostics or local validation errors.
pub type Result<T> = core::result::Result<T, Error>;

/// An engine failure or a typed SDK precondition/metadata failure.
#[derive(Clone, Debug, PartialEq, Eq, Hash)]
pub enum Error {
    /// Host or local validation failure with preserved numeric code and owned diagnostic.
    Engine {
        /// Original numeric engine error code, including unknown values.
        raw_code: i32,
        /// Recognised code, or `None` for unknown numeric values.
        code: Option<ErrorCode>,
        /// Owned engine diagnostic or local validation message.
        message: String,
    },
    /// This ability kind, such as an attack reset, has no targeting range.
    NoRange,
    /// A draft design cannot supply a live action-target identity.
    MissingIdentity,
}
impl Error {
    pub(crate) fn invalid_argument(message: &str) -> Self {
        Self::local(ErrorCode::InvalidArgument, message)
    }
    pub(crate) fn invalid_value(category: &'static str, value: i64) -> Self {
        Self::local(
            ErrorCode::ApiValueOutOfRange,
            &format!("invalid {category} value: {value}"),
        )
    }
    pub(crate) fn local(code: ErrorCode, message: &str) -> Self {
        Self::Engine {
            raw_code: code as i32,
            code: Some(code),
            message: message.into(),
        }
    }
    /// Return the recognised engine error code, or `None` for unknown/SDK-only errors.
    #[must_use]
    pub fn code(&self) -> Option<ErrorCode> {
        match self {
            Self::Engine { code, .. } => *code,
            _ => None,
        }
    }
    /// Return the original numeric engine code, including unknown values.
    #[must_use]
    pub fn raw_code(&self) -> Option<i32> {
        match self {
            Self::Engine { raw_code, .. } => Some(*raw_code),
            _ => None,
        }
    }
}
impl core::fmt::Display for Error {
    fn fmt(&self, f: &mut core::fmt::Formatter<'_>) -> core::fmt::Result {
        match self {
            Self::Engine {
                raw_code, message, ..
            } => write!(f, "engine error {raw_code}: {message}"),
            _ => write!(f, "{self:?}"),
        }
    }
}
impl std::error::Error for Error {}
impl From<core::convert::Infallible> for Error {
    fn from(value: core::convert::Infallible) -> Self {
        match value {}
    }
}
/// Convert a host status into an owned diagnostic, preserving unknown numeric error codes.
pub(crate) fn check(raw_code: i32) -> Result<()> {
    if raw_code == 0 {
        return Ok(());
    }
    let size = unsafe { sys::bc_get_last_error(core::ptr::null_mut(), 0) };
    let mut bytes = vec![0; size.max(0) as usize];
    if !bytes.is_empty() {
        unsafe { sys::bc_get_last_error(bytes.as_mut_ptr(), bytes.len() as u32) };
    }
    Err(Error::Engine {
        raw_code,
        code: ErrorCode::try_from(raw_code).ok(),
        message: String::from_utf8_lossy(&bytes).into_owned(),
    })
}
/// Initialise output storage, call the host, and return it only after checking success.
pub(crate) fn query<T: Default>(f: impl FnOnce(*mut T) -> i32) -> Result<T> {
    let mut value = T::default();
    check(f(&mut value))?;
    Ok(value)
}
/// Like `query`, without first zeroing the output.
///
/// # Safety
/// `f` must write every field of `T` whenever it returns success.
pub(crate) unsafe fn query_init<T>(f: impl FnOnce(*mut T) -> i32) -> Result<T> {
    let mut value = core::mem::MaybeUninit::<T>::uninit();
    check(f(value.as_mut_ptr()))?;
    // SAFETY: the caller guarantees that success means `value` is fully written.
    Ok(unsafe { value.assume_init() })
}
/// Query required list capacity, then read an independently owned bounded list.
pub(crate) fn list<T: Default + Clone>(f: impl Fn(*mut T, u32, *mut u32) -> i32) -> Result<Vec<T>> {
    let count = query(|p| f(core::ptr::null_mut(), 0, p))?;
    counted_list(count, f)
}
/// Allocate a known-capacity list and reject oversized host writes before exposing values.
pub(crate) fn counted_list<T: Default + Clone>(
    count: u32,
    f: impl FnOnce(*mut T, u32, *mut u32) -> i32,
) -> Result<Vec<T>> {
    let capacity =
        usize::try_from(count).map_err(|_| Error::invalid_argument("list size overflow"))?;
    let mut values = Vec::new();
    values
        .try_reserve_exact(capacity)
        .map_err(|_| Error::local(ErrorCode::OutOfMemory, "list allocation failed"))?;
    values.resize(capacity, T::default());
    let written = query(|p| f(values.as_mut_ptr(), count, p))?;
    if written > count {
        return Err(Error::invalid_argument("host returned oversized list"));
    }
    values.truncate(written as usize);
    Ok(values)
}
/// Reject actions on draft or non-self champion handles before entering the host.
pub(crate) fn require_self(handle: ChampionHandle) -> Result<()> {
    let own = unsafe { sys::my_champion() };
    if own < 0 || handle.0 != own as u32 {
        Err(Error::local(
            ErrorCode::ApiNotOwner,
            "action is only available for self",
        ))
    } else {
        Ok(())
    }
}
