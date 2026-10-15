//! Checked engine enums, relative teams, handle-backed targets and owning inventory values.
use crate::{Error, Result};
macro_rules! engine_enum {
($name:ident, $ty:ty, {$($variant:ident = $value:expr),* $(,)?}) => {
#[doc = concat!("Checked engine ", stringify!($name), " values; unknown runtime tags return an error.")]
#[repr($ty)] #[derive(Clone, Copy, Debug, PartialEq, Eq, Hash)] pub enum $name {$(#[doc = concat!(stringify!($variant), " engine value (`", stringify!($value), "`).")] $variant = $value),*}
impl $name {#[doc = "All recognised values in engine declaration order."] pub const ALL: &'static [Self] = &[$(Self::$variant),*];}
impl core::fmt::Display for $name {fn fmt(&self, f: &mut core::fmt::Formatter<'_>) -> core::fmt::Result {core::fmt::Debug::fmt(self, f)}}
impl TryFrom<$ty> for $name {type Error = Error; fn try_from(value:$ty) -> Result<Self> {match value {$($value => Ok(Self::$variant),)* _ => Err(Error::invalid_value(stringify!($name), value as i64))}}}
};
}
engine_enum!(Symmetry,i32,{Horizontal = 0,Vertical = 1,Rotational180 = 2});
engine_enum!(CampType,i32,{Wolf = 0,Elk = 1,GemCluster = 2,GoldCluster = 3,Monolith = 4});
engine_enum!(PassiveType,u8,{None = 0,BonusGold = 1,RemoteShop = 2,GemFocus = 3,SplashAttack = 4,HealthStrength = 5,ResetCooldownsOnKill = 6,Retaliation = 7,StunImmunity = 8});
engine_enum!(ChampionDraft,i32,{Ryth = 0,VesperVale = 1,Vendetta = 2,CassiAndIssac = 3,Stellar = 4,Prism = 5,Marcella = 6,Willow = 7,Ren = 8,Varg = 9,Tavi = 10});
engine_enum!(ChampionType,i32,{Ryth = 0,VesperVale = 1,Vendetta = 2,Cassi = 3,Issac = 4,Stellar = 5,Prism = 6,Marcella = 7,Willow = 8,Ren = 9,Varg = 10,Tavi = 11});
engine_enum!(AbilityKind,u32,{AttackReset = 1,Damage = 2,AoeDamage = 3,Cc = 4,Heal = 5,Dash = 6,Shield = 7});
engine_enum!(ItemType,u32,{MonsterShardOfStrength = 0,MonsterEmblemOfStrength = 1,MonsterShardOfFocus = 2,MonsterEmblemOfFocus = 3,MonsterShardOfHealth = 4,MonsterEmblemOfHealth = 5,PowerShardOfStrength = 6,PowerEmblemOfStrength = 7,PowerShardOfFocus = 8,PowerEmblemOfFocus = 9,PowerShardOfHealth = 10,PowerEmblemOfHealth = 11,DurabilityShardOfStrength = 12,DurabilityEmblemOfStrength = 13,DurabilityShardOfFocus = 14,DurabilityEmblemOfFocus = 15,DurabilityShardOfHealth = 16,DurabilityEmblemOfHealth = 17,TreasureShardOfStrength = 18,TreasureEmblemOfStrength = 19,TreasureShardOfFocus = 20,TreasureEmblemOfFocus = 21,TreasureShardOfHealth = 22,TreasureEmblemOfHealth = 23,JungleShardOfStrength = 24,JungleEmblemOfStrength = 25,JungleShardOfFocus = 26,JungleEmblemOfFocus = 27,JungleShardOfHealth = 28,JungleEmblemOfHealth = 29,HomeShardOfStrength = 30,HomeEmblemOfStrength = 31,HomeShardOfFocus = 32,HomeEmblemOfFocus = 33,HomeShardOfHealth = 34,HomeEmblemOfHealth = 35,BruiserShardOfStrength = 36,BruiserEmblemOfStrength = 37,BruiserShardOfFocus = 38,BruiserEmblemOfFocus = 39,BruiserShardOfHealth = 40,BruiserEmblemOfHealth = 41});
#[repr(u32)]
#[derive(Clone, Copy, Debug, PartialEq, Eq, Hash)]
/// Public consumable kinds in their dense indexing order.
pub enum ConsumableType {
    /// A teleport crystal used to begin a teleport channel.
    TpCrystal = 0,
    /// An altar lock placed on a submission altar.
    AltarLock = 1,
}
impl ConsumableType {
    /// All public consumable values in dense indexing order.
    pub const ALL: &'static [Self] = &[Self::TpCrystal, Self::AltarLock];

    /// Map a dense public key to the sparse engine consumable ID.
    pub(crate) const fn raw(self) -> u32 {
        match self {
            Self::TpCrystal => 0,
            Self::AltarLock => 2,
        }
    }
}
impl TryFrom<u32> for ConsumableType {
    type Error = Error;
    fn try_from(value: u32) -> Result<Self> {
        match value {
            0 => Ok(Self::TpCrystal),
            1 => Ok(Self::AltarLock),
            _ => Err(Error::invalid_value("ConsumableType", value as i64)),
        }
    }
}
engine_enum!(BuffKind,u32,{Shield = 0,Stun = 1,EnhancedAttack = 2});
engine_enum!(ErrorCode,i32,{Success = 0,InvalidHandle = 1,OutOfMemory = 2,FileNotFound = 3,NullPointer = 4,InvalidArgument = 5,MapParseError = 6,InvalidMapFile = 7,InvalidMapVersion = 8,ReplayParseError = 9,InvalidReplayFile = 10,InvalidReplayVersion = 11,InvalidDraft = 12,DraftNotDrafting = 13,DraftDuplicateChampion = 14,DraftInvalidRound = 15,DraftUnavailable = 16,AlreadyInitialized = 17,InvalidAction = 18,InvalidActionMemoryBadRange = 19,InvalidActionMemoryDraftFrozen = 20,InvalidActionMoveCantMove = 21,InvalidActionMoveBadDestination = 22,InvalidActionSpellCastInvalidSource = 23,InvalidActionSpellCastInvalidSlot = 24,InvalidActionSpellCastNoAbility = 25,InvalidActionSpellCastCantCast = 26,InvalidActionSpellCastAlreadyCast = 27,InvalidActionSpellCastOnCooldown = 28,InvalidActionSpellCastNoCharges = 29,InvalidActionSpellCastBadTarget = 30,InvalidActionSpellCastOutOfRange = 31,InvalidActionSpellCastBadDestination = 32,InvalidActionCantAct = 33,InvalidActionIsRecalling = 34,InvalidActionIsSubmittingGems = 35,InvalidActionMoveAlreadyMoved = 36,InvalidActionMoveOutOfRange = 37,InvalidActionMoveUnreachable = 38,InvalidActionAttackInvalidSource = 39,InvalidActionAttackCantAttack = 40,InvalidActionAttackAlreadyAttacked = 41,InvalidActionAttackBadTarget = 42,InvalidActionAttackOutOfRange = 43,InvalidActionRecallCantChannel = 44,InvalidActionSubmitGemsCantChannel = 45,InvalidActionSubmitGemsNoGems = 46,InvalidActionSubmitGemsBadAltar = 47,InvalidActionSubmitGemsOutOfRange = 48,InvalidActionBuyItemNotOnSpawn = 49,InvalidActionBuyItemInvalidItem = 50,InvalidActionBuyItemInvalidCount = 51,InvalidActionBuyItemNotEnoughGold = 52,InvalidActionBuyItemInventoryFull = 53,InvalidActionBuyItemStatsOverflow = 54,InvalidActionBuyConsumableNotOnSpawn = 55,InvalidActionBuyConsumableInvalidType = 56,InvalidActionBuyConsumableInvalidCount = 57,InvalidActionBuyConsumableInventoryFull = 58,InvalidActionBuyConsumableNotEnoughGold = 59,InvalidActionUseConsumableInvalidType = 60,InvalidActionUseConsumableNotOwned = 61,InvalidActionPlaceWardNoCharges = 62,InvalidActionPlaceWardCantPlace = 63,InvalidActionPlaceWardBadDestination = 64,InvalidActionPlaceWardOutOfRange = 65,MapSetTeamAspawnAreaBadPosition = 66,MapAddBushBadPosition = 67,MapRemoveBushBadPosition = 68,MapAddWallBadPosition = 69,MapRemoveWallBadPosition = 70,MapAddHoleBadPosition = 71,MapRemoveHoleBadPosition = 72,MapAddAltarBadPosition = 73,MapRemoveAltarBadPosition = 74,MapGetBushBadPosition = 75,MapGetWallBadPosition = 76,MapGetHoleBadPosition = 77,MapGetAltarBadPosition = 78,MapGetCampHandleAtBadPosition = 79,MapAddCampBadPosition = 80,MapSetCampBadPosition = 81,InvalidActionChampionTeleportBadDestination = 82,InvalidActionRespawnGroupBlockedSpawn = 83,InvalidActionCampTeleportBadDestination = 84,TimelineBadSeekPosition = 85,TimelineBadSeekSubstep = 86,ApiNoMap = 87,ManagedResourcesBadPosition = 88,ManagedResourcesBadRadius = 89,ManagedResourcesAliasing = 90,ManagedResourcesMetadataMismatch = 91,ManagedResourcesOpaqueOrigin = 92,ApiInvalidTeam = 93,ApiInvalidPerspective = 94,ApiNotInitialized = 95,ApiBadPosition = 96,ApiNotInVision = 97,ApiNotFound = 98,ApiInvalidCampGroup = 99,ApiInvalidCampInfo = 100,ApiInvalidCamp = 101,ApiInvalidChampion = 102,ApiInvalidWard = 103,ApiInvalidAbility = 104,ApiNotTeammate = 105,ApiCampGroupNotWaitingToRespawn = 106,ApiChampionAlive = 107,ApiInvalidChampionType = 108,ApiInvalidCampType = 109,ApiValueOutOfRange = 110,ApiBufferTooSmall = 111,ApiNotOwner = 112,InvalidActionRecallAlreadyUsed = 113,InvalidActionSubmitGemsAlreadyUsed = 114,InvalidActionCancelChannelNotChanneling = 115,InvalidActionCancelChannelAlreadyUsed = 116,InvalidActionPlaceWardAllyWardPresent = 117,InvalidActionAltarLockBadAltar = 118,InvalidActionAltarLockOutOfRange = 119,InvalidActionAltarLockAlreadyLocked = 120,InvalidActionIsTeleporting = 123,InvalidActionCancelChannelNotCancellable = 124,InvalidActionSubmitGemsAltarLocked = 125,InvalidActionSpellCastAlreadyMoved = 126,InvalidActionMoveExclusiveSpellCast = 127,InvalidActionSellNotTavi = 128,InvalidActionSellInvalidItem = 129,InvalidActionSellNotOwned = 130,InvalidActionSellAlreadySold = 131,InvalidActionSellGoldOverflow = 132,InvalidActionSellNotOnSpawn = 133});

/// A team relative to the current guest perspective, not an absolute engine ID.
#[derive(Clone, Copy, Debug, PartialEq, Eq, Hash)]
pub enum Team {
    /// The observing guest's own team.
    Ally,
    /// The opposing team.
    Enemy,
}
impl core::fmt::Display for Team {
    fn fmt(&self, f: &mut core::fmt::Formatter<'_>) -> core::fmt::Result {
        core::fmt::Debug::fmt(self, f)
    }
}
impl core::fmt::Display for ConsumableType {
    fn fmt(&self, f: &mut core::fmt::Formatter<'_>) -> core::fmt::Result {
        core::fmt::Debug::fmt(self, f)
    }
}
impl core::fmt::Display for Vec2 {
    fn fmt(&self, f: &mut core::fmt::Formatter<'_>) -> core::fmt::Result {
        write!(f, "({}, {})", self.x, self.y)
    }
}
impl Team {
    /// The other team.
    #[must_use]
    pub fn opponent(self) -> Self {
        match self {
            Self::Ally => Self::Enemy,
            Self::Enemy => Self::Ally,
        }
    }
    pub(crate) fn absolute(self) -> Result<i32> {
        let own = unsafe { crate::sys::my_team() };
        if !(0..=1).contains(&own) {
            return Err(Error::invalid_value("team", own as i64));
        }
        Ok(match self {
            Self::Ally => own,
            Self::Enemy => 1 - own,
        })
    }
    pub(crate) fn from_absolute(value: i32) -> Result<Self> {
        if !(0..=1).contains(&value) {
            return Err(Error::invalid_value("team", value as i64));
        }
        Ok(if value == Self::Ally.absolute()? {
            Self::Ally
        } else {
            Self::Enemy
        })
    }
}
/// An engine champion ID; carrying it does not guarantee visibility or ownership.
#[derive(Clone, Copy, Debug, PartialEq, Eq, Hash)]
pub(crate) struct ChampionHandle(pub(crate) u32);
/// An engine camp ID; carrying it does not guarantee visibility.
#[derive(Clone, Copy, Debug, PartialEq, Eq, Hash)]
pub(crate) struct CampHandle(pub(crate) u32);
/// An engine camp-group ID used to reference initialised map metadata.
#[derive(Clone, Copy, Debug, PartialEq, Eq, Hash)]
pub(crate) struct CampGroupHandle(pub(crate) u32);
/// Implementation-only engine target identities.
pub(crate) mod detail {
    use super::*;
    engine_enum!(EntityKind,u32,{Champion = 1,Camp = 2,AltarLock = 6});
    /// An opaque champion, camp, or altar-lock action target.
    #[derive(Clone, Copy, Debug, PartialEq, Eq, Hash)]
    pub struct Entity(pub(crate) EntityTarget);
    #[derive(Clone, Copy, Debug, PartialEq, Eq, Hash)]
    pub enum EntityTarget {
        Champion(ChampionHandle),
        Camp(CampHandle),
        AltarLock(u32),
    }
    impl Entity {
        /// Return the engine target-kind tag without changing its ABI numeric value.
        pub fn kind(self) -> EntityKind {
            match self.0 {
                EntityTarget::Champion(_) => EntityKind::Champion,
                EntityTarget::Camp(_) => EntityKind::Camp,
                EntityTarget::AltarLock(_) => EntityKind::AltarLock,
            }
        }
        pub(crate) fn handle(self) -> u32 {
            match self.0 {
                EntityTarget::Champion(h) => h.0,
                EntityTarget::Camp(h) => h.0,
                EntityTarget::AltarLock(h) => h,
            }
        }
    }
    impl From<ChampionHandle> for Entity {
        fn from(h: ChampionHandle) -> Self {
            Self(EntityTarget::Champion(h))
        }
    }
    impl From<CampHandle> for Entity {
        fn from(h: CampHandle) -> Self {
            Self(EntityTarget::Camp(h))
        }
    }
    /// Seals the public target traits; only SDK types can be targets.
    pub trait Sealed {
        #[doc(hidden)]
        fn entity(&self) -> Result<Entity>;
    }
    #[cfg(test)]
    impl Sealed for Entity {
        fn entity(&self) -> Result<Entity> {
            Ok(*self)
        }
    }
    #[cfg(test)]
    impl super::CastTarget for Entity {}
    #[cfg(test)]
    impl super::AttackTarget for Entity {}
}
/// A champion, camp or unit that an ability can be cast on.
pub trait CastTarget: detail::Sealed {}
/// A champion, camp, unit or altar lock that a champion can attack.
pub trait AttackTarget: detail::Sealed {}
/// Integer map coordinates or a displacement; arithmetic does not clamp to the map.
#[repr(C)]
#[derive(Clone, Copy, Debug, Default, PartialEq, Eq, PartialOrd, Ord, Hash)]
pub struct Vec2 {
    /// Horizontal integer coordinate.
    pub x: i32,
    /// Vertical integer coordinate.
    pub y: i32,
}
impl Vec2 {
    /// Unit vectors for the 8 standard directions; north is +y.
    pub const NORTH: Self = Self::new(0, 1);
    pub const NORTH_EAST: Self = Self::new(1, 1);
    pub const EAST: Self = Self::new(1, 0);
    pub const SOUTH_EAST: Self = Self::new(1, -1);
    pub const SOUTH: Self = Self::new(0, -1);
    pub const SOUTH_WEST: Self = Self::new(-1, -1);
    pub const WEST: Self = Self::new(-1, 0);
    pub const NORTH_WEST: Self = Self::new(-1, 1);
    /// Construct integer coordinates without map validation.
    #[must_use]
    pub const fn new(x: i32, y: i32) -> Self {
        Self { x, y }
    }
    /// Reflect the vector horizontally across the origin.
    #[must_use]
    pub const fn reflect_horizontal(self) -> Self {
        Self::new(-self.x, self.y)
    }
    /// Reflect the vector vertically across the origin.
    #[must_use]
    pub const fn reflect_vertical(self) -> Self {
        Self::new(self.x, -self.y)
    }
    /// Rotate the vector 180 degrees about the origin.
    #[must_use]
    pub const fn rotate_180(self) -> Self {
        Self::new(-self.x, -self.y)
    }
    /// Return the dot product with widened 64-bit integer arithmetic.
    #[must_use]
    ///
    /// # Arguments
    /// * `b` - Other vector.
    pub fn dot(self, b: Self) -> i64 {
        self.x as i64 * b.x as i64 + self.y as i64 * b.y as i64
    }
    /// Return the signed 2D cross product with widened 64-bit integer arithmetic.
    #[must_use]
    ///
    /// # Arguments
    /// * `b` - Other vector.
    pub fn cross(self, b: Self) -> i64 {
        self.x as i64 * b.y as i64 - self.y as i64 * b.x as i64
    }
    /// Return the squared Euclidean length as a 64-bit integer.
    #[must_use]
    pub fn length_squared(self) -> i64 {
        self.dot(self)
    }
    /// Rotate this displacement clockwise 90 degrees about the origin.
    #[must_use]
    pub const fn rotate_cw(self) -> Self {
        Self::new(self.y, -self.x)
    }
    /// Rotate this displacement counterclockwise 90 degrees about the origin.
    #[must_use]
    pub const fn rotate_ccw(self) -> Self {
        Self::new(-self.y, self.x)
    }
    /// Return the maximum absolute coordinate difference from `b`.
    #[must_use]
    ///
    /// # Arguments
    /// * `b` - Other vector.
    pub fn chebyshev_distance(self, b: Self) -> i64 {
        (self.x as i64 - b.x as i64)
            .abs()
            .max((self.y as i64 - b.y as i64).abs())
    }
    /// Divide each coordinate by `n`; zero divisors and signed overflow return an error.
    ///
    /// # Arguments
    /// * `n` - Divisor applied to both coordinates.
    pub fn checked_div(self, n: i32) -> Result<Self> {
        let x = self
            .x
            .checked_div(n)
            .ok_or_else(|| Error::invalid_argument("invalid integer division"))?;
        let y = self
            .y
            .checked_div(n)
            .ok_or_else(|| Error::invalid_argument("invalid integer division"))?;
        Ok(Self::new(x, y))
    }
}
impl core::ops::Add for Vec2 {
    type Output = Self;
    fn add(self, b: Self) -> Self {
        Self::new(self.x + b.x, self.y + b.y)
    }
}
impl core::ops::Sub for Vec2 {
    type Output = Self;
    fn sub(self, b: Self) -> Self {
        Self::new(self.x - b.x, self.y - b.y)
    }
}
impl core::ops::Neg for Vec2 {
    type Output = Self;
    fn neg(self) -> Self {
        Self::new(-self.x, -self.y)
    }
}
impl core::ops::Mul<i32> for Vec2 {
    type Output = Self;
    fn mul(self, n: i32) -> Self {
        Self::new(self.x * n, self.y * n)
    }
}
impl core::ops::Mul<Vec2> for i32 {
    type Output = Vec2;
    fn mul(self, p: Vec2) -> Vec2 {
        p * self
    }
}
impl core::ops::Div<i32> for Vec2 {
    type Output = Self;
    fn div(self, n: i32) -> Self {
        Self::new(self.x / n, self.y / n)
    }
}
impl core::ops::AddAssign for Vec2 {
    fn add_assign(&mut self, b: Self) {
        *self = *self + b;
    }
}
impl core::ops::SubAssign for Vec2 {
    fn sub_assign(&mut self, b: Self) {
        *self = *self - b;
    }
}
impl core::ops::MulAssign<i32> for Vec2 {
    fn mul_assign(&mut self, n: i32) {
        *self = *self * n;
    }
}
impl core::ops::DivAssign<i32> for Vec2 {
    fn div_assign(&mut self, n: i32) {
        *self = *self / n;
    }
}
/// Index-safe owning item-count snapshots.
#[derive(Clone, Copy, Debug, PartialEq, Eq, Hash)]
pub struct ItemCounts(pub(crate) [u32; 42]);
impl Default for ItemCounts {
    fn default() -> Self {
        Self([0; 42])
    }
}
impl ItemCounts {
    /// Iterate over every item type and captured count, including zero counts.
    pub fn iter(&self) -> impl Iterator<Item = (ItemType, u32)> + '_ {
        ItemType::ALL.iter().copied().zip(self.0.iter().copied())
    }
}
impl core::ops::Index<ItemType> for ItemCounts {
    type Output = u32;
    fn index(&self, k: ItemType) -> &u32 {
        &self.0[k as usize]
    }
}
bitflags::bitflags! {
    /// Allowed ability target categories; allies and enemies exclude self.
    #[repr(transparent)]
    #[derive(Clone, Copy, Debug, Default, PartialEq, Eq, Hash)]
    pub struct AbilityTargetMask: u32 {
        /// Neutral camps may be targeted.
        const CAMPS = 1 << 0;
        /// Other enemy champions may be targeted.
        const ENEMIES = 1 << 1;
        /// Other allied champions may be targeted.
        const ALLIES = 1 << 2;
        /// The caster may be targeted.
        const SELF = 1 << 3;
        /// Ground positions may be targeted.
        const GROUND = 1 << 4;
        const _ = !0;
    }
    /// Prohibited actions, distinct from the active-buff kind mask.
    #[repr(transparent)]
    #[derive(Clone, Copy, Debug, Default, PartialEq, Eq, Hash)]
    pub struct BuffRestrictions: u8 {
        /// Movement is prohibited.
        const MOVE = 1 << 0;
        /// Attacks are prohibited.
        const ATTACK = 1 << 1;
        /// Ability casting is prohibited.
        const CAST = 1 << 2;
        /// Shopping is prohibited.
        const SHOP = 1 << 3;
        /// Consumable use is prohibited.
        const CONSUMABLE = 1 << 4;
        /// Ward placement is prohibited.
        const WARD = 1 << 5;
        const _ = !0;
    }
}
