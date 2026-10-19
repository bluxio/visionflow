//! Typed cached ability metadata, live casts and eagerly captured owning buff collections.
use crate::{
    query, require_self, sys, AbilityKind, AbilityTargetMask, Bitboard, BuffKind,
    ChampionHandle, Entity, Error, ErrorCode, Result, Vec2,
};

/// Immutable cooldown, targeting and attribute metadata for one ability.
#[derive(Clone, Copy, Debug, PartialEq, Eq, Hash)]
pub struct AbilityProperties {
    /// Design cooldown, not the current remaining cooldown.
    pub max_cooldown: i32,
    /// Target-category bitmask: camps, enemies, allies, self and ground occupy bits 0–4.
    pub valid_targets: AbilityTargetMask,
    /// Casting and normal movement are mutually exclusive in a round.
    pub exclusive_with_movement: bool,
}
/// Object-safe cached immutable metadata. Range is an error for attack resets.
pub trait AbilityMetadata {
    /// Return the cached typed kind.
    fn kind(&self) -> AbilityKind;
    /// Borrow the immutable common ability metadata.
    fn properties(&self) -> &AbilityProperties;
    /// Return the cached range; attack resets return `Error::NoRange`.
    fn range(&self) -> Result<f32>;
    /// Return the design cooldown without querying the live timer.
    fn max_cooldown(&self) -> i32 {
        self.properties().max_cooldown
    }
    /// Return the cached target-category mask.
    fn valid_targets(&self) -> AbilityTargetMask {
        self.properties().valid_targets
    }
    /// Check whether casting and normal movement are mutually exclusive in a round.
    fn is_exclusive_with_movement(&self) -> bool {
        self.properties().exclusive_with_movement
    }
}
/// Metadata for resetting or enhancing the next attack; has no range.
#[derive(Clone, Copy, Debug, PartialEq, Eq, Hash)]
pub struct AbilityAttackReset {
    /// Common immutable ability properties.
    pub properties: AbilityProperties,
    /// Whether the reset also enhances the next attack.
    pub enhanced_attack: bool,
}
impl AbilityMetadata for AbilityAttackReset {
    fn kind(&self) -> AbilityKind {
        AbilityKind::AttackReset
    }
    fn properties(&self) -> &AbilityProperties {
        &self.properties
    }
    fn range(&self) -> Result<f32> {
        Err(Error::NoRange)
    }
}
/// Metadata for direct damage, including range and its engine modifier.
#[derive(Clone, Copy, Debug, PartialEq)]
pub struct AbilityDamage {
    /// Common immutable ability properties.
    pub properties: AbilityProperties,
    /// Ability range in map-coordinate units.
    pub range: f32,
    /// Ability-specific engine scaling modifier.
    pub modifier: f32,
}
impl AbilityMetadata for AbilityDamage {
    fn kind(&self) -> AbilityKind {
        AbilityKind::Damage
    }
    fn properties(&self) -> &AbilityProperties {
        &self.properties
    }
    fn range(&self) -> Result<f32> {
        Ok(self.range)
    }
}
/// Metadata for area damage, including radius and effect duration.
#[derive(Clone, Copy, Debug, PartialEq)]
pub struct AbilityAoeDamage {
    /// Common immutable ability properties.
    pub properties: AbilityProperties,
    /// Ability range in map-coordinate units.
    pub range: f32,
    /// Ability-specific engine scaling modifier.
    pub modifier: f32,
    /// Area-of-effect radius in map-coordinate units.
    pub radius: f32,
    /// Effect duration reported by the engine.
    pub duration: i32,
}
impl AbilityMetadata for AbilityAoeDamage {
    fn kind(&self) -> AbilityKind {
        AbilityKind::AoeDamage
    }
    fn properties(&self) -> &AbilityProperties {
        &self.properties
    }
    fn range(&self) -> Result<f32> {
        Ok(self.range)
    }
}
/// Metadata for crowd control, including range and duration.
#[derive(Clone, Copy, Debug, PartialEq)]
pub struct AbilityCc {
    /// Common immutable ability properties.
    pub properties: AbilityProperties,
    /// Ability range in map-coordinate units.
    pub range: f32,
    /// Ability-specific engine scaling modifier.
    pub modifier: f32,
    /// Effect duration reported by the engine.
    pub duration: i32,
}
impl AbilityMetadata for AbilityCc {
    fn kind(&self) -> AbilityKind {
        AbilityKind::Cc
    }
    fn properties(&self) -> &AbilityProperties {
        &self.properties
    }
    fn range(&self) -> Result<f32> {
        Ok(self.range)
    }
}
/// Metadata for healing, including range and its engine modifier.
#[derive(Clone, Copy, Debug, PartialEq)]
pub struct AbilityHeal {
    /// Common immutable ability properties.
    pub properties: AbilityProperties,
    /// Ability range in map-coordinate units.
    pub range: f32,
    /// Ability-specific engine scaling modifier.
    pub modifier: f32,
}
impl AbilityMetadata for AbilityHeal {
    fn kind(&self) -> AbilityKind {
        AbilityKind::Heal
    }
    fn properties(&self) -> &AbilityProperties {
        &self.properties
    }
    fn range(&self) -> Result<f32> {
        Ok(self.range)
    }
}
/// Metadata for a movement ability and its range.
#[derive(Clone, Copy, Debug, PartialEq)]
pub struct AbilityDash {
    /// Common immutable ability properties.
    pub properties: AbilityProperties,
    /// Ability range in map-coordinate units.
    pub range: f32,
}
impl AbilityMetadata for AbilityDash {
    fn kind(&self) -> AbilityKind {
        AbilityKind::Dash
    }
    fn properties(&self) -> &AbilityProperties {
        &self.properties
    }
    fn range(&self) -> Result<f32> {
        Ok(self.range)
    }
}
/// Metadata for shielding, including range, modifier and duration.
#[derive(Clone, Copy, Debug, PartialEq)]
pub struct AbilityShield {
    /// Common immutable ability properties.
    pub properties: AbilityProperties,
    /// Ability range in map-coordinate units.
    pub range: f32,
    /// Ability-specific engine scaling modifier.
    pub modifier: f32,
    /// Effect duration reported by the engine.
    pub duration: i32,
}
impl AbilityMetadata for AbilityShield {
    fn kind(&self) -> AbilityKind {
        AbilityKind::Shield
    }
    fn properties(&self) -> &AbilityProperties {
        &self.properties
    }
    fn range(&self) -> Result<f32> {
        Ok(self.range)
    }
}
/// Owned, checked ability metadata; an unused slot is represented by `None`.
#[derive(Clone, Copy, Debug, PartialEq)]
pub enum AbilityInfo {
    /// Attack-reset metadata; querying range returns `NoRange`.
    AttackReset(AbilityAttackReset),
    /// Direct-damage metadata.
    Damage(AbilityDamage),
    /// Area-damage metadata.
    AoeDamage(AbilityAoeDamage),
    /// Crowd-control metadata.
    Cc(AbilityCc),
    /// Healing metadata.
    Heal(AbilityHeal),
    /// Dash metadata.
    Dash(AbilityDash),
    /// Shield metadata.
    Shield(AbilityShield),
}
impl AbilityInfo {
    /// Borrow an object-safe view of this owned typed metadata.
    #[must_use]
    pub fn as_dyn(&self) -> &dyn AbilityMetadata {
        match self {
            Self::AttackReset(v) => v,
            Self::Damage(v) => v,
            Self::AoeDamage(v) => v,
            Self::Cc(v) => v,
            Self::Heal(v) => v,
            Self::Dash(v) => v,
            Self::Shield(v) => v,
        }
    }
    /// Decode the checked tag and payload words; zero kind is an unused slot.
    fn decode(raw: sys::AbilityProps) -> Result<Option<Self>> {
        if raw.kind == 0 {
            return Ok(None);
        }
        let kind = AbilityKind::try_from(raw.kind)?;
        let properties = AbilityProperties {
            max_cooldown: raw.cooldown,
            valid_targets: AbilityTargetMask::from_bits_retain(raw.targets),
            exclusive_with_movement: raw.attributes & 1 != 0,
        };
        let range = f32::from_bits(raw.payload[0]);
        let modifier = f32::from_bits(raw.payload[1]);
        Ok(Some(match kind {
            AbilityKind::AttackReset => Self::AttackReset(AbilityAttackReset {
                properties,
                enhanced_attack: raw.payload[0] != 0,
            }),
            AbilityKind::Damage => Self::Damage(AbilityDamage {
                properties,
                range,
                modifier,
            }),
            AbilityKind::AoeDamage => Self::AoeDamage(AbilityAoeDamage {
                properties,
                range,
                radius: modifier,
                modifier: f32::from_bits(raw.payload[2]),
                duration: raw.payload[3] as i32,
            }),
            AbilityKind::Cc => Self::Cc(AbilityCc {
                properties,
                range,
                modifier,
                duration: raw.payload[2] as i32,
            }),
            AbilityKind::Heal => Self::Heal(AbilityHeal {
                properties,
                range,
                modifier,
            }),
            AbilityKind::Dash => Self::Dash(AbilityDash { properties, range }),
            AbilityKind::Shield => Self::Shield(AbilityShield {
                properties,
                range,
                modifier,
                duration: raw.payload[2] as i32,
            }),
        }))
    }
}

/// Immutable constructor-cached slot properties. The live cooldown is queried separately.
#[derive(Clone, Debug)]
pub struct Ability {
    owner: Option<ChampionHandle>,
    slot: u32,
    info: Result<Option<AbilityInfo>>,
}
impl PartialEq for Ability {
    fn eq(&self, b: &Self) -> bool {
        self.owner == b.owner && self.slot == b.slot
    }
}
impl Eq for Ability {}
impl core::hash::Hash for Ability {
    fn hash<H: core::hash::Hasher>(&self, state: &mut H) {
        (self.owner, self.slot).hash(state);
    }
}
impl Ability {
    /// Capture slot metadata once, retaining visibility failure for hidden roster members.
    pub(crate) fn live(owner: ChampionHandle, slot: u32) -> Result<Self> {
        let raw = query(|p| unsafe { sys::champion_ability_props(owner.0, slot, p) });
        let info = match raw {
            Ok(p) => AbilityInfo::decode(p),
            Err(e) if e.code() == Some(ErrorCode::ApiNotInVision) => Err(e),
            Err(e) => return Err(e),
        };
        if let Err(e) = &info {
            if e.code() != Some(ErrorCode::ApiNotInVision) {
                return Err(e.clone());
            }
        }
        Ok(Self {
            owner: Some(owner),
            slot,
            info,
        })
    }
    /// Decode draft-safe slot metadata straight into `dst`.
    ///
    /// # Safety
    /// `dst` must be valid for writes; on error it is left unwritten.
    pub(crate) unsafe fn write_design(
        raw: &sys::AbilityProps,
        slot: u32,
        dst: *mut Self,
    ) -> Result<()> {
        let info = AbilityInfo::decode(*raw)?;
        unsafe {
            core::ptr::addr_of_mut!((*dst).owner).write(None);
            core::ptr::addr_of_mut!((*dst).slot).write(slot);
            core::ptr::addr_of_mut!((*dst).info).write(Ok(info));
        }
        Ok(())
    }
    /// Decode draft-safe slot metadata without creating a live owner.
    #[cfg(test)]
    pub(crate) fn design(raw: sys::AbilityProps, slot: u32) -> Result<Self> {
        Ok(Self {
            owner: None,
            slot,
            info: Ok(AbilityInfo::decode(raw)?),
        })
    }
    /// Require a live owner; draft designs cannot query timers or cast.
    fn owner(&self) -> Result<ChampionHandle> {
        self.owner.ok_or_else(|| {
            Error::local(
                ErrorCode::ApiNotInVision,
                "design ability has no live owner",
            )
        })
    }
    /// Return the zero-based ability slot.
    #[must_use]
    pub fn slot(&self) -> u32 {
        self.slot
    }
    /// Borrow cached metadata, return `None` for an unused slot, or preserve its cached error.
    pub fn info(&self) -> Result<Option<&AbilityInfo>> {
        self.info.as_ref().map(Option::as_ref).map_err(Clone::clone)
    }
    /// Always false for unused, unavailable, or different kinds; never queries the host.
    #[must_use]
    ///
    /// # Arguments
    /// * `kind` - Ability kind to test for.
    pub fn is(&self, kind: AbilityKind) -> bool {
        matches!(&self.info,Ok(Some(info)) if info.as_dyn().kind()==kind)
    }
    /// Return whether this is an attack-reset ability.
    #[must_use]
    pub fn is_attack_reset(&self) -> bool {
        self.is(AbilityKind::AttackReset)
    }
    /// Return whether this is a damage ability.
    #[must_use]
    pub fn is_damage(&self) -> bool {
        self.is(AbilityKind::Damage)
    }
    /// Return whether this is an area-damage ability.
    #[must_use]
    pub fn is_aoe_damage(&self) -> bool {
        self.is(AbilityKind::AoeDamage)
    }
    /// Return whether this is a crowd-control ability.
    #[must_use]
    pub fn is_cc(&self) -> bool {
        self.is(AbilityKind::Cc)
    }
    /// Return whether this is a healing ability.
    #[must_use]
    pub fn is_heal(&self) -> bool {
        self.is(AbilityKind::Heal)
    }
    /// Return whether this is a dash ability.
    #[must_use]
    pub fn is_dash(&self) -> bool {
        self.is(AbilityKind::Dash)
    }
    /// Return whether this is a shield ability.
    #[must_use]
    pub fn is_shield(&self) -> bool {
        self.is(AbilityKind::Shield)
    }
    /// Query the current cooldown; requires a live owner and propagates host errors.
    pub fn cooldown(&self) -> Result<i32> {
        let owner = self.owner()?;
        query(|p| unsafe { sys::champion_cooldown(owner.0, self.slot, p) })
    }
    /// Query whether the current cooldown is positive.
    pub fn is_on_cooldown(&self) -> Result<bool> {
        Ok(self.cooldown()? > 0)
    }
    /// Check whether this ability has metadata, is off cooldown, and its owner can cast.
    pub fn is_ready(&self) -> Result<bool> {
        if self.info()?.is_none() || self.is_on_cooldown()? {
            return Ok(false);
        }
        crate::unit::Champion::new(self.owner()?)?.can_cast()
    }
    /// Query current live targeting flags, or use cached flags for a draft design.
    pub fn valid_targets(&self) -> Result<AbilityTargetMask> {
        match self.owner {
            Some(h) => Ok(AbilityTargetMask::from_bits_retain(
                query(|p| unsafe { sys::champion_ability_props(h.0, self.slot, p) })?.targets,
            )),
            None => Ok(self
                .info()?
                .map_or(AbilityTargetMask::empty(), |p| p.as_dyn().valid_targets())),
        }
    }
    /// Build a fresh Chebyshev targeting-range mask around the live owner.
    /// Unused/unavailable slots and attack resets return errors; this is not a legality mask.
    pub fn valid_tiles(&self) -> Result<Bitboard> {
        let mut mask = Bitboard::new()?;
        self.mask_into(&mut mask)?;
        Ok(mask)
    }
    /// Like [`Self::valid_tiles`], but writes into `out`.
    ///
    /// # Arguments
    /// * `out` - Destination mask; its previous contents are overwritten.
    pub fn mask_into(&self, out: &mut Bitboard) -> Result<()> {
        let Some(payload) = self.info()? else {
            return Err(Error::local(
                ErrorCode::InvalidActionSpellCastNoAbility,
                "slot has no ability",
            ));
        };
        let range = match payload {
            AbilityInfo::AttackReset(_) => return Err(Error::NoRange),
            AbilityInfo::Damage(p) => p.range,
            AbilityInfo::AoeDamage(p) => p.range,
            AbilityInfo::Cc(p) => p.range,
            AbilityInfo::Heal(p) => p.range,
            AbilityInfo::Dash(p) => p.range,
            AbilityInfo::Shield(p) => p.range,
        };
        let owner = self.owner()?;
        let info = query(|p| unsafe { sys::champion(owner.0, p) })?;
        out.clear();
        out.draw_chebyshev(Vec2::new(info.x, info.y), range)
    }
    /// Test the current camp-target category flag.
    pub fn can_target_camps(&self) -> Result<bool> {
        Ok(self.valid_targets()?.contains(AbilityTargetMask::CAMPS))
    }
    /// Test the current enemy-target category flag.
    pub fn can_target_enemies(&self) -> Result<bool> {
        Ok(self.valid_targets()?.contains(AbilityTargetMask::ENEMIES))
    }
    /// Test the current ally-target category flag.
    pub fn can_target_allies(&self) -> Result<bool> {
        Ok(self.valid_targets()?.contains(AbilityTargetMask::ALLIES))
    }
    /// Test the current self-target category flag.
    pub fn can_target_self(&self) -> Result<bool> {
        Ok(self.valid_targets()?.contains(AbilityTargetMask::SELF))
    }
    /// Test the current ground-target category flag.
    pub fn can_target_ground(&self) -> Result<bool> {
        Ok(self.valid_targets()?.contains(AbilityTargetMask::GROUND))
    }
    /// Query this live ability's damage; unsupported kinds propagate the engine error.
    pub fn damage_amount(&self) -> Result<f32> {
        let owner = self.owner()?;
        query(|p| unsafe { sys::champion_ability_damage(owner.0, self.slot, p) })
    }
    /// Predict health lost after mitigation and shields, capped at current health. The target must be visible.
    ///
    /// # Arguments
    /// * `target` - Visible champion that would be hit.
    pub fn damage_amount_against(&self, target: &crate::Champion) -> Result<f32> {
        let owner = self.owner()?;
        let target = target.handle()?;
        query(|p| unsafe { sys::champion_ability_damage_against(owner.0, self.slot, target.0, p) })
    }
    /// Query this live ability's healing; unsupported kinds propagate the engine error.
    pub fn heal_amount(&self) -> Result<f32> {
        let owner = self.owner()?;
        query(|p| unsafe { sys::champion_ability_heal(owner.0, self.slot, p) })
    }
    /// Cap healing at the visible target's missing health. This does not cast or consume resources.
    ///
    /// # Arguments
    /// * `target` - Visible champion that would be healed.
    pub fn heal_amount_against(&self, target: &crate::Champion) -> Result<f32> {
        let owner = self.owner()?;
        let target = target.handle()?;
        query(|p| unsafe { sys::champion_ability_heal_against(owner.0, self.slot, target.0, p) })
    }
    /// Query this live ability's shielding; unsupported kinds propagate the engine error.
    pub fn shield_amount(&self) -> Result<f32> {
        let owner = self.owner()?;
        query(|p| unsafe { sys::champion_ability_shield(owner.0, self.slot, p) })
    }
    /// Query shielding against a particular champion, including engine target modifiers.
    ///
    /// # Arguments
    /// * `target` - Visible champion that would be shielded.
    pub fn shield_amount_against(&self, target: &crate::Champion) -> Result<f32> {
        let owner = self.owner()?;
        let target = target.handle()?;
        query(|p| unsafe { sys::champion_ability_shield_against(owner.0, self.slot, target.0, p) })
    }

    /// Cast for the current champion. `None` omits an entity or ground destination.
    /// The engine validates targeting, cooldown and action restrictions.
    pub(crate) fn cast(&self, target: Option<Entity>, destination: Option<Vec2>) -> Result<bool> {
        require_self(self.owner()?)?;
        let pos = destination.unwrap_or(Vec2::new(-1, -1));
        query(|out| unsafe {
            sys::cast(
                self.slot,
                target.map_or(0, |t| t.kind() as u32),
                target.map_or(0, Entity::handle),
                pos.x,
                pos.y,
                out,
            )
        }).map(|successful| successful != 0)
    }
    /// Cast without an entity or destination; requires the current champion.
    /// Returns `Ok(false)` for a newly revealed obstruction; invalid actions return `Err`.
    pub fn cast_untargeted(&self) -> Result<bool> {
        self.cast(None, None)
    }
    /// Cast at a ground destination; requires the current champion.
    /// Returns `Ok(false)` for a newly revealed obstruction; invalid actions return `Err`.
    ///
    /// # Arguments
    /// * `destination` - Ground tile to cast at; must be within the ability's range.
    pub fn cast_at(&self, destination: Vec2) -> Result<bool> {
        self.cast(None, Some(destination))
    }
    /// Cast on a champion, camp or unit; requires the current champion.
    /// Unit casts land on the target's tile; returns `Ok(false)` for a newly revealed obstruction.
    /// Invalid actions return `Err`.
    ///
    /// # Arguments
    /// * `target` - Champion or camp to cast on; it must be a valid target category for this ability.
    pub fn cast_on<T: crate::CastTarget + ?Sized>(&self, target: &T) -> Result<bool> {
        self.cast(Some(crate::detail::Sealed::entity(target)?), None)
    }
}

/// Object-safe metadata for an owning live buff snapshot.
pub trait BuffInfo {
    /// Return the cached typed kind.
    fn kind(&self) -> BuffKind;
    /// Return the captured buff duration without another host query.
    fn duration(&self) -> i32;
    /// Return the captured shield magnitude, or `None` for other buff kinds.
    fn shield_value(&self) -> Option<f32> {
        None
    }
}
/// An immutable buff value captured by a fresh collection query.
#[derive(Clone, Copy, Debug, PartialEq)]
pub enum BuffPayload {
    /// A captured shield with remaining duration and magnitude.
    Shield {
        /// Remaining shield duration captured by the collection query.
        duration: i32,
        /// Remaining shield magnitude captured by the collection query.
        value: f32,
    },
    /// A captured stun with remaining duration.
    Stun {
        /// Remaining effect duration captured by the collection query.
        duration: i32,
    },
    /// A captured enhancement of the next attack.
    EnhancedAttack {
        /// Remaining effect duration captured by the collection query.
        duration: i32,
    },
}
impl BuffInfo for BuffPayload {
    fn kind(&self) -> BuffKind {
        match self {
            Self::Shield { .. } => BuffKind::Shield,
            Self::Stun { .. } => BuffKind::Stun,
            Self::EnhancedAttack { .. } => BuffKind::EnhancedAttack,
        }
    }
    fn duration(&self) -> i32 {
        match self {
            Self::Shield { duration, .. }
            | Self::Stun { duration }
            | Self::EnhancedAttack { duration } => *duration,
        }
    }
    fn shield_value(&self) -> Option<f32> {
        if let Self::Shield { value, .. } = self {
            Some(*value)
        } else {
            None
        }
    }
}
impl BuffPayload {
    /// Borrow an object-safe view of this owned typed metadata.
    #[must_use]
    pub fn as_dyn(&self) -> &dyn BuffInfo {
        self
    }
    /// Decode a checked buff kind into an owned value, without enum transmutation.
    fn decode(kind: u32, duration: i32, value: f32) -> Result<Self> {
        Ok(match BuffKind::try_from(kind)? {
            BuffKind::Shield => Self::Shield { duration, value },
            BuffKind::Stun => Self::Stun { duration },
            BuffKind::EnhancedAttack => Self::EnhancedAttack { duration },
        })
    }
}
/// Unit buffs captured at construction; call buffs() again for current values.
#[derive(Clone, Copy, Debug, PartialEq)]
pub struct BuffCollection {
    values: [Option<BuffPayload>; 3],
}
impl BuffCollection {
    /// Eagerly query all buff values once; designs lack identity and duplicate kinds replace earlier values.
    pub(crate) fn new(owner: Option<Entity>) -> Result<Self> {
        let mut values = [None, None, None];
        let mut insert = |buff: BuffPayload| {
            let index = buff.kind() as usize;
            values[index] = Some(buff);
        };
        match owner {
            None => {
                return Err(Error::local(
                    ErrorCode::ApiNotInVision,
                    "design champion has no live buffs",
                ))
            }
            Some(Entity(crate::EntityTarget::Champion(h))) => {
                let raw =
                    crate::list(|p, n, out| unsafe { sys::champion_buff_data(h.0, p, n, out) })?;
                if raw.len() > 3 {
                    return Err(Error::invalid_argument("invalid host buff count"));
                }
                for b in raw {
                    insert(BuffPayload::decode(b.kind, b.duration, b.value)?);
                }
            }
            Some(Entity(crate::EntityTarget::Camp(h))) => {
                let raw =
                    crate::counted_list(3, |p, n, out| unsafe { sys::camp_buffs(h.0, p, n, out) })?;
                if raw.len() > 3 {
                    return Err(Error::invalid_argument("invalid host buff count"));
                }
                for b in raw {
                    insert(BuffPayload::decode(b.kind, b.timer, b.magnitude)?);
                }
            }
            _ => return Err(Error::invalid_argument("buff owner must be a unit")),
        }
        Ok(Self { values })
    }
    /// Get an owned buff from this collection, or None when it was absent at construction.
    #[must_use]
    ///
    /// # Arguments
    /// * `kind` - Buff kind to look up.
    pub fn get(&self, kind: BuffKind) -> Option<BuffPayload> {
        self.values[kind as usize]
    }
    /// Get an owned object-safe buff view without querying the host again.
    #[must_use]
    ///
    /// # Arguments
    /// * `kind` - Buff kind to look up.
    pub fn dyn_info(&self, kind: BuffKind) -> Option<Box<dyn BuffInfo>> {
        self.get(kind).map(|b| Box::new(b) as Box<dyn BuffInfo>)
    }
    /// Test whether a buff was present when this collection was constructed.
    #[must_use]
    ///
    /// # Arguments
    /// * `kind` - Buff kind to test for.
    pub fn has(&self, kind: BuffKind) -> bool {
        self.values[kind as usize].is_some()
    }
    /// Return whether the captured collection has a shield buff.
    #[must_use]
    pub fn has_shield(&self) -> bool {
        self.has(BuffKind::Shield)
    }
    /// Return whether the captured collection has a stun buff.
    #[must_use]
    pub fn has_stun(&self) -> bool {
        self.has(BuffKind::Stun)
    }
    /// Return whether the captured collection has an enhanced-attack buff.
    #[must_use]
    pub fn has_enhanced_attack(&self) -> bool {
        self.has(BuffKind::EnhancedAttack)
    }
    /// Count the captured active buff kinds.
    #[must_use]
    pub fn len(&self) -> usize {
        self.values.iter().flatten().count()
    }
    /// Return whether no buffs were captured.
    #[must_use]
    pub fn is_empty(&self) -> bool {
        self.len() == 0
    }
    /// Iterate owned buff values without re-querying the host.
    pub fn iter(&self) -> impl Iterator<Item = BuffPayload> {
        self.values.into_iter().flatten()
    }
}
