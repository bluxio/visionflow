/// @file
/// Raw Runner imports and C-compatible engine records; prefer the checked cambc.h wrappers.
/// Buffer addresses refer to guest linear memory and must satisfy the recorded sizes and alignment.
#pragma once

#include <cstddef>
#include <cstdint>
#include <new>
#include <stdexcept>
#include <string>

#if defined(__wasm__)
#define BC_IMPORT(name) __attribute__((import_module("env"), import_name(name)))
#else
#define BC_IMPORT(name)
#endif

namespace cambc
{
/// Camp design stats and growth parameters; layout matches Game_CampStats.
struct camp_stats
{
	float vision_radius;
	std::int32_t base_move_speed, base_attack_range;
	float base_health, health_growth, base_strength, strength_growth;
	float base_focus, focus_growth;
	friend bool operator==(const camp_stats&, const camp_stats&) = default;
};

/// Champion design stats and growth values.
struct champion_stats
{
	std::int32_t vision_radius, base_move_speed, base_attack_range;
	float		 base_health, health_growth, base_strength, strength_growth;
	float		 base_focus, focus_growth, camp_damage_multiplier;
	friend bool operator==(const champion_stats&, const champion_stats&) = default;
};
} // namespace cambc

/// Low-level host ABI. Buffer addresses refer to guest linear memory.
namespace cambc::detail
{

/// Map dimensions and byte sizes for engine-chunked buffers.
struct metadata
{
	/// Map dimensions in tiles.
	std::int32_t  width, height;
	/// Counts of 8-by-8 chunks and padded bitboard chunk stride.
	std::uint32_t chunked_width, chunked_height, chunk_stride;
	/// Required bitboard and distance-field raw storage sizes in bytes.
	std::uint32_t board_bytes, field_bytes;
};

/// Tile bounds with an exclusive maximum corner.
struct aabb
{
	std::int32_t min_x, min_y, max_x, max_y;
};

/// Runtime camp-group capture rewards.
struct camp_group_info
{
	std::uint32_t gems, gold;
};

/// Physical camp type and spawn tile.
struct camp_info
{
	std::int32_t x, y, type;
};

/// Visible camp combat state.
struct camp_state
{
	std::int32_t x, y;
	float health, max_health, strength, focus;
	std::int32_t cooldowns[2];
};

using camp_base_stats = cambc::camp_stats;

/// Champion properties visible to the current perspective.
struct champion_info
{
	std::int32_t team, type, x, y;
	float		 health, max_health, strength, focus;
	std::int32_t move_speed, attack_range, state;
};

using champion_base_stats = cambc::champion_stats;

inline constexpr std::uint32_t ability_target_camps = 1u << 0;
inline constexpr std::uint32_t ability_target_enemies = 1u << 1;
inline constexpr std::uint32_t ability_target_allies = 1u << 2;
inline constexpr std::uint32_t ability_target_self = 1u << 3;
inline constexpr std::uint32_t ability_target_ground = 1u << 4;

/// Ability-specific data selected by ability_props::kind.
union ability_payload
{
	std::uint32_t enhanced_attack;

	struct
	{
		float range, modifier;
	} damage, heal;

	struct
	{
		float		 range, radius, modifier;
		std::int32_t duration;
	} aoe_damage;

	struct
	{
		float		 range, modifier;
		std::int32_t duration;
	} cc, shield;

	float dash_range;
};

/// Ability properties with a tagged payload.
struct ability_props
{
	std::uint32_t	kind;
	std::int32_t	cooldown;
	/// ABI-mandated storage and target/attribute masks; public metadata exposes only the masks.
	std::uint32_t	max_charges, targets, attributes;
	ability_payload payload;
};

/// Engine unit definition with tagged ability slots and explicit ABI padding.
struct unit_definition
{
	std::int32_t type;
	std::uint32_t vision_type;
	std::uint8_t passive_type;
	std::byte padding[3];
	ability_props abilities[2];
	std::uint8_t attack_type;
	std::byte tail_padding[3];
};

/// Engine camp definition followed by its base stats; preserves the raw ABI layout.
struct camp_design
{
	unit_definition definition;
	camp_base_stats base_stats;
};

/// Camp buff tag, magnitude and remaining round count from the raw host ABI.
struct camp_buff_data
{
	std::uint32_t type;
	float magnitude;
	std::int32_t timer;
};

/// Visible altar-lock identity, owning team, tile and remaining hits.
struct altar_lock_info
{
	std::uint32_t kind, handle, team;
	std::int32_t x, y;
	std::uint32_t hits_remaining;
	friend bool operator==(const altar_lock_info&, const altar_lock_info&) = default;
};

/// Private self ward inventory and remaining recharge time in rounds.
struct ward_resources
{
	std::uint32_t charges;
	std::int32_t cooldown;
};

/// Integer tile coordinates in the engine host ABI.
struct position
{
	std::int32_t x, y;
	friend bool operator==(const position&, const position&) = default;
};

/// Visible ordinary-ward observation; negative timer means unlimited lifetime.
struct ward_info
{
	std::uint32_t team;
	std::int32_t x, y, timer;
	std::uint8_t disabled;
};

/// Engine ward observation extension with the exact ABI layout.
struct control_ward_info
{
	std::uint32_t team;
	std::int32_t x, y, timer;
};

/// Visible teleport reservation containing caster identity, destination and remaining rounds.
struct teleport_info
{
	std::uint32_t champion;
	std::int32_t type;
	std::uint32_t team;
	position destination;
	std::int32_t timer;
	std::uint32_t channel_type;
	friend bool operator==(const teleport_info&, const teleport_info&) = default;
};

/// Exact 40-byte Game_ItemInfo record; absent item/passive tags are decoded by the public wrapper.
struct item_info
{
	std::uint32_t cost, purchasable;
	std::int32_t health_points, strength_points, focus_points;
	std::uint32_t passive;
	float passive_magnitude;
	std::uint32_t combine_count, result, combine_cost;
};

/// Exact 8-byte, aligned-4 Game_ConsumableInfo record containing cost and carry limit.
struct consumable_info
{
	std::uint32_t cost, carry_limit;
};

static_assert(sizeof(consumable_info) == 8 && alignof(consumable_info) == 4);
static_assert(offsetof(consumable_info, carry_limit) == 4);

/// Engine buff tags used to decode Game_BuffData.
enum class buff_kind : std::uint32_t
{
	shield,
	stun,
	enhanced_attack,
	count
};

/// Active buff tag and its type-specific payload; matches Game_BuffData.
struct buff_data
{
	std::uint32_t type;

	union
	{
		struct
		{
			std::int32_t duration;
			float		 value;
		} shield;

		struct
		{
			std::int32_t duration;
		} stun, enhanced_attack;
	} data;
};

static_assert(sizeof(buff_data) == 12);

/// Opaque definition bytes followed by base stats; layout matches the engine ABI.
struct champion_design
{
	unit_definition definition;
	champion_base_stats base_stats;
};

static_assert(sizeof(champion_info) == 44);
static_assert(sizeof(champion_base_stats) == 40);
static_assert(sizeof(ability_props) == 36);
static_assert(sizeof(champion_design) == 128);

/// Status-returning imports use zero for success and an engine error code for failure.
/// List imports write a count separately from the buffer; count-only reads use zero capacity.
// Status-returning imports use zero for success and an engine error code for failure.
extern "C"
{
	/// Copy one item's engine metadata into a caller-provided 40-byte record.
	BC_IMPORT("item_info") std::int32_t sdk_item_info(std::uint32_t, item_info*);
	/// Copy one consumable's engine metadata into a caller-provided 8-byte record.
	BC_IMPORT("consumable_info") std::int32_t sdk_consumable_info(std::uint32_t, consumable_info*);
	/// Typed buff snapshots and pure ability effect queries.
	BC_IMPORT("champion_buff_data") std::int32_t sdk_champion_buff_data(std::uint32_t, buff_data*, std::uint32_t, std::uint32_t*);
	BC_IMPORT("champion_ability_damage") std::int32_t sdk_champion_ability_damage(std::uint32_t, std::uint32_t, float*);
	BC_IMPORT("champion_ability_damage_against")
	std::int32_t sdk_champion_ability_damage_against(std::uint32_t, std::uint32_t, std::uint32_t, float*);
	BC_IMPORT("champion_ability_heal") std::int32_t sdk_champion_ability_heal(std::uint32_t, std::uint32_t, float*);
	BC_IMPORT("champion_ability_heal_against")
	std::int32_t sdk_champion_ability_heal_against(std::uint32_t, std::uint32_t, std::uint32_t, float*);
	BC_IMPORT("champion_ability_shield") std::int32_t sdk_champion_ability_shield(std::uint32_t, std::uint32_t, float*);
	BC_IMPORT("champion_ability_shield_against")
	std::int32_t sdk_champion_ability_shield_against(std::uint32_t, std::uint32_t, std::uint32_t, float*);

	/// Get the diagnostic byte length; copy up to capacity bytes without a terminating NUL.
	/// A zero capacity queries the required size without writing.
	BC_IMPORT("bc_get_last_error") std::int32_t sdk_bc_get_last_error(char*, std::uint32_t);

	/// Identity, roster and occupancy queries.
	BC_IMPORT("my_champion") std::int32_t sdk_my_champion();
	BC_IMPORT("champion_is_alive") std::int32_t sdk_champion_is_alive(std::uint32_t, std::int32_t*);
	BC_IMPORT("num_allies") std::int32_t sdk_num_allies(std::uint32_t*);
	BC_IMPORT("num_enemies") std::int32_t sdk_num_enemies(std::uint32_t*);
	BC_IMPORT("allies") std::int32_t sdk_allies(std::uint32_t*, std::uint32_t, std::uint32_t*);
	BC_IMPORT("enemies") std::int32_t sdk_enemies(std::uint32_t*, std::uint32_t, std::uint32_t*);
	BC_IMPORT("board_ally_occupancy") std::int32_t sdk_ally_occupancy(std::uint64_t*);
	BC_IMPORT("board_enemy_occupancy") std::int32_t sdk_enemy_occupancy(std::uint64_t*);
	BC_IMPORT("board_camp_occupancy") std::int32_t sdk_camp_occupancy(std::uint64_t*);
	BC_IMPORT("board_ally_ward_occupancy") std::int32_t sdk_ally_ward_occupancy(std::uint64_t*);
	BC_IMPORT("board_enemy_ward_occupancy") std::int32_t sdk_enemy_ward_occupancy(std::uint64_t*);
	BC_IMPORT("ward_at") std::int32_t sdk_ward_at(std::int32_t, std::int32_t, std::uint32_t*);
	BC_IMPORT("ally_ward_at") std::int32_t sdk_ally_ward_at(std::int32_t, std::int32_t, std::uint32_t*);
	BC_IMPORT("enemy_ward_at") std::int32_t sdk_enemy_ward_at(std::int32_t, std::int32_t, std::uint32_t*);
	BC_IMPORT("ward") std::int32_t sdk_ward(std::uint32_t, ward_info*);
	BC_IMPORT("board_teleport_occupancy") std::int32_t sdk_teleport_occupancy(std::uint64_t*);
	BC_IMPORT("is_occupied_by_teleport") std::int32_t sdk_has_teleport(std::int32_t, std::int32_t, std::int32_t*);
	BC_IMPORT("teleports") std::int32_t sdk_teleports(teleport_info*, std::uint32_t, std::uint32_t*);
	BC_IMPORT("board_ward_vision_mask") std::int32_t sdk_ward_vision(std::int32_t, std::int32_t, std::uint64_t*);
	BC_IMPORT("board_ally_altar_lock_occupancy") std::int32_t sdk_ally_altar_lock_occupancy(std::uint64_t*);
	BC_IMPORT("board_enemy_altar_lock_occupancy") std::int32_t sdk_enemy_altar_lock_occupancy(std::uint64_t*);
	BC_IMPORT("altar_lock") std::int32_t sdk_altar_lock(std::int32_t, std::int32_t, std::int32_t, altar_lock_info*);
	BC_IMPORT("altar_locks") std::int32_t sdk_altar_locks(altar_lock_info*, std::uint32_t, std::uint32_t*);

	/// Champion actions; the host validates ownership and action availability.
	BC_IMPORT("attack") std::int32_t sdk_attack(std::uint32_t, std::uint32_t);
	BC_IMPORT("cast_with_result") std::int32_t sdk_cast(std::uint32_t, std::uint32_t, std::uint32_t, std::int32_t, std::int32_t, std::int32_t*);
	BC_IMPORT("use_consumable_with_result") std::int32_t sdk_use_consumable(std::uint32_t, std::int32_t, std::int32_t, std::int32_t*);
	BC_IMPORT("buy_consumable") std::int32_t sdk_buy_consumable(std::uint32_t);
	BC_IMPORT("buy_item") std::int32_t sdk_buy_item(std::uint32_t, std::uint32_t);
	BC_IMPORT("sell") std::int32_t sdk_sell(std::uint32_t);
	BC_IMPORT("recall") std::int32_t sdk_recall();
	BC_IMPORT("cancel_channel") std::int32_t sdk_cancel_channel();
	BC_IMPORT("submit_gems") std::int32_t sdk_submit_gems(std::int32_t, std::int32_t);
	BC_IMPORT("place_ward_with_result") std::int32_t sdk_place_ward(std::int32_t, std::int32_t, std::int32_t*);
	BC_IMPORT("move_to_with_result") std::int32_t sdk_move_to(std::int32_t, std::int32_t, std::int32_t*);
	BC_IMPORT("set_priority") std::int32_t sdk_set_priority(std::int32_t);

	/// Scheduling and team progress queries.
	BC_IMPORT("bc_champion_priority") std::int32_t sdk_bc_champion_priority(std::uint32_t, std::int32_t, std::int32_t*);
	BC_IMPORT("bc_submitted_gems") std::int32_t sdk_bc_submitted_gems(std::uint32_t, std::uint32_t*);
	BC_IMPORT("bc_round_number") std::int32_t sdk_bc_round_number(std::uint32_t*);

	/// Resign the game.
	BC_IMPORT("resign") std::int32_t sdk_resign();

	/// Team shared-memory capacity and byte transfers.
	BC_IMPORT("shared_memory_size") std::int32_t sdk_shared_memory_size(std::uint32_t*);
	BC_IMPORT("read_shared_memory") std::int32_t sdk_read_shared_memory(std::int32_t, void*, std::uint32_t);
	BC_IMPORT("write_shared_memory") std::int32_t sdk_write_shared_memory(std::int32_t, const void*, std::uint32_t);

	/// Champion inspection and inventory queries.
	BC_IMPORT("champion") std::int32_t sdk_champion(std::uint32_t, champion_info*);
	BC_IMPORT("champion_design") std::int32_t sdk_champion_design(std::int32_t, champion_design*);
	BC_IMPORT("champion_restrictions") std::int32_t sdk_champion_restrictions(std::uint32_t, std::uint8_t*);
	BC_IMPORT("champion_ability_props") std::int32_t sdk_champion_ability_props(std::uint32_t, std::uint32_t, ability_props*);
	BC_IMPORT("champion_cooldown") std::int32_t sdk_champion_cooldown(std::uint32_t, std::uint32_t, std::int32_t*);
	BC_IMPORT("champion_items") std::int32_t sdk_champion_items(std::uint32_t, std::uint32_t*);
	BC_IMPORT("champion_consumables") std::int32_t sdk_champion_consumables(std::uint32_t, std::uint32_t*);
	BC_IMPORT("champion_gems") std::int32_t sdk_champion_gems(std::uint32_t, std::uint32_t*);
	BC_IMPORT("champion_gold") std::int32_t sdk_champion_gold(std::uint32_t, std::uint32_t*);
	BC_IMPORT("bc_champion_vision") std::int32_t sdk_bc_champion_vision(std::uint32_t, std::uint64_t*);
	BC_IMPORT("board_vision_mask") std::int32_t sdk_hypothetical_vision(std::int32_t, std::int32_t, std::int32_t, std::uint64_t*);
	BC_IMPORT("champion_wards") std::int32_t sdk_champion_wards(std::uint32_t, ward_resources*);
	BC_IMPORT("bc_champion_ward_positions") std::int32_t sdk_champion_ward_positions(std::uint32_t, position*, std::uint32_t, std::uint32_t*);
	BC_IMPORT("bc_champion_channel_time") std::int32_t sdk_champion_channel_time(std::uint32_t, std::int32_t*);
	BC_IMPORT("champion_can_shop") std::int32_t sdk_champion_can_shop(std::uint32_t, std::int32_t, std::int32_t*);
	BC_IMPORT("champion_can_sell") std::int32_t sdk_champion_can_sell(std::uint32_t, std::int32_t*);
	BC_IMPORT("wards") std::int32_t sdk_wards(ward_info*, std::uint32_t, std::uint32_t*);

	/// Map layout, terrain, symmetry and camp queries.
	BC_IMPORT("bc_metadata") std::int32_t sdk_bc_metadata(metadata*);
	BC_IMPORT("my_team") std::int32_t sdk_my_team();
	BC_IMPORT("bc_map_mask") std::int32_t sdk_bc_map_mask(std::int32_t, std::uint64_t*);
	BC_IMPORT("bc_map_symmetry") std::int32_t sdk_bc_map_symmetry(std::int32_t*);
	BC_IMPORT("map_num_camp_groups") std::int32_t sdk_map_num_camp_groups(std::uint32_t*);
	BC_IMPORT("map_camp_groups") std::int32_t sdk_map_camp_groups(std::uint32_t*, std::uint32_t, std::uint32_t*);
	BC_IMPORT("map_camp_group_num_camps") std::int32_t sdk_map_camp_group_num_camps(std::uint32_t, std::uint32_t*);
	BC_IMPORT("map_camp_group_camps") std::int32_t sdk_map_camp_group_camps(std::uint32_t, std::uint32_t*, std::uint32_t, std::uint32_t*);
	BC_IMPORT("map_camp_group") std::int32_t sdk_map_camp_group(std::uint32_t, camp_group_info*);
	BC_IMPORT("map_camp_info") std::int32_t sdk_map_camp_info(std::uint32_t, camp_info*);
	BC_IMPORT("map_camp_layout") std::int32_t sdk_map_camp_layout(std::uint32_t*, std::uint32_t, std::uint32_t*);
	BC_IMPORT("map_camp_group_of") std::int32_t sdk_map_camp_group_of(std::uint32_t, std::uint32_t*);
	BC_IMPORT("map_camp_at") std::int32_t sdk_map_camp_at(std::int32_t, std::int32_t, std::uint32_t*);
	BC_IMPORT("camp_group_cooldown") std::int32_t sdk_camp_group_cooldown(std::uint32_t, std::int32_t*);
	BC_IMPORT("camp_group_state") std::int32_t sdk_camp_group_state(std::uint32_t, std::uint32_t*);
	BC_IMPORT("camp_buffs") std::int32_t sdk_camp_buffs(std::uint32_t, camp_buff_data*, std::uint32_t, std::uint32_t*);
	BC_IMPORT("bc_camp_group_max_respawn") std::int32_t sdk_bc_camp_group_max_respawn(std::uint32_t, std::int32_t*);
	BC_IMPORT("camp") std::int32_t sdk_camp(std::uint32_t, camp_state*);
	BC_IMPORT("camp_design") std::int32_t sdk_camp_design(std::int32_t, camp_design*);
	BC_IMPORT("camp_at") std::int32_t sdk_camp_at(std::int32_t, std::int32_t, std::uint32_t*);
	BC_IMPORT("champion_at") std::int32_t sdk_champion_at(std::int32_t, std::int32_t, std::uint32_t*);
	BC_IMPORT("has_camp") std::int32_t sdk_has_camp(std::int32_t, std::int32_t, std::int32_t*);
	BC_IMPORT("has_champion") std::int32_t sdk_has_champion(std::int32_t, std::int32_t, std::int32_t*);
	BC_IMPORT("board_vision") std::int32_t sdk_vision(std::uint64_t*);
	BC_IMPORT("spawn_region") std::int32_t sdk_spawn_region(std::int32_t, aabb*);

	/// Draft queries, selections and team memory submission.
	BC_IMPORT("draft_num_rounds") std::int32_t sdk_draft_num_rounds(std::uint8_t*);
	BC_IMPORT("draft_get") std::int32_t sdk_draft_get(std::int32_t, std::uint32_t, std::int32_t*);
	BC_IMPORT("draft_champion") std::int32_t sdk_draft_champion(std::int32_t);
	BC_IMPORT("bc_submit_draft_memory") std::int32_t sdk_bc_submit_draft_memory(const void*, std::size_t);
	BC_IMPORT("draft_memory_size") std::int32_t sdk_draft_memory_size(std::uint32_t*);
	BC_IMPORT("read_draft_memory") std::int32_t sdk_read_draft_memory(std::int32_t, void*, std::uint32_t);
	BC_IMPORT("debug_dot") std::int32_t sdk_debug_dot(std::int32_t, std::int32_t, std::uint32_t);
	BC_IMPORT("debug_line") std::int32_t sdk_debug_line(std::int32_t, std::int32_t, std::int32_t, std::int32_t, std::uint32_t);

	/// Yield the current turn to the host.
	BC_IMPORT("yield") void sdk_yield();

	/// Get the fuel remaining in the current turn.
	BC_IMPORT("fuel_remaining") std::int64_t sdk_fuel_remaining();

	/// Initialise engine-chunked bitboard storage.
	BC_IMPORT("bc_Bitboard_Zero") std::int32_t sdk_Bitboard_Zero(std::uint64_t* buffer);
	BC_IMPORT("bc_Bitboard_One") std::int32_t sdk_Bitboard_One(std::uint64_t* buffer);

	/// Transfer raw or packed row-major bitboard buffers.
	BC_IMPORT("bc_Bitboard_ImportRawBuffer")
	std::int32_t sdk_Bitboard_ImportRawBuffer(std::uint64_t* buffer, const std::uint8_t* bytes, std::size_t size);
	BC_IMPORT("bc_Bitboard_ImportLinearBuffer")
	std::int32_t sdk_Bitboard_ImportLinearBuffer(std::uint64_t* buffer, const std::uint8_t* bytes, std::size_t size);
	BC_IMPORT("bc_Bitboard_ExportRawBuffer")
	std::int32_t sdk_Bitboard_ExportRawBuffer(const std::uint64_t* buffer, std::uint8_t* bytes, std::size_t size);
	BC_IMPORT("bc_Bitboard_ExportLinearBuffer")
	std::int32_t sdk_Bitboard_ExportLinearBuffer(const std::uint64_t* buffer, std::uint8_t* bytes, std::size_t size);

	/// Print and inspect map tiles.
	BC_IMPORT("bc_Bitboard_Print") std::int32_t sdk_Bitboard_Print(const std::uint64_t* buffer);
	BC_IMPORT("bc_Bitboard_Test")
	std::int32_t sdk_Bitboard_Test(const std::uint64_t* buffer, std::int32_t x, std::int32_t y, std::int32_t* value);
	BC_IMPORT("bc_Bitboard_Set") std::int32_t sdk_Bitboard_Set(std::uint64_t* buffer, std::int32_t x, std::int32_t y, std::int32_t value);
	BC_IMPORT("bc_Bitboard_Toggle") std::int32_t sdk_Bitboard_Toggle(std::uint64_t* buffer, std::int32_t x, std::int32_t y);
	BC_IMPORT("bc_Bitboard_IsZero") std::int32_t sdk_Bitboard_IsZero(const std::uint64_t* buffer, std::int32_t* value);
	BC_IMPORT("bc_Bitboard_NumSetBits") std::int32_t sdk_Bitboard_NumSetBits(const std::uint64_t* buffer, std::uint32_t* value);
	BC_IMPORT("bc_Bitboard_GetSetBits")
	std::int32_t sdk_Bitboard_GetSetBits(const std::uint64_t* buffer, void* positions, std::size_t capacity, std::uint32_t* count);

	/// Bitwise operations on map tiles.
	BC_IMPORT("bc_Bitboard_Not") std::int32_t sdk_Bitboard_Not(const std::uint64_t* buffer, std::uint64_t* result);
	BC_IMPORT("bc_Bitboard_Union") std::int32_t sdk_Bitboard_Union(const std::uint64_t* a, const std::uint64_t* b, std::uint64_t* result);
	BC_IMPORT("bc_Bitboard_Intersect")
	std::int32_t sdk_Bitboard_Intersect(const std::uint64_t* a, const std::uint64_t* b, std::uint64_t* result);
	BC_IMPORT("bc_Bitboard_Xor") std::int32_t sdk_Bitboard_Xor(const std::uint64_t* a, const std::uint64_t* b, std::uint64_t* result);
	BC_IMPORT("bc_Bitboard_IsEqual") std::int32_t sdk_Bitboard_IsEqual(const std::uint64_t* a, const std::uint64_t* b, std::int32_t* value);

	/// Shift and reflect map tiles.
	BC_IMPORT("bc_Bitboard_ShiftRight")
	std::int32_t sdk_Bitboard_ShiftRight(const std::uint64_t* buffer, std::uint64_t* result, std::int32_t count);
	BC_IMPORT("bc_Bitboard_ShiftUp")
	std::int32_t sdk_Bitboard_ShiftUp(const std::uint64_t* buffer, std::uint64_t* result, std::int32_t count);
	BC_IMPORT("bc_Bitboard_Reflect")
	std::int32_t sdk_Bitboard_Reflect(const std::uint64_t* buffer, std::uint64_t* result, std::int32_t symmetry);

	/// Add tiles within Euclidean or Chebyshev radii.
	BC_IMPORT("bc_Bitboard_DrawCircle")
	std::int32_t sdk_Bitboard_DrawCircle(std::uint64_t* buffer, std::int32_t x, std::int32_t y, float radius);
	BC_IMPORT("bc_Bitboard_DrawChebyshev")
	std::int32_t sdk_Bitboard_DrawChebyshev(std::uint64_t* buffer, std::int32_t x, std::int32_t y, float radius);

	/// Transform individual tile positions using map dimensions.
	BC_IMPORT("bc_Bitboard_ReflectHorizontalPosition")
	std::int32_t sdk_Bitboard_ReflectHorizontalPosition(std::int32_t x, std::int32_t y, std::int32_t* out_x, std::int32_t* out_y);
	BC_IMPORT("bc_Bitboard_ReflectVerticalPosition")
	std::int32_t sdk_Bitboard_ReflectVerticalPosition(std::int32_t x, std::int32_t y, std::int32_t* out_x, std::int32_t* out_y);
	BC_IMPORT("bc_Bitboard_Rotate180Position")
	std::int32_t sdk_Bitboard_Rotate180Position(std::int32_t x, std::int32_t y, std::int32_t* out_x, std::int32_t* out_y);

	/// Reachability and shadow-cast queries using wall masks.
	BC_IMPORT("bc_Bitboard_ReachableMask")
	std::int32_t sdk_Bitboard_ReachableMask(const std::uint64_t* walls, std::int32_t x, std::int32_t y, std::uint64_t* result,
											std::uint16_t distance);
	BC_IMPORT("bc_Bitboard_ShadowCast")
	std::int32_t sdk_Bitboard_ShadowCast(const std::uint64_t* walls, std::int32_t x, std::int32_t y, std::uint64_t* result,
										 std::uint16_t distance);
	BC_IMPORT("bc_Bitboard_ReachableMaskWithScratch")
	std::int32_t sdk_Bitboard_ReachableMaskWithScratch(const std::uint64_t* walls, std::int32_t x, std::int32_t y, std::uint64_t* result,
													   std::uint16_t distance, std::uint64_t* frontier, std::uint64_t* scratch);

	/// Transfer raw or row-major distance buffers.
	BC_IMPORT("bc_DistanceField_ImportRawBuffer")
	std::int32_t sdk_DistanceField_ImportRawBuffer(std::uint16_t* buffer, const std::uint8_t* bytes, std::size_t size);
	BC_IMPORT("bc_DistanceField_ImportLinearBuffer")
	std::int32_t sdk_DistanceField_ImportLinearBuffer(std::uint16_t* buffer, const std::uint8_t* bytes, std::size_t size);
	BC_IMPORT("bc_DistanceField_ExportRawBuffer")
	std::int32_t sdk_DistanceField_ExportRawBuffer(const std::uint16_t* buffer, std::uint8_t* bytes, std::size_t size);
	BC_IMPORT("bc_DistanceField_ExportLinearBuffer")
	std::int32_t sdk_DistanceField_ExportLinearBuffer(const std::uint16_t* buffer, std::uint8_t* bytes, std::size_t size);

	/// Print, read and write tile distances.
	BC_IMPORT("bc_DistanceField_Print") std::int32_t sdk_DistanceField_Print(const std::uint16_t* buffer);
	BC_IMPORT("bc_DistanceField_Get")
	std::int32_t sdk_DistanceField_Get(const std::uint16_t* buffer, std::int32_t x, std::int32_t y, std::uint16_t* value);
	BC_IMPORT("bc_DistanceField_Set")
	std::int32_t sdk_DistanceField_Set(std::uint16_t* buffer, std::int32_t x, std::int32_t y, std::uint16_t value);

	/// Compute distances with caller-provided visited storage and an expansion count.
	BC_IMPORT("bc_DistanceField_ComputeWithExpansionCount")
	std::int32_t sdk_DistanceField_ComputeWithExpansionCount(const std::uint64_t* walls, const std::uint64_t* roots, std::uint64_t* visited,
															 std::uint16_t* distances, std::uint16_t max_distance, std::uint16_t* count);
	BC_IMPORT("bc_DistanceField_ComputeToWithExpansionCount")
	std::int32_t sdk_DistanceField_ComputeToWithExpansionCount(const std::uint64_t* walls, const std::uint64_t* roots,
															   const std::uint64_t* destinations, std::uint64_t* visited,
															   std::uint16_t* distances, std::uint16_t max_distance, std::uint16_t* count);
}

/// Copy the most recent engine diagnostic from host memory.
[[nodiscard]] inline std::string get_last_error()
{
	const auto size = sdk_bc_get_last_error(nullptr, 0);
	if (size <= 0)
		return {};
	std::string message(static_cast<std::size_t>(size), '\0');
	sdk_bc_get_last_error(message.data(), static_cast<std::uint32_t>(size));
	return message;
}

/// Throw on a failed import; engine failures preserve the code and diagnostic.
inline void check(std::int32_t code)
{
	if (code == 0)
		return;
	if (code == -1)
		throw std::invalid_argument("invalid guest pointer or buffer (-1)");
	throw error{ code };
}

/// Engine shared-memory capacity captured once at bot startup.
inline const std::uint32_t shared_memory_capacity = []
{
	std::uint32_t result{};
	check(sdk_shared_memory_size(&result));
	return result;
}();

/// Map dimensions and buffer layout captured during dynamic initialisation for this bot.
inline const metadata cached_metadata = []
{
	metadata result{};
	check(sdk_bc_metadata(&result));
	return result;
}();

/// Get the map layout captured at startup.
[[nodiscard]] inline const metadata& layout()
{
	return cached_metadata;
}

/// Translate a relative team into the engine's absolute team index; invalid tags throw.
[[nodiscard]] inline std::int32_t absolute_team(team value)
{
	const auto own = sdk_my_team();
	switch (value)
	{
	case team::ally:
		return own;
	case team::enemy:
		return 1 - own;
	}
	throw std::invalid_argument("invalid team");
}

/// Release the 16-byte-aligned raw storage owned by a bitboard.
struct aligned_delete
{
	void operator()(std::uint64_t* p) const noexcept { ::operator delete(p, std::align_val_t{ 16 }); }
};
} // namespace cambc::detail

namespace cambc
{
inline error::error(std::int32_t code) : error(static_cast<error_code>(code), detail::get_last_error()) {}

inline error::error(error_code code) : error(code, detail::get_last_error()) {}
} // namespace cambc

#undef BC_IMPORT
