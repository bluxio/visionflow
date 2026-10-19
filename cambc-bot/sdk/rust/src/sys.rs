//! Private exact C records and env imports, with test-only fake-host equivalents.
//! All pointer calls require valid aligned buffers, the declared capacity, and the correct guest phase.
//! Most i32 results are zero-success engine status codes; identity/fuel/diagnostic-length queries differ.
//! Raw enum tags and ABI-only fields are validated or kept private by typed wrappers.
#![allow(non_snake_case)]
use crate::Vec2;
/// Exact engine item record; private fields retained even when not publicly exposed.
#[repr(C)]
#[derive(Clone, Copy, Default)]
pub(crate) struct GameItemInfo {
    /// Gold cost to purchase one unit.
    pub cost: u32,
    /// Whether this item can be purchased directly (0 or 1).
    pub purchasable: u32,
    /// Item health bonus.
    pub health_points: i32,
    /// Strength points reported by this record.
    pub strength_points: i32,
    /// Focus points reported by this record.
    pub focus_points: i32,
    /// Raw engine tag; the typed shop wrapper decodes the absence sentinel and validates other values.
    pub passive: u32,
    /// Magnitude interpreted according to the item passive.
    pub passive_magnitude: f32,
    /// Number of identical ingredients required for the combination.
    pub combine_count: u32,
    /// Raw engine tag; the typed shop wrapper decodes the absence sentinel and validates other values.
    pub result: u32,
    /// Ingredient-value ABI field, deliberately absent from public `ItemInfo`.
    pub combine_cost: u32,
}
const _: () = {
    assert!(core::mem::size_of::<GameItemInfo>() == 40);
    assert!(core::mem::align_of::<GameItemInfo>() == 4);
    assert!(core::mem::offset_of!(GameItemInfo, strength_points) == 12);
    assert!(core::mem::offset_of!(GameItemInfo, focus_points) == 16);
    assert!(core::mem::offset_of!(GameItemInfo, passive_magnitude) == 24);
    assert!(core::mem::offset_of!(GameItemInfo, combine_cost) == 36);
};
/// Exact chunked map/storage layout returned by the host; queried without a global cache.
#[repr(C)]
#[derive(Clone, Copy, Default, Debug)]
pub(crate) struct Metadata {
    /// Map width in tiles.
    pub width: i32,
    /// Map height in tiles.
    pub height: i32,
    /// Number of horizontal bitboard chunks.
    pub chunked_width: u32,
    /// Number of vertical bitboard chunks.
    pub chunked_height: u32,
    /// Engine stride between chunk rows.
    pub chunk_stride: u32,
    /// Required chunked bitboard storage size in bytes.
    pub board_bytes: u32,
    /// Required chunked distance-field storage size in bytes.
    pub field_bytes: u32,
}
/// Engine rectangle with inclusive minimum and exclusive maximum coordinates.
#[repr(C)]
#[derive(Clone, Copy, Default)]
pub(crate) struct Aabb {
    /// Inclusive minimum horizontal coordinate.
    pub min_x: i32,
    /// Inclusive minimum vertical coordinate.
    pub min_y: i32,
    /// Exclusive maximum horizontal coordinate.
    pub max_x: i32,
    /// Exclusive maximum vertical coordinate.
    pub max_y: i32,
}
/// Engine camp-group reward record.
#[repr(C)]
#[derive(Clone, Copy, Default)]
pub(crate) struct CampGroupInfo {
    /// Camp-group gem reward.
    pub gems: u32,
    /// Camp-group gold reward.
    pub gold: u32,
}
/// Engine camp spawn position and raw type tag.
#[repr(C)]
#[derive(Clone, Copy, Default)]
pub(crate) struct CampInfo {
    /// Horizontal integer coordinate.
    pub x: i32,
    /// Vertical integer coordinate.
    pub y: i32,
    /// Raw engine type tag, validated before exposure as a typed value.
    pub kind: i32,
}
/// Engine live camp stats and cooldown record.
#[repr(C)]
#[derive(Clone, Copy, Default)]
pub(crate) struct CampState {
    /// Horizontal integer coordinate.
    pub x: i32,
    /// Vertical integer coordinate.
    pub y: i32,
    /// Current health points.
    pub health: f32,
    /// Current maximum health points.
    pub max_health: f32,
    /// Strength reported by this record.
    pub strength: f32,
    /// Focus reported by this record.
    pub focus: f32,
    /// Remaining cooldowns for the two engine ability slots.
    pub cooldowns: [i32; 2],
}
/// Engine live champion position, statistics and packed state flags.
#[repr(C)]
#[derive(Clone, Copy, Default)]
pub(crate) struct ChampionInfo {
    /// Absolute engine team ID, decoded into a relative team by typed wrappers.
    pub team: i32,
    /// Raw engine type tag, validated before exposure as a typed value.
    pub kind: i32,
    /// Horizontal integer coordinate.
    pub x: i32,
    /// Vertical integer coordinate.
    pub y: i32,
    /// Current health points.
    pub health: f32,
    /// Current maximum health points.
    pub max_health: f32,
    /// Strength reported by this record.
    pub strength: f32,
    /// Focus reported by this record.
    pub focus: f32,
    /// Current engine movement-speed value.
    pub move_speed: i32,
    /// Current engine attack-range value.
    pub attack_range: i32,
    /// Packed engine state flags.
    pub state: i32,
}
/// Immutable champion design stats in the engine C layout, not current combat stats.
#[repr(C)]
#[derive(Clone, Copy, Default, Debug, PartialEq)]
pub struct ChampionStats {
    /// Design vision radius.
    pub vision_radius: i32,
    /// Design movement speed before live modifiers.
    pub base_move_speed: i32,
    /// Design attack range before live modifiers.
    pub base_attack_range: i32,
    /// Design base health.
    pub base_health: f32,
    /// Design health-growth parameter.
    pub health_growth: f32,
    /// Design base strength.
    pub base_strength: f32,
    /// Design strength-growth parameter.
    pub strength_growth: f32,
    /// Design base focus.
    pub base_focus: f32,
    /// Design focus-growth parameter.
    pub focus_growth: f32,
    /// Design damage multiplier against camps.
    pub camp_damage_multiplier: f32,
}
/// Immutable camp design stats in the engine C layout, not current combat stats.
#[repr(C)]
#[derive(Clone, Copy, Default, Debug, PartialEq)]
pub struct CampStats {
    /// Design vision radius.
    pub vision_radius: f32,
    /// Design movement speed before live modifiers.
    pub base_move_speed: i32,
    /// Design attack range before live modifiers.
    pub base_attack_range: i32,
    /// Design base health.
    pub base_health: f32,
    /// Design health-growth parameter.
    pub health_growth: f32,
    /// Design base strength.
    pub base_strength: f32,
    /// Design strength-growth parameter.
    pub strength_growth: f32,
    /// Design base focus.
    pub base_focus: f32,
    /// Design focus-growth parameter.
    pub focus_growth: f32,
}
/// Exact ABI ability record; payload words are decoded according to the checked kind.
#[repr(C)]
#[derive(Clone, Copy, Default)]
pub(crate) struct AbilityProps {
    /// Raw engine type tag, validated before exposure as a typed value.
    pub kind: u32,
    /// Engine cooldown value.
    pub cooldown: i32,
    /// ABI-preserved metadata not exposed by the typed ability API.
    pub max_charges: u32,
    /// Raw target-category mask.
    pub targets: u32,
    /// Raw engine ability-attribute bitmask.
    pub attributes: u32,
    /// Four ABI words interpreted according to the ability kind.
    pub payload: [u32; 4],
}
/// Engine unit definition, including explicit C padding and two ability slots.
#[repr(C)]
#[derive(Clone, Copy, Default)]
pub(crate) struct UnitDefinition {
    /// Raw engine type tag, validated before exposure as a typed value.
    pub kind: i32,
    /// Raw engine vision-type tag.
    pub vision_type: u32,
    /// Raw engine passive-type tag.
    pub passive_type: u8,
    /// Padding required by the C ABI; not a public bot property.
    pub padding: [u8; 3],
    /// Two immutable slot wrappers; cooldowns and actions remain live queries.
    pub abilities: [AbilityProps; 2],
    pub attack_type: u8,
    /// Trailing padding required by the C ABI.
    pub tail_padding: [u8; 3],
}
/// Engine champion definition and immutable base-stat record.
#[repr(C)]
#[derive(Clone, Copy, Default)]
pub(crate) struct ChampionDesign {
    /// Engine unit definition.
    pub definition: UnitDefinition,
    /// Immutable engine design statistics.
    pub base_stats: ChampionStats,
}
/// Engine camp definition and immutable base-stat record.
#[repr(C)]
#[derive(Clone, Copy, Default)]
pub(crate) struct CampDesign {
    /// Engine unit definition.
    pub definition: UnitDefinition,
    /// Immutable engine design statistics.
    pub base_stats: CampStats,
}
/// Engine camp buff record, decoded into an owning typed buff value.
#[repr(C)]
#[derive(Clone, Copy, Default)]
pub(crate) struct CampBuffData {
    /// Raw engine type tag, validated before exposure as a typed value.
    pub kind: u32,
    /// Engine buff magnitude.
    pub magnitude: f32,
    /// Engine remaining timer; a negative value may indicate no finite timer.
    pub timer: i32,
}
/// Engine champion buff record, decoded into an owning typed buff value.
#[repr(C)]
#[derive(Clone, Copy, Default)]
pub(crate) struct BuffData {
    /// Raw engine type tag, validated before exposure as a typed value.
    pub kind: u32,
    /// Effect duration reported by the engine.
    pub duration: i32,
    /// Captured shield magnitude.
    pub value: f32,
}
/// Engine altar-lock record with raw entity kind and absolute team.
#[repr(C)]
#[derive(Clone, Copy, Default)]
pub(crate) struct AltarLockInfo {
    /// Raw engine type tag, validated before exposure as a typed value.
    pub kind: u32,
    /// Engine identity, not a visibility or ownership guarantee.
    pub handle: u32,
    /// Absolute engine team ID, decoded into a relative team by typed wrappers.
    pub team: u32,
    /// Horizontal integer coordinate.
    pub x: i32,
    /// Vertical integer coordinate.
    pub y: i32,
    /// Hits still required to destroy this altar lock.
    pub hits_remaining: u32,
}
/// Engine ward-resource record for cooldown and available ward uses.
#[repr(C)]
#[derive(Clone, Copy, Default)]
pub(crate) struct WardResources {
    /// Available ordinary-ward uses.
    pub charges: u32,
    /// Engine cooldown value.
    pub cooldown: i32,
}
/// Engine ordinary-ward observation record; does not imply an action-target handle.
#[repr(C)]
#[derive(Clone, Copy, Default)]
pub(crate) struct WardInfo {
    /// Absolute engine team ID, decoded into a relative team by typed wrappers.
    pub team: u32,
    /// Horizontal integer coordinate.
    pub x: i32,
    /// Vertical integer coordinate.
    pub y: i32,
    /// Engine remaining timer; a negative value may indicate no finite timer.
    pub timer: i32,
    /// Whether this ward is disabled.
    pub disabled: u8,
}
/// Engine control-ward observation record; does not imply an action-target handle.
#[repr(C)]
#[derive(Clone, Copy, Default)]
#[allow(dead_code)]
pub(crate) struct ControlWardInfo {
    /// Absolute engine team ID, decoded into a relative team by typed wrappers.
    pub team: u32,
    /// Horizontal integer coordinate.
    pub x: i32,
    /// Vertical integer coordinate.
    pub y: i32,
    /// Engine remaining timer; a negative value may indicate no finite timer.
    pub timer: i32,
}
/// Exact engine teleport record; raw caster type remains private ABI data.
#[repr(C)]
#[derive(Clone, Copy, Default)]
pub(crate) struct TeleportInfo {
    /// Caster champion handle, not a copied live champion.
    pub champion: u32,
    /// Raw engine type tag, validated before exposure as a typed value.
    pub kind: i32,
    /// Absolute engine team ID, decoded into a relative team by typed wrappers.
    pub team: u32,
    /// Reserved teleport destination.
    pub destination: Vec2,
    /// Engine remaining timer; a negative value may indicate no finite timer.
    pub timer: i32,
    /// Channel type: 4 for teleport crystal; 3 is the retired scroll.
    pub channel_type: u32,
}
#[cfg(not(test))]
#[link(wasm_import_module = "env")]
unsafe extern "C" {
    pub(crate) fn game_settings(output: *mut crate::Constants) -> i32;
    pub(crate) fn item_info(kind: u32, output: *mut GameItemInfo) -> i32;
    pub(crate) fn consumable_info(kind: u32, output: *mut crate::shop::ConsumableInfo) -> i32;
    #[link_name = "champion_buff_data"]
    pub(crate) fn champion_buff_data(
        champion: u32,
        buffs: *mut BuffData,
        capacity: u32,
        out_count: *mut u32,
    ) -> i32;
    #[link_name = "champion_ability_damage"]
    pub(crate) fn champion_ability_damage(champion: u32, slot: u32, out_damage: *mut f32) -> i32;
    #[link_name = "champion_ability_damage_against"]
    pub(crate) fn champion_ability_damage_against(
        champion: u32,
        slot: u32,
        target: u32,
        out_damage: *mut f32,
    ) -> i32;
    #[link_name = "champion_ability_heal"]
    pub(crate) fn champion_ability_heal(champion: u32, slot: u32, out_heal: *mut f32) -> i32;
    #[link_name = "champion_ability_heal_against"]
    pub(crate) fn champion_ability_heal_against(
        champion: u32,
        slot: u32,
        target: u32,
        out_heal: *mut f32,
    ) -> i32;
    #[link_name = "champion_ability_shield"]
    pub(crate) fn champion_ability_shield(champion: u32, slot: u32, out_shield: *mut f32) -> i32;
    #[link_name = "champion_ability_shield_against"]
    pub(crate) fn champion_ability_shield_against(
        champion: u32,
        slot: u32,
        target: u32,
        out_shield: *mut f32,
    ) -> i32;
    #[link_name = "bc_get_last_error"]
    pub(crate) fn bc_get_last_error(buffer: *mut u8, capacity: u32) -> i32;
    #[link_name = "my_champion"]
    pub(crate) fn my_champion() -> i32;
    #[link_name = "champion_is_alive"]
    pub(crate) fn champion_is_alive(champion: u32, out_alive: *mut i32) -> i32;
    #[link_name = "num_allies"]
    pub(crate) fn num_allies(out_count: *mut u32) -> i32;
    #[link_name = "num_enemies"]
    pub(crate) fn num_enemies(out_count: *mut u32) -> i32;
    #[link_name = "allies"]
    pub(crate) fn allies(champions: *mut u32, capacity: u32, out_count: *mut u32) -> i32;
    #[link_name = "enemies"]
    pub(crate) fn enemies(champions: *mut u32, capacity: u32, out_count: *mut u32) -> i32;
    #[link_name = "board_ally_occupancy"]
    pub(crate) fn ally_occupancy(out_board: *mut u64) -> i32;
    #[link_name = "board_enemy_occupancy"]
    pub(crate) fn enemy_occupancy(out_board: *mut u64) -> i32;
    #[link_name = "board_camp_occupancy"]
    pub(crate) fn camp_occupancy(out_board: *mut u64) -> i32;
    #[link_name = "board_ally_ward_occupancy"]
    pub(crate) fn ally_ward_occupancy(out_board: *mut u64) -> i32;
    #[link_name = "board_enemy_ward_occupancy"]
    pub(crate) fn enemy_ward_occupancy(out_board: *mut u64) -> i32;
    #[link_name = "ward_at"]
    pub(crate) fn ward_at(x: i32, y: i32, out_handle: *mut u32) -> i32;
    #[link_name = "ally_ward_at"]
    pub(crate) fn ally_ward_at(x: i32, y: i32, out_handle: *mut u32) -> i32;
    #[link_name = "enemy_ward_at"]
    pub(crate) fn enemy_ward_at(x: i32, y: i32, out_handle: *mut u32) -> i32;
    #[link_name = "ward"]
    pub(crate) fn ward(handle: u32, out_info: *mut WardInfo) -> i32;
    #[link_name = "board_teleport_occupancy"]
    pub(crate) fn teleport_occupancy(out_board: *mut u64) -> i32;
    #[link_name = "is_occupied_by_teleport"]
    pub(crate) fn has_teleport(x: i32, y: i32, out_present: *mut i32) -> i32;
    #[link_name = "teleports"]
    pub(crate) fn teleports(
        teleports: *mut TeleportInfo,
        capacity: u32,
        out_count: *mut u32,
    ) -> i32;
    #[link_name = "board_ward_vision_mask"]
    pub(crate) fn ward_vision(x: i32, y: i32, out_board: *mut u64) -> i32;
    #[link_name = "board_ally_altar_lock_occupancy"]
    pub(crate) fn ally_altar_lock_occupancy(out_board: *mut u64) -> i32;
    #[link_name = "board_enemy_altar_lock_occupancy"]
    pub(crate) fn enemy_altar_lock_occupancy(out_board: *mut u64) -> i32;
    #[link_name = "altar_lock"]
    pub(crate) fn altar_lock(x: i32, y: i32, team: i32, out_info: *mut AltarLockInfo) -> i32;
    #[link_name = "altar_locks"]
    pub(crate) fn altar_locks(locks: *mut AltarLockInfo, capacity: u32, out_count: *mut u32)
        -> i32;
    #[link_name = "attack"]
    pub(crate) fn attack(target_kind: u32, target_handle: u32) -> i32;
    #[link_name = "cast_with_result"]
    pub(crate) fn cast(slot: u32, target_kind: u32, target_handle: u32, x: i32, y: i32, out_successful: *mut i32) -> i32;
    #[link_name = "use_consumable_with_result"]
    pub(crate) fn use_consumable(kind: u32, x: i32, y: i32, out_successful: *mut i32) -> i32;
    #[link_name = "buy_consumable"]
    pub(crate) fn buy_consumable(kind: u32) -> i32;
    #[link_name = "buy_item"]
    pub(crate) fn buy_item(kind: u32, count: u32) -> i32;
    #[link_name = "sell"]
    pub(crate) fn sell(kind: u32) -> i32;
    #[link_name = "recall"]
    pub(crate) fn recall() -> i32;
    #[link_name = "cancel_channel"]
    pub(crate) fn cancel_channel() -> i32;
    #[link_name = "submit_gems"]
    pub(crate) fn submit_gems(x: i32, y: i32) -> i32;
    #[link_name = "place_ward_with_result"]
    pub(crate) fn place_ward(x: i32, y: i32, out_successful: *mut i32) -> i32;
    #[link_name = "move_to_with_result"]
    pub(crate) fn move_to(x: i32, y: i32, out_successful: *mut i32) -> i32;
    #[link_name = "set_priority"]
    pub(crate) fn set_priority(priority: i32) -> i32;
    #[link_name = "bc_champion_priority"]
    pub(crate) fn bc_champion_priority(
        champion: u32,
        previous_turn: i32,
        out_priority: *mut i32,
    ) -> i32;
    #[link_name = "bc_submitted_gems"]
    pub(crate) fn bc_submitted_gems(team: u32, out_gems: *mut u32) -> i32;
    #[link_name = "bc_round_number"]
    pub(crate) fn bc_round_number(out_round: *mut u32) -> i32;
    #[link_name = "resign"]
    pub(crate) fn resign() -> i32;
    #[link_name = "shared_memory_size"]
    pub(crate) fn shared_memory_size(out_size: *mut u32) -> i32;
    #[link_name = "read_shared_memory"]
    pub(crate) fn read_shared_memory(
        offset: i32,
        buffer: *mut core::ffi::c_void,
        byte_count: u32,
    ) -> i32;
    #[link_name = "write_shared_memory"]
    pub(crate) fn write_shared_memory(
        offset: i32,
        buffer: *const core::ffi::c_void,
        byte_count: u32,
    ) -> i32;
    #[link_name = "champion"]
    pub(crate) fn champion(champion: u32, out_info: *mut ChampionInfo) -> i32;
    #[link_name = "champion_design"]
    pub(crate) fn champion_design(kind: i32, out_design: *mut ChampionDesign) -> i32;
    #[link_name = "champion_restrictions"]
    pub(crate) fn champion_restrictions(champion: u32, out_restrictions: *mut u8) -> i32;
    #[link_name = "champion_ability_props"]
    pub(crate) fn champion_ability_props(
        champion: u32,
        slot: u32,
        out_properties: *mut AbilityProps,
    ) -> i32;
    #[link_name = "champion_cooldown"]
    pub(crate) fn champion_cooldown(champion: u32, slot: u32, out_cooldown: *mut i32) -> i32;
    #[link_name = "champion_items"]
    pub(crate) fn champion_items(champion: u32, out_items: *mut u32) -> i32;
    #[link_name = "champion_consumables"]
    pub(crate) fn champion_consumables(champion: u32, out_counts: *mut u32) -> i32;
    #[link_name = "champion_gems"]
    pub(crate) fn champion_gems(champion: u32, out_gems: *mut u32) -> i32;
    #[link_name = "champion_gold"]
    pub(crate) fn champion_gold(champion: u32, out_gold: *mut u32) -> i32;
    #[link_name = "bc_champion_vision"]
    pub(crate) fn bc_champion_vision(champion: u32, out_board: *mut u64) -> i32;
    #[link_name = "board_vision_mask"]
    pub(crate) fn hypothetical_vision(x: i32, y: i32, vision_type: i32, out_board: *mut u64)
        -> i32;
    #[link_name = "champion_wards"]
    pub(crate) fn champion_wards(champion: u32, out_resources: *mut WardResources) -> i32;
    #[link_name = "bc_champion_ward_positions"]
    pub(crate) fn champion_ward_positions(
        champion: u32,
        positions: *mut Vec2,
        capacity: u32,
        out_count: *mut u32,
    ) -> i32;
    #[link_name = "bc_champion_channel_time"]
    pub(crate) fn champion_channel_time(champion: u32, out_time: *mut i32) -> i32;
    #[link_name = "champion_can_shop"]
    pub(crate) fn champion_can_shop(champion: u32, consumable: i32, out_allowed: *mut i32) -> i32;
    #[link_name = "champion_can_sell"]
    pub(crate) fn champion_can_sell(champion: u32, out_allowed: *mut i32) -> i32;
    #[link_name = "wards"]
    pub(crate) fn wards(wards: *mut WardInfo, capacity: u32, out_count: *mut u32) -> i32;
    /// Exact C-ABI `bc_metadata` host operation; pointer buffers obey the module safety contract.
    #[link_name = "bc_metadata"]
    pub(crate) fn bc_metadata(out_metadata: *mut Metadata) -> i32;
    /// Return the absolute observing team ID directly, not an engine status code.
    #[link_name = "my_team"]
    pub(crate) fn my_team() -> i32;
    /// Exact C-ABI `bc_map_mask` host operation; pointer buffers obey the module safety contract.
    #[link_name = "bc_map_mask"]
    pub(crate) fn bc_map_mask(feature: i32, out_board: *mut u64) -> i32;
    /// Exact C-ABI `bc_map_symmetry` host operation; pointer buffers obey the module safety contract.
    #[link_name = "bc_map_symmetry"]
    pub(crate) fn bc_map_symmetry(out_symmetry: *mut i32) -> i32;
    /// Exact C-ABI `map_camp_group_num_camps` host operation; pointer buffers obey the module safety contract.
    #[link_name = "map_camp_group_num_camps"]
    pub(crate) fn map_camp_group_num_camps(group: u32, out_count: *mut u32) -> i32;
    /// Exact C-ABI `map_camp_group_camps` host operation; pointer buffers obey the module safety contract.
    #[link_name = "map_camp_group_camps"]
    pub(crate) fn map_camp_group_camps(
        group: u32,
        camps: *mut u32,
        capacity: u32,
        out_count: *mut u32,
    ) -> i32;
    /// Exact C-ABI `map_camp_group` host operation; pointer buffers obey the module safety contract.
    #[link_name = "map_camp_group"]
    pub(crate) fn map_camp_group(group: u32, out_info: *mut CampGroupInfo) -> i32;
    /// Exact C-ABI `map_camp_info` host operation; pointer buffers obey the module safety contract.
    #[link_name = "map_camp_info"]
    pub(crate) fn map_camp_info(camp: u32, out_info: *mut CampInfo) -> i32;
    /// Exact C-ABI `map_camp_layout` host operation; pointer buffers obey the module safety contract.
    #[link_name = "map_camp_layout"]
    pub(crate) fn map_camp_layout(out_words: *mut u32, capacity: u32, out_count: *mut u32) -> i32;
    /// Exact C-ABI `map_camp_group_of` host operation; pointer buffers obey the module safety contract.
    #[link_name = "map_camp_group_of"]
    pub(crate) fn map_camp_group_of(camp: u32, out_group: *mut u32) -> i32;
    /// Exact C-ABI `map_camp_at` host operation; pointer buffers obey the module safety contract.
    #[link_name = "map_camp_at"]
    pub(crate) fn map_camp_at(x: i32, y: i32, out_camp: *mut u32) -> i32;
    /// Exact C-ABI `camp_group_cooldown` host operation; pointer buffers obey the module safety contract.
    #[link_name = "camp_group_cooldown"]
    pub(crate) fn camp_group_cooldown(group: u32, out_cooldown: *mut i32) -> i32;
    /// Exact C-ABI `camp_group_state` host operation; pointer buffers obey the module safety contract.
    #[link_name = "camp_group_state"]
    pub(crate) fn camp_group_state(group: u32, out_state: *mut u32) -> i32;
    /// Exact C-ABI `camp_buffs` host operation; pointer buffers obey the module safety contract.
    #[link_name = "camp_buffs"]
    pub(crate) fn camp_buffs(
        camp: u32,
        buffs: *mut CampBuffData,
        capacity: u32,
        out_count: *mut u32,
    ) -> i32;
    /// Exact C-ABI `bc_camp_group_max_respawn` host operation; pointer buffers obey the module safety contract.
    #[link_name = "bc_camp_group_max_respawn"]
    pub(crate) fn bc_camp_group_max_respawn(group: u32, out_timer: *mut i32) -> i32;
    /// Exact C-ABI `camp` host operation; pointer buffers obey the module safety contract.
    #[link_name = "camp"]
    pub(crate) fn camp(camp: u32, out_state: *mut CampState) -> i32;
    /// Exact C-ABI `camp_design` host operation; pointer buffers obey the module safety contract.
    #[link_name = "camp_design"]
    pub(crate) fn camp_design(kind: i32, out_design: *mut CampDesign) -> i32;
    /// Exact C-ABI `camp_at` host operation; pointer buffers obey the module safety contract.
    #[link_name = "camp_at"]
    pub(crate) fn camp_at(x: i32, y: i32, out_handle: *mut u32) -> i32;
    /// Exact C-ABI `champion_at` host operation; pointer buffers obey the module safety contract.
    #[link_name = "champion_at"]
    pub(crate) fn champion_at(x: i32, y: i32, out_handle: *mut u32) -> i32;
    /// Exact C-ABI `has_camp` host operation; pointer buffers obey the module safety contract.
    #[link_name = "has_camp"]
    pub(crate) fn has_camp(x: i32, y: i32, out_present: *mut i32) -> i32;
    /// Exact C-ABI `has_champion` host operation; pointer buffers obey the module safety contract.
    #[link_name = "has_champion"]
    pub(crate) fn has_champion(x: i32, y: i32, out_present: *mut i32) -> i32;
    /// Exact C-ABI `vision` host operation; pointer buffers obey the module safety contract.
    #[link_name = "board_vision"]
    pub(crate) fn vision(out_board: *mut u64) -> i32;
    /// Exact C-ABI `spawn_region` host operation; pointer buffers obey the module safety contract.
    #[link_name = "spawn_region"]
    pub(crate) fn spawn_region(team: i32, out_region: *mut Aabb) -> i32;
    /// Exact C-ABI `draft_num_rounds` host operation; pointer buffers obey the module safety contract.
    #[link_name = "draft_num_rounds"]
    pub(crate) fn draft_num_rounds(out_rounds: *mut u8) -> i32;
    /// Exact C-ABI `draft_get` host operation; pointer buffers obey the module safety contract.
    #[link_name = "draft_get"]
    pub(crate) fn draft_get(team: i32, round: u32, out_pick: *mut i32) -> i32;
    /// Exact C-ABI `draft_champion` host operation; pointer buffers obey the module safety contract.
    #[link_name = "draft_champion"]
    pub(crate) fn draft_champion(pick: i32) -> i32;
    /// Exact C-ABI `bc_submit_draft_memory` host operation; pointer buffers obey the module safety contract.
    #[link_name = "bc_submit_draft_memory"]
    pub(crate) fn bc_submit_draft_memory(
        buffer: *const core::ffi::c_void,
        byte_count: usize,
    ) -> i32;
    /// Exact C-ABI `draft_memory_size` host operation; pointer buffers obey the module safety contract.
    #[link_name = "draft_memory_size"]
    pub(crate) fn draft_memory_size(out_size: *mut u32) -> i32;
    /// Exact C-ABI `read_draft_memory` host operation; pointer buffers obey the module safety contract.
    #[link_name = "read_draft_memory"]
    pub(crate) fn read_draft_memory(
        offset: i32,
        buffer: *mut core::ffi::c_void,
        byte_count: u32,
    ) -> i32;
    /// Exact C-ABI `debug_dot` host operation; pointer buffers obey the module safety contract.
    #[link_name = "debug_dot"]
    pub(crate) fn debug_dot(x: i32, y: i32, color: u32) -> i32;
    /// Exact C-ABI `debug_line` host operation; pointer buffers obey the module safety contract.
    #[link_name = "debug_line"]
    pub(crate) fn debug_line(start_x: i32, start_y: i32, end_x: i32, end_y: i32, color: u32)
        -> i32;
    /// Yield the guest to finish its turn; resumes after this import when scheduled again.
    #[link_name = "yield"]
    pub(crate) fn r#yield() -> ();
    /// Return remaining fuel directly as an i64, not an engine status code.
    #[link_name = "fuel_remaining"]
    pub(crate) fn fuel_remaining() -> i64;
    /// Exact C-ABI `Bitboard_One` host operation; pointer buffers obey the module safety contract.
    #[link_name = "bc_Bitboard_One"]
    pub(crate) fn Bitboard_One(board: *mut u64) -> i32;
    /// Exact C-ABI `Bitboard_ImportRawBuffer` host operation; pointer buffers obey the module safety contract.
    #[link_name = "bc_Bitboard_ImportRawBuffer"]
    pub(crate) fn Bitboard_ImportRawBuffer(
        board: *mut u64,
        buffer: *const u8,
        byte_count: usize,
    ) -> i32;
    /// Exact C-ABI `Bitboard_ImportLinearBuffer` host operation; pointer buffers obey the module safety contract.
    #[link_name = "bc_Bitboard_ImportLinearBuffer"]
    pub(crate) fn Bitboard_ImportLinearBuffer(
        board: *mut u64,
        buffer: *const u8,
        byte_count: usize,
    ) -> i32;
    /// Exact C-ABI `Bitboard_ExportRawBuffer` host operation; pointer buffers obey the module safety contract.
    #[link_name = "bc_Bitboard_ExportRawBuffer"]
    pub(crate) fn Bitboard_ExportRawBuffer(
        board: *const u64,
        buffer: *mut u8,
        byte_count: usize,
    ) -> i32;
    /// Exact C-ABI `Bitboard_ExportLinearBuffer` host operation; pointer buffers obey the module safety contract.
    #[link_name = "bc_Bitboard_ExportLinearBuffer"]
    pub(crate) fn Bitboard_ExportLinearBuffer(
        board: *const u64,
        buffer: *mut u8,
        byte_count: usize,
    ) -> i32;
    /// Exact C-ABI `Bitboard_Print` host operation; pointer buffers obey the module safety contract.
    #[link_name = "bc_Bitboard_Print"]
    pub(crate) fn Bitboard_Print(board: *const u64) -> i32;
    /// Exact C-ABI `Bitboard_Test` host operation; pointer buffers obey the module safety contract.
    #[link_name = "bc_Bitboard_Test"]
    pub(crate) fn Bitboard_Test(board: *const u64, x: i32, y: i32, out_value: *mut i32) -> i32;
    /// Exact C-ABI `Bitboard_Set` host operation; pointer buffers obey the module safety contract.
    #[link_name = "bc_Bitboard_Set"]
    pub(crate) fn Bitboard_Set(board: *mut u64, x: i32, y: i32, value: i32) -> i32;
    /// Exact C-ABI `Bitboard_Toggle` host operation; pointer buffers obey the module safety contract.
    #[link_name = "bc_Bitboard_Toggle"]
    pub(crate) fn Bitboard_Toggle(board: *mut u64, x: i32, y: i32) -> i32;
    /// Exact C-ABI `Bitboard_IsZero` host operation; pointer buffers obey the module safety contract.
    #[link_name = "bc_Bitboard_IsZero"]
    pub(crate) fn Bitboard_IsZero(board: *const u64, out_empty: *mut i32) -> i32;
    /// Exact C-ABI `Bitboard_Zero` host operation; pointer buffers obey the module safety contract.
    #[link_name = "bc_Bitboard_Zero"]
    pub(crate) fn Bitboard_Zero(board: *mut u64) -> i32;
    /// Exact C-ABI `Bitboard_NumSetBits` host operation; pointer buffers obey the module safety contract.
    #[link_name = "bc_Bitboard_NumSetBits"]
    pub(crate) fn Bitboard_NumSetBits(board: *const u64, out_count: *mut u32) -> i32;
    /// Exact C-ABI `Bitboard_GetSetBits` host operation; pointer buffers obey the module safety contract.
    #[link_name = "bc_Bitboard_GetSetBits"]
    pub(crate) fn Bitboard_GetSetBits(
        board: *const u64,
        positions: *mut core::ffi::c_void,
        capacity: usize,
        out_count: *mut u32,
    ) -> i32;
    /// Exact C-ABI `Bitboard_Not` host operation; pointer buffers obey the module safety contract.
    #[link_name = "bc_Bitboard_Not"]
    pub(crate) fn Bitboard_Not(board: *const u64, out_board: *mut u64) -> i32;
    /// Exact C-ABI `Bitboard_Union` host operation; pointer buffers obey the module safety contract.
    #[link_name = "bc_Bitboard_Union"]
    pub(crate) fn Bitboard_Union(left: *const u64, right: *const u64, out_board: *mut u64) -> i32;
    /// Exact C-ABI `Bitboard_Intersect` host operation; pointer buffers obey the module safety contract.
    #[link_name = "bc_Bitboard_Intersect"]
    pub(crate) fn Bitboard_Intersect(
        left: *const u64,
        right: *const u64,
        out_board: *mut u64,
    ) -> i32;
    /// Exact C-ABI `Bitboard_Xor` host operation; pointer buffers obey the module safety contract.
    #[link_name = "bc_Bitboard_Xor"]
    pub(crate) fn Bitboard_Xor(left: *const u64, right: *const u64, out_board: *mut u64) -> i32;
    /// Exact C-ABI `Bitboard_IsEqual` host operation; pointer buffers obey the module safety contract.
    #[link_name = "bc_Bitboard_IsEqual"]
    pub(crate) fn Bitboard_IsEqual(left: *const u64, right: *const u64, out_equal: *mut i32)
        -> i32;
    /// Exact C-ABI `Bitboard_ShiftRight` host operation; pointer buffers obey the module safety contract.
    #[link_name = "bc_Bitboard_ShiftRight"]
    pub(crate) fn Bitboard_ShiftRight(board: *const u64, out_board: *mut u64, columns: i32) -> i32;
    /// Exact C-ABI `Bitboard_ShiftUp` host operation; pointer buffers obey the module safety contract.
    #[link_name = "bc_Bitboard_ShiftUp"]
    pub(crate) fn Bitboard_ShiftUp(board: *const u64, out_board: *mut u64, rows: i32) -> i32;
    /// Exact C-ABI `Bitboard_Reflect` host operation; pointer buffers obey the module safety contract.
    #[link_name = "bc_Bitboard_Reflect"]
    pub(crate) fn Bitboard_Reflect(board: *const u64, out_board: *mut u64, reflection: i32) -> i32;
    /// Exact C-ABI `Bitboard_DrawCircle` host operation; pointer buffers obey the module safety contract.
    #[link_name = "bc_Bitboard_DrawCircle"]
    pub(crate) fn Bitboard_DrawCircle(board: *mut u64, x: i32, y: i32, radius: f32) -> i32;
    /// Exact C-ABI `Bitboard_DrawChebyshev` host operation; pointer buffers obey the module safety contract.
    #[link_name = "bc_Bitboard_DrawChebyshev"]
    pub(crate) fn Bitboard_DrawChebyshev(board: *mut u64, x: i32, y: i32, radius: f32) -> i32;
    /// Exact C-ABI `Bitboard_ReflectHorizontalPosition` host operation; pointer buffers obey the module safety contract.
    #[link_name = "bc_Bitboard_ReflectHorizontalPosition"]
    pub(crate) fn Bitboard_ReflectHorizontalPosition(
        x: i32,
        y: i32,
        out_x: *mut i32,
        out_y: *mut i32,
    ) -> i32;
    /// Exact C-ABI `Bitboard_ReflectVerticalPosition` host operation; pointer buffers obey the module safety contract.
    #[link_name = "bc_Bitboard_ReflectVerticalPosition"]
    pub(crate) fn Bitboard_ReflectVerticalPosition(
        x: i32,
        y: i32,
        out_x: *mut i32,
        out_y: *mut i32,
    ) -> i32;
    /// Exact C-ABI `Bitboard_Rotate180Position` host operation; pointer buffers obey the module safety contract.
    #[link_name = "bc_Bitboard_Rotate180Position"]
    pub(crate) fn Bitboard_Rotate180Position(
        x: i32,
        y: i32,
        out_x: *mut i32,
        out_y: *mut i32,
    ) -> i32;
    /// Exact C-ABI `Bitboard_ReachableMask` host operation; pointer buffers obey the module safety contract.
    #[link_name = "bc_Bitboard_ReachableMask"]
    pub(crate) fn Bitboard_ReachableMask(
        walls: *const u64,
        x: i32,
        y: i32,
        out_board: *mut u64,
        max_distance: u16,
    ) -> i32;
    /// Exact C-ABI `Bitboard_ShadowCast` host operation; pointer buffers obey the module safety contract.
    #[link_name = "bc_Bitboard_ShadowCast"]
    pub(crate) fn Bitboard_ShadowCast(
        walls: *const u64,
        x: i32,
        y: i32,
        out_board: *mut u64,
        max_distance: u16,
    ) -> i32;
    /// Exact C-ABI `Bitboard_ReachableMaskWithScratch` host operation; pointer buffers obey the module safety contract.
    #[link_name = "bc_Bitboard_ReachableMaskWithScratch"]
    pub(crate) fn Bitboard_ReachableMaskWithScratch(
        walls: *const u64,
        x: i32,
        y: i32,
        out_board: *mut u64,
        max_distance: u16,
        frontier: *mut u64,
        scratch: *mut u64,
    ) -> i32;
    /// Exact C-ABI `DistanceField_ImportRawBuffer` host operation; pointer buffers obey the module safety contract.
    #[link_name = "bc_DistanceField_ImportRawBuffer"]
    pub(crate) fn DistanceField_ImportRawBuffer(
        field: *mut u16,
        buffer: *const u8,
        byte_count: usize,
    ) -> i32;
    /// Exact C-ABI `DistanceField_ImportLinearBuffer` host operation; pointer buffers obey the module safety contract.
    #[link_name = "bc_DistanceField_ImportLinearBuffer"]
    pub(crate) fn DistanceField_ImportLinearBuffer(
        field: *mut u16,
        buffer: *const u8,
        byte_count: usize,
    ) -> i32;
    /// Exact C-ABI `DistanceField_ExportRawBuffer` host operation; pointer buffers obey the module safety contract.
    #[link_name = "bc_DistanceField_ExportRawBuffer"]
    pub(crate) fn DistanceField_ExportRawBuffer(
        field: *const u16,
        buffer: *mut u8,
        byte_count: usize,
    ) -> i32;
    /// Exact C-ABI `DistanceField_ExportLinearBuffer` host operation; pointer buffers obey the module safety contract.
    #[link_name = "bc_DistanceField_ExportLinearBuffer"]
    pub(crate) fn DistanceField_ExportLinearBuffer(
        field: *const u16,
        buffer: *mut u8,
        byte_count: usize,
    ) -> i32;
    /// Exact C-ABI `DistanceField_Print` host operation; pointer buffers obey the module safety contract.
    #[link_name = "bc_DistanceField_Print"]
    pub(crate) fn DistanceField_Print(field: *const u16) -> i32;
    /// Exact C-ABI `DistanceField_Get` host operation; pointer buffers obey the module safety contract.
    #[link_name = "bc_DistanceField_Get"]
    pub(crate) fn DistanceField_Get(
        field: *const u16,
        x: i32,
        y: i32,
        out_distance: *mut u16,
    ) -> i32;
    /// Exact C-ABI `DistanceField_Set` host operation; pointer buffers obey the module safety contract.
    #[link_name = "bc_DistanceField_Set"]
    pub(crate) fn DistanceField_Set(field: *mut u16, x: i32, y: i32, distance: u16) -> i32;
    /// Exact C-ABI `DistanceField_ComputeWithExpansionCount` host operation; pointer buffers obey the module safety contract.
    #[link_name = "bc_DistanceField_ComputeWithExpansionCount"]
    pub(crate) fn DistanceField_ComputeWithExpansionCount(
        walls: *const u64,
        roots: *const u64,
        visited: *mut u64,
        distances: *mut u16,
        max_distance: u16,
        out_expansions: *mut u16,
    ) -> i32;
    /// Exact C-ABI `DistanceField_ComputeToWithExpansionCount` host operation; pointer buffers obey the module safety contract.
    #[link_name = "bc_DistanceField_ComputeToWithExpansionCount"]
    pub(crate) fn DistanceField_ComputeToWithExpansionCount(
        walls: *const u64,
        roots: *const u64,
        destinations: *const u64,
        visited: *mut u64,
        distances: *mut u16,
        max_distance: u16,
        out_expansions: *mut u16,
    ) -> i32;
}

/// Exact C-ABI `game_settings` host operation; pointer buffers obey the module safety contract.
#[cfg(test)]
pub(crate) unsafe fn game_settings(output: *mut crate::Constants) -> i32 {
    crate::fake::call("game_settings", &[output as usize]) as i32
}
/// Exact C-ABI `item_info` host operation; pointer buffers obey the module safety contract.
#[cfg(test)]
pub(crate) unsafe fn item_info(kind: u32, output: *mut GameItemInfo) -> i32 {
    crate::fake::call("item_info", &[kind as usize, output as usize]) as i32
}
/// Exact C-ABI `consumable_info` host operation; pointer buffers obey the module safety contract.
#[cfg(test)]
pub(crate) unsafe fn consumable_info(kind: u32, output: *mut crate::shop::ConsumableInfo) -> i32 {
    crate::fake::call("consumable_info", &[kind as usize, output as usize]) as i32
}
/// Exact C-ABI `champion_buff_data` host operation; pointer buffers obey the module safety contract.
#[cfg(test)]
pub(crate) unsafe fn champion_buff_data(
    champion: u32,
    buffs: *mut BuffData,
    capacity: u32,
    out_count: *mut u32,
) -> i32 {
    crate::fake::call(
        "champion_buff_data",
        &[
            champion as usize,
            buffs as usize,
            capacity as usize,
            out_count as usize,
        ],
    ) as i32
}
/// Exact C-ABI `champion_ability_damage` host operation; pointer buffers obey the module safety contract.
#[cfg(test)]
pub(crate) unsafe fn champion_ability_damage(
    champion: u32,
    slot: u32,
    out_damage: *mut f32,
) -> i32 {
    crate::fake::call(
        "champion_ability_damage",
        &[champion as usize, slot as usize, out_damage as usize],
    ) as i32
}
/// Exact C-ABI `champion_ability_damage_against` host operation; pointer buffers obey the module safety contract.
#[cfg(test)]
pub(crate) unsafe fn champion_ability_damage_against(
    champion: u32,
    slot: u32,
    target: u32,
    out_damage: *mut f32,
) -> i32 {
    crate::fake::call(
        "champion_ability_damage_against",
        &[
            champion as usize,
            slot as usize,
            target as usize,
            out_damage as usize,
        ],
    ) as i32
}
/// Exact C-ABI `champion_ability_heal` host operation; pointer buffers obey the module safety contract.
#[cfg(test)]
pub(crate) unsafe fn champion_ability_heal(champion: u32, slot: u32, out_heal: *mut f32) -> i32 {
    crate::fake::call(
        "champion_ability_heal",
        &[champion as usize, slot as usize, out_heal as usize],
    ) as i32
}
/// Exact C-ABI `champion_ability_heal_against` host operation; pointer buffers obey the module safety contract.
#[cfg(test)]
pub(crate) unsafe fn champion_ability_heal_against(
    champion: u32,
    slot: u32,
    target: u32,
    out_heal: *mut f32,
) -> i32 {
    crate::fake::call(
        "champion_ability_heal_against",
        &[
            champion as usize,
            slot as usize,
            target as usize,
            out_heal as usize,
        ],
    ) as i32
}
/// Exact C-ABI `champion_ability_shield` host operation; pointer buffers obey the module safety contract.
#[cfg(test)]
pub(crate) unsafe fn champion_ability_shield(
    champion: u32,
    slot: u32,
    out_shield: *mut f32,
) -> i32 {
    crate::fake::call(
        "champion_ability_shield",
        &[champion as usize, slot as usize, out_shield as usize],
    ) as i32
}
/// Exact C-ABI `champion_ability_shield_against` host operation; pointer buffers obey the module safety contract.
#[cfg(test)]
pub(crate) unsafe fn champion_ability_shield_against(
    champion: u32,
    slot: u32,
    target: u32,
    out_shield: *mut f32,
) -> i32 {
    crate::fake::call(
        "champion_ability_shield_against",
        &[
            champion as usize,
            slot as usize,
            target as usize,
            out_shield as usize,
        ],
    ) as i32
}
/// Return diagnostic byte length and optionally copy diagnostic bytes into the bounded buffer.
#[cfg(test)]
pub(crate) unsafe fn bc_get_last_error(buffer: *mut u8, capacity: u32) -> i32 {
    crate::fake::call("bc_get_last_error", &[buffer as usize, capacity as usize]) as i32
}
/// Return self champion ID directly, or -1 in draft; not an engine status code.
#[cfg(test)]
pub(crate) unsafe fn my_champion() -> i32 {
    crate::fake::call("my_champion", &[]) as i32
}
/// Exact C-ABI `champion_is_alive` host operation; pointer buffers obey the module safety contract.
#[cfg(test)]
pub(crate) unsafe fn champion_is_alive(champion: u32, out_alive: *mut i32) -> i32 {
    crate::fake::call(
        "champion_is_alive",
        &[champion as usize, out_alive as usize],
    ) as i32
}
/// Exact C-ABI `num_allies` host operation; pointer buffers obey the module safety contract.
#[cfg(test)]
pub(crate) unsafe fn num_allies(out_count: *mut u32) -> i32 {
    crate::fake::call("num_allies", &[out_count as usize]) as i32
}
/// Exact C-ABI `num_enemies` host operation; pointer buffers obey the module safety contract.
#[cfg(test)]
pub(crate) unsafe fn num_enemies(out_count: *mut u32) -> i32 {
    crate::fake::call("num_enemies", &[out_count as usize]) as i32
}
/// Exact C-ABI `allies` host operation; pointer buffers obey the module safety contract.
#[cfg(test)]
pub(crate) unsafe fn allies(champions: *mut u32, capacity: u32, out_count: *mut u32) -> i32 {
    crate::fake::call(
        "allies",
        &[champions as usize, capacity as usize, out_count as usize],
    ) as i32
}
/// Exact C-ABI `enemies` host operation; pointer buffers obey the module safety contract.
#[cfg(test)]
pub(crate) unsafe fn enemies(champions: *mut u32, capacity: u32, out_count: *mut u32) -> i32 {
    crate::fake::call(
        "enemies",
        &[champions as usize, capacity as usize, out_count as usize],
    ) as i32
}
/// Exact C-ABI `ally_occupancy` host operation; pointer buffers obey the module safety contract.
#[cfg(test)]
pub(crate) unsafe fn ally_occupancy(out_board: *mut u64) -> i32 {
    crate::fake::call("board_ally_occupancy", &[out_board as usize]) as i32
}
/// Exact C-ABI `enemy_occupancy` host operation; pointer buffers obey the module safety contract.
#[cfg(test)]
pub(crate) unsafe fn enemy_occupancy(out_board: *mut u64) -> i32 {
    crate::fake::call("board_enemy_occupancy", &[out_board as usize]) as i32
}
/// Exact C-ABI `camp_occupancy` host operation; pointer buffers obey the module safety contract.
#[cfg(test)]
pub(crate) unsafe fn camp_occupancy(out_board: *mut u64) -> i32 {
    crate::fake::call("board_camp_occupancy", &[out_board as usize]) as i32
}
/// Exact C-ABI `ally_ward_occupancy` host operation; pointer buffers obey the module safety contract.
#[cfg(test)]
pub(crate) unsafe fn ally_ward_occupancy(out_board: *mut u64) -> i32 {
    crate::fake::call("board_ally_ward_occupancy", &[out_board as usize]) as i32
}
/// Exact C-ABI `enemy_ward_occupancy` host operation; pointer buffers obey the module safety contract.
#[cfg(test)]
pub(crate) unsafe fn enemy_ward_occupancy(out_board: *mut u64) -> i32 {
    crate::fake::call("board_enemy_ward_occupancy", &[out_board as usize]) as i32
}
/// Exact C-ABI `ward_at` host operation; pointer buffers obey the module safety contract.
#[cfg(test)]
pub(crate) unsafe fn ward_at(x: i32, y: i32, out_handle: *mut u32) -> i32 {
    crate::fake::call("ward_at", &[x as usize, y as usize, out_handle as usize]) as i32
}
/// Exact C-ABI `ally_ward_at` host operation; pointer buffers obey the module safety contract.
#[cfg(test)]
pub(crate) unsafe fn ally_ward_at(x: i32, y: i32, out_handle: *mut u32) -> i32 {
    crate::fake::call(
        "ally_ward_at",
        &[x as usize, y as usize, out_handle as usize],
    ) as i32
}
/// Exact C-ABI `enemy_ward_at` host operation; pointer buffers obey the module safety contract.
#[cfg(test)]
pub(crate) unsafe fn enemy_ward_at(x: i32, y: i32, out_handle: *mut u32) -> i32 {
    crate::fake::call(
        "enemy_ward_at",
        &[x as usize, y as usize, out_handle as usize],
    ) as i32
}
/// Exact C-ABI `ward` host operation; pointer buffers obey the module safety contract.
#[cfg(test)]
pub(crate) unsafe fn ward(handle: u32, out_info: *mut WardInfo) -> i32 {
    crate::fake::call("ward", &[handle as usize, out_info as usize]) as i32
}
/// Exact C-ABI `teleport_occupancy` host operation; pointer buffers obey the module safety contract.
#[cfg(test)]
pub(crate) unsafe fn teleport_occupancy(out_board: *mut u64) -> i32 {
    crate::fake::call("board_teleport_occupancy", &[out_board as usize]) as i32
}
/// Exact C-ABI `has_teleport` host operation; pointer buffers obey the module safety contract.
#[cfg(test)]
pub(crate) unsafe fn has_teleport(x: i32, y: i32, out_present: *mut i32) -> i32 {
    crate::fake::call(
        "is_occupied_by_teleport",
        &[x as usize, y as usize, out_present as usize],
    ) as i32
}
/// Exact C-ABI `teleports` host operation; pointer buffers obey the module safety contract.
#[cfg(test)]
pub(crate) unsafe fn teleports(
    teleports: *mut TeleportInfo,
    capacity: u32,
    out_count: *mut u32,
) -> i32 {
    crate::fake::call(
        "teleports",
        &[teleports as usize, capacity as usize, out_count as usize],
    ) as i32
}
/// Exact C-ABI `ward_vision` host operation; pointer buffers obey the module safety contract.
#[cfg(test)]
pub(crate) unsafe fn ward_vision(x: i32, y: i32, out_board: *mut u64) -> i32 {
    crate::fake::call(
        "board_ward_vision_mask",
        &[x as usize, y as usize, out_board as usize],
    ) as i32
}
/// Exact C-ABI `ally_altar_lock_occupancy` host operation; pointer buffers obey the module safety contract.
#[cfg(test)]
pub(crate) unsafe fn ally_altar_lock_occupancy(out_board: *mut u64) -> i32 {
    crate::fake::call("board_ally_altar_lock_occupancy", &[out_board as usize]) as i32
}
/// Exact C-ABI `enemy_altar_lock_occupancy` host operation; pointer buffers obey the module safety contract.
#[cfg(test)]
pub(crate) unsafe fn enemy_altar_lock_occupancy(out_board: *mut u64) -> i32 {
    crate::fake::call("board_enemy_altar_lock_occupancy", &[out_board as usize]) as i32
}
/// Exact C-ABI `altar_lock` host operation; pointer buffers obey the module safety contract.
#[cfg(test)]
pub(crate) unsafe fn altar_lock(x: i32, y: i32, team: i32, out_info: *mut AltarLockInfo) -> i32 {
    crate::fake::call(
        "altar_lock",
        &[x as usize, y as usize, team as usize, out_info as usize],
    ) as i32
}
/// Exact C-ABI `altar_locks` host operation; pointer buffers obey the module safety contract.
#[cfg(test)]
pub(crate) unsafe fn altar_locks(
    locks: *mut AltarLockInfo,
    capacity: u32,
    out_count: *mut u32,
) -> i32 {
    crate::fake::call(
        "altar_locks",
        &[locks as usize, capacity as usize, out_count as usize],
    ) as i32
}
/// Exact C-ABI `attack` host operation; pointer buffers obey the module safety contract.
#[cfg(test)]
pub(crate) unsafe fn attack(target_kind: u32, target_handle: u32) -> i32 {
    crate::fake::call("attack", &[target_kind as usize, target_handle as usize]) as i32
}
/// Exact C-ABI `cast` host operation; pointer buffers obey the module safety contract.
#[cfg(test)]
pub(crate) unsafe fn cast(slot: u32, target_kind: u32, target_handle: u32, x: i32, y: i32, out_successful: *mut i32) -> i32 {
    crate::fake::call(
        "cast",
        &[
            slot as usize,
            target_kind as usize,
            target_handle as usize,
            x as usize,
            y as usize,
            out_successful as usize,
        ],
    ) as i32
}
/// Exact C-ABI `use_consumable` host operation; pointer buffers obey the module safety contract.
#[cfg(test)]
pub(crate) unsafe fn use_consumable(kind: u32, x: i32, y: i32, out_successful: *mut i32) -> i32 {
    crate::fake::call("use_consumable", &[kind as usize, x as usize, y as usize, out_successful as usize]) as i32
}
/// Exact C-ABI `buy_consumable` host operation; pointer buffers obey the module safety contract.
#[cfg(test)]
pub(crate) unsafe fn buy_consumable(kind: u32) -> i32 {
    crate::fake::call("buy_consumable", &[kind as usize]) as i32
}
/// Exact C-ABI `buy_item` host operation; pointer buffers obey the module safety contract.
#[cfg(test)]
pub(crate) unsafe fn buy_item(kind: u32, count: u32) -> i32 {
    crate::fake::call("buy_item", &[kind as usize, count as usize]) as i32
}
/// Exact C-ABI `sell` host operation; pointer buffers obey the module safety contract.
#[cfg(test)]
pub(crate) unsafe fn sell(kind: u32) -> i32 {
    crate::fake::call("sell", &[kind as usize]) as i32
}
/// Exact C-ABI `recall` host operation; pointer buffers obey the module safety contract.
#[cfg(test)]
pub(crate) unsafe fn recall() -> i32 {
    crate::fake::call("recall", &[]) as i32
}
/// Exact C-ABI `cancel_channel` host operation; pointer buffers obey the module safety contract.
#[cfg(test)]
pub(crate) unsafe fn cancel_channel() -> i32 {
    crate::fake::call("cancel_channel", &[]) as i32
}
/// Exact C-ABI `submit_gems` host operation; pointer buffers obey the module safety contract.
#[cfg(test)]
pub(crate) unsafe fn submit_gems(x: i32, y: i32) -> i32 {
    crate::fake::call("submit_gems", &[x as usize, y as usize]) as i32
}
/// Exact C-ABI `place_ward` host operation; pointer buffers obey the module safety contract.
#[cfg(test)]
pub(crate) unsafe fn place_ward(x: i32, y: i32, out_successful: *mut i32) -> i32 {
    crate::fake::call("place_ward", &[x as usize, y as usize, out_successful as usize]) as i32
}
/// Exact C-ABI `move_to` host operation; pointer buffers obey the module safety contract.
#[cfg(test)]
pub(crate) unsafe fn move_to(x: i32, y: i32, out_successful: *mut i32) -> i32 {
    crate::fake::call("move_to", &[x as usize, y as usize, out_successful as usize]) as i32
}
/// Exact C-ABI `set_priority` host operation; pointer buffers obey the module safety contract.
#[cfg(test)]
pub(crate) unsafe fn set_priority(priority: i32) -> i32 {
    crate::fake::call("set_priority", &[priority as usize]) as i32
}
/// Exact C-ABI `bc_champion_priority` host operation; pointer buffers obey the module safety contract.
#[cfg(test)]
pub(crate) unsafe fn bc_champion_priority(
    champion: u32,
    previous_turn: i32,
    out_priority: *mut i32,
) -> i32 {
    crate::fake::call(
        "bc_champion_priority",
        &[
            champion as usize,
            previous_turn as usize,
            out_priority as usize,
        ],
    ) as i32
}
/// Exact C-ABI `bc_submitted_gems` host operation; pointer buffers obey the module safety contract.
#[cfg(test)]
pub(crate) unsafe fn bc_submitted_gems(team: u32, out_gems: *mut u32) -> i32 {
    crate::fake::call("bc_submitted_gems", &[team as usize, out_gems as usize]) as i32
}
/// Exact C-ABI `bc_round_number` host operation; pointer buffers obey the module safety contract.
#[cfg(test)]
pub(crate) unsafe fn bc_round_number(out_round: *mut u32) -> i32 {
    crate::fake::call("bc_round_number", &[out_round as usize]) as i32
}
/// Exact C-ABI `resign` host operation; pointer buffers obey the module safety contract.
#[cfg(test)]
pub(crate) unsafe fn resign() -> i32 {
    crate::fake::call("resign", &[]) as i32
}
/// Exact C-ABI `shared_memory_size` host operation; pointer buffers obey the module safety contract.
#[cfg(test)]
pub(crate) unsafe fn shared_memory_size(out_size: *mut u32) -> i32 {
    crate::fake::call("shared_memory_size", &[out_size as usize]) as i32
}
/// Exact C-ABI `read_shared_memory` host operation; pointer buffers obey the module safety contract.
#[cfg(test)]
pub(crate) unsafe fn read_shared_memory(
    offset: i32,
    buffer: *mut core::ffi::c_void,
    byte_count: u32,
) -> i32 {
    crate::fake::call(
        "read_shared_memory",
        &[offset as usize, buffer as usize, byte_count as usize],
    ) as i32
}
/// Exact C-ABI `write_shared_memory` host operation; pointer buffers obey the module safety contract.
#[cfg(test)]
pub(crate) unsafe fn write_shared_memory(
    offset: i32,
    buffer: *const core::ffi::c_void,
    byte_count: u32,
) -> i32 {
    crate::fake::call(
        "write_shared_memory",
        &[offset as usize, buffer as usize, byte_count as usize],
    ) as i32
}
/// Exact C-ABI `champion` host operation; pointer buffers obey the module safety contract.
#[cfg(test)]
pub(crate) unsafe fn champion(champion: u32, out_info: *mut ChampionInfo) -> i32 {
    crate::fake::call("champion", &[champion as usize, out_info as usize]) as i32
}
/// Exact C-ABI `champion_design` host operation; pointer buffers obey the module safety contract.
#[cfg(test)]
pub(crate) unsafe fn champion_design(kind: i32, out_design: *mut ChampionDesign) -> i32 {
    crate::fake::call("champion_design", &[kind as usize, out_design as usize]) as i32
}
/// Exact C-ABI `champion_restrictions` host operation; pointer buffers obey the module safety contract.
#[cfg(test)]
pub(crate) unsafe fn champion_restrictions(champion: u32, out_restrictions: *mut u8) -> i32 {
    crate::fake::call(
        "champion_restrictions",
        &[champion as usize, out_restrictions as usize],
    ) as i32
}
/// Exact C-ABI `champion_ability_props` host operation; pointer buffers obey the module safety contract.
#[cfg(test)]
pub(crate) unsafe fn champion_ability_props(
    champion: u32,
    slot: u32,
    out_properties: *mut AbilityProps,
) -> i32 {
    crate::fake::call(
        "champion_ability_props",
        &[champion as usize, slot as usize, out_properties as usize],
    ) as i32
}
/// Exact C-ABI `champion_cooldown` host operation; pointer buffers obey the module safety contract.
#[cfg(test)]
pub(crate) unsafe fn champion_cooldown(champion: u32, slot: u32, out_cooldown: *mut i32) -> i32 {
    crate::fake::call(
        "champion_cooldown",
        &[champion as usize, slot as usize, out_cooldown as usize],
    ) as i32
}
/// Exact C-ABI `champion_items` host operation; pointer buffers obey the module safety contract.
#[cfg(test)]
pub(crate) unsafe fn champion_items(champion: u32, out_items: *mut u32) -> i32 {
    crate::fake::call("champion_items", &[champion as usize, out_items as usize]) as i32
}
/// Exact C-ABI `champion_consumables` host operation; pointer buffers obey the module safety contract.
#[cfg(test)]
pub(crate) unsafe fn champion_consumables(champion: u32, out_counts: *mut u32) -> i32 {
    crate::fake::call(
        "champion_consumables",
        &[champion as usize, out_counts as usize],
    ) as i32
}
/// Exact C-ABI `champion_gems` host operation; pointer buffers obey the module safety contract.
#[cfg(test)]
pub(crate) unsafe fn champion_gems(champion: u32, out_gems: *mut u32) -> i32 {
    crate::fake::call("champion_gems", &[champion as usize, out_gems as usize]) as i32
}
/// Exact C-ABI `champion_gold` host operation; pointer buffers obey the module safety contract.
#[cfg(test)]
pub(crate) unsafe fn champion_gold(champion: u32, out_gold: *mut u32) -> i32 {
    crate::fake::call("champion_gold", &[champion as usize, out_gold as usize]) as i32
}
/// Exact C-ABI `bc_champion_vision` host operation; pointer buffers obey the module safety contract.
#[cfg(test)]
pub(crate) unsafe fn bc_champion_vision(champion: u32, out_board: *mut u64) -> i32 {
    crate::fake::call(
        "bc_champion_vision",
        &[champion as usize, out_board as usize],
    ) as i32
}
/// Exact C-ABI `hypothetical_vision` host operation; pointer buffers obey the module safety contract.
#[cfg(test)]
pub(crate) unsafe fn hypothetical_vision(
    x: i32,
    y: i32,
    vision_type: i32,
    out_board: *mut u64,
) -> i32 {
    crate::fake::call(
        "board_vision_mask",
        &[
            x as usize,
            y as usize,
            vision_type as usize,
            out_board as usize,
        ],
    ) as i32
}
/// Exact C-ABI `champion_wards` host operation; pointer buffers obey the module safety contract.
#[cfg(test)]
pub(crate) unsafe fn champion_wards(champion: u32, out_resources: *mut WardResources) -> i32 {
    crate::fake::call(
        "champion_wards",
        &[champion as usize, out_resources as usize],
    ) as i32
}
/// Exact C-ABI `champion_ward_positions` host operation; pointer buffers obey the module safety contract.
#[cfg(test)]
pub(crate) unsafe fn champion_ward_positions(
    champion: u32,
    positions: *mut Vec2,
    capacity: u32,
    out_count: *mut u32,
) -> i32 {
    crate::fake::call(
        "bc_champion_ward_positions",
        &[
            champion as usize,
            positions as usize,
            capacity as usize,
            out_count as usize,
        ],
    ) as i32
}
/// Exact C-ABI `champion_channel_time` host operation; pointer buffers obey the module safety contract.
#[cfg(test)]
pub(crate) unsafe fn champion_channel_time(champion: u32, out_time: *mut i32) -> i32 {
    crate::fake::call(
        "bc_champion_channel_time",
        &[champion as usize, out_time as usize],
    ) as i32
}
/// Exact C-ABI `champion_can_shop` host operation; pointer buffers obey the module safety contract.
#[cfg(test)]
pub(crate) unsafe fn champion_can_shop(
    champion: u32,
    consumable: i32,
    out_allowed: *mut i32,
) -> i32 {
    crate::fake::call(
        "champion_can_shop",
        &[champion as usize, consumable as usize, out_allowed as usize],
    ) as i32
}
/// Exact C-ABI `champion_can_sell` host operation; pointer buffers obey the module safety contract.
#[cfg(test)]
pub(crate) unsafe fn champion_can_sell(champion: u32, out_allowed: *mut i32) -> i32 {
    crate::fake::call(
        "champion_can_sell",
        &[champion as usize, out_allowed as usize],
    ) as i32
}
/// Exact C-ABI `wards` host operation; pointer buffers obey the module safety contract.
#[cfg(test)]
pub(crate) unsafe fn wards(wards: *mut WardInfo, capacity: u32, out_count: *mut u32) -> i32 {
    crate::fake::call(
        "wards",
        &[wards as usize, capacity as usize, out_count as usize],
    ) as i32
}
/// Exact C-ABI `bc_metadata` host operation; pointer buffers obey the module safety contract.
#[cfg(test)]
pub(crate) unsafe fn bc_metadata(out_metadata: *mut Metadata) -> i32 {
    crate::fake::call("bc_metadata", &[out_metadata as usize]) as i32
}
/// Return the absolute observing team ID directly, not an engine status code.
#[cfg(test)]
pub(crate) unsafe fn my_team() -> i32 {
    crate::fake::call("my_team", &[]) as i32
}
/// Exact C-ABI `bc_map_mask` host operation; pointer buffers obey the module safety contract.
#[cfg(test)]
pub(crate) unsafe fn bc_map_mask(feature: i32, out_board: *mut u64) -> i32 {
    crate::fake::call("bc_map_mask", &[feature as usize, out_board as usize]) as i32
}
/// Exact C-ABI `bc_map_symmetry` host operation; pointer buffers obey the module safety contract.
#[cfg(test)]
pub(crate) unsafe fn bc_map_symmetry(out_symmetry: *mut i32) -> i32 {
    crate::fake::call("bc_map_symmetry", &[out_symmetry as usize]) as i32
}
/// Exact C-ABI `map_camp_group_num_camps` host operation; pointer buffers obey the module safety contract.
#[cfg(test)]
pub(crate) unsafe fn map_camp_group_num_camps(group: u32, out_count: *mut u32) -> i32 {
    crate::fake::call(
        "map_camp_group_num_camps",
        &[group as usize, out_count as usize],
    ) as i32
}
/// Exact C-ABI `map_camp_group_camps` host operation; pointer buffers obey the module safety contract.
#[cfg(test)]
pub(crate) unsafe fn map_camp_group_camps(
    group: u32,
    camps: *mut u32,
    capacity: u32,
    out_count: *mut u32,
) -> i32 {
    crate::fake::call(
        "map_camp_group_camps",
        &[
            group as usize,
            camps as usize,
            capacity as usize,
            out_count as usize,
        ],
    ) as i32
}
/// Exact C-ABI `map_camp_group` host operation; pointer buffers obey the module safety contract.
#[cfg(test)]
pub(crate) unsafe fn map_camp_group(group: u32, out_info: *mut CampGroupInfo) -> i32 {
    crate::fake::call("map_camp_group", &[group as usize, out_info as usize]) as i32
}
/// Exact C-ABI `map_camp_info` host operation; pointer buffers obey the module safety contract.
#[cfg(test)]
pub(crate) unsafe fn map_camp_info(camp: u32, out_info: *mut CampInfo) -> i32 {
    crate::fake::call("map_camp_info", &[camp as usize, out_info as usize]) as i32
}
/// Exact C-ABI `map_camp_layout` host operation; pointer buffers obey the module safety contract.
#[cfg(test)]
pub(crate) unsafe fn map_camp_layout(out_words: *mut u32, capacity: u32, out_count: *mut u32) -> i32 {
    crate::fake::call("map_camp_layout", &[out_words as usize, capacity as usize, out_count as usize]) as i32
}
/// Exact C-ABI `map_camp_group_of` host operation; pointer buffers obey the module safety contract.
#[cfg(test)]
pub(crate) unsafe fn map_camp_group_of(camp: u32, out_group: *mut u32) -> i32 {
    crate::fake::call("map_camp_group_of", &[camp as usize, out_group as usize]) as i32
}
/// Exact C-ABI `map_camp_at` host operation; pointer buffers obey the module safety contract.
#[cfg(test)]
pub(crate) unsafe fn map_camp_at(x: i32, y: i32, out_camp: *mut u32) -> i32 {
    crate::fake::call("map_camp_at", &[x as usize, y as usize, out_camp as usize]) as i32
}
/// Exact C-ABI `camp_group_cooldown` host operation; pointer buffers obey the module safety contract.
#[cfg(test)]
pub(crate) unsafe fn camp_group_cooldown(group: u32, out_cooldown: *mut i32) -> i32 {
    crate::fake::call(
        "camp_group_cooldown",
        &[group as usize, out_cooldown as usize],
    ) as i32
}
/// Exact C-ABI `camp_group_state` host operation; pointer buffers obey the module safety contract.
#[cfg(test)]
pub(crate) unsafe fn camp_group_state(group: u32, out_state: *mut u32) -> i32 {
    crate::fake::call("camp_group_state", &[group as usize, out_state as usize]) as i32
}
/// Exact C-ABI `camp_buffs` host operation; pointer buffers obey the module safety contract.
#[cfg(test)]
pub(crate) unsafe fn camp_buffs(
    camp: u32,
    buffs: *mut CampBuffData,
    capacity: u32,
    out_count: *mut u32,
) -> i32 {
    crate::fake::call(
        "camp_buffs",
        &[
            camp as usize,
            buffs as usize,
            capacity as usize,
            out_count as usize,
        ],
    ) as i32
}
/// Exact C-ABI `bc_camp_group_max_respawn` host operation; pointer buffers obey the module safety contract.
#[cfg(test)]
pub(crate) unsafe fn bc_camp_group_max_respawn(group: u32, out_timer: *mut i32) -> i32 {
    crate::fake::call(
        "bc_camp_group_max_respawn",
        &[group as usize, out_timer as usize],
    ) as i32
}
/// Exact C-ABI `camp` host operation; pointer buffers obey the module safety contract.
#[cfg(test)]
pub(crate) unsafe fn camp(camp: u32, out_state: *mut CampState) -> i32 {
    crate::fake::call("camp", &[camp as usize, out_state as usize]) as i32
}
/// Exact C-ABI `camp_design` host operation; pointer buffers obey the module safety contract.
#[cfg(test)]
pub(crate) unsafe fn camp_design(kind: i32, out_design: *mut CampDesign) -> i32 {
    crate::fake::call("camp_design", &[kind as usize, out_design as usize]) as i32
}
/// Exact C-ABI `camp_at` host operation; pointer buffers obey the module safety contract.
#[cfg(test)]
pub(crate) unsafe fn camp_at(x: i32, y: i32, out_handle: *mut u32) -> i32 {
    crate::fake::call("camp_at", &[x as usize, y as usize, out_handle as usize]) as i32
}
/// Exact C-ABI `champion_at` host operation; pointer buffers obey the module safety contract.
#[cfg(test)]
pub(crate) unsafe fn champion_at(x: i32, y: i32, out_handle: *mut u32) -> i32 {
    crate::fake::call(
        "champion_at",
        &[x as usize, y as usize, out_handle as usize],
    ) as i32
}
/// Exact C-ABI `has_camp` host operation; pointer buffers obey the module safety contract.
#[cfg(test)]
pub(crate) unsafe fn has_camp(x: i32, y: i32, out_present: *mut i32) -> i32 {
    crate::fake::call("has_camp", &[x as usize, y as usize, out_present as usize]) as i32
}
/// Exact C-ABI `has_champion` host operation; pointer buffers obey the module safety contract.
#[cfg(test)]
pub(crate) unsafe fn has_champion(x: i32, y: i32, out_present: *mut i32) -> i32 {
    crate::fake::call(
        "has_champion",
        &[x as usize, y as usize, out_present as usize],
    ) as i32
}
/// Exact C-ABI `vision` host operation; pointer buffers obey the module safety contract.
#[cfg(test)]
pub(crate) unsafe fn vision(out_board: *mut u64) -> i32 {
    crate::fake::call("board_vision", &[out_board as usize]) as i32
}
/// Exact C-ABI `spawn_region` host operation; pointer buffers obey the module safety contract.
#[cfg(test)]
pub(crate) unsafe fn spawn_region(team: i32, out_region: *mut Aabb) -> i32 {
    crate::fake::call("spawn_region", &[team as usize, out_region as usize]) as i32
}
/// Exact C-ABI `draft_num_rounds` host operation; pointer buffers obey the module safety contract.
#[cfg(test)]
pub(crate) unsafe fn draft_num_rounds(out_rounds: *mut u8) -> i32 {
    crate::fake::call("draft_num_rounds", &[out_rounds as usize]) as i32
}
/// Exact C-ABI `draft_get` host operation; pointer buffers obey the module safety contract.
#[cfg(test)]
pub(crate) unsafe fn draft_get(team: i32, round: u32, out_pick: *mut i32) -> i32 {
    crate::fake::call(
        "draft_get",
        &[team as usize, round as usize, out_pick as usize],
    ) as i32
}
/// Exact C-ABI `draft_champion` host operation; pointer buffers obey the module safety contract.
#[cfg(test)]
pub(crate) unsafe fn draft_champion(pick: i32) -> i32 {
    crate::fake::call("draft_champion", &[pick as usize]) as i32
}
/// Exact C-ABI `bc_submit_draft_memory` host operation; pointer buffers obey the module safety contract.
#[cfg(test)]
pub(crate) unsafe fn bc_submit_draft_memory(
    buffer: *const core::ffi::c_void,
    byte_count: usize,
) -> i32 {
    crate::fake::call("bc_submit_draft_memory", &[buffer as usize, byte_count]) as i32
}
/// Exact C-ABI `draft_memory_size` host operation; pointer buffers obey the module safety contract.
#[cfg(test)]
pub(crate) unsafe fn draft_memory_size(out_size: *mut u32) -> i32 {
    crate::fake::call("draft_memory_size", &[out_size as usize]) as i32
}
/// Exact C-ABI `read_draft_memory` host operation; pointer buffers obey the module safety contract.
#[cfg(test)]
pub(crate) unsafe fn read_draft_memory(
    offset: i32,
    buffer: *mut core::ffi::c_void,
    byte_count: u32,
) -> i32 {
    crate::fake::call(
        "read_draft_memory",
        &[offset as usize, buffer as usize, byte_count as usize],
    ) as i32
}
/// Exact C-ABI `debug_dot` host operation; pointer buffers obey the module safety contract.
#[cfg(test)]
pub(crate) unsafe fn debug_dot(x: i32, y: i32, color: u32) -> i32 {
    crate::fake::call("debug_dot", &[x as usize, y as usize, color as usize]) as i32
}
/// Exact C-ABI `debug_line` host operation; pointer buffers obey the module safety contract.
#[cfg(test)]
pub(crate) unsafe fn debug_line(
    start_x: i32,
    start_y: i32,
    end_x: i32,
    end_y: i32,
    color: u32,
) -> i32 {
    crate::fake::call(
        "debug_line",
        &[
            start_x as usize,
            start_y as usize,
            end_x as usize,
            end_y as usize,
            color as usize,
        ],
    ) as i32
}
/// Yield the guest to finish its turn; resumes after this import when scheduled again.
#[cfg(test)]
pub(crate) unsafe fn r#yield() {
    let _ = crate::fake::call("yield", &[]);
}
/// Return remaining fuel directly as an i64, not an engine status code.
#[cfg(test)]
pub(crate) unsafe fn fuel_remaining() -> i64 {
    crate::fake::call("fuel_remaining", &[])
}
/// Exact C-ABI `Bitboard_One` host operation; pointer buffers obey the module safety contract.
#[cfg(test)]
pub(crate) unsafe fn Bitboard_One(board: *mut u64) -> i32 {
    crate::fake::call("bc_Bitboard_One", &[board as usize]) as i32
}
/// Exact C-ABI `Bitboard_ImportRawBuffer` host operation; pointer buffers obey the module safety contract.
#[cfg(test)]
pub(crate) unsafe fn Bitboard_ImportRawBuffer(
    board: *mut u64,
    buffer: *const u8,
    byte_count: usize,
) -> i32 {
    crate::fake::call(
        "bc_Bitboard_ImportRawBuffer",
        &[board as usize, buffer as usize, byte_count],
    ) as i32
}
/// Exact C-ABI `Bitboard_ImportLinearBuffer` host operation; pointer buffers obey the module safety contract.
#[cfg(test)]
pub(crate) unsafe fn Bitboard_ImportLinearBuffer(
    board: *mut u64,
    buffer: *const u8,
    byte_count: usize,
) -> i32 {
    crate::fake::call(
        "bc_Bitboard_ImportLinearBuffer",
        &[board as usize, buffer as usize, byte_count],
    ) as i32
}
/// Exact C-ABI `Bitboard_ExportRawBuffer` host operation; pointer buffers obey the module safety contract.
#[cfg(test)]
pub(crate) unsafe fn Bitboard_ExportRawBuffer(
    board: *const u64,
    buffer: *mut u8,
    byte_count: usize,
) -> i32 {
    crate::fake::call(
        "bc_Bitboard_ExportRawBuffer",
        &[board as usize, buffer as usize, byte_count],
    ) as i32
}
/// Exact C-ABI `Bitboard_ExportLinearBuffer` host operation; pointer buffers obey the module safety contract.
#[cfg(test)]
pub(crate) unsafe fn Bitboard_ExportLinearBuffer(
    board: *const u64,
    buffer: *mut u8,
    byte_count: usize,
) -> i32 {
    crate::fake::call(
        "bc_Bitboard_ExportLinearBuffer",
        &[board as usize, buffer as usize, byte_count],
    ) as i32
}
/// Exact C-ABI `Bitboard_Print` host operation; pointer buffers obey the module safety contract.
#[cfg(test)]
pub(crate) unsafe fn Bitboard_Print(board: *const u64) -> i32 {
    crate::fake::call("bc_Bitboard_Print", &[board as usize]) as i32
}
/// Exact C-ABI `Bitboard_Test` host operation; pointer buffers obey the module safety contract.
#[cfg(test)]
pub(crate) unsafe fn Bitboard_Test(board: *const u64, x: i32, y: i32, out_value: *mut i32) -> i32 {
    crate::fake::call(
        "bc_Bitboard_Test",
        &[board as usize, x as usize, y as usize, out_value as usize],
    ) as i32
}
/// Exact C-ABI `Bitboard_Set` host operation; pointer buffers obey the module safety contract.
#[cfg(test)]
pub(crate) unsafe fn Bitboard_Set(board: *mut u64, x: i32, y: i32, value: i32) -> i32 {
    crate::fake::call(
        "bc_Bitboard_Set",
        &[board as usize, x as usize, y as usize, value as usize],
    ) as i32
}
/// Exact C-ABI `Bitboard_Toggle` host operation; pointer buffers obey the module safety contract.
#[cfg(test)]
pub(crate) unsafe fn Bitboard_Toggle(board: *mut u64, x: i32, y: i32) -> i32 {
    crate::fake::call(
        "bc_Bitboard_Toggle",
        &[board as usize, x as usize, y as usize],
    ) as i32
}
/// Exact C-ABI `Bitboard_IsZero` host operation; pointer buffers obey the module safety contract.
#[cfg(test)]
pub(crate) unsafe fn Bitboard_IsZero(board: *const u64, out_empty: *mut i32) -> i32 {
    crate::fake::call("bc_Bitboard_IsZero", &[board as usize, out_empty as usize]) as i32
}
/// Exact C-ABI `Bitboard_Zero` host operation; pointer buffers obey the module safety contract.
#[cfg(test)]
pub(crate) unsafe fn Bitboard_Zero(board: *mut u64) -> i32 {
    crate::fake::call("bc_Bitboard_Zero", &[board as usize]) as i32
}
/// Exact C-ABI `Bitboard_NumSetBits` host operation; pointer buffers obey the module safety contract.
#[cfg(test)]
pub(crate) unsafe fn Bitboard_NumSetBits(board: *const u64, out_count: *mut u32) -> i32 {
    crate::fake::call(
        "bc_Bitboard_NumSetBits",
        &[board as usize, out_count as usize],
    ) as i32
}
/// Exact C-ABI `Bitboard_GetSetBits` host operation; pointer buffers obey the module safety contract.
#[cfg(test)]
pub(crate) unsafe fn Bitboard_GetSetBits(
    board: *const u64,
    positions: *mut core::ffi::c_void,
    capacity: usize,
    out_count: *mut u32,
) -> i32 {
    crate::fake::call(
        "bc_Bitboard_GetSetBits",
        &[
            board as usize,
            positions as usize,
            capacity,
            out_count as usize,
        ],
    ) as i32
}
/// Exact C-ABI `Bitboard_Not` host operation; pointer buffers obey the module safety contract.
#[cfg(test)]
pub(crate) unsafe fn Bitboard_Not(board: *const u64, out_board: *mut u64) -> i32 {
    crate::fake::call("bc_Bitboard_Not", &[board as usize, out_board as usize]) as i32
}
/// Exact C-ABI `Bitboard_Union` host operation; pointer buffers obey the module safety contract.
#[cfg(test)]
pub(crate) unsafe fn Bitboard_Union(
    left: *const u64,
    right: *const u64,
    out_board: *mut u64,
) -> i32 {
    crate::fake::call(
        "bc_Bitboard_Union",
        &[left as usize, right as usize, out_board as usize],
    ) as i32
}
/// Exact C-ABI `Bitboard_Intersect` host operation; pointer buffers obey the module safety contract.
#[cfg(test)]
pub(crate) unsafe fn Bitboard_Intersect(
    left: *const u64,
    right: *const u64,
    out_board: *mut u64,
) -> i32 {
    crate::fake::call(
        "bc_Bitboard_Intersect",
        &[left as usize, right as usize, out_board as usize],
    ) as i32
}
/// Exact C-ABI `Bitboard_Xor` host operation; pointer buffers obey the module safety contract.
#[cfg(test)]
pub(crate) unsafe fn Bitboard_Xor(left: *const u64, right: *const u64, out_board: *mut u64) -> i32 {
    crate::fake::call(
        "bc_Bitboard_Xor",
        &[left as usize, right as usize, out_board as usize],
    ) as i32
}
/// Exact C-ABI `Bitboard_IsEqual` host operation; pointer buffers obey the module safety contract.
#[cfg(test)]
pub(crate) unsafe fn Bitboard_IsEqual(
    left: *const u64,
    right: *const u64,
    out_equal: *mut i32,
) -> i32 {
    crate::fake::call(
        "bc_Bitboard_IsEqual",
        &[left as usize, right as usize, out_equal as usize],
    ) as i32
}
/// Exact C-ABI `Bitboard_ShiftRight` host operation; pointer buffers obey the module safety contract.
#[cfg(test)]
pub(crate) unsafe fn Bitboard_ShiftRight(
    board: *const u64,
    out_board: *mut u64,
    columns: i32,
) -> i32 {
    crate::fake::call(
        "bc_Bitboard_ShiftRight",
        &[board as usize, out_board as usize, columns as usize],
    ) as i32
}
/// Exact C-ABI `Bitboard_ShiftUp` host operation; pointer buffers obey the module safety contract.
#[cfg(test)]
pub(crate) unsafe fn Bitboard_ShiftUp(board: *const u64, out_board: *mut u64, rows: i32) -> i32 {
    crate::fake::call(
        "bc_Bitboard_ShiftUp",
        &[board as usize, out_board as usize, rows as usize],
    ) as i32
}
/// Exact C-ABI `Bitboard_Reflect` host operation; pointer buffers obey the module safety contract.
#[cfg(test)]
pub(crate) unsafe fn Bitboard_Reflect(
    board: *const u64,
    out_board: *mut u64,
    reflection: i32,
) -> i32 {
    crate::fake::call(
        "bc_Bitboard_Reflect",
        &[board as usize, out_board as usize, reflection as usize],
    ) as i32
}
/// Exact C-ABI `Bitboard_DrawCircle` host operation; pointer buffers obey the module safety contract.
#[cfg(test)]
pub(crate) unsafe fn Bitboard_DrawCircle(board: *mut u64, x: i32, y: i32, radius: f32) -> i32 {
    crate::fake::call(
        "bc_Bitboard_DrawCircle",
        &[
            board as usize,
            x as usize,
            y as usize,
            radius.to_bits() as usize,
        ],
    ) as i32
}
/// Exact C-ABI `Bitboard_DrawChebyshev` host operation; pointer buffers obey the module safety contract.
#[cfg(test)]
pub(crate) unsafe fn Bitboard_DrawChebyshev(board: *mut u64, x: i32, y: i32, radius: f32) -> i32 {
    crate::fake::call(
        "bc_Bitboard_DrawChebyshev",
        &[
            board as usize,
            x as usize,
            y as usize,
            radius.to_bits() as usize,
        ],
    ) as i32
}
/// Exact C-ABI `Bitboard_ReflectHorizontalPosition` host operation; pointer buffers obey the module safety contract.
#[cfg(test)]
pub(crate) unsafe fn Bitboard_ReflectHorizontalPosition(
    x: i32,
    y: i32,
    out_x: *mut i32,
    out_y: *mut i32,
) -> i32 {
    crate::fake::call(
        "bc_Bitboard_ReflectHorizontalPosition",
        &[x as usize, y as usize, out_x as usize, out_y as usize],
    ) as i32
}
/// Exact C-ABI `Bitboard_ReflectVerticalPosition` host operation; pointer buffers obey the module safety contract.
#[cfg(test)]
pub(crate) unsafe fn Bitboard_ReflectVerticalPosition(
    x: i32,
    y: i32,
    out_x: *mut i32,
    out_y: *mut i32,
) -> i32 {
    crate::fake::call(
        "bc_Bitboard_ReflectVerticalPosition",
        &[x as usize, y as usize, out_x as usize, out_y as usize],
    ) as i32
}
/// Exact C-ABI `Bitboard_Rotate180Position` host operation; pointer buffers obey the module safety contract.
#[cfg(test)]
pub(crate) unsafe fn Bitboard_Rotate180Position(
    x: i32,
    y: i32,
    out_x: *mut i32,
    out_y: *mut i32,
) -> i32 {
    crate::fake::call(
        "bc_Bitboard_Rotate180Position",
        &[x as usize, y as usize, out_x as usize, out_y as usize],
    ) as i32
}
/// Exact C-ABI `Bitboard_ReachableMask` host operation; pointer buffers obey the module safety contract.
#[cfg(test)]
pub(crate) unsafe fn Bitboard_ReachableMask(
    walls: *const u64,
    x: i32,
    y: i32,
    out_board: *mut u64,
    max_distance: u16,
) -> i32 {
    crate::fake::call(
        "bc_Bitboard_ReachableMask",
        &[
            walls as usize,
            x as usize,
            y as usize,
            out_board as usize,
            max_distance as usize,
        ],
    ) as i32
}
/// Exact C-ABI `Bitboard_ShadowCast` host operation; pointer buffers obey the module safety contract.
#[cfg(test)]
pub(crate) unsafe fn Bitboard_ShadowCast(
    walls: *const u64,
    x: i32,
    y: i32,
    out_board: *mut u64,
    max_distance: u16,
) -> i32 {
    crate::fake::call(
        "bc_Bitboard_ShadowCast",
        &[
            walls as usize,
            x as usize,
            y as usize,
            out_board as usize,
            max_distance as usize,
        ],
    ) as i32
}
/// Exact C-ABI `Bitboard_ReachableMaskWithScratch` host operation; pointer buffers obey the module safety contract.
#[cfg(test)]
pub(crate) unsafe fn Bitboard_ReachableMaskWithScratch(
    walls: *const u64,
    x: i32,
    y: i32,
    out_board: *mut u64,
    max_distance: u16,
    frontier: *mut u64,
    scratch: *mut u64,
) -> i32 {
    crate::fake::call(
        "bc_Bitboard_ReachableMaskWithScratch",
        &[
            walls as usize,
            x as usize,
            y as usize,
            out_board as usize,
            max_distance as usize,
            frontier as usize,
            scratch as usize,
        ],
    ) as i32
}
/// Exact C-ABI `DistanceField_ImportRawBuffer` host operation; pointer buffers obey the module safety contract.
#[cfg(test)]
pub(crate) unsafe fn DistanceField_ImportRawBuffer(
    field: *mut u16,
    buffer: *const u8,
    byte_count: usize,
) -> i32 {
    crate::fake::call(
        "bc_DistanceField_ImportRawBuffer",
        &[field as usize, buffer as usize, byte_count],
    ) as i32
}
/// Exact C-ABI `DistanceField_ImportLinearBuffer` host operation; pointer buffers obey the module safety contract.
#[cfg(test)]
pub(crate) unsafe fn DistanceField_ImportLinearBuffer(
    field: *mut u16,
    buffer: *const u8,
    byte_count: usize,
) -> i32 {
    crate::fake::call(
        "bc_DistanceField_ImportLinearBuffer",
        &[field as usize, buffer as usize, byte_count],
    ) as i32
}
/// Exact C-ABI `DistanceField_ExportRawBuffer` host operation; pointer buffers obey the module safety contract.
#[cfg(test)]
pub(crate) unsafe fn DistanceField_ExportRawBuffer(
    field: *const u16,
    buffer: *mut u8,
    byte_count: usize,
) -> i32 {
    crate::fake::call(
        "bc_DistanceField_ExportRawBuffer",
        &[field as usize, buffer as usize, byte_count],
    ) as i32
}
/// Exact C-ABI `DistanceField_ExportLinearBuffer` host operation; pointer buffers obey the module safety contract.
#[cfg(test)]
pub(crate) unsafe fn DistanceField_ExportLinearBuffer(
    field: *const u16,
    buffer: *mut u8,
    byte_count: usize,
) -> i32 {
    crate::fake::call(
        "bc_DistanceField_ExportLinearBuffer",
        &[field as usize, buffer as usize, byte_count],
    ) as i32
}
/// Exact C-ABI `DistanceField_Print` host operation; pointer buffers obey the module safety contract.
#[cfg(test)]
pub(crate) unsafe fn DistanceField_Print(field: *const u16) -> i32 {
    crate::fake::call("bc_DistanceField_Print", &[field as usize]) as i32
}
/// Exact C-ABI `DistanceField_Get` host operation; pointer buffers obey the module safety contract.
#[cfg(test)]
pub(crate) unsafe fn DistanceField_Get(
    field: *const u16,
    x: i32,
    y: i32,
    out_distance: *mut u16,
) -> i32 {
    crate::fake::call(
        "bc_DistanceField_Get",
        &[
            field as usize,
            x as usize,
            y as usize,
            out_distance as usize,
        ],
    ) as i32
}
/// Exact C-ABI `DistanceField_Set` host operation; pointer buffers obey the module safety contract.
#[cfg(test)]
pub(crate) unsafe fn DistanceField_Set(field: *mut u16, x: i32, y: i32, distance: u16) -> i32 {
    crate::fake::call(
        "bc_DistanceField_Set",
        &[field as usize, x as usize, y as usize, distance as usize],
    ) as i32
}
/// Exact C-ABI `DistanceField_ComputeWithExpansionCount` host operation; pointer buffers obey the module safety contract.
#[cfg(test)]
pub(crate) unsafe fn DistanceField_ComputeWithExpansionCount(
    walls: *const u64,
    roots: *const u64,
    visited: *mut u64,
    distances: *mut u16,
    max_distance: u16,
    out_expansions: *mut u16,
) -> i32 {
    crate::fake::call(
        "bc_DistanceField_ComputeWithExpansionCount",
        &[
            walls as usize,
            roots as usize,
            visited as usize,
            distances as usize,
            max_distance as usize,
            out_expansions as usize,
        ],
    ) as i32
}
/// Exact C-ABI `DistanceField_ComputeToWithExpansionCount` host operation; pointer buffers obey the module safety contract.
#[cfg(test)]
pub(crate) unsafe fn DistanceField_ComputeToWithExpansionCount(
    walls: *const u64,
    roots: *const u64,
    destinations: *const u64,
    visited: *mut u64,
    distances: *mut u16,
    max_distance: u16,
    out_expansions: *mut u16,
) -> i32 {
    crate::fake::call(
        "bc_DistanceField_ComputeToWithExpansionCount",
        &[
            walls as usize,
            roots as usize,
            destinations as usize,
            visited as usize,
            distances as usize,
            max_distance as usize,
            out_expansions as usize,
        ],
    ) as i32
}
