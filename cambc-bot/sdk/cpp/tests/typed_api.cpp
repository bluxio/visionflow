// Standalone SDK test with deterministic host responses; no live game is required.
#if defined(__wasm__)
#undef __wasm__
#include <unordered_set>
#include <cambc.h>
#define __wasm__ 1
#else
#include <cambc.h>
#endif
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <limits>
#include <ranges>
#include <bit>

namespace
{
template <class T>
concept has_none = requires { T::none; };
static_assert(!has_none<cambc::champion_draft> && !has_none<cambc::champion_type> && !has_none<cambc::ability_kind> &&
			  !has_none<cambc::item_type> && !has_none<cambc::detail::entity_kind>);

template <class T>
concept has_spell = requires { T::spell; };
template <class T>
concept has_trinket = requires { T::trinket; };
static_assert(!has_spell<cambc::detail::entity_kind> && !has_trinket<cambc::buff_restrictions>);
static_assert(static_cast<std::uint32_t>(cambc::detail::entity_kind::altar_lock) == 6);
static_assert(cambc::buff_restrictions::ward.bits() == (1 << 5));
constexpr auto combined_targets = cambc::ability_target_mask::camps | cambc::ability_target_mask::enemies;
static_assert(combined_targets.contains(cambc::ability_target_mask::camps));
static_assert(combined_targets.bits() == 3);
static_assert(std::is_same_v<decltype(cambc::ability_target_mask::camps | cambc::ability_target_mask::enemies),
	cambc::ability_target_mask>);

template <class T>
concept has_manhattan_distance = requires(T value) { value.manhattan_distance(value); };
static_assert(!has_manhattan_distance<cambc::vec2>);
template <class T> concept has_manhattan_drawing = requires(T value) { value.draw_manhattan(cambc::vec2{}, 1.0f); };
static_assert(!has_manhattan_drawing<cambc::bitboard>);

template <class T>
concept has_charges = requires(T value) { value.charges; } || requires(T value) { value.max_charges; } ||
					 requires(T value) { value.get_charges(); };
static_assert(!has_charges<cambc::ability_properties> && !has_charges<cambc::ability>);
template <class T> concept has_cooldown_snapshot = requires(T value) { value.cooldown; };
static_assert(!has_cooldown_snapshot<cambc::ability_properties>);
static_assert(!has_none<cambc::item_passive>);
static_assert(std::is_convertible_v<cambc::item_type, cambc::item_type::enum_type>);
static_assert(std::is_convertible_v<cambc::consumable_type, cambc::consumable_type::enum_type>);
static_assert(std::is_convertible_v<cambc::item_type::enum_type, cambc::item_type>);
static_assert(std::is_convertible_v<cambc::consumable_type::enum_type, cambc::consumable_type>);
static_assert(!std::is_convertible_v<cambc::item_type, std::uint32_t>);
static_assert(!std::is_convertible_v<cambc::consumable_type, std::uint32_t>);
static_assert(std::is_trivially_copyable_v<cambc::item_type> && std::is_standard_layout_v<cambc::item_type>);
static_assert(std::is_trivially_copyable_v<cambc::consumable_type> && std::is_standard_layout_v<cambc::consumable_type>);
static_assert(sizeof(cambc::item_type) == sizeof(std::uint32_t) && alignof(cambc::item_type) == alignof(std::uint32_t));
static_assert(sizeof(cambc::consumable_type) == sizeof(std::uint32_t) && alignof(cambc::consumable_type) == alignof(std::uint32_t));
static_assert([](cambc::item_type::enum_type value) { return value; }(cambc::item_type::monster_shard_of_strength) == cambc::item_type::enum_type::monster_shard_of_strength);
static_assert([](cambc::consumable_type::enum_type value) { return value; }(cambc::consumable_type::tp_crystal) == cambc::consumable_type::enum_type::tp_crystal);
static_assert([](cambc::item_type value) { switch (value) { case cambc::item_type::monster_shard_of_strength: return true; default: return false; } }(cambc::item_type{}));
static_assert([](cambc::consumable_type value) { switch (value) { case cambc::consumable_type::tp_crystal: return true; default: return false; } }(cambc::consumable_type{}));
static_assert(cambc::item_type::monster_shard_of_strength < cambc::item_type::monster_emblem_of_strength);
static_assert(std::is_same_v<decltype(cambc::item_type::monster_shard_of_strength.info()), cambc::item_info>);
static_assert(std::is_same_v<decltype(cambc::consumable_type::tp_crystal.info()), cambc::consumable_info>);

template <class T>
concept has_old_ability_fields = requires(T value) { value.base_cooldown; } || requires(T value) { value.targets; };
static_assert(!has_old_ability_fields<cambc::ability_properties>);
template <class T>
concept has_ability_slots = requires(T value) { value.ability_slots; };
static_assert(!has_ability_slots<cambc::champion>);
static_assert(std::is_same_v<decltype(std::declval<const cambc::ability&>().info()), const std::optional<cambc::ability_info>&>);
static_assert(std::is_same_v<decltype(std::declval<const cambc::ability&>().slot()), std::uint32_t>);
static_assert(noexcept(std::declval<const cambc::ability&>().slot()));
static_assert(std::is_same_v<decltype(std::declval<const cambc::ability&>().is_ready()), bool>);
static_assert(std::is_same_v<decltype(std::declval<const cambc::ability&>().valid_tiles()), cambc::bitboard>);
static_assert(std::is_same_v<decltype(std::declval<const cambc::ability&>().is_attack_reset()), bool>);
static_assert(std::is_same_v<decltype(std::declval<const cambc::ability&>().is_damage()), bool>);
static_assert(std::is_same_v<decltype(std::declval<const cambc::ability&>().is_aoe_damage()), bool>);
static_assert(std::is_same_v<decltype(std::declval<const cambc::ability&>().is_cc()), bool>);
static_assert(std::is_same_v<decltype(std::declval<const cambc::ability&>().is_heal()), bool>);
static_assert(std::is_same_v<decltype(std::declval<const cambc::ability&>().is_dash()), bool>);
static_assert(std::is_same_v<decltype(std::declval<const cambc::ability&>().is_shield()), bool>);
static_assert(std::is_same_v<decltype(std::declval<const cambc::champion&>().can_move()), bool>);
static_assert(std::is_same_v<decltype(std::declval<const cambc::champion&>().can_attack()), bool>);
static_assert(std::is_same_v<decltype(std::declval<const cambc::champion&>().can_cast()), bool>);
template <class T> concept has_can_shop = requires(T value) { value.can_shop(); };
static_assert(!has_can_shop<cambc::champion>);
static_assert(std::is_same_v<decltype(std::declval<const cambc::champion&>().can_buy(cambc::item_type{}, 1)), bool>);
static_assert(std::is_same_v<decltype(std::declval<const cambc::champion&>().can_buy(cambc::consumable_type{})), bool>);
static_assert(std::is_same_v<decltype(std::declval<const cambc::champion&>().is_recalling()), bool>);
static_assert(std::is_same_v<decltype(std::declval<const cambc::champion&>().is_submitting_gems()), bool>);
static_assert(std::is_same_v<decltype(std::declval<const cambc::champion&>().is_teleporting()), bool>);
static_assert(std::is_same_v<decltype(std::declval<const cambc::champion&>().has_completed_turn()), bool>);
static_assert(std::is_same_v<decltype(std::declval<const cambc::champion&>().is_ally()), bool>);
static_assert(std::is_same_v<decltype(std::declval<const cambc::champion&>().is_enemy()), bool>);
static_assert(std::is_same_v<decltype(std::declval<const cambc::buff_collection&>().has_shield()), bool>);
static_assert(std::is_same_v<decltype(std::declval<const cambc::buff_collection&>().has_stun()), bool>);
static_assert(std::is_same_v<decltype(std::declval<const cambc::buff_collection&>().has_enhanced_attack()), bool>);
static_assert(std::is_same_v<decltype(std::declval<const cambc::camp&>().is_wolf()), bool>);
static_assert(std::is_same_v<decltype(std::declval<const cambc::camp&>().is_elk()), bool>);
static_assert(std::is_same_v<decltype(std::declval<const cambc::camp&>().is_gem_cluster()), bool>);
static_assert(std::is_same_v<decltype(std::declval<const cambc::camp&>().is_gold_cluster()), bool>);
static_assert(std::is_same_v<decltype(std::declval<const cambc::camp&>().is_monolith()), bool>);
static_assert(std::is_same_v<decltype(std::declval<const cambc::ward&>().is_ally()), bool>);
static_assert(std::is_same_v<decltype(std::declval<const cambc::ward&>().is_enemy()), bool>);
static_assert(std::is_same_v<decltype(std::declval<const cambc::teleport&>().is_ally()), bool>);
static_assert(std::is_same_v<decltype(std::declval<const cambc::teleport&>().is_enemy()), bool>);
static_assert(std::is_same_v<decltype(std::declval<const cambc::altar_lock&>().is_ally()), bool>);
static_assert(std::is_same_v<decltype(std::declval<const cambc::altar_lock&>().is_enemy()), bool>);
static_assert(std::is_same_v<decltype(std::declval<const cambc::ability&>().info(cambc::ability_kind::damage)), const cambc::ability_info&>);
static_assert(noexcept(std::declval<const cambc::ability&>().is(cambc::ability_kind::damage)));
static_assert(noexcept(std::declval<const cambc::ability&>().is(cambc::ability_types::damage)));
static_assert(std::is_same_v<decltype(std::declval<cambc::champion>().abilities()), std::array<cambc::ability, 2>>);
static_assert(std::is_abstract_v<cambc::unit> && std::is_base_of_v<cambc::unit, cambc::camp> && std::is_base_of_v<cambc::unit, cambc::champion>);
static_assert(std::is_same_v<decltype(cambc::map::camp_groups()), std::vector<cambc::map::camp_group>>);
static_assert(std::is_same_v<decltype(std::declval<const cambc::map::camp_group&>().spawn_tiles()), cambc::bitboard>);
static_assert(std::is_same_v<decltype(cambc::ward::visible_tiles_from(cambc::vec2{})), cambc::bitboard>);
static_assert(std::is_same_v<decltype(std::declval<const cambc::ward&>().visible_tiles()), cambc::bitboard>);
static_assert(cambc::vec2{2, 3}.reflect_horizontal() == cambc::vec2{-2, 3});
static_assert(cambc::vec2{2, 3}.reflect_vertical() == cambc::vec2{2, -3});
static_assert(cambc::vec2{2, 3}.rotate_180() == cambc::vec2{-2, -3});
static_assert(cambc::vec2::north == cambc::vec2{0, 1} && cambc::vec2::south_west == cambc::vec2{-1, -1});
static_assert(cambc::vec2::north.rotate_cw() == cambc::vec2::east && cambc::vec2::north_east.rotate_cw() == cambc::vec2::south_east);
static_assert(cambc::vec2::north + cambc::vec2::west == cambc::vec2::north_west);
static_assert(std::is_same_v<decltype(cambc::vision::champion_at(cambc::vec2{})), cambc::champion>);
static_assert(std::is_same_v<decltype(cambc::vision::camps()), std::vector<cambc::camp>>);
static_assert(std::is_same_v<decltype(cambc::vision::champions()), std::vector<cambc::champion>>);
static_assert(std::is_same_v<decltype(cambc::vision::camp_groups()), std::vector<cambc::map::camp_group>>);
static_assert(std::is_same_v<decltype(cambc::draft::current_draft(cambc::team::ally)), std::vector<cambc::champion_draft>>);
static_assert(std::is_same_v<decltype(cambc::draft_memory[0]), const std::byte&>);
static_assert(std::is_same_v<decltype(cambc::shared_memory[0]), cambc::shared_memory_buffer::reference>);
static_assert(!has_none<cambc::camp_type>);
template <class T> concept has_inventory = requires(T value) { value.consumables()[cambc::consumable_type::tp_crystal]; } || requires(T value) { value.item_count(); };
static_assert(!has_inventory<cambc::camp>);

std::uint32_t active_buffs = 0b111;
std::uint32_t champion_buff_calls = 0, camp_buff_calls = 0;
std::uint8_t mock_restrictions = 0;
std::int32_t mock_champion_attack_range = 2;
std::int32_t  query_error = 0;
std::int32_t action_error = 0;
std::int32_t action_successful = 1;
std::uint32_t mock_ability_kind = 3;
std::uint32_t mock_valid_targets = 6;
std::uint32_t mock_attributes = 8;
std::uint32_t ability_props_calls = 0;
std::uint32_t cast_target = 0;
std::uint32_t cast_kind = 0;
std::uint32_t sell_calls = 0;
std::uint32_t sell_error_at = 0;
std::uint32_t move_calls = 0;
std::uint32_t metadata_calls = 0, terrain_calls = 0, spawn_calls = 0, symmetry_calls = 0, group_calls = 0, memory_size_calls = 0;
std::uint32_t my_champion_calls = 0;
std::uint32_t settings_calls = 0, item_calls = 0, item_price = 33, consumable_calls = 0;
std::uint32_t shop_gold = 100;
std::array<std::uint32_t, 4> shop_consumables{};
std::array<std::array<std::optional<cambc::champion_draft>, 3>, 2> draft_picks{};
std::int32_t mock_state = 0;
std::int32_t mock_cooldown = 2;
std::int32_t ward_cooldown = 3;
bool shop_allowed = true, consumable_shop_allowed = false, sell_allowed = false;
std::uint32_t ward_queries = 0;
std::uint32_t draft_reads = 0;
std::uint32_t draft_submissions = 0;
std::int32_t draft_submission_error = 0;
std::array<std::byte, 4> mock_draft_memory{std::byte{'S'}, std::byte{'D'}, std::byte{'K'}, std::byte{'!'}};
cambc::vec2 mock_position{ 5, 6 };
cambc::vec2 cast_position{};
std::uint64_t visible_tiles{};
bool camps_alive = true;
std::int32_t mock_champion_team = 0, mock_my_team = 0;
std::int32_t mock_ward_timer = 9;
std::uint8_t mock_ward_disabled = 1;
std::int32_t ward_at_error = 0;
bool teleports_active = true;
std::array<std::byte, 4> mock_shared_memory{};
std::int32_t shared_memory_error = 0;
std::uint32_t shared_memory_reads = 0, shared_memory_writes = 0;
std::optional<std::int32_t> camp_timer;
constexpr std::array<std::uint32_t, 3> camp_handles{42, 100, 15};
constexpr std::array<cambc::detail::camp_info, 3> camp_specs{{{1, 1, 0}, {4, 1, 1}, {5, 1, 2}}};

std::uint64_t tile_bit(cambc::vec2 position) { return std::uint64_t{1} << (position.y * 8 + position.x); }
int position_error(cambc::vec2 position) {
	if (position.x < 0 || position.y < 0 || position.x >= 8 || position.y >= 8)
		return static_cast<int>(cambc::error_code::api_bad_position);
	return (visible_tiles & tile_bit(position)) ? 0 : static_cast<int>(cambc::error_code::api_not_in_vision);
}
} // namespace

namespace cambc::detail
{
extern "C"
{
	std::int32_t sdk_get_settings(game_settings* out)
	{
		++settings_calls;
		*out = {15, 2, 60, 5, 0.5, 0, 1, 1, 2000, 1000, 50, 10, 0, 1.0f, 1, 1, 4, 1024, 1, 4, -1, 6.0f, 2.0f, 6.0f, 2.0f, 3, 3};
		return 0;
	}
	std::int32_t sdk_item_info(std::uint32_t type, item_info* out)
	{
		++item_calls;
		if (query_error != 0) return query_error;
		if (type >= static_cast<std::uint32_t>(item_type::count))
			return static_cast<std::int32_t>(error_code::invalid_action_buy_item_invalid_item);
		*out = type % 2 == 0 ? item_info{item_price, 1, 0, 1, 0, 0, 0.0f, 5, type + 1, 165}
			: item_info{165, 0, 0, 5, 0, 1, 1.0f, 0, 42, 0};
		return 0;
	}
	std::int32_t sdk_consumable_info(std::uint32_t type, consumable_info* out)
	{
		++consumable_calls;
		if (type != 0 && type != 2)
			return static_cast<std::int32_t>(error_code::invalid_action_buy_consumable_invalid_type);
		const auto index = type == 0 ? 0u : 1u;
		*out = { 50 + index, 1 + index };
		return 0;
	}
	std::int32_t sdk_bc_metadata(metadata* out) { ++metadata_calls; *out = {8, 8, 8, 8, 1, 8, 128}; return 0; }
	std::int32_t sdk_shared_memory_size(std::uint32_t* out) { ++memory_size_calls; *out = 4; return 0; }
	std::int32_t sdk_bc_map_symmetry(std::int32_t* out) { ++symmetry_calls; *out = 2; return 0; }
	std::int32_t sdk_bc_map_mask(std::int32_t kind, std::uint64_t* out) { ++terrain_calls; *out = std::uint64_t{1} << kind; return 0; }
	std::int32_t sdk_my_team() { return mock_my_team; }
	std::int32_t sdk_spawn_region(std::int32_t team, aabb* out) { ++spawn_calls; *out = team == 0 ? aabb{0, 0, 1, 1} : aabb{7, 7, 8, 8}; return 0; }
	std::int32_t sdk_draft_num_rounds(std::uint8_t* out) { *out = 3; return 0; }
	std::int32_t sdk_draft_memory_size(std::uint32_t* out) { *out = 4; return 0; }
	std::int32_t sdk_read_draft_memory(std::int32_t offset, void* out, std::uint32_t size) {
		assert(offset == 0 && size == 4); ++draft_reads;
		std::memcpy(out, "SDK!", size); return 0;
	}
	std::int32_t sdk_bc_submit_draft_memory(const void* bytes, std::size_t size) {
		++draft_submissions;
		if (draft_submission_error != 0) return draft_submission_error;
		const auto written = std::min(size, mock_draft_memory.size());
		if (written != 0) std::memcpy(mock_draft_memory.data(), bytes, written);
		return 0;
	}
	std::int32_t sdk_champion_items(std::uint32_t, std::uint32_t*) { std::abort(); }
	std::int32_t sdk_champion_consumables(std::uint32_t handle, std::uint32_t* out) {
		assert(handle == 0); std::copy(shop_consumables.begin(), shop_consumables.end(), out); return 0;
	}
	std::int32_t sdk_champion_gems(std::uint32_t, std::uint32_t*) { std::abort(); }
	std::int32_t sdk_champion_gold(std::uint32_t handle, std::uint32_t* out) {
		assert(handle == 0); *out = shop_gold; return 0;
	}
	std::int32_t sdk_bc_champion_vision(std::uint32_t, std::uint64_t*) { std::abort(); }
	std::int32_t sdk_champion_is_alive(std::uint32_t, std::int32_t*) { std::abort(); }
	std::int32_t sdk_use_consumable(std::uint32_t, std::int32_t, std::int32_t, std::int32_t* out) { *out = action_successful; return action_error; }
	std::int32_t sdk_place_ward(std::int32_t, std::int32_t, std::int32_t* out) { *out = action_successful; return action_error; }
	std::int32_t sdk_Bitboard_Test(const std::uint64_t* board, std::int32_t x, std::int32_t y, std::int32_t* out) {
		assert(x >= 0 && y >= 0 && x < 8 && y < 8); *out = (*board & tile_bit({x, y})) != 0; return 0;
	}
	std::int32_t sdk_Bitboard_Zero(std::uint64_t* out) { *out = 0; return 0; }
	std::int32_t sdk_champion_design(std::int32_t type, champion_design* out) {
		assert(type >= 0 && type < 13); *out = {};
		out->definition.passive_type = static_cast<std::uint8_t>(type % 10);
		out->definition.type = type; out->definition.abilities[0].kind = 2;
		out->definition.abilities[0].payload.damage = {4, 2};
		out->base_stats.base_health = 200; return 0;
	}
	std::int32_t sdk_Bitboard_DrawChebyshev(std::uint64_t* out, std::int32_t x, std::int32_t y, float radius) {
		for (int yy = 0; yy < 8; ++yy) for (int xx = 0; xx < 8; ++xx)
			if (std::max(std::abs(xx - x), std::abs(yy - y)) <= radius) *out |= tile_bit({xx, yy});
		return 0;
	}
	std::int32_t sdk_attack(std::uint32_t kind, std::uint32_t target) { cast_kind = kind; cast_target = target; return query_error; }
	std::int32_t sdk_champion_wards(std::uint32_t handle, ward_resources* out) {
		assert(handle == 0); ++ward_queries; *out = {1, ward_cooldown}; return 0;
	}
	std::int32_t sdk_champion_ward_positions(std::uint32_t handle, position* out, std::uint32_t capacity, std::uint32_t* count) {
		assert(handle == 0); ++ward_queries; *count = 1;
		if (capacity) out[0] = {2, 1}; return 0;
	}
	std::int32_t sdk_champion_can_shop(std::uint32_t handle, std::int32_t consumable, std::int32_t* out) {
		assert(handle == 0); *out = consumable ? consumable_shop_allowed : shop_allowed; return 0;
	}
	std::int32_t sdk_champion_can_sell(std::uint32_t handle, std::int32_t* out) {
		assert(handle == 0); *out = sell_allowed; return 0;
	}
	std::int32_t sdk_champion_channel_time(std::uint32_t, std::int32_t* out) {
		if (!(mock_state & (2 | 4 | 1024))) return static_cast<int>(error_code::invalid_action);
		*out = 5; return query_error;
	}
	std::int32_t sdk_hypothetical_vision(std::int32_t type, std::int32_t x, std::int32_t y, std::uint64_t* out) {
		assert(type >= 0 && type < 13); if (x < 0 || y < 0 || x >= 8 || y >= 8) return static_cast<int>(error_code::api_bad_position);
		*out = tile_bit({x, y}); return 0;
	}
	std::int32_t sdk_ally_ward_occupancy(std::uint64_t* out) { *out = visible_tiles & tile_bit({2, 1}); return 0; }
	std::int32_t sdk_enemy_ward_occupancy(std::uint64_t* out) { *out = visible_tiles & (tile_bit({4, 1}) | tile_bit({2, 1})); return 0; }


	std::int32_t sdk_ward_at(std::int32_t x, std::int32_t y, std::uint32_t* out) {
		if (ward_at_error) return ward_at_error;
		if (const auto code = position_error({x, y})) return code;
		if (y != 1 || (x != 2 && x != 4)) return static_cast<int>(error_code::api_not_found);
		*out = x; return 0;
	}
	std::int32_t sdk_ally_ward_at(std::int32_t x, std::int32_t y, std::uint32_t* out) {
		if (ward_at_error) return ward_at_error;
		if (const auto code = position_error({x, y})) return code;
		if (x != 2 || y != 1) return static_cast<int>(error_code::api_not_found);
		*out = x; return 0;
	}
	std::int32_t sdk_enemy_ward_at(std::int32_t x, std::int32_t y, std::uint32_t* out) {
		if (ward_at_error) return ward_at_error;
		if (const auto code = position_error({x, y})) return code;
		if ((x != 4 && x != 2) || y != 1) return static_cast<int>(error_code::api_not_found);
		*out = x == 2 ? 5u : static_cast<std::uint32_t>(x); return 0;
	}



	std::int32_t sdk_ward(std::uint32_t handle, ward_info* out) {
		if (const auto code = position_error({static_cast<int>(handle), 1})) return code;
		*out = {handle == 2 ? 0u : 1u, handle == 5 ? 2 : static_cast<int>(handle), 1, mock_ward_timer, mock_ward_disabled}; return 0;
	}

	std::int32_t sdk_wards(ward_info* out, std::uint32_t capacity, std::uint32_t* count) {
		if (query_error) return query_error;
		const std::array values{ward_info{0, 2, 1, mock_ward_timer, mock_ward_disabled},
			ward_info{1, 4, 1, mock_ward_timer, mock_ward_disabled}, ward_info{1, 2, 1, 5, 0}};
		*count = 0;
		for (const auto& value : values) if (visible_tiles & tile_bit({value.x, value.y})) ++*count;
		if (!capacity) return 0;
		if (capacity < *count) return static_cast<int>(error_code::api_buffer_too_small);
		std::uint32_t i{};
		for (const auto& value : values) if (visible_tiles & tile_bit({value.x, value.y})) out[i++] = value;
		return 0;
	}

	std::int32_t sdk_ward_vision(std::int32_t x, std::int32_t y, std::uint64_t* out) {
		if (query_error) return query_error;
		if (x < 0 || y < 0 || x >= 8 || y >= 8) return static_cast<int>(error_code::api_bad_position);
		*out = tile_bit({x, y}); return 0;
	}

	std::int32_t sdk_teleport_occupancy(std::uint64_t* out) { *out = teleports_active ? visible_tiles & tile_bit({6, 1}) : 0; return 0; }
	std::int32_t sdk_has_teleport(std::int32_t x, std::int32_t y, std::int32_t* out) {
		if (const auto code = position_error({x, y})) return code;
		*out = teleports_active && vec2{x, y} == vec2{6, 1}; return 0;
	}
	std::int32_t sdk_teleports(teleport_info* out, std::uint32_t capacity, std::uint32_t* count) {
		*count = teleports_active && (visible_tiles & tile_bit({6, 1})) ? 1 : 0;
		if (capacity && *count) out[0] = {0, 2, 1, {6, 1}, 5}; return 0;
	}
	std::int32_t sdk_ally_altar_lock_occupancy(std::uint64_t* out) { *out = visible_tiles & tile_bit({1, 1}); return 0; }
	std::int32_t sdk_enemy_altar_lock_occupancy(std::uint64_t* out) { *out = visible_tiles & tile_bit({1, 1}); return 0; }
	std::int32_t sdk_num_allies(std::uint32_t* out) { *out = 1; return 0; }
	std::int32_t sdk_num_enemies(std::uint32_t* out) { *out = 1; return 0; }
	std::int32_t sdk_allies(std::uint32_t* out, std::uint32_t capacity, std::uint32_t* count) { assert(capacity == 1); out[0] = 0; *count = 1; return 0; }
	std::int32_t sdk_enemies(std::uint32_t* out, std::uint32_t capacity, std::uint32_t* count) { assert(capacity == 1); out[0] = 1; *count = 1; return 0; }
	std::int32_t sdk_read_shared_memory(std::int32_t offset, void* out, std::uint32_t size) {
		++shared_memory_reads;
		if (shared_memory_error != 0) return shared_memory_error;
		assert(offset >= 0 && offset + size <= mock_shared_memory.size()); std::memcpy(out, mock_shared_memory.data() + offset, size); return 0;
	}
	std::int32_t sdk_write_shared_memory(std::int32_t offset, const void* bytes, std::uint32_t size) {
		++shared_memory_writes;
		if (shared_memory_error != 0) return shared_memory_error;
		assert(offset >= 0 && offset + size <= mock_shared_memory.size()); std::memcpy(mock_shared_memory.data() + offset, bytes, size); return 0;
	}
	std::int32_t sdk_altar_lock(std::int32_t x, std::int32_t y, std::int32_t team, altar_lock_info* out) {
		if (const auto code = position_error({x, y})) return code;
		if (vec2{x, y} == vec2{1, 2} && team == 0) return static_cast<int>(error_code::api_not_found);
		if (vec2{x, y} != vec2{1, 1} && vec2{x, y} != vec2{1, 2}) return static_cast<int>(error_code::api_not_found);
		*out = {6, std::uint32_t(80 + team), std::uint32_t(team), x, y, std::uint32_t(3 - team)}; return 0;
	}
	std::int32_t sdk_altar_locks(altar_lock_info* out, std::uint32_t capacity, std::uint32_t* count) {
		*count = (visible_tiles & tile_bit({1, 1})) ? 2 : 0;
		for (std::uint32_t i = 0; i < std::min(capacity, *count); ++i) sdk_altar_lock(1, 1, i, &out[i]);
		return 0;
	}
	std::int32_t sdk_debug_dot(std::int32_t x, std::int32_t y, std::uint32_t rgb) { assert(x == 1 && y == 2 && rgb == 0xffaa00); return 0; }
	std::int32_t sdk_debug_line(std::int32_t x, std::int32_t y, std::int32_t xx, std::int32_t yy, std::uint32_t rgb) {
		assert(x == 1 && y == 2 && xx == 3 && yy == 4 && rgb == 0xffaa00); return 0;
	}
	std::int32_t sdk_draft_get(std::int32_t team, std::uint32_t round, std::int32_t* out) {
		const auto pick = draft_picks[team][round];
		if (!pick) return static_cast<int>(error_code::draft_unavailable);
		*out = static_cast<std::int32_t>(*pick); return 0;
	}
	std::int32_t sdk_Bitboard_Set(std::uint64_t* out, std::int32_t x, std::int32_t y, std::int32_t value) {
		const auto bit = tile_bit({x, y});
		*out = value ? *out | bit : *out & ~bit;
		return 0;
	}
	std::int32_t sdk_Bitboard_IsZero(const std::uint64_t* board, std::int32_t* out) { *out = *board == 0; return 0; }
	std::int32_t sdk_Bitboard_IsEqual(const std::uint64_t* a, const std::uint64_t* b, std::int32_t* out) { *out = *a == *b; return 0; }
	std::int32_t sdk_Bitboard_NumSetBits(const std::uint64_t* board, std::uint32_t* out) { *out = std::popcount(*board); return 0; }
	std::int32_t sdk_Bitboard_GetSetBits(const std::uint64_t* board, void* out, std::size_t capacity, std::uint32_t* count) {
		*count = 0;
		for (int y = 0; y < 8; ++y) for (int x = 0; x < 8; ++x)
			if ((*board & tile_bit({x, y})) && *count < capacity) static_cast<vec2*>(out)[(*count)++] = {x, y};
		return 0;
	}
	std::int32_t sdk_Bitboard_Intersect(const std::uint64_t* a, const std::uint64_t* b, std::uint64_t* out) { *out = *a & *b; return 0; }
	std::int32_t sdk_Bitboard_Union(const std::uint64_t* a, const std::uint64_t* b, std::uint64_t* out) { *out = *a | *b; return 0; }
	std::int32_t sdk_vision(std::uint64_t* out) { *out = visible_tiles; return 0; }
	std::int32_t sdk_ally_occupancy(std::uint64_t* out) { *out = mock_champion_team == mock_my_team ? visible_tiles & tile_bit(mock_position) : 0; return 0; }
	std::int32_t sdk_enemy_occupancy(std::uint64_t* out) { *out = mock_champion_team != mock_my_team ? visible_tiles & tile_bit(mock_position) : 0; return 0; }
	std::int32_t sdk_camp_occupancy(std::uint64_t* out) {
		*out = 0;
		if (camps_alive) for (const auto& spec : camp_specs) *out |= visible_tiles & tile_bit({spec.x, spec.y});
		return 0;
	}
	std::int32_t sdk_map_num_camp_groups(std::uint32_t* out) {
		++group_calls;
		if (sdk_my_champion() == -1) return static_cast<int>(error_code::api_not_initialized);
		*out = 2; return 0;
	}
	std::int32_t sdk_map_camp_groups(std::uint32_t* out, std::uint32_t capacity, std::uint32_t* count) {
		assert(capacity == 2); out[0] = 21; out[1] = 7; *count = 2; return 0;
	}
	std::int32_t sdk_map_camp_group_num_camps(std::uint32_t group, std::uint32_t* out) { *out = group == 21 ? 2 : 1; return 0; }
	std::int32_t sdk_map_camp_group_camps(std::uint32_t group, std::uint32_t* out, std::uint32_t capacity, std::uint32_t* count) {
		*count = group == 21 ? 2 : 1; assert(capacity == *count);
		if (group == 21) { out[0] = 42; out[1] = 100; } else out[0] = 15;
		return 0;
	}
	std::int32_t sdk_map_camp_group(std::uint32_t group, camp_group_info* out) { *out = {group, group * 10}; return 0; }
	std::int32_t sdk_map_camp_info(std::uint32_t handle, camp_info* out) {
		for (std::size_t i = 0; i < camp_handles.size(); ++i) if (handle == camp_handles[i]) { *out = camp_specs[i]; return 0; }
		return static_cast<int>(error_code::api_invalid_camp_info);
	}
	std::int32_t sdk_bc_camp_group_max_respawn(std::uint32_t, std::int32_t* out) { *out = 30; return 0; }
	std::int32_t sdk_camp_group_cooldown(std::uint32_t, std::int32_t* out) {
		if (!camp_timer) return static_cast<int>(error_code::api_camp_group_not_waiting_to_respawn);
		*out = *camp_timer; return 0;
	}
	std::int32_t sdk_camp_group_state(std::uint32_t, std::uint32_t* out) {
		*out = camp_timer ? 3 : camps_alive ? 1 : 2; return 0;
	}
	std::int32_t sdk_camp_buffs(std::uint32_t handle, camp_buff_data* out, std::uint32_t capacity, std::uint32_t* count) {
		++camp_buff_calls;
		camp_state state{}; if (const auto code = sdk_camp(handle, &state)) return code;
		assert(capacity >= 3); *count = 3;
		out[0] = {0, 75.5f, 4}; out[1] = {1, 0, 2}; out[2] = {2, 0, 1}; return 0;
	}
	std::int32_t sdk_camp(std::uint32_t handle, camp_state* out) {
		camp_info spec{}; const auto code = sdk_map_camp_info(handle, &spec);
		if (code) return code;
		if (const auto error = position_error({spec.x, spec.y})) return error;
		if (!camps_alive) return static_cast<int>(error_code::api_not_found);
		*out = {spec.x, spec.y, 25, 50, 12, 3, {1, 2}};
		return 0;
	}
	std::int32_t sdk_camp_design(std::int32_t, camp_design* out) {
		*out = {}; out->base_stats.base_health = 50;
		out->definition.attack_type = 1; out->base_stats.base_attack_range = 3;
		return 0;
	}
	std::int32_t sdk_has_camp(std::int32_t x, std::int32_t y, std::int32_t* out) {
		if (const auto error = position_error({x, y})) return error;
		*out = camps_alive && std::ranges::any_of(camp_specs, [=](const auto& spec) { return spec.x == x && spec.y == y; });
		return 0;
	}
	std::int32_t sdk_has_champion(std::int32_t x, std::int32_t y, std::int32_t* out) {
		if (const auto error = position_error({x, y})) return error;
		*out = mock_position == vec2{x, y}; return 0;
	}
	std::int32_t sdk_camp_at(std::int32_t x, std::int32_t y, std::uint32_t* out) {
		std::int32_t has{}; if (const auto code = sdk_has_camp(x, y, &has)) return code;
		if (!has) return static_cast<int>(error_code::api_not_found);
		for (std::size_t i = 0; i < camp_specs.size(); ++i) if (camp_specs[i].x == x && camp_specs[i].y == y) *out = camp_handles[i];
		return 0;
	}
	std::int32_t sdk_champion_at(std::int32_t x, std::int32_t y, std::uint32_t* out) {
		std::int32_t has{}; if (const auto code = sdk_has_champion(x, y, &has)) return code;
		if (!has) return static_cast<int>(error_code::api_not_found);
		*out = 0; return 0;
	}
	std::int32_t sdk_my_champion()
	{
		++my_champion_calls;
		return std::getenv("BC_TEST_DRAFT") ? -1 : 0;
	}

	std::int32_t sdk_sell(std::uint32_t item)
	{
		assert(item == static_cast<std::uint32_t>(item_type::monster_shard_of_strength));
		return ++sell_calls == sell_error_at ? 97 : 0;
	}

	std::int32_t sdk_champion(std::uint32_t handle, champion_info* out)
	{
		assert(handle == 0);
		*out = {};
		out->team = mock_champion_team;
		out->x = mock_position.x;
		out->y = mock_position.y;
		out->health = 70; out->max_health = 100; out->strength = 18; out->focus = 12;
		out->move_speed = 3; out->attack_range = mock_champion_attack_range; out->state = mock_state;
		return query_error;
	}

	std::int32_t sdk_move_to(std::int32_t x, std::int32_t y, std::int32_t* out)
	{
		++move_calls;
		*out = 0;
		if (action_error || !action_successful) return action_error;
		if (query_error)
			return query_error;
		mock_position = { x, y };
		*out = 1;
		return 0;
	}

	std::int32_t sdk_bc_champion_priority(std::uint32_t handle, std::int32_t next, std::int32_t* out)
	{
		assert(handle == 0);
		*out = next ? 23 : 17;
		return query_error;
	}

	std::int32_t sdk_bc_get_last_error(char* out, std::uint32_t capacity)
	{
		constexpr char message[] = "Target champion is not in vision.";
		if (capacity)
			std::memcpy(out, message, std::min<std::size_t>(capacity, sizeof(message) - 1));
		return sizeof(message) - 1;
	}

	std::int32_t sdk_champion_buff_data(std::uint32_t, buff_data* out, std::uint32_t capacity, std::uint32_t* count)
	{
		++champion_buff_calls;
		if (query_error)
			return query_error;
		assert(capacity >= 3);
		const buff_data values[] = { { 0, { .shield = { 4, 75.5f } } }, { 1, { .stun = { 2 } } }, { 2, { .enhanced_attack = { 1 } } } };
		*count = 0;
		for (std::uint32_t i = 0; i < 3; ++i)
			if (active_buffs & (1u << i))
				out[(*count)++] = values[i];
		return 0;
	}

	std::int32_t sdk_champion_restrictions(std::uint32_t, std::uint8_t* out)
	{
		if (query_error) return query_error;
		*out = mock_restrictions;
		return 0;
	}

	std::int32_t sdk_champion_cooldown(std::uint32_t, std::uint32_t, std::int32_t* out)
	{
		*out = mock_cooldown;
		return 0;
	}

	std::int32_t sdk_champion_ability_props(std::uint32_t, std::uint32_t, ability_props* out)
	{
		++ability_props_calls;
		*out = {};
		out->kind = mock_ability_kind;
		out->cooldown = 8;
		out->max_charges = 3;
		out->targets = mock_valid_targets;
		out->attributes = mock_attributes;
		switch (mock_ability_kind)
		{
		case 1:
			out->payload.enhanced_attack = 1;
			break;
		case 2:
			out->payload.damage = { 4, 2 };
			break;
		case 3:
			out->payload.aoe_damage = { 4, 2, 1.5f, 3 };
			break;
		case 4:
			out->payload.cc = { 4, 2, 3 };
			break;
		case 5:
			out->payload.heal = { 4, 2 };
			break;
		case 6:
			out->payload.dash_range = 4;
			break;
		case 7:
			out->payload.shield = { 4, 2, 3 };
			break;
		}
		return query_error;
	}

#define EFFECT(name, amount)                                                                                                               \
	std::int32_t sdk_champion_ability_##name(std::uint32_t, std::uint32_t, float* out)                                                     \
	{                                                                                                                                      \
		*out = amount;                                                                                                                     \
		return query_error;                                                                                                                \
	}                                                                                                                                      \
	std::int32_t sdk_champion_ability_##name##_against(std::uint32_t, std::uint32_t, std::uint32_t target, float* out)                     \
	{                                                                                                                                      \
		assert(target == 1);                                                                                                               \
		*out = amount / 2;                                                                                                                 \
		return query_error;                                                                                                                \
	}
	EFFECT(damage, 100.0f)
	EFFECT(heal, 50.0f)
	EFFECT(shield, 80.0f)
#undef EFFECT

	std::int32_t sdk_cast(std::uint32_t, std::uint32_t kind, std::uint32_t target, std::int32_t x, std::int32_t y, std::int32_t* out)
	{
		cast_target = target;
		cast_kind = kind;
		cast_position = { x, y };
		*out = action_successful;
		return action_error;
	}
}
} // namespace cambc::detail

template <class Exception, class F> void throws(F test)
{
	bool caught = false;
	try
	{
		test();
	}
	catch (const Exception&)
	{
		caught = true;
	}
	assert(caught);
}

static_assert(!std::is_convertible_v<std::uint32_t, cambc::handle_champion_t>);
static_assert(!std::is_convertible_v<cambc::handle_champion_t, std::uint32_t>);
static_assert(std::is_convertible_v<cambc::champion, cambc::handle_champion_t>);
template <class T>
concept has_get_handle = requires(T value) { value.get_handle(); };
static_assert(!has_get_handle<cambc::champion>);
template <class T>
concept has_get_next_priority = requires(T value) { value.get_next_priority(); };
static_assert(!has_get_next_priority<cambc::champion>);
static_assert(std::is_enum_v<cambc::buff_types>);
static_assert(std::is_same_v<decltype(cambc::self()), cambc::champion&>);
static_assert(noexcept(cambc::is_draft()));
static_assert(std::is_same_v<decltype(cambc::self().buffs()), cambc::buff_collection>);
static_assert(std::is_same_v<decltype(cambc::self().buffs().get<cambc::buff_stun>()), cambc::buff_stun>);
static_assert(std::is_same_v<decltype(cambc::self().buffs().get(cambc::buff_types::shield)), cambc::buff_t>);
template <class T> concept has_stored_buffs = requires(T value) { value.buff; };
template <class T> concept has_buff_refresh = requires(T value) { value.refresh(); } || requires(T value) { value.snapshot(); };
static_assert(!has_stored_buffs<cambc::unit> && !has_stored_buffs<cambc::champion> && !has_stored_buffs<cambc::camp>);
static_assert(!has_buff_refresh<cambc::buff_collection>);
static_assert(std::ranges::forward_range<cambc::buff_collection>);

void test_camp_vision()
{
	query_error = 0;
	mock_ability_kind = 3;
	mock_position = {5, 6};
	visible_tiles = tile_bit({1, 1}) | tile_bit(mock_position) | tile_bit({0, 0});
	const auto groups = cambc::map::camp_groups();
	assert(groups.size() == 2 && groups[0].camps().size() == 2 && groups[1].camps().size() == 1);
	assert(static_cast<std::uint32_t>(static_cast<cambc::handle_camp_group_t>(groups[0])) == 21);
	assert(groups[0].gold_value() == 210 && groups[0].gem_value() == 21 && groups[0].max_respawn_timer() == 30);
	for (const auto& group : groups)
	{
		assert(group.spawn_tiles().count() == group.camps().size());
		for (const auto position : group.spawn_tiles().positions())
			assert(std::ranges::any_of(group, [&](const auto& member) { return member.spawn_position() == position; }));
		for (const auto& camp : group) {
			assert(static_cast<cambc::handle_camp_group_t>(camp.camp_group()) == static_cast<cambc::handle_camp_group_t>(group));
			assert(static_cast<cambc::handle_camp_t>(cambc::map::camp(camp.spawn_position())) == static_cast<cambc::handle_camp_t>(camp));
		}
	}
	auto mask_copy = groups[0].spawn_tiles();
	mask_copy.clear();
	assert(mask_copy.is_empty() && groups[0].spawn_tiles().count() == 2);
	const auto& camp = cambc::vision::camp_at({1, 1});
	assert(static_cast<cambc::handle_camp_t>(camp) == static_cast<cambc::handle_camp_t>(groups[0].camps()[0]));
	const cambc::unit& unit = camp;
	assert(unit.health() == 25 && unit.max_health() == 50 && unit.position() == cambc::vec2(1, 1) && unit.is_visible());
	assert(unit.attack_range() == 3 && camp.attack_range() == camp.base_stats().base_attack_range);
	assert(camp.strength() == 12 && camp.focus() == 3);
	assert(camp.base_stats().base_health == 50 && camp.kind() == cambc::camp_type::wolf);
	const cambc::ability ability = cambc::detail::handle_access::make_ability(cambc::handle_champion_t{ 0 }, 0);
	ability.use(camp);
	assert(cast_kind == 2 && cast_target == 42);
	assert(cambc::vision::has_camp({1, 1}) && !cambc::vision::has_camp({4, 1}) && !cambc::vision::has_camp({0, 0}));
	assert(cambc::vision::has_champion(mock_position) && !cambc::vision::has_champion({1, 1}));
	assert(cambc::vision::camps().size() == 1 && cambc::vision::champions().size() == 1);
	const auto champion = cambc::vision::champion_at(mock_position);
	const auto previous_targets = mock_valid_targets;
	mock_valid_targets = 31;
	const auto fresh_champion = cambc::vision::champion_at(mock_position);
	assert(champion.abilities()[0].get<cambc::ability_aoe_damage>().valid_targets.bits() == previous_targets);
	assert(fresh_champion.abilities()[0].get<cambc::ability_aoe_damage>().valid_targets.bits() == 31);
	const auto champions = cambc::vision::champions();
	const auto camps = cambc::vision::camps();
	const auto visible_groups = cambc::vision::camp_groups();
	assert(champions[0].abilities()[0].get<cambc::ability_aoe_damage>().valid_targets.bits() == 31);
	mock_valid_targets = previous_targets;
	assert(cambc::vision::champions()[0].abilities()[0].get<cambc::ability_aoe_damage>().valid_targets.bits() == previous_targets);
	assert(champions[0].abilities()[0].get<cambc::ability_aoe_damage>().valid_targets.bits() == 31);
	throws<cambc::error>([] { (void)cambc::vision::camp_at({4, 1}); });
	throws<cambc::error>([] { (void)cambc::vision::camp_at({0, 0}); });
	throws<cambc::error>([] { (void)cambc::vision::champion_at({0, 0}); });
	throws<cambc::error>([] { (void)cambc::vision::champion_at({7, 7}); });
	for (const auto position : {cambc::vec2{-1, 0}, cambc::vec2{8, 0}}) {
		throws<cambc::error>([&] { (void)cambc::vision::camp_at(position); });
		throws<cambc::error>([&] { (void)cambc::vision::champion_at(position); });
		throws<cambc::error>([&] { (void)cambc::vision::has_camp(position); });
		throws<cambc::error>([&] { (void)cambc::vision::has_champion(position); });
	}
	assert(std::holds_alternative<cambc::map::camp_group_alive>(groups[0].state()));
	camp_timer = 7;
	assert(std::get<cambc::map::camp_group_respawning>(groups[0].state()).timer == 7);
	camp_timer = 0;
	assert(std::get<cambc::map::camp_group_respawning>(groups[0].state()).timer == 0);
	camps_alive = false;
	camp_timer.reset();
	assert(std::holds_alternative<cambc::map::camp_group_reward_available>(groups[0].state()));
	assert(!camp.is_visible() && cambc::vision::camps().empty() && !cambc::vision::has_camp({1, 1}));
	assert(cambc::vision::camp_groups().size() == 1);
	visible_tiles = 0;
	assert(!camp.is_visible() && cambc::vision::camps().empty() && cambc::vision::champions().empty());
	assert(cambc::vision::camp_groups().empty());
	assert(champions.size() == 1 && camps.size() == 1 && visible_groups.size() == 1);
	assert(static_cast<cambc::handle_champion_t>(champions[0]) == static_cast<cambc::handle_champion_t>(champion));
	assert(camps[0].spawn_position() == cambc::vec2(1, 1));
	assert(visible_groups[0].spawn_tiles().count() == 2);
	throws<cambc::error>([&] { (void)groups[0].state(); });
	camp_timer.reset();
	throws<cambc::error>([&] { (void)groups[0].state(); });
	visible_tiles = tile_bit({1, 1});
	assert(std::holds_alternative<cambc::map::camp_group_reward_available>(groups[0].state()));
	visible_tiles = 0;
	throws<cambc::error>([&] { (void)camp.health(); });
	assert(static_cast<cambc::handle_camp_t>(camp) == static_cast<cambc::handle_camp_t>(groups[0].camps()[0])
		&& static_cast<cambc::handle_camp_group_t>(camp.camp_group()) == static_cast<cambc::handle_camp_group_t>(groups[0]));
}

void test_draft_snapshots()
{
	draft_picks[0] = {cambc::champion_draft::ryth, std::nullopt, cambc::champion_draft::tavi};
	draft_picks[1] = {cambc::champion_draft::varg, std::nullopt, std::nullopt};
	auto first = cambc::draft::current_draft(cambc::team::ally);
	assert((first == std::vector{cambc::champion_draft::ryth, cambc::champion_draft::tavi}));
	first[0] = cambc::champion_draft::willow;
	assert(cambc::draft::current_draft(cambc::team::ally)[0] == cambc::champion_draft::ryth);
	draft_picks[0][1] = cambc::champion_draft::prism;
	assert((cambc::draft::current_draft(cambc::team::ally) == std::vector{cambc::champion_draft::ryth, cambc::champion_draft::prism, cambc::champion_draft::tavi}));
	assert((cambc::draft::current_draft(cambc::team::enemy) == std::vector{cambc::champion_draft::varg}));
	assert(first.size() == 2 && first[0] == cambc::champion_draft::willow);
}

void test_design_globals()
{
	static_assert(std::is_const_v<std::remove_reference_t<decltype(cambc::champion_designs)>>);
	assert(cambc::draft::team_size() == 3);
	assert(cambc::draft_memory.size() == 4 && std::memcmp(cambc::draft_memory.data(), "SDK!", 4) == 0);
	assert(draft_reads == 1);
	assert(cambc::champion_designs.size() == 12);
	for (std::size_t i = 0; i < cambc::champion_designs.size(); ++i)
	{
		const auto& design = cambc::champion_designs[i];
		assert(design.champion_name() == static_cast<cambc::champion_type>(i));
		assert(design.passive() == static_cast<cambc::passive_type>(i % 10));
		assert(!design.is_visible() && design.base_stats().base_health == 200);
		assert(design.abilities()[0].slot() == 0 && design.abilities()[1].slot() == 1);
		assert(design.abilities()[0].get<cambc::ability_damage>().range == 4);
		assert(design.abilities()[0].is(cambc::ability_kind::damage));
		assert(!design.abilities()[1].info());
		assert(design.visible_tiles_from({2, 2}).is_set({2, 2}));
		throws<cambc::error>([&] { (void)design.health(); });
		throws<cambc::error>([&] { (void)design.max_health(); });
		throws<cambc::error>([&] { (void)design.position(); });
		throws<cambc::error>([&] { (void)design.state(); });
		throws<cambc::error>([&] { (void)design.restrictions(); });
		throws<cambc::error>([&] { (void)design.is_alive(); });
		throws<cambc::error>([&] { (void)design.strength(); });
		throws<cambc::error>([&] { (void)design.focus(); });
		throws<cambc::error>([&] { (void)design.attack_range(); });
		throws<cambc::error>([&] { (void)design.attack_tiles(); });
		throws<cambc::error>([&] { (void)design.item_count(); });
	assert(design.consumables()[cambc::consumable_type::tp_crystal].kind() == cambc::consumable_type::tp_crystal);
		throws<cambc::error>([&] { (void)design.gem_count(); });
		throws<cambc::error>([&] { (void)design.visible_tiles(); });
		throws<cambc::error>([&] { (void)design.priority(); });
		throws<cambc::error>([&] { (void)design.remaining_channel_time(); });
		throws<cambc::error>([&] { (void)design.buffs(); });
		throws<cambc::error>([&] { (void)design.abilities()[0].cooldown(); });
		throws<cambc::error>([&] { (void)design.abilities()[0].is_ready(); });
		throws<cambc::error>([&] { (void)design.abilities()[0].valid_tiles(); });
		throws<cambc::error>([&] { (void)design.abilities()[0].damage_amount(); });
		throws<std::invalid_argument>([&] { design.abilities()[0].use(); });
		throws<std::invalid_argument>([&] { design.attack(design); });
				if (i) assert(design != cambc::champion_designs[0]);
	}
}

void test_new_game_api()
{
	query_error = 0; mock_state = 0; mock_position = {2, 2}; visible_tiles = ~std::uint64_t{};
	camps_alive = true; camp_timer.reset(); mock_ability_kind = 3;
	const cambc::champion owner = cambc::detail::handle_access::make_champion(cambc::handle_champion_t{ 0 }), other = cambc::detail::handle_access::make_champion(cambc::handle_champion_t{ 1 });
	(void)owner.health();
	(void)owner.max_health();
	assert(owner.strength() == 18);
	assert(owner.focus() == 12);
	assert(owner.attack_range() == 2);
	const cambc::unit& unit = owner;
	assert(unit.attack_range() == 2);
	assert(unit.strength() == 18 && unit.focus() == 12);
	const auto check_attack_tiles = [](const cambc::unit& value) {
		const auto mask = value.attack_tiles();
		const auto origin = value.position();
		const auto range = value.attack_range();
		for (auto y = 0; y < mask.height(); ++y)
			for (auto x = 0; x < mask.width(); ++x)
				assert(mask.is_set({x, y}) == (cambc::vec2{x, y}.chebyshev_distance(origin) <= range));
	};
	check_attack_tiles(unit);
	mock_champion_attack_range = 1;
	assert(unit.attack_range() == 1 && owner.attack_range() == 1);
	check_attack_tiles(owner);
	mock_position = {0, 0}; check_attack_tiles(owner); mock_position = {2, 2};
	mock_champion_attack_range = 2;
	assert(owner.passive() == cambc::passive_type::none);
	assert(owner.restrictions() == cambc::buff_restrictions{});
	const auto previous = owner.restrictions();
	mock_restrictions = 63;
	assert(owner.restrictions() == (cambc::buff_restrictions::move | cambc::buff_restrictions::attack |
		cambc::buff_restrictions::cast | cambc::buff_restrictions::shop | cambc::buff_restrictions::consumable | cambc::buff_restrictions::ward));
	assert((owner.restrictions() & cambc::buff_restrictions::move) == cambc::buff_restrictions::move);
	assert(previous == cambc::buff_restrictions{});
	for (std::uint8_t flag = 1; flag <= 32; flag <<= 1) {
		mock_restrictions = flag;
		assert(other.restrictions().bits() == flag);
	}
	mock_restrictions = 0;
	mock_state = 2;
	assert(owner.restrictions() == cambc::buff_restrictions{});
	mock_state = 0;
	query_error = 97;
	throws<cambc::error>([&] { (void)owner.strength(); });
	throws<cambc::error>([&] { (void)owner.attack_tiles(); });
	throws<cambc::error>([&] { (void)owner.restrictions(); });
	throws<cambc::error>([&] { (void)owner.passive(); });
	throws<cambc::error>([&] { (void)owner.abilities()[0].valid_tiles(); });
	query_error = 0;
	owner.attack(other); assert(cast_kind == 1 && cast_target == 1);
	const cambc::unit& champion_target = other;
	owner.attack(champion_target); assert(cast_kind == 1 && cast_target == 1);
	const auto camp = cambc::map::camp_groups()[0].camps()[0];
	assert(camp.is_wolf() == (camp.kind() == cambc::camp_type::wolf));
	assert(camp.is_elk() == (camp.kind() == cambc::camp_type::elk));
	assert(camp.is_gem_cluster() == (camp.kind() == cambc::camp_type::gem_cluster));
	assert(camp.is_gold_cluster() == (camp.kind() == cambc::camp_type::gold_cluster));
	assert(camp.is_monolith() == (camp.kind() == cambc::camp_type::monolith));
	check_attack_tiles(camp);
	owner.attack(camp); assert(cast_kind == 2 && cast_target == 42);
	const cambc::unit& camp_target = camp;
	owner.attack(camp_target); assert(cast_kind == 2 && cast_target == 42);
	throws<std::invalid_argument>([&] { other.attack(owner); });
	const auto camp_calls_before = camp_buff_calls;
	const auto camp_buffs = camp.buffs();
	assert(camp_buff_calls == camp_calls_before + 1);
	assert(camp_buffs.get<cambc::buff_shield>().value == 75.5f);
	assert(camp_buffs.get<cambc::buff_stun>().duration == 2);
	assert(std::get<cambc::buff_enhanced_attack>(camp_buffs.get(cambc::buff_types::enhanced_attack)).duration == 1);
	assert(camp_buffs.has_shield() && camp_buffs.has_stun() && camp_buffs.has_enhanced_attack());
	assert(camp_buffs.size() == 3 && camp_buff_calls == camp_calls_before + 1);
	visible_tiles = 0;
	assert(camp_buffs.size() == 3);
	throws<cambc::error>([&] { (void)camp.buffs(); });
	throws<cambc::error>([&] { (void)camp.attack_tiles(); });
	visible_tiles = ~std::uint64_t{};
	for (mock_ability_kind = 0; mock_ability_kind < 8; ++mock_ability_kind)
	{
		const cambc::ability sample = cambc::detail::handle_access::make_ability(owner, 0);
		assert(sample.is_attack_reset() == sample.is(cambc::ability_kind::attack_reset));
		assert(sample.is_damage() == sample.is(cambc::ability_kind::damage));
		assert(sample.is_aoe_damage() == sample.is(cambc::ability_kind::aoe_damage));
		assert(sample.is_cc() == sample.is(cambc::ability_kind::cc));
		assert(sample.is_heal() == sample.is(cambc::ability_kind::heal));
		assert(sample.is_dash() == sample.is(cambc::ability_kind::dash));
		assert(sample.is_shield() == sample.is(cambc::ability_kind::shield));
		if (mock_ability_kind == 0) { throws<std::bad_optional_access>([&] { (void)sample.valid_tiles(); }); continue; }
		if (mock_ability_kind == 1) { throws<std::invalid_argument>([&] { (void)sample.valid_tiles(); }); continue; }
		const auto range = std::visit([](const auto& value) -> float {
			if constexpr (requires { value.range; }) return value.range;
			else return 0;
		}, *sample.info());
		for (const auto position : sample.valid_tiles().positions())
			assert(position.chebyshev_distance(mock_position) <= range);
		assert(sample.valid_tiles().is_set(mock_position));
		mock_position = {7, 7}; assert(sample.valid_tiles().is_set(mock_position)); mock_position = {2, 2};
	}
	mock_ability_kind = 3;
	assert((owner.ward_positions() == std::vector{cambc::vec2{2, 1}}));
	assert(owner.ward_cooldown() == 3); ward_cooldown = 0; assert(owner.ward_cooldown() == 0);
	const auto queries = ward_queries;
	throws<std::invalid_argument>([&] { (void)other.ward_positions(); });
	throws<std::invalid_argument>([&] { (void)other.ward_cooldown(); });
	throws<std::invalid_argument>([&] { (void)other.ward_placement_tiles(); });
	assert(ward_queries == queries);
	for (const auto position : owner.ward_placement_tiles().positions())
		assert(position.chebyshev_distance(mock_position) <= cambc::constants::ward_placement_radius);
	assert(owner.ward_placement_tiles().is_set(mock_position));
	shop_allowed = true; consumable_shop_allowed = true; shop_gold = 100; shop_consumables.fill(0);
	assert(owner.can_buy(cambc::item_type::monster_shard_of_strength));
	assert(!owner.can_buy(cambc::item_type::monster_shard_of_strength, 0));
	shop_gold = 1; assert(!owner.can_buy(cambc::item_type::monster_shard_of_strength));
	shop_gold = 100; assert(!owner.can_buy(cambc::item_type::monster_emblem_of_strength));
	assert(!owner.can_buy(cambc::item_type{std::numeric_limits<std::uint32_t>::max()}));
	shop_gold = std::numeric_limits<std::uint32_t>::max();
	assert(!owner.can_buy(cambc::item_type::monster_shard_of_strength, std::numeric_limits<std::uint32_t>::max()));
	shop_gold = 100; assert(owner.can_buy(cambc::consumable_type::tp_crystal));
	shop_consumables[0] = 1; assert(!owner.can_buy(cambc::consumable_type::tp_crystal));
	assert(!owner.can_buy(cambc::consumable_type{std::numeric_limits<std::uint32_t>::max()}));
	shop_gold = 1; shop_consumables.fill(0); assert(!owner.can_buy(cambc::consumable_type::tp_crystal));
	shop_allowed = false; assert(!owner.can_buy(cambc::item_type::monster_shard_of_strength));
	consumable_shop_allowed = false; assert(!owner.can_buy(cambc::consumable_type::tp_crystal));
	assert(!owner.can_sell()); sell_allowed = true; assert(owner.can_sell());
	throws<std::invalid_argument>([&] { (void)other.can_buy(cambc::item_type::monster_shard_of_strength); });
	throws<std::invalid_argument>([&] { (void)other.can_buy(cambc::consumable_type::tp_crystal); });
	throws<std::invalid_argument>([&] { (void)other.can_sell(); });
	throws<cambc::error>([&] { (void)owner.remaining_channel_time(); });
	for (const auto channel : {2, 4, 1024}) { mock_state = channel; assert(owner.remaining_channel_time() == 5); }
	mock_state = 0;
	assert(cambc::vision::ward_tiles(cambc::team::ally).is_set({2, 1})
		&& cambc::vision::ward_tiles(cambc::team::enemy).is_set({4, 1}));
	visible_tiles = tile_bit({1, 1}); assert(cambc::vision::ward_tiles(cambc::team::enemy).is_empty());
	assert(cambc::vision::ward_tiles(cambc::team::ally).is_empty());
	const auto locks = cambc::vision::altar_locks(); assert(locks.size() == 2);
	const auto lock = cambc::vision::altar_lock_at({1, 1});
	assert(lock.position() == cambc::vec2(1, 1) && lock.team() == cambc::team::ally && lock.hits_remaining() == 3);
	assert(lock.is_ally() && !lock.is_enemy());
	visible_tiles |= tile_bit({1, 2});
	assert(cambc::vision::altar_lock_at({1, 2}).team() == cambc::team::enemy);
	assert(!cambc::vision::altar_lock_at({1, 2}).is_ally() && cambc::vision::altar_lock_at({1, 2}).is_enemy());
	assert(cambc::vision::has_altar_lock({1, 1}) && cambc::vision::has_altar_lock({1, 2}));
	owner.attack(lock); assert(cast_kind == 6 && cast_target == 80);
	visible_tiles = 0;
	assert(!cambc::vision::has_altar_lock({1, 1}) && cambc::vision::altar_locks().empty());
	assert(locks.size() == 2 && locks[0].hits_remaining() == 3);
	throws<cambc::error>([&] { (void)cambc::vision::altar_lock_at({1, 1}); });
	throws<cambc::error>([&] { (void)cambc::vision::has_altar_lock({-1, 0}); });
	visible_tiles = ~std::uint64_t{};
	assert(!cambc::vision::has_altar_lock({0, 0}));
	throws<cambc::error>([&] { (void)cambc::vision::altar_lock_at({0, 0}); });
	assert(owner.visible_tiles_from({3, 4}).is_set({3, 4}));
	cambc::bitboard at;
	owner.visible_tiles_from_into({3, 4}, at);
	assert(at == owner.visible_tiles_from({3, 4}));
	throws<cambc::error>([&] { (void)owner.visible_tiles_from({-1, 0}); });
	cambc::debug::draw_dot({1, 2}, 0xffaa00); cambc::debug::draw_line({1, 2}, {3, 4}, 0xffaa00);
}

void test_draft_memory_assignment()
{
	const auto initial = cambc::draft_memory;
	const auto pointer = cambc::draft_memory.data();
	const auto check = [&] {
		assert(cambc::draft_memory.size() == initial.size());
		assert(std::equal(cambc::draft_memory.begin(), cambc::draft_memory.end(), initial.begin(), initial.end()));
		assert(cambc::draft_memory.data() == pointer && draft_reads == 1);
	};
	if (!cambc::is_draft()) {
		const auto submissions = draft_submissions;
		try { cambc::draft_memory = {std::byte{99}}; assert(false); }
		catch (const cambc::error& error) { assert(error.code() == cambc::error_code::invalid_action); }
		assert(draft_submissions == submissions);
		check();
		return;
	}
	cambc::draft_memory = std::vector<std::uint8_t>{1, 2, 3, 4, 5};
	assert(cambc::draft_memory[3] == std::byte{'!'});
	check();
	const std::array<std::uint16_t, 2> words{0x1234, 0xabcd};
	cambc::draft_memory = words;
	assert(std::memcmp(cambc::draft_memory.data(), "SDK!", 4) == 0);
	check();
	cambc::draft_memory = std::span{words}.subspan(1);
	check();
	cambc::draft_memory = {std::byte{7}, std::byte{8}};
	check();
	const auto before_empty = mock_draft_memory;
	cambc::draft_memory = {};
	check();
	assert(mock_draft_memory == before_empty);
	cambc::draft_memory = std::span{cambc::draft_memory}.subspan(1);
	check();
	const auto& current = cambc::draft_memory;
	cambc::draft_memory = current;
	check();
	cambc::draft_memory = initial;
	assert(std::memcmp(cambc::draft_memory.data(), "SDK!", 4) == 0);
	check();
	const auto submissions = draft_submissions;
	draft_submission_error = 12345;
	try { cambc::draft_memory = {std::byte{99}}; assert(false); }
	catch (const cambc::error& error) { assert(static_cast<std::int32_t>(error.code()) == 12345); }
	assert(draft_submissions == submissions + 1);
	check();
	draft_submission_error = 0;
	throws<std::out_of_range>([] { (void)cambc::draft_memory[4]; });
	throws<std::out_of_range>([] { (void)cambc::draft_memory[std::size_t(-1)]; });
}

void test_shop_and_shared_memory()
{
    const cambc::item_type wrapped_item = cambc::item_type::enum_type::monster_shard_of_strength;
    const cambc::consumable_type wrapped_consumable = cambc::consumable_type::enum_type::tp_crystal;
    cambc::enum_map<cambc::item_type> items;
    cambc::enum_map<cambc::consumable_type> consumables;
    items[wrapped_item] = 2;
    consumables[wrapped_consumable] = 3;
    assert(items[cambc::item_type::enum_type::monster_shard_of_strength] == 2);
    assert(consumables[cambc::consumable_type::enum_type::tp_crystal] == 3);
    assert(wrapped_item.info().cost == 33 && wrapped_consumable.info().cost == 50);
	const auto item = cambc::item_type::monster_shard_of_strength.info();
	assert(item.cost == 33 && item.combine_count == 5 && item.result == cambc::item_type::monster_emblem_of_strength);
	assert(item.health_points == 0 && item.strength_points == 1 && item.focus_points == 0);
	assert(!item.passive && item.passive_magnitude == 0.0f && item.purchasable && item.cost * item.combine_count == 165);
	const auto kind = cambc::item_type::monster_shard_of_strength;
	assert(kind.cost() == item.cost && kind.combine_count() == item.combine_count && kind.result() == item.result);
	assert(kind.health_points() == item.health_points && kind.strength_points() == item.strength_points);
	assert(kind.focus_points() == item.focus_points && kind.passive() == item.passive);
	assert(kind.passive_magnitude() == item.passive_magnitude && kind.is_purchasable() == item.purchasable);
    const auto upgraded = cambc::item_type::monster_emblem_of_strength.info();
    assert(!upgraded.result && upgraded.passive == cambc::item_passive::monster);
    assert(upgraded.passive_magnitude == 1.0f && upgraded.strength_points == 5 && upgraded.cost == 165 && !upgraded.purchasable && upgraded.cost * upgraded.combine_count == 0);
    item_price = 99;
    assert(cambc::item_type::monster_shard_of_strength.info().cost == 99 && item.cost == 33);
    item_price = 33;
	const auto consumable = cambc::consumable_type::tp_crystal.info();
	assert(consumable.cost == 50 && consumable.carry_limit == 1);
	assert(cambc::consumable_type::tp_crystal.cost() == consumable.cost);
	assert(cambc::consumable_type::tp_crystal.carry_limit() == consumable.carry_limit);
    assert(consumable_calls == static_cast<std::size_t>(cambc::consumable_type::count));
    for (std::size_t i = 0; i < static_cast<std::size_t>(cambc::consumable_type::count); ++i)
    {
        const auto info = static_cast<cambc::consumable_type>(i).info();
        assert(info.cost == 50 + i && info.carry_limit == 1 + i);
    }
    try { (void)cambc::item_type::count.info(); assert(false); }
    catch (const cambc::error& e) { assert(e.code() == cambc::error_code::invalid_action_buy_item_invalid_item); }
    try { (void)cambc::consumable_type::count.info(); assert(false); }
    catch (const cambc::error& e) { assert(e.code() == cambc::error_code::invalid_action_buy_consumable_invalid_type); }
	const std::array<std::uint8_t, 4> bytes{1, 2, 3, 4};
	cambc::shared_memory.write(0, std::span{bytes});
	std::array<std::uint8_t, 4> restored{};
	cambc::shared_memory.read(0, std::span{restored});
	assert(restored == bytes);
	cambc::shared_memory[0] = std::byte{42};
	cambc::shared_memory[1] = cambc::shared_memory[0];
	assert(cambc::shared_memory[1] == 42);
	throws<std::out_of_range>([] { (void)cambc::shared_memory[4]; });
	throws<std::out_of_range>([&] { cambc::shared_memory.read(1, std::span{restored}); });
	throws<std::out_of_range>([&] { cambc::shared_memory.write(1, std::span{bytes}); });
	cambc::shared_memory.read(4, std::span<std::byte>{});
	cambc::shared_memory.write(4, std::span<const std::byte>{});
	const std::array<std::uint16_t, 2> words{0x1234, 0xabcd};
	std::array<std::uint16_t, 2> restored_words{};
	cambc::shared_memory.write(0, std::span{words});
	cambc::shared_memory.read(0, std::span{restored_words});
	assert(restored_words == words);
	const auto reads = shared_memory_reads, writes = shared_memory_writes;
	shared_memory_error = static_cast<std::int32_t>(cambc::error_code::invalid_action);
	throws<cambc::error>([] { (void)static_cast<std::uint8_t>(cambc::shared_memory[0]); });
	throws<cambc::error>([] { cambc::shared_memory[0] = 99; });
	throws<cambc::error>([&] { cambc::shared_memory.read(0, std::span{restored_words}); });
	throws<cambc::error>([&] { cambc::shared_memory.write(0, std::span{words}); });
	assert(std::memcmp(mock_shared_memory.data(), words.data(), sizeof(words)) == 0);
	cambc::shared_memory.read(4, std::span<std::byte>{});
	cambc::shared_memory.write(4, std::span<const std::byte>{});
	assert(shared_memory_reads == reads + 2 && shared_memory_writes == writes + 2);
	shared_memory_error = 0;
}

void test_dynamic_sensing()
{
	visible_tiles = ~std::uint64_t{};
	query_error = 0;
	const auto& owner = cambc::self();
	assert(owner.team() == cambc::team::ally);
	assert(owner.is_ally() && !owner.is_enemy());
	mock_champion_team = 1;
	assert(owner.team() == cambc::team::enemy);
	assert(!owner.is_ally() && owner.is_enemy());
	assert(cambc::vision::champions(cambc::team::ally).empty());
	assert(cambc::vision::champions(cambc::team::enemy).size() == 1);
	mock_my_team = 1;
	assert(owner.team() == cambc::team::ally);
	assert(owner.is_ally() && !owner.is_enemy());
	assert(cambc::vision::champions(cambc::team::enemy).empty());
	mock_champion_team = mock_my_team = 0;
	assert(cambc::vision::champions(cambc::team::ally).size() == 1 && cambc::vision::champions(cambc::team::enemy).empty());
	assert(cambc::champions().size() == 2);
	assert(static_cast<cambc::handle_champion_t>(cambc::champions(cambc::team::ally)[0]) == static_cast<cambc::handle_champion_t>(0));
	assert(static_cast<cambc::handle_champion_t>(cambc::champions(cambc::team::enemy)[0]) == static_cast<cambc::handle_champion_t>(1));
	throws<cambc::error>([] { (void)cambc::champion_designs[0].team(); });
	throws<cambc::error>([] { (void)cambc::champion_designs[0].is_ally(); });
	throws<cambc::error>([] { (void)cambc::champion_designs[0].is_enemy(); });
	throws<cambc::error>([] { (void)cambc::champion_designs[0].is_ally(); });
	throws<cambc::error>([] { (void)cambc::champion_designs[0].is_enemy(); });
	throws<std::invalid_argument>([] { (void)cambc::champions(static_cast<cambc::team>(2)); });
	throws<std::invalid_argument>([] { (void)cambc::vision::champions(static_cast<cambc::team>(2)); });
	assert(cambc::vision::ward_tiles().count() == 2);
	assert(cambc::vision::ward_tiles(cambc::team::ally).count() == 1);
	assert(cambc::vision::ward_tiles(cambc::team::enemy).count() == 2);
	assert(cambc::vision::ward_tiles(cambc::team::enemy).is_set({4, 1}));
	const auto wards = cambc::vision::wards();
	assert(wards.size() == 3 && wards.size() > cambc::vision::ward_tiles().count());
	assert(cambc::vision::wards(cambc::team::ally).size() == 1);
	assert(cambc::vision::wards(cambc::team::enemy).size() == 2);
	assert(wards[0].position() == wards[2].position() && wards[0].team() != wards[2].team());
	assert(wards[0].remaining_time() == 9);
	assert(wards[2].remaining_time() == 5);
	for (const auto& value : wards) {
		assert(cambc::vision::ward_tiles(value.team()).is_set(value.position()));
		assert(value.is_ally() == (value.team() == cambc::team::ally));
		assert(value.is_enemy() == (value.team() == cambc::team::enemy));
		assert(value.visible_tiles() == cambc::ward::visible_tiles_from(value.position()));
		cambc::bitboard into;
		value.visible_tiles_into(into);
		assert(into == value.visible_tiles());
	}
	throws<std::invalid_argument>([] { (void)cambc::vision::wards(static_cast<cambc::team>(2)); });
	query_error = static_cast<int>(cambc::error_code::api_not_initialized);
	throws<cambc::error>([] { (void)cambc::vision::wards(); });
	throws<cambc::error>([] { (void)cambc::ward::visible_tiles_from({4, 4}); });
	throws<cambc::error>([&] { (void)wards[0].visible_tiles(); });
	query_error = 0;
	const auto ordinary = cambc::vision::ward_at({2, 1});
	assert(ordinary.position() == cambc::vec2(2, 1) && ordinary.team() == cambc::team::ally);
	assert(ordinary.remaining_time() == 9);
	assert(ordinary.visible_tiles() == cambc::ward::visible_tiles_from(ordinary.position()));
	mock_ward_timer = 3; mock_ward_disabled = 0;
	assert(ordinary.remaining_time() == 9);
	assert(wards[0].remaining_time() == 9);
	const auto refreshed = cambc::vision::wards();
	assert(refreshed[0].remaining_time() == 3);
	assert(cambc::vision::ward_at({2, 1}, cambc::team::enemy).team() == cambc::team::enemy);
	assert(cambc::vision::ward_at({4, 1}, cambc::team::enemy).team() == cambc::team::enemy);
	assert(cambc::vision::ward_at({2, 1}, cambc::team::ally).team() == cambc::team::ally);
	assert(cambc::vision::has_ward({2, 1}, cambc::team::ally) && cambc::vision::has_ward({4, 1}, cambc::team::enemy));
	assert(cambc::vision::has_ward({2, 1}, cambc::team::enemy));
	assert(!cambc::vision::has_ward({5, 1}, cambc::team::ally));
	visible_tiles = tile_bit({1, 1});
	assert(!cambc::vision::has_ward({0, 0}, cambc::team::ally));
	visible_tiles = ~std::uint64_t{};
	ward_at_error = static_cast<int>(cambc::error_code::api_bad_position);
	throws<cambc::error>([&] { (void)cambc::vision::ward_at({2, 1}, cambc::team::ally); });
	throws<cambc::error>([&] { (void)cambc::vision::has_ward({2, 1}, cambc::team::ally); });
	ward_at_error = 0;
	assert(!cambc::vision::has_ward({0, 0}));
	throws<cambc::error>([] { (void)cambc::vision::ward_at({0, 0}); });
	assert(cambc::ward::visible_tiles_from({4, 4}).count() == 1);
	throws<cambc::error>([] { (void)cambc::ward::visible_tiles_from({-1, 0}); });
	throws<cambc::error>([] { (void)cambc::ward::visible_tiles_from({8, 0}); });
	assert(cambc::vision::teleport_tiles().count() == 1 && cambc::vision::has_teleport({6, 1}));
	assert(!cambc::vision::has_teleport({0, 0}));
	assert((cambc::vision::teleport_at({6, 1}).position() == cambc::vec2{6, 1}));
	throws<cambc::error>([] { (void)cambc::vision::teleport_at({0, 0}); });
	const auto teleports = cambc::vision::teleports();
	assert(teleports.size() == 1);
	const auto& tp = teleports[0];
	assert(tp.position() == cambc::vec2(6, 1) && tp.remaining_time() == 5);
	assert(tp.team() == cambc::team::enemy && static_cast<cambc::handle_champion_t>(tp.champion()) == static_cast<cambc::handle_champion_t>(0));
	assert(!tp.is_ally() && tp.is_enemy());
	assert(tp.visible_tiles().is_set(tp.position()));
	teleports_active = false;
	assert(cambc::vision::teleports().empty() && cambc::vision::teleport_tiles().is_empty());
	assert(teleports.size() == 1 && !cambc::vision::has_teleport({6, 1}));
	teleports_active = true;
	assert(cambc::vision::altar_lock_tiles().is_set({1, 1}));
	assert(cambc::vision::altar_lock_tiles(cambc::team::ally).is_set({1, 1})
		&& cambc::vision::altar_lock_tiles(cambc::team::enemy).is_set({1, 1}));
	assert(cambc::vision::altar_locks(cambc::team::ally).size() == 1 && cambc::vision::altar_locks(cambc::team::enemy).size() == 1);
	active_buffs = 2;
	const auto buffs = owner.buffs();
	active_buffs = 0;
	assert(buffs.has<cambc::buff_stun>());
	active_buffs = 7;
	mock_state = 1;
	const auto state = owner.state();
	mock_state = 4097;
	assert(state.contains(cambc::champion_state::alive) && !state.contains(cambc::champion_state::teleporting) && owner.state().contains(cambc::champion_state::teleporting));
	mock_state = 0;
	visible_tiles = 0;
	assert(cambc::vision::ward_tiles().is_empty() && !cambc::vision::has_ward({2, 1}));
	assert(cambc::vision::wards().empty());
	assert(cambc::vision::wards(cambc::team::ally).empty() && cambc::vision::wards(cambc::team::enemy).empty());
	assert(wards.size() == 3 && wards[0].position() == cambc::vec2(2, 1));
	assert(cambc::vision::teleport_tiles().is_empty() && cambc::vision::teleports().empty() && !cambc::vision::has_teleport({6, 1}));
	assert(cambc::vision::altar_lock_tiles().is_empty() && cambc::vision::altar_locks().empty());
	assert(ordinary.position() == cambc::vec2(2, 1));
	throws<cambc::error>([] { (void)cambc::vision::ward_at({2, 1}); });
	throws<cambc::error>([] { (void)cambc::vision::has_ward({-1, 0}); });
	throws<cambc::error>([] { (void)cambc::vision::has_teleport({-1, 0}); });
	visible_tiles = ~std::uint64_t{};
}

template <class Mask, std::size_t N>
void test_flag_type(std::array<Mask, N> flags)
{
	using bits_type = decltype(Mask{}.bits());
	static_assert(sizeof(Mask) == sizeof(bits_type) && alignof(Mask) == alignof(bits_type));
	static_assert(std::is_trivially_copyable_v<Mask> && std::is_standard_layout_v<Mask>);
	static_assert(!std::is_convertible_v<bits_type, Mask> && !std::is_convertible_v<Mask, bits_type>);
	const auto empty = Mask::empty();
	const auto unknown = static_cast<bits_type>(bits_type{1} << (sizeof(bits_type) * 8 - 1));
	const auto future = Mask::from_bits_retain(unknown);
	assert(empty.bits() == 0 && empty.is_empty() && empty.contains(empty));
	assert(future.bits() == unknown && future != empty);
	assert((~empty) == Mask::all());
	assert((~future) == Mask::all());
	auto combined = future;
	auto bits = unknown;
	for (std::size_t i = 0; i < flags.size(); ++i)
	{
		const Mask flag = flags[i];
		const auto expected = flag.bits();
		assert(expected != 0 && (expected & (expected - 1)) == 0);
		assert((combined.bits() & expected) == 0);
		assert(flag.bits() == expected && flag.contains(flag) && flag.contains(empty));
		assert(!empty.contains(flag) && !future.contains(flag));
		combined |= flag;
		bits |= expected;
		assert(combined.bits() == bits && combined.contains(flag) && combined.contains(future));
		assert((combined & flag) == flag && (future | flag) == (flag | future));
		assert((combined ^ combined) == empty);
	}
	assert(combined == Mask::from_bits_retain(bits));
	auto changed = combined;
	changed ^= future;
	assert(changed.bits() == static_cast<bits_type>(bits ^ unknown));
	changed &= future;
	assert(changed == empty);
	assert((flags.front() | flags.back()) == (Mask{flags.front()} | flags.back()));
	assert((flags.front() & flags.back()) == (Mask{flags.front()} & flags.back()));
	assert((flags.front() ^ flags.back()) == (Mask{flags.front()} ^ flags.back()));
	assert((~flags.front()) == ~Mask{flags.front()});
}

void test_logical_masks()
{
	using state = cambc::champion_state;
	test_flag_type<cambc::champion_state>(std::array{state::alive, state::recalling, state::submitting_gems,
		state::can_move, state::can_attack, state::completed_turn, state::recall_started,
		state::submit_gems_started, state::recall_cancelled, state::submit_gems_cancelled,
		state::teleporting, state::can_cast});
	using target = cambc::ability_target_mask;
	test_flag_type<cambc::ability_target_mask>(std::array{target::camps, target::enemies, target::allies, target::self, target::ground});	using restriction = cambc::buff_restrictions;
	test_flag_type<cambc::buff_restrictions>(std::array{restriction::move, restriction::attack, restriction::cast,
		restriction::shop, restriction::consumable, restriction::ward});
	assert(cambc::champion_state{}.is_dead() && !cambc::champion_state{state::alive}.is_dead());
}

void test_value_semantics()
{
	std::unordered_set<cambc::vec2> tiles{ { 1, 2 }, { 1, 2 }, { 2, 1 } };
	assert(tiles.size() == 2);
	static_assert(std::is_invocable_v<std::hash<cambc::champion>, const cambc::champion&>);
	static_assert(std::is_invocable_v<std::hash<cambc::camp>, const cambc::camp&>);
	static_assert(std::is_invocable_v<std::hash<cambc::map::camp_group>, const cambc::map::camp_group&>);
	assert((cambc::consumable_info{ 1, 2 } == cambc::consumable_info{ 1, 2 }));
	cambc::ability_damage a{}, b{};
	a.range = b.range = 3.0f;
	assert(a == b);
	b.max_cooldown = 1;
	assert(a != b);
	assert((cambc::buff_shield{ 2, 5.0f } == cambc::buff_shield{ 2, 5.0f }));
	assert((cambc::champion_stats{} == cambc::champion_stats{}));
}

void test_host_flag_decoding()
{
	const cambc::champion owner = cambc::detail::handle_access::make_champion(cambc::handle_champion_t{ 0 });
	for (std::uint32_t bit = 1; bit < 8192; bit <<= 1)
	{
		mock_state = static_cast<std::int32_t>(bit | 0x80000000u);
		const auto state = owner.state();
		assert(state.bits() == (bit | 0x80000000u));
		assert(state.contains(cambc::champion_state::from_bits_retain(bit)));
		assert(owner.can_move() == state.contains(cambc::champion_state::can_move));
		assert(owner.can_attack() == state.contains(cambc::champion_state::can_attack));
		assert(owner.can_cast() == state.contains(cambc::champion_state::can_cast));
		assert(owner.is_recalling() == state.contains(cambc::champion_state::recalling));
		assert(owner.is_submitting_gems() == state.contains(cambc::champion_state::submitting_gems));
		assert(owner.is_teleporting() == state.contains(cambc::champion_state::teleporting));
		assert(owner.has_completed_turn() == state.contains(cambc::champion_state::completed_turn));
	}
	mock_state = 1 << 11;
	mock_cooldown = 0;
	const auto ready_ability = cambc::detail::handle_access::make_ability(owner, 0);
	assert(ready_ability.is_ready());
	assert(ready_ability.is_ready() == (ready_ability.info().has_value() && !ready_ability.is_on_cooldown()));
	mock_cooldown = 2;
	assert(!ready_ability.is_ready());
	assert(ready_ability.is_ready() == (ready_ability.info().has_value() && !ready_ability.is_on_cooldown()));
	mock_state = 0;
	assert(!ready_ability.is_ready());
	mock_cooldown = 2;
	mock_restrictions = 0x80 | 32;
	assert(owner.restrictions().bits() == (0x80 | 32));
	assert(owner.restrictions().contains(cambc::buff_restrictions::ward));
	mock_restrictions = 0;
	mock_valid_targets = 0x80000000u | 31;
	static_assert(std::is_same_v<decltype(cambc::ability_properties{}.exclusive_with_movement), bool>);
	for (const std::uint32_t raw : {0u, 1u, 0x80000000u, 0x80000001u})
	{
		mock_attributes = raw;
		const cambc::ability ability = cambc::detail::handle_access::make_ability(cambc::handle_champion_t{ 0 }, 0);
		assert(ability.valid_targets().bits() == mock_valid_targets);
		const auto metadata = ability.get<cambc::ability_aoe_damage>();
		assert(metadata.valid_targets.bits() == mock_valid_targets);
		assert(metadata.exclusive_with_movement == bool(raw & 1));
	}
	mock_valid_targets = 6;
	mock_attributes = 8;
}

int main()
{
	test_logical_masks();
	test_value_semantics();
	if (!cambc::is_draft()) test_host_flag_decoding();
	assert(cambc::is_draft() == (std::getenv("BC_TEST_DRAFT") != nullptr));
	if (cambc::is_draft())
		throws<std::bad_optional_access>([] { (void)cambc::self(); });
	else
	{
		assert(&cambc::self() == &cambc::self());
		assert(static_cast<cambc::handle_champion_t>(cambc::self()) == cambc::handle_champion_t{0});
	}
	const auto initial_self_calls = my_champion_calls;
	(void)cambc::detail::cached_self.has_value();
	assert(my_champion_calls == initial_self_calls);
	assert(settings_calls == 1 && cambc::constants::capture_time == 15);
	assert(metadata_calls == 1 && terrain_calls == 4 && spawn_calls == 2 && symmetry_calls == 1 && memory_size_calls == 1);
	assert(&cambc::detail::layout() == &cambc::detail::cached_metadata);
	assert(&cambc::map::walls() == &cambc::map::cached_walls && &cambc::map::bushes() == &cambc::map::cached_bushes);
	assert(&cambc::map::holes() == &cambc::map::cached_holes && &cambc::map::altars() == &cambc::map::cached_altars);
	assert(&cambc::map::spawn_region(cambc::team::ally) == &cambc::map::cached_spawn_regions[0]);
	assert(&cambc::map::spawn_region(cambc::team::enemy) == &cambc::map::cached_spawn_regions[1]);
	assert(cambc::map::symmetry() == cambc::symmetry::rotational180 && cambc::shared_memory.size() == 4);
	assert(metadata_calls == 1 && terrain_calls == 4 && spawn_calls == 2 && symmetry_calls == 1 && memory_size_calls == 1);
	test_draft_snapshots();
	test_draft_memory_assignment();
	test_design_globals();
	test_shop_and_shared_memory();
	if (cambc::is_draft())
	{
		throws<std::bad_optional_access>([] { (void)cambc::self(); });
		assert(group_calls == 0);
		try { (void)cambc::map::camp_groups(); assert(false); }
		catch (const cambc::error& error) { assert(error.code() == cambc::error_code::api_not_initialized); }
		std::puts("draft SDK global initialization PASS");
		return 0;
	}
	assert(group_calls == 0);
	test_dynamic_sensing();
	const auto initial_calls = ability_props_calls;
	const cambc::champion owner = cambc::detail::handle_access::make_champion(cambc::handle_champion_t{ 0 }), target = cambc::detail::handle_access::make_champion(cambc::handle_champion_t{ 1 });
	assert(ability_props_calls == initial_calls + 4);
	assert(owner.prev_priority() == 17 && owner.priority() == 23);
	owner.sell(cambc::item_type::monster_shard_of_strength);
	assert(sell_calls == 1);
	owner.sell(cambc::item_type::monster_shard_of_strength, 3);
	assert(sell_calls == 4);
	owner.sell(cambc::item_type::monster_shard_of_strength, 0);
	assert(sell_calls == 4);
	throws<std::invalid_argument>([&] { target.sell(cambc::item_type::monster_shard_of_strength, 3); });
	assert(sell_calls == 4);
	sell_error_at = sell_calls + 2;
	throws<cambc::error>([&] { owner.sell(cambc::item_type::monster_shard_of_strength, 3); });
	assert(sell_calls == sell_error_at);
	owner.move({ 2, -3 });
	assert(mock_position == cambc::vec2(7, 3) && move_calls == 1);
	owner.move_absolute({ 1, 9 });
	assert(mock_position == cambc::vec2(1, 9) && move_calls == 2);
	owner.move({ -1, 2 });
	assert(mock_position == cambc::vec2(0, 11) && move_calls == 3);
	throws<std::invalid_argument>([&] { target.move({ 1, 0 }); });
	throws<std::invalid_argument>([&] { target.move_absolute({ 1, 0 }); });
	assert(move_calls == 3);
	query_error = 97;
	throws<cambc::error>([&] { (void)owner.prev_priority(); });
	throws<cambc::error>([&] { (void)owner.priority(); });
	throws<cambc::error>([&] { owner.move({ 1, 0 }); });
	assert(move_calls == 3);
	throws<cambc::error>([&] { owner.move_absolute({ 1, 0 }); });
	assert(move_calls == 4 && mock_position == cambc::vec2(0, 11));
	const cambc::champion unseen = cambc::detail::handle_access::make_champion(cambc::handle_champion_t{ 2 });
	assert(ability_props_calls == initial_calls + 6);
	assert(!unseen.abilities()[0].is(cambc::ability_kind::damage));
	assert(!unseen.abilities()[0].is(cambc::ability_types::damage));
	throws<cambc::error>([&] { (void)unseen.abilities()[0].info(); });
	query_error = 0;
	throws<cambc::error>([&] { (void)unseen.abilities()[0].info(); });
	const auto unseen_abilities = unseen.abilities();
	throws<cambc::error>([&] { (void)unseen_abilities[1].info(); });
	assert(ability_props_calls == initial_calls + 6);
	const cambc::champion observed = cambc::detail::handle_access::make_champion(unseen);
	assert(observed.abilities()[0].info().has_value());
	query_error = 102;
	throws<cambc::error>([] { (void)cambc::detail::handle_access::make_champion(cambc::handle_champion_t{ 2 }); });
	query_error = 0;
	const auto buff_calls_before = champion_buff_calls;
	const auto buffs = owner.buffs();
	assert(champion_buff_calls == buff_calls_before + 1);
	assert(buffs.has(cambc::buff_types::stun));
	assert(buffs.has<cambc::buff_stun>());
	assert(buffs.get<cambc::buff_stun>().duration == 2);
	assert(std::get<cambc::buff_stun>(buffs.get(cambc::buff_types::stun)).duration == 2);
	assert(buffs.get<cambc::buff_shield>().value == 75.5f);
	assert(buffs.get<cambc::buff_enhanced_attack>().duration == 1);
	auto saved = buffs.begin();
	auto copied = saved;
	assert(champion_buff_calls == buff_calls_before + 1);
	active_buffs = 0;
	assert(buffs.size() == 3 && buffs.has<cambc::buff_stun>());
	const auto empty_buffs = owner.buffs();
	assert(champion_buff_calls == buff_calls_before + 2);
	assert(empty_buffs.is_empty() && !empty_buffs.has(cambc::buff_types::stun));
	assert(!empty_buffs.has<cambc::buff_stun>());
	assert(std::get<cambc::buff_shield>(*saved).value == 75.5f);
	++saved;
	assert(std::get<cambc::buff_stun>(*saved).duration == 2);
	assert(std::holds_alternative<cambc::buff_shield>(*copied));
	throws<std::out_of_range>([&] { (void)empty_buffs.get(cambc::buff_types::stun); });
	throws<std::out_of_range>([&] { (void)empty_buffs.get<cambc::buff_stun>(); });
	throws<std::out_of_range>([&] { (void)buffs.get(cambc::buff_types::count); });
	active_buffs = 0b100;
	const auto sparse_buffs = owner.buffs();
	assert(sparse_buffs.size() == 1 && !sparse_buffs.has(cambc::buff_types::shield));
	assert(sparse_buffs.get<cambc::buff_enhanced_attack>().duration == 1);
	auto sparse = sparse_buffs.begin();
	assert(std::holds_alternative<cambc::buff_enhanced_attack>(*sparse));
	assert(++sparse == std::default_sentinel);
	active_buffs = 0b111;
	int count = 0;
	for (const auto& buff : buffs)
	{
		std::visit(
			[&](const auto& value)
			{
				assert(value.duration > 0);
				++count;
			},
			buff);
	}
	assert(count == 3);
	query_error = 97;
	assert(buffs.size() == 3 && buffs.get<cambc::buff_shield>().value == 75.5f);
	throws<cambc::error>([&] { (void)owner.buffs(); });
	query_error = 0;
	const auto abilities = owner.abilities();
	const auto& ability = abilities[0];
	assert(abilities.size() == 2);
	assert(&ability.info() == &ability.info());
	const auto cached_kind = ability.info()->index();
	const auto calls_before = ability_props_calls;
	mock_ability_kind = 0;
	assert(ability.info()->index() == cached_kind && ability_props_calls == calls_before);
	mock_ability_kind = 3;
	throws<std::out_of_range>([&] { (void)abilities.at(2); });
	for (std::uint32_t flags = 0; flags < 32; ++flags)
	{
		for (const auto extra : { 0u, 1u << 31 })
		{
			mock_valid_targets = flags | extra;
			assert(ability.valid_targets().bits() == mock_valid_targets);
			assert(ability.can_target_camps() == ((flags & 1) != 0));
			assert(ability.can_target_enemies() == ((flags & 2) != 0));
			assert(ability.can_target_allies() == ((flags & 4) != 0));
			assert(ability.can_target_self() == ((flags & 8) != 0));
			assert(ability.can_target_ground() == ((flags & 16) != 0));
		}
	}
	mock_ability_kind = 0;
	mock_valid_targets = 0;
	assert(ability.valid_targets().bits() == 0);
	assert(!ability.can_target_camps() && !ability.can_target_enemies() && !ability.can_target_allies() &&
		   !ability.can_target_self() && !ability.can_target_ground());
	mock_valid_targets = 6;
	for (mock_ability_kind = 0; mock_ability_kind < 8; ++mock_ability_kind)
	{
		const cambc::ability sample = cambc::detail::handle_access::make_ability(owner, 0);
		const auto& info = sample.info();
		assert(info.has_value() == (mock_ability_kind != 0));
		const auto calls = ability_props_calls;
		for (std::uint32_t kind = 0; kind <= 8; ++kind)
			assert(sample.is(static_cast<cambc::ability_kind>(kind)) == (mock_ability_kind != 0 && kind == mock_ability_kind));
		assert(ability_props_calls == calls);
		if (!info)
		{
			assert(!sample.is(cambc::ability_types::damage));
			throws<std::bad_optional_access>([&] { (void)sample.get<cambc::ability_damage>(); });
			throws<std::bad_optional_access>([&] { (void)sample.info(cambc::ability_kind::damage); });
			continue;
		}
		assert(info->index() == mock_ability_kind - 1);
		std::visit([](const auto& value)
				   { assert(value.max_cooldown == 8 && value.valid_targets.bits() == 6 && !value.exclusive_with_movement); }, *info);
	}
	mock_ability_kind = 8;
	throws<std::runtime_error>([&] { (void)cambc::detail::handle_access::make_ability(owner, 0); });
	mock_ability_kind = 3;
	assert(&ability.info(cambc::ability_kind::aoe_damage) == &ability.info().value());
	assert(std::get<cambc::ability_aoe_damage>(ability.info(cambc::ability_kind::aoe_damage)).radius == 2);
	assert(ability.get<cambc::ability_aoe_damage>().modifier == 1.5f);
	throws<std::bad_variant_access>([&] { (void)ability.get<cambc::ability_heal>(); });
	throws<std::bad_variant_access>([&] { (void)ability.info(cambc::ability_kind::heal); });
	assert(ability.damage_amount() == 100 && ability.damage_amount(target) == 50);
	assert(ability.heal_amount() == 50 && ability.heal_amount(target) == 25);
	assert(ability.shield_amount() == 80 && ability.shield_amount(target) == 40);
	ability.use(target);
	assert(cast_target == 1);
	assert(cast_kind == static_cast<std::uint32_t>(cambc::detail::entity_kind::champion));
	ability.use(target);
	assert(cast_target == 1 && cast_kind == static_cast<std::uint32_t>(cambc::detail::entity_kind::champion) && cast_position == cambc::vec2(-1, -1));
	ability.use();
	assert(cast_kind == 0 && cast_target == 0 && cast_position == cambc::vec2(-1, -1));
	ability.use(cambc::vec2{ 3, 4 });
	assert(cast_kind == 0 && cast_target == 0 && cast_position == cambc::vec2(3, 4));
    const auto check_action = [&](auto action, std::int32_t blocked_error) {
        action_successful = 1;
        assert(action());
        const auto position = mock_position;
        action_successful = 0;
        assert(!action());
        assert(mock_position == position);
        action_error = blocked_error;
        throws<cambc::error>([&] { action(); });
        action_error = 0;
        action_successful = 1;
    };
    check_action([&] { return owner.move_absolute({ 2, 3 }); }, 22);
    check_action([&] { return owner.move({ 1, 0 }); }, 22);
    check_action([&] { return owner.abilities()[0].use(cambc::vec2{ 2, 3 }); }, 32);
    check_action([&] { return owner.place_ward({ 2, 3 }); }, 117);
    check_action([&] { return owner.consumables()[cambc::consumable_type::tp_crystal].use({ 2, 3 }); }, 22);

	query_error = 97;
	assert(ability.info()->index() == cached_kind);
	assert(ability.is(cambc::ability_kind::aoe_damage));
	assert(!ability.is(cambc::ability_kind::damage));
	const cambc::ability unobserved = cambc::detail::handle_access::make_ability(owner, 0);
	throws<cambc::error>([&] { (void)unobserved.info(); });
	assert(!unobserved.is(cambc::ability_kind::damage));
	assert(!unobserved.is(cambc::ability_types::damage));
	throws<cambc::error>([&] { (void)ability.valid_targets(); });
	for (const auto helper : { &cambc::ability::can_target_camps, &cambc::ability::can_target_enemies, &cambc::ability::can_target_allies,
							  &cambc::ability::can_target_self, &cambc::ability::can_target_ground })
		throws<cambc::error>([&] { (void)(ability.*helper)(); });
	assert(buffs.has(cambc::buff_types::stun));
	assert(std::holds_alternative<cambc::buff_shield>(*buffs.begin()));
	throws<cambc::error>([&] { (void)owner.buffs(); });
	try
	{
		(void)ability.damage_amount(target);
		assert(false);
	}
	catch (const cambc::error& error)
	{
		assert(error.code() == cambc::error_code::api_not_in_vision);
		assert(error.message() == "Target champion is not in vision.");
	}
	throws<cambc::error>([&] { (void)ability.heal_amount(target); });
	test_camp_vision();
	test_new_game_api();
	std::puts("typed SDK tests PASS");
}

static_assert([] {
	auto m = cambc::buff_restrictions::empty();
	m.insert(cambc::buff_restrictions::move | cambc::buff_restrictions::ward);
	m.remove(cambc::buff_restrictions::move);
	m.toggle(cambc::buff_restrictions::cast);
	m.set(cambc::buff_restrictions::shop, true);
	m -= cambc::buff_restrictions::cast;
	return m == (cambc::buff_restrictions::ward | cambc::buff_restrictions::shop);
}());
static_assert([] {
	int n = 0;
	for (auto f : cambc::buff_restrictions::from_bits_retain(0x15))
		n += static_cast<int>(f.bits());
	return n;
}() == 0x15);
