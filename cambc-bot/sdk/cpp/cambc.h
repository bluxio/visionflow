/// @file
/// Bot-facing C++ API with immutable metadata initialised before the bot entry point.
/// Live queries return fresh values and propagate engine failures as cambc::error.
#pragma once

#include <algorithm>
#include <array>
#include <compare>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <functional>
#include <limits>
#include <memory>
#include <new>
#include <optional>
#include <span>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>
#include <variant>
#include <iterator>
#include <initializer_list>
#include <ranges>

namespace cambc::detail
{
/// Exact Game_Settings ABI record, copied once into immutable global storage.
struct game_settings
{
	/// Capture, camp sleep, respawn and recall durations in rounds.
	std::int32_t capture_time, camp_sleep_time, respawn_time, recall_time;
	/// Engine multiplier used to calculate gem-submission duration.
	double submit_gem_duration_multiplier;
	/// Base and minimum submission durations, and allowed altar range in tiles.
	std::int32_t submit_gem_base_duration, submit_gem_minimum_duration, submit_gems_range;
	/// Round limit, winning gem count, and gold rewards for kills, assists and passive income.
	std::uint32_t max_rounds, winning_gems, kill_gold, assist_gold, passive_gold;
	/// Engine spawn-healing parameter.
	float spawn_healing;
	/// Ward limits, memory capacities in bytes, and tile-reveal duration in rounds.
	std::uint32_t active_wards, ward_charges, shared_memory_bytes, draft_memory_bytes, tile_reveal_rounds;
	/// Ward recharge and ordinary-ward lifetime in rounds.
	std::int32_t ward_cooldown, ward_lifetime;
	/// Vision and placement radii in tiles for wards.
	float ward_vision_radius, ward_placement_radius, control_ward_vision_radius, control_ward_placement_radius;
	/// Number of hits required to destroy a newly placed altar lock.
	std::uint32_t altar_lock_hits;
	/// Retired teleport-scroll channel duration; unused.
	std::int32_t tp_scroll_time;
	/// Teleport-crystal channel duration in rounds.
	std::int32_t tp_crystal_time;
	/// Zero padding.
	std::uint32_t reserved;
};

#if defined(__wasm__)
/// Copy the exact Game_Settings ABI record into guest-owned storage.
extern "C" __attribute__((import_module("env"), import_name("game_settings"))) std::int32_t sdk_get_settings(game_settings*);
#else
/// Copy the exact Game_Settings ABI record into guest-owned storage.
extern "C" std::int32_t sdk_get_settings(game_settings*);
#endif

/// Throw cambc::error for a nonzero engine status; zero means success.
inline void check(std::int32_t code);

/// Engine settings captured during dynamic initialisation for this bot instance.
inline const game_settings cached_game_settings = []
{
	game_settings result{};
	check(sdk_get_settings(&result));
	return result;
}();
} // namespace cambc::detail

namespace cambc
{

/// Game balance values for the bundled engine. Durations are in rounds unless stated otherwise.
namespace constants
{
	/// Rounds required to capture a defeated camp group.
	inline const std::int32_t capture_time = detail::cached_game_settings.capture_time;

	/// Rounds without an in-range champion before an awake camp sleeps.
	inline const std::int32_t camp_sleep_time = detail::cached_game_settings.camp_sleep_time;

	/// Rounds before a dead champion respawns.
	inline const std::int32_t respawn_time = detail::cached_game_settings.respawn_time;

	/// Base recall channel duration in rounds, before passive modifiers.
	inline const std::int32_t recall_time = detail::cached_game_settings.recall_time;

	/// Rounds per submitted gem; the multiplied duration is rounded up.
	inline const double submit_gem_duration_multiplier = detail::cached_game_settings.submit_gem_duration_multiplier;

	/// Fixed rounds added to the gem submission duration.
	inline const std::int32_t submit_gem_base_duration = detail::cached_game_settings.submit_gem_base_duration;

	/// Minimum gem submission channel duration in rounds.
	inline const std::int32_t submit_gem_minimum_duration = detail::cached_game_settings.submit_gem_minimum_duration;

	/// Maximum Chebyshev distance in tiles from a champion to the submission altar.
	inline const std::int32_t submit_gems_range = detail::cached_game_settings.submit_gems_range;

	/// Round limit before the game resolves its winner using tiebreakers.
	inline const std::uint32_t max_rounds = detail::cached_game_settings.max_rounds;

	/// Submitted gems required for a team to win.
	inline const std::uint32_t winning_gems = detail::cached_game_settings.winning_gems;

	/// Base gold awarded to the champion credited with a kill.
	inline const std::uint32_t kill_gold = detail::cached_game_settings.kill_gold;

	/// Base gold pool split among champions credited with an assist.
	inline const std::uint32_t assist_gold = detail::cached_game_settings.assist_gold;

	/// Base gold granted to each champion during round upkeep.
	inline const std::uint32_t passive_gold = detail::cached_game_settings.passive_gold;

	/// Health restored each round to a living champion on its team spawn.
	/// Fraction of maximum health restored each round on spawn, in [0, 1].
	inline const float spawn_healing = detail::cached_game_settings.spawn_healing;

	/// Shared active placement limit for a champion's wards.
	inline const std::uint32_t active_wards = detail::cached_game_settings.active_wards;

	/// Capacity in bytes of each team's writable shared memory.
	inline const std::uint32_t shared_memory_bytes = detail::cached_game_settings.shared_memory_bytes;

	/// Capacity in bytes of each team's memory that freezes after draft.
	inline const std::uint32_t draft_memory_bytes = detail::cached_game_settings.draft_memory_bytes;

	/// Collision reveal duration including the current round; one expires at the next round.
	inline const std::uint32_t tile_reveal_rounds = detail::cached_game_settings.tile_reveal_rounds;

	/// Rounds required to recharge one free ward charge.
	inline const std::int32_t ward_cooldown = detail::cached_game_settings.ward_cooldown;

	/// Ward lifetime in rounds; -1 disables expiration.
	inline const std::int32_t ward_lifetime = detail::cached_game_settings.ward_lifetime;

	/// Vision radius in tiles for ordinary wards.
	inline const float ward_vision_radius = detail::cached_game_settings.ward_vision_radius;

	/// Maximum placement range in tiles for free wards.
	inline const float ward_placement_radius = detail::cached_game_settings.ward_placement_radius;

	/// Successful attacks required to destroy an altar lock.
	inline const std::uint32_t altar_lock_hits = detail::cached_game_settings.altar_lock_hits;
	/// Teleport-crystal channel duration in rounds.
	inline const std::int32_t tp_crystal_time = detail::cached_game_settings.tp_crystal_time;

	namespace passive_abilities
	{
		/// Prism: focus gained per carried gem.
		inline constexpr float prism_focus_per_gem = 0.2f;

		/// Varg: attack splash radius in tiles.
		inline constexpr float varg_splash_radius = 1.0f;

		/// Vesper Vale: additional gold as a percentage of base gold.
		inline constexpr std::uint32_t vesper_vale_bonus_gold_percent = 20;

		/// Ryth: strength gained per point of bonus health.
		inline constexpr float ryth_health_strength_ratio = 0.01f;

		/// Ren: retaliation damage as a fraction of focus.
		inline constexpr float ren_retaliation_focus_ratio = 0.2f;
	} // namespace passive_abilities
} // namespace constants

/// Strong champion identity; convert to an integer only at the host ABI boundary.
enum class handle_champion_t : std::uint32_t
{
};

/// Strong camp identity; convert to an integer only at the host ABI boundary.
enum class handle_camp_t : std::uint32_t
{
};

/// Strong identity of an engine-defined physical camp group.
enum class handle_camp_group_t : std::uint32_t
{
};

/// Select a typed ability or buff payload without constructing a payload value.
template <class T> struct type_tag_t
{
	/// The tagged type.
	using type = T;
};

/// Teams relative to the bot.
enum class team
{
	/// The bot's own team.
	ally,
	/// The opposing team.
	enemy
};

/// Get the other team.
/// @param side Team to flip.
[[nodiscard]] constexpr team opponent(team side) noexcept { return side == team::ally ? team::enemy : team::ally; }

/// Map symmetry used to reflect tiles.
enum class symmetry : std::int32_t
{
	/// Mirror image with tile x coordinates reversed.
	horizontal,
	/// Mirror image with tile y coordinates reversed.
	vertical,
	/// Rotated 180 degrees about the map centre, reversing both coordinates.
	rotational180
};

/// Physical camp types.
enum class camp_type : std::int32_t
{
	/// Wolf camp: 1,350 health, 20 strength; drops gold and gems.
	wolf,
	/// Elk camp: 2,400 health, 60 strength; drops more gold and gems than a wolf.
	elk,
	/// Gem cluster: 500 health, does not attack; drops gems only.
	gem_cluster,
	/// Gold cluster: 500 health, does not attack; drops gold only.
	gold_cluster,
	/// Monolith: 20,000 health, 200 strength and attack range 3; the largest gold and gem reward.
	monolith
};

/// Champion passive effect tags from the engine definition.
enum class passive_type : std::uint8_t
{
	/// No passive.
	none,
	/// Extra gold from every gold gain (Vesper Vale).
	bonus_gold,
	/// Buy equipment away from spawn and sell it on spawn (Tavi).
	remote_shop,
	/// Focus gained per carried gem (Prism).
	gem_focus,
	/// Basic attacks also damage enemies around the target (Varg).
	splash_attack,
	/// Strength gained from bonus health (Ryth).
	health_strength,
	/// Credited kills, including camp monsters, reset ability cooldowns (Vendetta).
	reset_cooldowns_on_kill,
	/// Damages an attacker whenever this champion takes damage (Ren).
	retaliation,
	/// Cannot be stunned (Marcella).
	stun_immunity,
};

/// Draft choices; Cassi and Issac share one pick.
enum class champion_draft : std::int32_t
{
	/// Melee champion with an area stun and an attack reset; bonus health adds strength.
	ryth = 0,
	/// Ranged champion with strong basic attacks, bonus gold and a short dash.
	vesper_vale,
	/// Melee champion whose kills reset cooldowns, with an attack reset and a dash.
	vendetta,
	/// One pick that drafts both Cassi (ranged caster) and Issac (melee); if either dies, both die.
	cassi_and_issac,
	/// Ranged spellcaster choosing between single-target damage with a stun and an area hit.
	stellar,
	/// Ranged spellcaster that gains focus from carried gems.
	prism,
	/// Melee champion immune to stuns, with a stunning enhanced attack and a ranged damage spell.
	marcella,
	/// Ranged champion that heals another ally or resets its basic attack.
	willow,
	/// Melee champion with retaliation damage, a targeted stun and a short dash.
	ren,
	/// Melee champion with splash basic attacks, an attack reset and a self-shield.
	varg,
	/// Ranged champion that shops in the field and has a strong stunning spell.
	tavi,
};

/// Engine error codes; numeric values mirror EngineC/Errors.h.
enum class error_code : std::int32_t
{
	/// No error.
	success = 0,
	/// A handle does not refer to a live engine object.
	invalid_handle = 1,
	/// The engine could not allocate memory.
	out_of_memory = 2,
	/// A requested file does not exist.
	file_not_found = 3,
	/// A required pointer argument was null.
	null_pointer = 4,
	/// An argument was invalid, misaligned or overlapping.
	invalid_argument = 5,
	/// A map file could not be parsed.
	map_parse_error = 6,
	/// A map file is malformed.
	invalid_map_file = 7,
	/// A map file has an unsupported version.
	invalid_map_version = 8,
	/// A replay file could not be parsed.
	replay_parse_error = 9,
	/// A replay file is malformed.
	invalid_replay_file = 10,
	/// A replay file has an unsupported version.
	invalid_replay_version = 11,
	/// The draft request is invalid.
	invalid_draft = 12,
	/// The draft is not in progress.
	draft_not_drafting = 13,
	/// The champion has already been picked.
	draft_duplicate_champion = 14,
	/// The draft request does not belong to the current pick round.
	draft_invalid_round = 15,
	/// Draft information is unavailable in the current phase.
	draft_unavailable = 16,
	/// The object has already been initialised.
	already_initialized = 17,
	/// The action is not allowed.
	invalid_action = 18,
	/// A memory write falls outside the buffer.
	invalid_action_memory_bad_range = 19,
	/// Draft memory is read-only after the draft.
	invalid_action_memory_draft_frozen = 20,
	/// The champion cannot move this turn.
	invalid_action_move_cant_move = 21,
	/// The destination is not a legal landing tile.
	invalid_action_move_bad_destination = 22,
	/// The caster cannot cast spells.
	invalid_action_spell_cast_invalid_source = 23,
	/// The ability slot does not exist.
	invalid_action_spell_cast_invalid_slot = 24,
	/// The slot has no ability.
	invalid_action_spell_cast_no_ability = 25,
	/// The champion cannot cast this turn.
	invalid_action_spell_cast_cant_cast = 26,
	/// The champion has already cast a spell this turn.
	invalid_action_spell_cast_already_cast = 27,
	/// The ability is on cooldown.
	invalid_action_spell_cast_on_cooldown = 28,
	/// The ability has no charges left.
	invalid_action_spell_cast_no_charges = 29,
	/// The target is not valid for this ability.
	invalid_action_spell_cast_bad_target = 30,
	/// The target is out of the ability's range.
	invalid_action_spell_cast_out_of_range = 31,
	/// The ability's destination tile is not legal.
	invalid_action_spell_cast_bad_destination = 32,
	/// The champion cannot act, for example while stunned.
	invalid_action_cant_act = 33,
	/// The action is not allowed while recalling.
	invalid_action_is_recalling = 34,
	/// The action is not allowed while tributing gems.
	invalid_action_is_submitting_gems = 35,
	/// The champion has already moved this turn.
	invalid_action_move_already_moved = 36,
	/// The destination is beyond the champion's movement range.
	invalid_action_move_out_of_range = 37,
	/// No legal path reaches the destination.
	invalid_action_move_unreachable = 38,
	/// The attacker cannot attack.
	invalid_action_attack_invalid_source = 39,
	/// The champion cannot attack this turn.
	invalid_action_attack_cant_attack = 40,
	/// The champion has already attacked this turn.
	invalid_action_attack_already_attacked = 41,
	/// The target cannot be attacked.
	invalid_action_attack_bad_target = 42,
	/// The target is out of attack range.
	invalid_action_attack_out_of_range = 43,
	/// The champion cannot start a recall now.
	invalid_action_recall_cant_channel = 44,
	/// The champion cannot start a tribute now.
	invalid_action_submit_gems_cant_channel = 45,
	/// The champion carries no gems.
	invalid_action_submit_gems_no_gems = 46,
	/// The position is not an altar.
	invalid_action_submit_gems_bad_altar = 47,
	/// The altar is out of tribute range.
	invalid_action_submit_gems_out_of_range = 48,
	/// Items can only be bought on spawn or while dead.
	invalid_action_buy_item_not_on_spawn = 49,
	/// The item does not exist.
	invalid_action_buy_item_invalid_item = 50,
	/// The purchase count is invalid.
	invalid_action_buy_item_invalid_count = 51,
	/// The champion cannot afford the purchase.
	invalid_action_buy_item_not_enough_gold = 52,
	/// The inventory has no room for the item.
	invalid_action_buy_item_inventory_full = 53,
	/// The purchase would overflow a stat.
	invalid_action_buy_item_stats_overflow = 54,
	/// Consumables can only be bought on spawn or while dead.
	invalid_action_buy_consumable_not_on_spawn = 55,
	/// The consumable does not exist.
	invalid_action_buy_consumable_invalid_type = 56,
	/// The purchase count is invalid.
	invalid_action_buy_consumable_invalid_count = 57,
	/// The champion already carries that consumable.
	invalid_action_buy_consumable_inventory_full = 58,
	/// The champion cannot afford the consumable.
	invalid_action_buy_consumable_not_enough_gold = 59,
	/// The consumable does not exist.
	invalid_action_use_consumable_invalid_type = 60,
	/// The champion does not carry that consumable.
	invalid_action_use_consumable_not_owned = 61,
	/// The ward trinket has no charge.
	invalid_action_place_ward_no_charges = 62,
	/// The champion cannot place a ward now.
	invalid_action_place_ward_cant_place = 63,
	/// The ward tile is not legal.
	invalid_action_place_ward_bad_destination = 64,
	/// The ward tile is out of placement range.
	invalid_action_place_ward_out_of_range = 65,
	/// Map editing: bad spawn-area position.
	map_set_team_aspawn_area_bad_position = 66,
	/// Map editing: bad bush position.
	map_add_bush_bad_position = 67,
	/// Map editing: bad bush position.
	map_remove_bush_bad_position = 68,
	/// Map editing: bad wall position.
	map_add_wall_bad_position = 69,
	/// Map editing: bad wall position.
	map_remove_wall_bad_position = 70,
	/// Map editing: bad hole position.
	map_add_hole_bad_position = 71,
	/// Map editing: bad hole position.
	map_remove_hole_bad_position = 72,
	/// Map editing: bad altar position.
	map_add_altar_bad_position = 73,
	/// Map editing: bad altar position.
	map_remove_altar_bad_position = 74,
	/// The bush query position is outside the map.
	map_get_bush_bad_position = 75,
	/// The wall query position is outside the map.
	map_get_wall_bad_position = 76,
	/// The hole query position is outside the map.
	map_get_hole_bad_position = 77,
	/// The altar query position is outside the map.
	map_get_altar_bad_position = 78,
	/// The camp query position is outside the map.
	map_get_camp_handle_at_bad_position = 79,
	/// Map editing: bad camp position.
	map_add_camp_bad_position = 80,
	/// Map editing: bad camp position.
	map_set_camp_bad_position = 81,
	/// The teleport destination is not a legal landing tile.
	invalid_action_champion_teleport_bad_destination = 82,
	/// A camp group's respawn tiles are blocked.
	invalid_action_respawn_group_blocked_spawn = 83,
	/// A camp teleport destination is not legal.
	invalid_action_camp_teleport_bad_destination = 84,
	/// Replay timeline: bad seek position.
	timeline_bad_seek_position = 85,
	/// Replay timeline: bad seek substep.
	timeline_bad_seek_substep = 86,
	/// No map is loaded.
	api_no_map = 87,
	/// A bitboard or distance-field position is outside the map.
	managed_resources_bad_position = 88,
	/// A radius argument is invalid.
	managed_resources_bad_radius = 89,
	/// Input and output buffers alias where they must not.
	managed_resources_aliasing = 90,
	/// A buffer was built for different map dimensions.
	managed_resources_metadata_mismatch = 91,
	/// A buffer was not created by the engine.
	managed_resources_opaque_origin = 92,
	/// The team is invalid.
	api_invalid_team = 93,
	/// The query is not allowed from this bot's perspective.
	api_invalid_perspective = 94,
	/// The API is not initialised.
	api_not_initialized = 95,
	/// The position is outside the map.
	api_bad_position = 96,
	/// The target is not in this champion's vision.
	api_not_in_vision = 97,
	/// Nothing matches the query.
	api_not_found = 98,
	/// The camp group is invalid.
	api_invalid_camp_group = 99,
	/// The camp information is invalid.
	api_invalid_camp_info = 100,
	/// The camp is invalid.
	api_invalid_camp = 101,
	/// The champion is invalid.
	api_invalid_champion = 102,
	/// The ward is invalid.
	api_invalid_ward = 103,
	/// The ability is invalid.
	api_invalid_ability = 104,
	/// The champion is not on this bot's team.
	api_not_teammate = 105,
	/// The camp group is not waiting to respawn.
	api_camp_group_not_waiting_to_respawn = 106,
	/// The champion is alive, so it has no respawn timer.
	api_champion_alive = 107,
	/// The champion type is invalid.
	api_invalid_champion_type = 108,
	/// The camp type is invalid.
	api_invalid_camp_type = 109,
	/// A value is out of range.
	api_value_out_of_range = 110,
	/// The output buffer is too small.
	api_buffer_too_small = 111,
	/// Only the champion running this bot may act for it.
	api_not_owner = 112,
	/// A recall has already been started this turn.
	invalid_action_recall_already_used = 113,
	/// A tribute has already been started this turn.
	invalid_action_submit_gems_already_used = 114,
	/// The champion is not channelling.
	invalid_action_cancel_channel_not_channeling = 115,
	/// A channel has already been cancelled this turn.
	invalid_action_cancel_channel_already_used = 116,
	/// An allied ward already occupies the tile.
	invalid_action_place_ward_ally_ward_present = 117,
	/// The position is not an altar.
	invalid_action_altar_lock_bad_altar = 118,
	/// The altar is out of altar-lock range.
	invalid_action_altar_lock_out_of_range = 119,
	/// The altar is already locked.
	invalid_action_altar_lock_already_locked = 120,
	/// The action is not allowed while teleporting.
	invalid_action_is_teleporting = 123,
	/// This channel cannot be cancelled.
	invalid_action_cancel_channel_not_cancellable = 124,
	/// The altar is locked by the enemy.
	invalid_action_submit_gems_altar_locked = 125,
	/// This ability cannot be cast after moving.
	invalid_action_spell_cast_already_moved = 126,
	/// The champion cannot move after casting this ability.
	invalid_action_move_exclusive_spell_cast = 127,
	/// Only Tavi can sell items.
	invalid_action_sell_not_tavi = 128,
	/// The item does not exist.
	invalid_action_sell_invalid_item = 129,
	/// The champion does not own that item.
	invalid_action_sell_not_owned = 130,
	/// An item has already been sold this turn.
	invalid_action_sell_already_sold = 131,
	/// The sale would overflow gold.
	invalid_action_sell_gold_overflow = 132,
	/// Items can only be sold on spawn.
	invalid_action_sell_not_on_spawn = 133,
};

/// Engine failure with its error code and a copied diagnostic.
class error : public std::runtime_error
{
public:
	/// Capture the engine diagnostic before another SDK call changes it.
	explicit error(std::int32_t code);

	/// Construct an engine error with its code and diagnostic message.
	explicit error(error_code code);

	/// Get the engine error code.
	[[nodiscard]] error_code code() const noexcept { return code_; }

	/// Get the engine diagnostic captured when this error was created.
	[[nodiscard]] const std::string& message() const noexcept { return message_; }

	/// Construct an error with an explicit code and message.
	error(error_code code, std::string message)
		: std::runtime_error("engine error " + std::to_string(static_cast<std::int32_t>(code)) + ": " + message), code_{ code },
		  message_{ std::move(message) }
	{
	}

private:
	error_code	code_;
	std::string message_;
};

/// Integer tile position or displacement.
class vec2
{
public:
	/// Integer horizontal and vertical coordinates, or displacement components.
	std::int32_t x{}, y{};

	/// Construct an integer tile vector; the default value is the origin.
	constexpr vec2() = default;

	/// Construct an integer tile vector; the default value is the origin.
	constexpr vec2(std::int32_t x, std::int32_t y) : x{ x }, y{ y } {}

	/// Compare by x, then by y.
	constexpr auto operator<=>(const vec2&) const = default;

	/// Unit vectors for the 8 standard directions; north is +y.
	static const vec2 north, north_east, east, south_east, south, south_west, west, north_west;

	/// Reflect the vector horizontally across the origin.
	[[nodiscard]] constexpr vec2 reflect_horizontal() const { return { -x, y }; }

	/// Reflect the vector vertically across the origin.
	[[nodiscard]] constexpr vec2 reflect_vertical() const { return { x, -y }; }

	/// Rotate the vector 180 degrees about the origin.
	[[nodiscard]] constexpr vec2 rotate_180() const { return { -x, -y }; }

	/// Return the vector unchanged.
	constexpr vec2 operator+() const { return *this; }

	/// Negate both components.
	constexpr vec2 operator-() const { return { -x, -y }; }

	/// Add a displacement in place.
	constexpr vec2& operator+=(vec2 b)
	{
		x += b.x;
		y += b.y;
		return *this;
	}

	/// Subtract a displacement in place.
	constexpr vec2& operator-=(vec2 b)
	{
		x -= b.x;
		y -= b.y;
		return *this;
	}

	/// Scale both components in place.
	constexpr vec2& operator*=(std::int32_t n)
	{
		x *= n;
		y *= n;
		return *this;
	}

	/// Divide both components in place, truncating towards zero.
	/// Throws std::invalid_argument when n is zero.
	constexpr vec2& operator/=(std::int32_t n)
	{
		if (n == 0)
			throw std::invalid_argument("division by zero");
		x /= n;
		y /= n;
		return *this;
	}

	/// Add two vectors.
	friend constexpr vec2 operator+(vec2 a, vec2 b) { return a += b; }

	/// Subtract b from a.
	friend constexpr vec2 operator-(vec2 a, vec2 b) { return a -= b; }

	/// Scale a vector by an integer.
	friend constexpr vec2 operator*(vec2 a, std::int32_t n) { return a *= n; }

	/// Scale a vector by an integer.
	friend constexpr vec2 operator*(std::int32_t n, vec2 a) { return a *= n; }

	/// Divide both components, truncating towards zero.
	/// Throws std::invalid_argument when n is zero.
	friend constexpr vec2 operator/(vec2 a, std::int32_t n) { return a /= n; }

	/// Dot and cross products use 64-bit arithmetic.
	/// @param b Other vector.
	[[nodiscard]] constexpr std::int64_t dot(vec2 b) const { return std::int64_t{ x } * b.x + std::int64_t{ y } * b.y; }

	/// Get the signed two-dimensional cross product with a 64-bit result.
	/// @param b Other vector.
	[[nodiscard]] constexpr std::int64_t cross(vec2 b) const { return std::int64_t{ x } * b.y - std::int64_t{ y } * b.x; }

	/// Get the squared Euclidean length.
	[[nodiscard]] constexpr std::int64_t length_squared() const { return dot(*this); }

	/// Rotate a displacement by a quarter turn.
	[[nodiscard]] constexpr vec2 rotate_cw() const { return { y, -x }; }

	/// Rotate the vector 90 degrees counterclockwise about the origin.
	[[nodiscard]] constexpr vec2 rotate_ccw() const { return { -y, x }; }

	/// Get the largest absolute coordinate difference.
	/// @param b Other vector.
	[[nodiscard]] constexpr std::int64_t chebyshev_distance(vec2 b) const
	{
		return std::max(abs(std::int64_t{ x } - b.x), abs(std::int64_t{ y } - b.y));
	}

private:
	/// Return the absolute value used by tile-distance calculations.
	static constexpr std::int64_t abs(std::int64_t n) { return n < 0 ? -n : n; }
};

/// (0, 1).
inline constexpr vec2 vec2::north{ 0, 1 };
/// (1, 1).
inline constexpr vec2 vec2::north_east{ 1, 1 };
/// (1, 0).
inline constexpr vec2 vec2::east{ 1, 0 };
/// (1, -1).
inline constexpr vec2 vec2::south_east{ 1, -1 };
/// (0, -1).
inline constexpr vec2 vec2::south{ 0, -1 };
/// (-1, -1).
inline constexpr vec2 vec2::south_west{ -1, -1 };
/// (-1, 0).
inline constexpr vec2 vec2::west{ -1, 0 };
/// (-1, 1).
inline constexpr vec2 vec2::north_west{ -1, 1 };

} // namespace cambc

#include "imports.h"

namespace cambc
{

/// Tag for constructing a bitboard without initialising its contents.
struct uninitialized_t
{
	/// Explicit so that `{}` cannot select the tag by accident.
	explicit uninitialized_t() = default;
};
/// Pass to `bitboard` to skip zeroing; read it only after every chunk is written.
inline constexpr uninitialized_t uninitialized{};

/// Aligned tile mask, empty by default.
/// Raw buffers are engine-chunked; linear buffers are row-major, least-significant bit first.
class bitboard
{
public:
	/// Allocate an empty mask.
	bitboard() : bitboard{ uninitialized } { clear(); }

	/// Allocate aligned storage without initialising it.
	explicit bitboard(uninitialized_t) : buffer_{ static_cast<std::uint64_t*>(::operator new(raw_buffer_size(), std::align_val_t{ 16 })) } {}

	/// Copy the raw buffer, including padding.
	bitboard(const bitboard& other) : bitboard{ uninitialized } { copy_from(other); }

	/// Copy another mask's tiles.
	bitboard& operator=(const bitboard& other)
	{
		if (this != &other)
		{
			bitboard copy{ other };
			swap(copy);
		}
		return *this;
	}

	/// Transfer board storage without querying the host.
	bitboard(bitboard&&) noexcept = default;
	/// Take over another mask's storage.
	bitboard& operator=(bitboard&&) noexcept = default;

	/// Get the map dimensions in tiles.
	[[nodiscard]] std::int32_t width() const { return detail::layout().width; }

	/// Get the map height in tiles.
	[[nodiscard]] std::int32_t height() const { return detail::layout().height; }

	/// Get the size of the engine-chunked buffer in bytes.
	[[nodiscard]] static std::size_t raw_buffer_size() { return detail::layout().board_bytes; }

	/// Get the size of the packed row-major buffer in bytes.
	[[nodiscard]] static std::size_t linear_buffer_size()
	{
		return (std::size_t(detail::layout().width) * detail::layout().height + 7) / 8;
	}

	/// Access aligned raw storage for SDK imports.
	[[nodiscard]] std::uint64_t* ptr() noexcept { return buffer_.get(); }

	/// Access contiguous backing storage; preserve its layout when passing it to the host.
	[[nodiscard]] const std::uint64_t* ptr() const noexcept { return buffer_.get(); }

	/// Exchange ownership of raw board storage without querying the host.
	/// @param other Bitboard to exchange contents with.
	void swap(bitboard& other) noexcept { buffer_.swap(other.buffer_); }

	/// Read a tile; throws if it is outside the map.
	/// @param p Tile to query.
	[[nodiscard]] bool is_set(vec2 p) const
	{
		std::int32_t value{};
		detail::check(detail::sdk_Bitboard_Test(ptr(), p.x, p.y, &value));
		return value != 0;
	}

	/// Set or clear a tile; throws if it is outside the map.
	/// @param p Tile to update.
	/// @param value `true` to set the tile, `false` to clear it.
	void set(vec2 p, bool value = true)
	{
		detail::check(detail::sdk_Bitboard_Set(ptr(), p.x, p.y, value));
	}

	/// Invert a tile; throws if it is outside the map.
	/// @param p Tile to flip.
	void toggle(vec2 p)
	{
		detail::check(detail::sdk_Bitboard_Toggle(ptr(), p.x, p.y));
	}

	/// Clear all storage, including padding bits.
	void clear() { detail::check(detail::sdk_Bitboard_Zero(ptr())); }

	/// Set every in-bounds tile bit; padding is handled by the engine.
	void fill() { detail::check(detail::sdk_Bitboard_One(ptr())); }

	/// Check whether all map tiles are clear.
	[[nodiscard]] bool is_empty() const
	{
		std::int32_t value{};
		detail::check(detail::sdk_Bitboard_IsZero(ptr(), &value));
		return value != 0;
	}

	/// Count set map tiles.
	[[nodiscard]] std::uint32_t count() const
	{
		std::uint32_t value{};
		detail::check(detail::sdk_Bitboard_NumSetBits(ptr(), &value));
		return value;
	}

	/// Get the positions of all set map tiles.
	[[nodiscard]] std::vector<vec2> positions() const
	{
		std::vector<vec2> positions(count());
		std::uint32_t	  count{};
		detail::check(detail::sdk_Bitboard_GetSetBits(ptr(), positions.data(), positions.size(), &count));
		positions.resize(count);
		return positions;
	}

	/// Print the buffer through the engine.
	void print() const { detail::check(detail::sdk_Bitboard_Print(ptr())); }

	/// Copy an engine-chunked buffer into this object.
	/// @param bytes Engine-chunked bytes, exactly `raw_buffer_size()` long.
	void import_raw_buffer(std::span<const std::uint8_t> bytes)
	{
		detail::check(detail::sdk_Bitboard_ImportRawBuffer(ptr(), bytes.data(), bytes.size()));
	}

	/// Copy raw storage to a caller-provided buffer.
	/// @param bytes Destination, exactly `raw_buffer_size()` long.
	void export_raw_buffer(std::span<std::uint8_t> bytes) const
	{
		detail::check(detail::sdk_Bitboard_ExportRawBuffer(ptr(), bytes.data(), bytes.size()));
	}

	/// Return a copy of raw storage.
	[[nodiscard]] std::vector<std::uint8_t> export_raw_buffer() const
	{
		std::vector<std::uint8_t> bytes(raw_buffer_size());
		export_raw_buffer(bytes);
		return bytes;
	}

	/// Import a packed row-major buffer.
	/// @param bytes One byte per tile in row-major order, exactly `linear_buffer_size()` long; nonzero sets the tile.
	void import_linear_buffer(std::span<const std::uint8_t> bytes)
	{
		detail::check(detail::sdk_Bitboard_ImportLinearBuffer(ptr(), bytes.data(), bytes.size()));
	}

	/// Export to a caller-provided row-major buffer.
	/// @param bytes Destination with one byte per tile in row-major order, exactly `linear_buffer_size()` long.
	void export_linear_buffer(std::span<std::uint8_t> bytes) const
	{
		detail::check(detail::sdk_Bitboard_ExportLinearBuffer(ptr(), bytes.data(), bytes.size()));
	}

	/// Return a packed row-major copy.
	[[nodiscard]] std::vector<std::uint8_t> export_linear_buffer() const
	{
		std::vector<std::uint8_t> bytes(linear_buffer_size());
		export_linear_buffer(bytes);
		return bytes;
	}

	/// Combine map tiles in place.
	bitboard& operator|=(const bitboard& b)
	{
		if (this != &b)
			detail::check(detail::sdk_Bitboard_Union(ptr(), b.ptr(), ptr()));
		return *this;
	}

	/// Keep only tiles also set in b.
	bitboard& operator&=(const bitboard& b)
	{
		if (this != &b)
			detail::check(detail::sdk_Bitboard_Intersect(ptr(), b.ptr(), ptr()));
		return *this;
	}

	/// Toggle every tile set in b.
	bitboard& operator^=(const bitboard& b)
	{
		if (this == &b)
			clear();
		else
			detail::check(detail::sdk_Bitboard_Xor(ptr(), b.ptr(), ptr()));
		return *this;
	}

	/// (const bitboard& a|Tiles set in either mask.
	friend bitboard operator|(const bitboard& a, const bitboard& b)
	{
		if (&a == &b)
			return a;
		bitboard result{ uninitialized };
		detail::check(detail::sdk_Bitboard_Union(a.ptr(), b.ptr(), result.ptr()));
		return result;
	}

	/// (bitboard&& a|Tiles set in either mask, reusing a's storage.
	friend bitboard operator|(bitboard&& a, const bitboard& b)
	{
		a |= b;
		return std::move(a);
	}

	/// Tiles set in both masks.
	friend bitboard operator&(const bitboard& a, const bitboard& b)
	{
		if (&a == &b)
			return a;
		bitboard result{ uninitialized };
		detail::check(detail::sdk_Bitboard_Intersect(a.ptr(), b.ptr(), result.ptr()));
		return result;
	}

	/// Tiles set in both masks, reusing a's storage.
	friend bitboard operator&(bitboard&& a, const bitboard& b)
	{
		a &= b;
		return std::move(a);
	}

	/// Tiles set in exactly one mask.
	friend bitboard operator^(const bitboard& a, const bitboard& b)
	{
		if (&a == &b)
		{
			bitboard result;
			return result;
		}
		bitboard result{ uninitialized };
		detail::check(detail::sdk_Bitboard_Xor(a.ptr(), b.ptr(), result.ptr()));
		return result;
	}

	/// Tiles set in exactly one mask, reusing a's storage.
	friend bitboard operator^(bitboard&& a, const bitboard& b)
	{
		a ^= b;
		return std::move(a);
	}

	/// Return the complement of the map tiles.
	[[nodiscard]] bitboard operator~() const
	{
		bitboard result{ uninitialized };
		detail::check(detail::sdk_Bitboard_Not(ptr(), result.ptr()));
		return result;
	}

	/// Check whether both masks contain the same map tiles.
	friend bool operator==(const bitboard& a, const bitboard& b)
	{
		if (&a == &b)
			return true;
		std::int32_t value{};
		detail::check(detail::sdk_Bitboard_IsEqual(a.ptr(), b.ptr(), &value));
		return value != 0;
	}

	/// Shift tiles horizontally; negative counts shift left.
	/// @param count Columns to shift; negative values shift left.
	[[nodiscard]] bitboard shift_right(std::int32_t count) const
	{
		bitboard result{ uninitialized };
		shift_right_into(count, result);
		return result;
	}

	/// Like `shift_right`, but writes into `out` instead of returning a new board.
	/// @param count Columns to shift; negative values shift left.
	/// @param out Destination; its previous contents are overwritten.
	void shift_right_into(std::int32_t count, bitboard& out) const
	{
		detail::check(detail::sdk_Bitboard_ShiftRight(ptr(), out.ptr(), count));
	}

	/// Shift tiles vertically; negative counts shift down.
	/// @param count Rows to shift; negative values shift down.
	[[nodiscard]] bitboard shift_up(std::int32_t count) const
	{
		bitboard result{ uninitialized };
		shift_up_into(count, result);
		return result;
	}

	/// Like `shift_up`, but writes into `out` instead of returning a new board.
	/// @param count Rows to shift; negative values shift down.
	/// @param out Destination; its previous contents are overwritten.
	void shift_up_into(std::int32_t count, bitboard& out) const
	{
		detail::check(detail::sdk_Bitboard_ShiftUp(ptr(), out.ptr(), count));
	}

	/// Return a reflected copy using the requested map symmetry.
	/// @param value Map symmetry to apply.
	[[nodiscard]] bitboard reflect(symmetry value) const
	{
		bitboard result{ uninitialized };
		reflect_into(value, result);
		return result;
	}

	/// Like `reflect`, but writes into `out` instead of returning a new board.
	/// @param value Map symmetry to apply.
	/// @param out Destination; its previous contents are overwritten.
	void reflect_into(symmetry value, bitboard& out) const
	{
		detail::check(detail::sdk_Bitboard_Reflect(ptr(), out.ptr(), static_cast<std::int32_t>(value)));
	}

	/// Reflect across the map horizontally, reversing tile x coordinates.
	[[nodiscard]] bitboard reflect_horizontal() const { return reflect(symmetry::horizontal); }
	/// Like `reflect_horizontal`, but writes into `out`.
	/// @param out Destination; its previous contents are overwritten.
	void reflect_horizontal_into(bitboard& out) const { reflect_into(symmetry::horizontal, out); }

	/// Reflect across the map vertically, reversing tile y coordinates.
	[[nodiscard]] bitboard reflect_vertical() const { return reflect(symmetry::vertical); }
	/// Like `reflect_vertical`, but writes into `out`.
	/// @param out Destination; its previous contents are overwritten.
	void reflect_vertical_into(bitboard& out) const { reflect_into(symmetry::vertical, out); }

	/// Rotate tiles 180 degrees within the map dimensions.
	[[nodiscard]] bitboard rotate_180() const { return reflect(symmetry::rotational180); }
	/// Like `rotate_180`, but writes into `out`.
	/// @param out Destination; its previous contents are overwritten.
	void rotate_180_into(bitboard& out) const { reflect_into(symmetry::rotational180, out); }

	/// Add tiles within a Euclidean radius.
	/// @param center Centre tile.
	/// @param radius Euclidean radius in tiles.
	void draw_circle(vec2 center, float radius) { detail::check(detail::sdk_Bitboard_DrawCircle(ptr(), center.x, center.y, radius)); }

	/// Add tiles within a Chebyshev radius.
	/// @param center Centre tile.
	/// @param radius Chebyshev radius in tiles.
	void draw_chebyshev(vec2 center, float radius) { detail::check(detail::sdk_Bitboard_DrawChebyshev(ptr(), center.x, center.y, radius)); }

	/// Get reachable tiles using this board as walls.
	/// @param origin Start tile.
	/// @param distance Maximum number of 8-directional steps from `origin`.
	[[nodiscard]] bitboard reachable_tiles(vec2 origin, std::uint16_t distance) const
	{
		bitboard result{ uninitialized };
		reachable_tiles_into(origin, distance, result);
		return result;
	}

	/// Like `reachable_tiles`, but writes into `out` instead of returning a new board.
	/// @param origin Start tile.
	/// @param distance Maximum number of 8-directional steps from `origin`.
	/// @param out Destination; its previous contents are overwritten.
	void reachable_tiles_into(vec2 origin, std::uint16_t distance, bitboard& out) const
	{
		detail::check(detail::sdk_Bitboard_ReachableMask(ptr(), origin.x, origin.y, out.ptr(), distance));
	}

	/// Get reachable tiles using caller-provided working buffers.
	/// @param origin Start tile.
	/// @param distance Maximum number of 8-directional steps from `origin`.
	/// @param frontier Working buffer; its contents are overwritten.
	/// @param scratch Working buffer; its contents are overwritten.
	[[nodiscard]] bitboard reachable_tiles(vec2 origin, std::uint16_t distance, bitboard& frontier, bitboard& scratch) const
	{
		bitboard result{ uninitialized };
		reachable_tiles_into(origin, distance, frontier, scratch, result);
		return result;
	}

	/// Like `reachable_tiles`, but writes into `out` instead of returning a new board.
	/// @param origin Start tile.
	/// @param distance Maximum number of 8-directional steps from `origin`.
	/// @param frontier Working buffer; its contents are overwritten.
	/// @param scratch Working buffer; its contents are overwritten.
	/// @param out Destination; its previous contents are overwritten.
	void reachable_tiles_into(vec2 origin, std::uint16_t distance, bitboard& frontier, bitboard& scratch, bitboard& out) const
	{
		detail::check(detail::sdk_Bitboard_ReachableMaskWithScratch(ptr(), origin.x, origin.y, out.ptr(), distance, frontier.ptr(),
																	scratch.ptr()));
	}

	/// Get visible tiles using this board as opaque walls.
	/// @param origin Viewer tile.
	/// @param distance Maximum sight radius in tiles.
	[[nodiscard]] bitboard shadow_cast(vec2 origin, std::uint16_t distance) const
	{
		bitboard result{ uninitialized };
		shadow_cast_into(origin, distance, result);
		return result;
	}

	/// Like `shadow_cast`, but writes into `out` instead of returning a new board.
	/// @param origin Viewer tile.
	/// @param distance Maximum sight radius in tiles.
	/// @param out Destination; its previous contents are overwritten.
	void shadow_cast_into(vec2 origin, std::uint16_t distance, bitboard& out) const
	{
		detail::check(detail::sdk_Bitboard_ShadowCast(ptr(), origin.x, origin.y, out.ptr(), distance));
	}

private:
	void copy_from(const bitboard& other)
	{
		if (detail::sdk_Bitboard_ImportRawBuffer(ptr(), reinterpret_cast<const std::uint8_t*>(other.ptr()), raw_buffer_size()) != 0)
			std::memcpy(ptr(), other.ptr(), raw_buffer_size());
	}

	std::unique_ptr<std::uint64_t, detail::aligned_delete> buffer_;
};

/// Tile distances stored in engine-chunked order.
/// Linear buffers contain row-major uint16 values.
class distance_field
{
public:
	/// Allocate a distance buffer that reads as zero until written.
	distance_field() : buffer_{ new std::uint16_t[size()] } {}

	/// Copy another field's distances.
	distance_field(const distance_field& other) : distance_field{}
	{
		if (other.written_ &&
			detail::sdk_DistanceField_ImportRawBuffer(buffer_.get(), reinterpret_cast<const std::uint8_t*>(other.buffer_.get()),
													  raw_buffer_size()) != 0)
			std::memcpy(buffer_.get(), other.buffer_.get(), raw_buffer_size());
		written_ = other.written_;
	}

	/// Take over another field's storage.
	distance_field(distance_field&&) noexcept = default;

	/// Copy another field's distances.
	distance_field& operator=(const distance_field& other)
	{
		if (this != &other)
			*this = distance_field{ other };
		return *this;
	}

	/// Take over another field's storage.
	distance_field& operator=(distance_field&&) noexcept = default;

	/// Get the map dimensions in tiles.
	[[nodiscard]] std::int32_t width() const { return detail::layout().width; }

	/// Get the map height in tiles.
	[[nodiscard]] std::int32_t height() const { return detail::layout().height; }

	/// Get the size of the engine-chunked buffer in bytes.
	[[nodiscard]] static std::size_t raw_buffer_size() { return detail::layout().field_bytes; }

	/// Get the size of the packed row-major buffer in bytes.
	[[nodiscard]] static std::size_t linear_buffer_size()
	{
		return std::size_t(detail::layout().width) * detail::layout().height * sizeof(std::uint16_t);
	}

	/// Access raw distance storage for SDK imports.
	[[nodiscard]] std::uint16_t* ptr() noexcept
	{
		if (!written_)
		{
			std::memset(buffer_.get(), 0, raw_buffer_size());
			written_ = true;
		}
		return buffer_.get();
	}

	/// Access contiguous backing storage; preserve its layout when passing it to the host.
	[[nodiscard]] const std::uint16_t* ptr() const noexcept { return written_ ? buffer_.get() : zeros(); }

	/// Read a tile distance; throws if it is outside the map.
	/// @param p Tile to query.
	[[nodiscard]] std::uint16_t get(vec2 p) const
	{
		std::uint16_t result{};
		detail::check(detail::sdk_DistanceField_Get(ptr(), p.x, p.y, &result));
		return result;
	}

	/// Write a tile distance; throws if it is outside the map.
	/// @param p Tile to update.
	/// @param value Distance to store.
	void set(vec2 p, std::uint16_t value)
	{
		detail::check(detail::sdk_DistanceField_Set(ptr(), p.x, p.y, value));
	}

	/// Print the buffer through the engine.
	void print() const { detail::check(detail::sdk_DistanceField_Print(ptr())); }

	/// Copy an engine-chunked buffer into this object.
	/// @param bytes Engine-chunked bytes, exactly `raw_buffer_size()` long.
	void import_raw_buffer(std::span<const std::uint8_t> bytes)
	{
		detail::check(detail::sdk_DistanceField_ImportRawBuffer(ptr(), bytes.data(), bytes.size()));
	}

	/// Copy raw storage to a caller-provided buffer.
	/// @param bytes Destination, exactly `raw_buffer_size()` long.
	void export_raw_buffer(std::span<std::uint8_t> bytes) const
	{
		detail::check(detail::sdk_DistanceField_ExportRawBuffer(ptr(), bytes.data(), bytes.size()));
	}

	/// Return a copy of raw storage.
	[[nodiscard]] std::vector<std::uint8_t> export_raw_buffer() const
	{
		std::vector<std::uint8_t> bytes(raw_buffer_size());
		export_raw_buffer(bytes);
		return bytes;
	}

	/// Import a packed row-major buffer.
	/// @param values One distance per tile in row-major order.
	void import_linear_buffer(std::span<const std::uint16_t> values)
	{
		detail::check(
			detail::sdk_DistanceField_ImportLinearBuffer(ptr(), reinterpret_cast<const std::uint8_t*>(values.data()), values.size_bytes()));
	}

	/// Export to a caller-provided row-major buffer.
	/// @param values Destination with one distance per tile in row-major order.
	void export_linear_buffer(std::span<std::uint16_t> values) const
	{
		detail::check(
			detail::sdk_DistanceField_ExportLinearBuffer(ptr(), reinterpret_cast<std::uint8_t*>(values.data()), values.size_bytes()));
	}

	/// Return a row-major copy of the distances.
	[[nodiscard]] std::vector<std::uint16_t> export_linear_buffer() const
	{
		std::vector<std::uint16_t> values(linear_buffer_size() / sizeof(std::uint16_t));
		export_linear_buffer(values);
		return values;
	}

	/// Compute distances from roots, reusing the visited buffer.
	/// @param walls Impassable tiles.
	/// @param roots Tiles at distance zero.
	/// @param visited Receives the tiles the search reached.
	/// @param max_distance Tiles farther than this are left unreached.
	void compute(const bitboard& walls, const bitboard& roots, bitboard& visited, std::uint16_t max_distance = unreachable)
	{
		compute_with_expansion_count(walls, roots, visited, max_distance);
	}

	/// Compute distances and return the number of expansion steps.
	/// @param walls Impassable tiles.
	/// @param roots Tiles at distance zero.
	/// @param visited Receives the tiles the search reached.
	/// @param max_distance Tiles farther than this are left unreached.
	std::uint16_t compute_with_expansion_count(const bitboard& walls, const bitboard& roots, bitboard& visited,
											   std::uint16_t max_distance = unreachable)
	{
		std::uint16_t count{};
		detail::check(
			detail::sdk_DistanceField_ComputeWithExpansionCount(walls.ptr(), roots.ptr(), visited.ptr(), buffer_.get(), max_distance, &count));
		written_ = true;
		return count;
	}

	/// Compute distances until destinations are reached or the search is exhausted.
	/// @param walls Impassable tiles.
	/// @param roots Tiles at distance zero.
	/// @param destinations The search stops once these tiles are reached.
	/// @param visited Receives the tiles the search reached.
	/// @param max_distance Tiles farther than this are left unreached.
	void compute_to(const bitboard& walls, const bitboard& roots, const bitboard& destinations, bitboard& visited,
					std::uint16_t max_distance = unreachable)
	{
		compute_to_with_expansion_count(walls, roots, destinations, visited, max_distance);
	}

	/// Compute towards destinations and return the number of expansion steps.
	/// @param walls Impassable tiles.
	/// @param roots Tiles at distance zero.
	/// @param destinations The search stops once these tiles are reached.
	/// @param visited Receives the tiles the search reached.
	/// @param max_distance Tiles farther than this are left unreached.
	std::uint16_t compute_to_with_expansion_count(const bitboard& walls, const bitboard& roots, const bitboard& destinations,
												  bitboard& visited, std::uint16_t max_distance = unreachable)
	{
		std::uint16_t count{};
		detail::check(detail::sdk_DistanceField_ComputeToWithExpansionCount(walls.ptr(), roots.ptr(), destinations.ptr(), visited.ptr(),
																			buffer_.get(), max_distance, &count));
		written_ = true;
		return count;
	}

	/// Distance assigned to tiles not reached by the search.
	static constexpr std::uint16_t unreachable = std::numeric_limits<std::uint16_t>::max();

	/// First optimal step from `from` towards a root, as a direction delta in compass order from north.
	/// Returns nullopt at a root or unreached tile; throws if `from` is outside the map.
	/// @param from Tile to step from.
	[[nodiscard]] std::optional<vec2> next_move_from(vec2 from) const
	{
		std::array<std::uint16_t, 8> distances{};
		const std::uint16_t best{ neighbour_distances(from, distances) };
		for (std::size_t i{}; best != unreachable && i < directions.size(); ++i)
			if (distances[i] == best)
				return directions[i];
		return std::nullopt;
	}

	/// Every optimal step from `from` towards a root, as direction deltas in compass order from north.
	/// @param from Tile to step from.
	[[nodiscard]] std::vector<vec2> next_moves_from(vec2 from) const
	{
		std::array<std::uint16_t, 8> distances{};
		const std::uint16_t best{ neighbour_distances(from, distances) };
		std::vector<vec2> moves;
		for (std::size_t i{}; best != unreachable && i < directions.size(); ++i)
			if (distances[i] == best)
				moves.push_back(directions[i]);
		return moves;
	}

private:
	static constexpr std::array<vec2, 8> directions{ vec2::north, vec2::north_east, vec2::east, vec2::south_east,
													 vec2::south, vec2::south_west, vec2::west, vec2::north_west };

	// Fill neighbour distances and return the smallest one if it improves on `from`, else unreachable.
	std::uint16_t neighbour_distances(vec2 from, std::array<std::uint16_t, 8>& out) const
	{
		const std::uint16_t here{ get(from) };
		std::uint16_t best{ unreachable };
		for (std::size_t i{}; i < directions.size(); ++i)
		{
			const vec2 p{ from + directions[i] };
			out[i] = p.x >= 0 && p.y >= 0 && p.x < width() && p.y < height() ? get(p) : unreachable;
			best = std::min(best, out[i]);
		}
		return here != unreachable && best < here ? best : unreachable;
	}

	[[nodiscard]] static std::size_t size() { return raw_buffer_size() / sizeof(std::uint16_t); }

	[[nodiscard]] static const std::uint16_t* zeros()
	{
		static const std::vector<std::uint16_t> values(size());
		return values.data();
	}

	// Compute writes every entry; other writes start from zeroed storage.
	std::unique_ptr<std::uint16_t[]> buffer_;
	bool written_{};
};

/// Physical champion types; Cassi and Issac are separate champions.
enum class champion_type : std::int32_t
{
	/// Melee champion with an area stun and an attack reset; bonus health adds strength.
	ryth = 0,
	/// Ranged champion with strong basic attacks, bonus gold and a short dash.
	vesper_vale,
	/// Melee champion whose kills reset cooldowns, with an attack reset and a dash.
	vendetta,
	/// Ranged spellcaster half of the Cassi and Issac pick; dies when Issac dies.
	cassi,
	/// Melee half of the Cassi and Issac pick; dies when Cassi dies.
	issac,
	/// Ranged spellcaster choosing between single-target damage with a stun and an area hit.
	stellar,
	/// Ranged spellcaster that gains focus from carried gems.
	prism,
	/// Melee champion immune to stuns, with a stunning enhanced attack and a ranged damage spell.
	marcella,
	/// Ranged champion that heals another ally or resets its basic attack.
	willow,
	/// Melee champion with retaliation damage, a targeted stun and a short dash.
	ren,
	/// Melee champion with splash basic attacks, an attack reset and a self-shield.
	varg,
	/// Ranged champion that shops in the field and has a strong stunning spell.
	tavi,
};

/// Ability payload types reported by the engine.
enum class ability_kind : std::uint32_t
{
	/// Restores the basic attack allowance.
	attack_reset = 1,
	/// Damages a single target.
	damage,
	/// Damages every enemy in an area.
	aoe_damage,
	/// Crowd control: stuns, optionally with damage.
	cc,
	/// Heals another allied champion.
	heal,
	/// Moves the caster along a legal dash path.
	dash,
	/// Shields the caster against incoming damage.
	shield
};

struct item_info;
struct consumable_info;
enum class item_passive : std::uint32_t;

/// Purchasable item types.
class item_type
{
public:
	/// Underlying engine enum; wrapper values convert to it implicitly.
	enum class enum_type : std::uint32_t
	{
		/// Monster shard: 30 gold, adds the champion's strength growth.
		monster_shard_of_strength,
		/// Monster emblem: combined from five monster strength shards; 10% more damage to camps.
		monster_emblem_of_strength,
		/// Monster shard: 30 gold, adds the champion's focus growth.
		monster_shard_of_focus,
		/// Monster emblem: combined from five monster focus shards; 10% more damage to camps.
		monster_emblem_of_focus,
		/// Monster shard: 30 gold, adds the champion's health growth.
		monster_shard_of_health,
		/// Monster emblem: combined from five monster health shards; 10% more damage to camps.
		monster_emblem_of_health,
		/// Power shard: 30 gold, adds the champion's strength growth.
		power_shard_of_strength,
		/// Power emblem: combined from five power strength shards; counts as six strength shards instead of a passive.
		power_emblem_of_strength,
		/// Power shard: 30 gold, adds the champion's focus growth.
		power_shard_of_focus,
		/// Power emblem: combined from five power focus shards; counts as six focus shards instead of a passive.
		power_emblem_of_focus,
		/// Power shard: 30 gold, adds the champion's health growth.
		power_shard_of_health,
		/// Power emblem: combined from five power health shards; counts as six health shards instead of a passive.
		power_emblem_of_health,
		/// Durability shard: 30 gold, adds the champion's strength growth.
		durability_shard_of_strength,
		/// Durability emblem: combined from five durability strength shards; 10% less incoming damage.
		durability_emblem_of_strength,
		/// Durability shard: 30 gold, adds the champion's focus growth.
		durability_shard_of_focus,
		/// Durability emblem: combined from five durability focus shards; 10% less incoming damage.
		durability_emblem_of_focus,
		/// Durability shard: 30 gold, adds the champion's health growth.
		durability_shard_of_health,
		/// Durability emblem: combined from five durability health shards; 10% less incoming damage.
		durability_emblem_of_health,
		/// Treasure shard: 30 gold, adds the champion's strength growth.
		treasure_shard_of_strength,
		/// Treasure emblem: combined from five treasure strength shards; 20% more gold gained.
		treasure_emblem_of_strength,
		/// Treasure shard: 30 gold, adds the champion's focus growth.
		treasure_shard_of_focus,
		/// Treasure emblem: combined from five treasure focus shards; 20% more gold gained.
		treasure_emblem_of_focus,
		/// Treasure shard: 30 gold, adds the champion's health growth.
		treasure_shard_of_health,
		/// Treasure emblem: combined from five treasure health shards; 20% more gold gained.
		treasure_emblem_of_health,
		/// Jungle shard: 30 gold, adds the champion's strength growth.
		jungle_shard_of_strength,
		/// Jungle emblem: combined from five jungle strength shards; 30% less damage from camps.
		jungle_emblem_of_strength,
		/// Jungle shard: 30 gold, adds the champion's focus growth.
		jungle_shard_of_focus,
		/// Jungle emblem: combined from five jungle focus shards; 30% less damage from camps.
		jungle_emblem_of_focus,
		/// Jungle shard: 30 gold, adds the champion's health growth.
		jungle_shard_of_health,
		/// Jungle emblem: combined from five jungle health shards; 30% less damage from camps.
		jungle_emblem_of_health,
		/// Home shard: 30 gold, adds the champion's strength growth.
		home_shard_of_strength,
		/// Home emblem: combined from five home strength shards; recall takes three turn ends instead of five.
		home_emblem_of_strength,
		/// Home shard: 30 gold, adds the champion's focus growth.
		home_shard_of_focus,
		/// Home emblem: combined from five home focus shards; recall takes three turn ends instead of five.
		home_emblem_of_focus,
		/// Home shard: 30 gold, adds the champion's health growth.
		home_shard_of_health,
		/// Home emblem: combined from five home health shards; recall takes three turn ends instead of five.
		home_emblem_of_health,
		/// Bruiser shard: 30 gold, adds the champion's strength growth.
		bruiser_shard_of_strength,
		/// Bruiser emblem: combined from five bruiser strength shards; five strength shards plus one of each other stat instead of a passive.
		bruiser_emblem_of_strength,
		/// Bruiser shard: 30 gold, adds the champion's focus growth.
		bruiser_shard_of_focus,
		/// Bruiser emblem: combined from five bruiser focus shards; five focus shards plus one of each other stat instead of a passive.
		bruiser_emblem_of_focus,
		/// Bruiser shard: 30 gold, adds the champion's health growth.
		bruiser_shard_of_health,
		/// Bruiser emblem: combined from five bruiser health shards; five health shards plus one of each other stat instead of a passive.
		bruiser_emblem_of_health,
		/// Number of item types; not a valid item.
		count
	};

	/// Known values and the end sentinel, exposed as wrapper constants.
	static const item_type monster_shard_of_strength;
	/// Monster emblem: combined from five monster strength shards; 10% more damage to camps.
	static const item_type monster_emblem_of_strength;
	/// Monster shard: 30 gold, adds the champion's focus growth.
	static const item_type monster_shard_of_focus;
	/// Monster emblem: combined from five monster focus shards; 10% more damage to camps.
	static const item_type monster_emblem_of_focus;
	/// Monster shard: 30 gold, adds the champion's health growth.
	static const item_type monster_shard_of_health;
	/// Monster emblem: combined from five monster health shards; 10% more damage to camps.
	static const item_type monster_emblem_of_health;
	/// Power shard: 30 gold, adds the champion's strength growth.
	static const item_type power_shard_of_strength;
	/// Power emblem: combined from five power strength shards; counts as six strength shards instead of a passive.
	static const item_type power_emblem_of_strength;
	/// Power shard: 30 gold, adds the champion's focus growth.
	static const item_type power_shard_of_focus;
	/// Power emblem: combined from five power focus shards; counts as six focus shards instead of a passive.
	static const item_type power_emblem_of_focus;
	/// Power shard: 30 gold, adds the champion's health growth.
	static const item_type power_shard_of_health;
	/// Power emblem: combined from five power health shards; counts as six health shards instead of a passive.
	static const item_type power_emblem_of_health;
	/// Durability shard: 30 gold, adds the champion's strength growth.
	static const item_type durability_shard_of_strength;
	/// Durability emblem: combined from five durability strength shards; 10% less incoming damage.
	static const item_type durability_emblem_of_strength;
	/// Durability shard: 30 gold, adds the champion's focus growth.
	static const item_type durability_shard_of_focus;
	/// Durability emblem: combined from five durability focus shards; 10% less incoming damage.
	static const item_type durability_emblem_of_focus;
	/// Durability shard: 30 gold, adds the champion's health growth.
	static const item_type durability_shard_of_health;
	/// Durability emblem: combined from five durability health shards; 10% less incoming damage.
	static const item_type durability_emblem_of_health;
	/// Treasure shard: 30 gold, adds the champion's strength growth.
	static const item_type treasure_shard_of_strength;
	/// Treasure emblem: combined from five treasure strength shards; 20% more gold gained.
	static const item_type treasure_emblem_of_strength;
	/// Treasure shard: 30 gold, adds the champion's focus growth.
	static const item_type treasure_shard_of_focus;
	/// Treasure emblem: combined from five treasure focus shards; 20% more gold gained.
	static const item_type treasure_emblem_of_focus;
	/// Treasure shard: 30 gold, adds the champion's health growth.
	static const item_type treasure_shard_of_health;
	/// Treasure emblem: combined from five treasure health shards; 20% more gold gained.
	static const item_type treasure_emblem_of_health;
	/// Jungle shard: 30 gold, adds the champion's strength growth.
	static const item_type jungle_shard_of_strength;
	/// Jungle emblem: combined from five jungle strength shards; 30% less damage from camps.
	static const item_type jungle_emblem_of_strength;
	/// Jungle shard: 30 gold, adds the champion's focus growth.
	static const item_type jungle_shard_of_focus;
	/// Jungle emblem: combined from five jungle focus shards; 30% less damage from camps.
	static const item_type jungle_emblem_of_focus;
	/// Jungle shard: 30 gold, adds the champion's health growth.
	static const item_type jungle_shard_of_health;
	/// Jungle emblem: combined from five jungle health shards; 30% less damage from camps.
	static const item_type jungle_emblem_of_health;
	/// Home shard: 30 gold, adds the champion's strength growth.
	static const item_type home_shard_of_strength;
	/// Home emblem: combined from five home strength shards; recall takes three turn ends instead of five.
	static const item_type home_emblem_of_strength;
	/// Home shard: 30 gold, adds the champion's focus growth.
	static const item_type home_shard_of_focus;
	/// Home emblem: combined from five home focus shards; recall takes three turn ends instead of five.
	static const item_type home_emblem_of_focus;
	/// Home shard: 30 gold, adds the champion's health growth.
	static const item_type home_shard_of_health;
	/// Home emblem: combined from five home health shards; recall takes three turn ends instead of five.
	static const item_type home_emblem_of_health;
	/// Bruiser shard: 30 gold, adds the champion's strength growth.
	static const item_type bruiser_shard_of_strength;
	/// Bruiser emblem: combined from five bruiser strength shards; five strength shards plus one of each other stat instead of a passive.
	static const item_type bruiser_emblem_of_strength;
	/// Bruiser shard: 30 gold, adds the champion's focus growth.
	static const item_type bruiser_shard_of_focus;
	/// Bruiser emblem: combined from five bruiser focus shards; five focus shards plus one of each other stat instead of a passive.
	static const item_type bruiser_emblem_of_focus;
	/// Bruiser shard: 30 gold, adds the champion's health growth.
	static const item_type bruiser_shard_of_health;
	/// Bruiser emblem: combined from five bruiser health shards; five health shards plus one of each other stat instead of a passive.
	static const item_type bruiser_emblem_of_health;
	/// Number of item types; not a valid item.
	static const item_type count;

	/// Construct the first enum value.
	constexpr item_type() noexcept = default;
	/// Wrap an underlying enum value without changing its identity.
	constexpr item_type(enum_type value) noexcept : value_{ value } {}
	/// Explicitly decode an ABI integer; metadata queries reject invalid values.
	explicit constexpr item_type(std::uint32_t value) noexcept : value_{ static_cast<enum_type>(value) } {}
	/// Preserve compatibility with functions expecting the underlying enum.
	[[nodiscard]] constexpr operator enum_type() const noexcept { return value_; }
	/// Explicit integer conversion for indexing and host ABI calls.
	template <class T> requires std::is_integral_v<T>
	[[nodiscard]] constexpr explicit operator T() const noexcept { return static_cast<T>(value_); }
	/// Get an owning metadata value; invalid kinds and host failures throw.
	[[nodiscard]] item_info info() const;
	/// Get the full value in gold: the purchase price, refunded in full when sold.
	[[nodiscard]] std::uint32_t cost() const;
	/// Get the ingredient count needed to create the result item.
	[[nodiscard]] std::uint32_t combine_count() const;
	/// Get the combination result, if this item can be combined.
	[[nodiscard]] std::optional<item_type> result() const;
	/// Get the health bonus.
	[[nodiscard]] std::int32_t health_points() const;
	/// Get the strength bonus.
	[[nodiscard]] std::int32_t strength_points() const;
	/// Get the focus bonus.
	[[nodiscard]] std::int32_t focus_points() const;
	/// Get the optional passive effect.
	[[nodiscard]] std::optional<item_passive> passive() const;
	/// Get the passive magnitude.
	[[nodiscard]] float passive_magnitude() const;
	/// Whether the item can be bought directly; emblems must be combined.
	[[nodiscard]] bool is_purchasable() const;

private:
	enum_type value_{};
};

inline constexpr item_type item_type::monster_shard_of_strength{ enum_type::monster_shard_of_strength };
inline constexpr item_type item_type::monster_emblem_of_strength{ enum_type::monster_emblem_of_strength };
inline constexpr item_type item_type::monster_shard_of_focus{ enum_type::monster_shard_of_focus };
inline constexpr item_type item_type::monster_emblem_of_focus{ enum_type::monster_emblem_of_focus };
inline constexpr item_type item_type::monster_shard_of_health{ enum_type::monster_shard_of_health };
inline constexpr item_type item_type::monster_emblem_of_health{ enum_type::monster_emblem_of_health };
inline constexpr item_type item_type::power_shard_of_strength{ enum_type::power_shard_of_strength };
inline constexpr item_type item_type::power_emblem_of_strength{ enum_type::power_emblem_of_strength };
inline constexpr item_type item_type::power_shard_of_focus{ enum_type::power_shard_of_focus };
inline constexpr item_type item_type::power_emblem_of_focus{ enum_type::power_emblem_of_focus };
inline constexpr item_type item_type::power_shard_of_health{ enum_type::power_shard_of_health };
inline constexpr item_type item_type::power_emblem_of_health{ enum_type::power_emblem_of_health };
inline constexpr item_type item_type::durability_shard_of_strength{ enum_type::durability_shard_of_strength };
inline constexpr item_type item_type::durability_emblem_of_strength{ enum_type::durability_emblem_of_strength };
inline constexpr item_type item_type::durability_shard_of_focus{ enum_type::durability_shard_of_focus };
inline constexpr item_type item_type::durability_emblem_of_focus{ enum_type::durability_emblem_of_focus };
inline constexpr item_type item_type::durability_shard_of_health{ enum_type::durability_shard_of_health };
inline constexpr item_type item_type::durability_emblem_of_health{ enum_type::durability_emblem_of_health };
inline constexpr item_type item_type::treasure_shard_of_strength{ enum_type::treasure_shard_of_strength };
inline constexpr item_type item_type::treasure_emblem_of_strength{ enum_type::treasure_emblem_of_strength };
inline constexpr item_type item_type::treasure_shard_of_focus{ enum_type::treasure_shard_of_focus };
inline constexpr item_type item_type::treasure_emblem_of_focus{ enum_type::treasure_emblem_of_focus };
inline constexpr item_type item_type::treasure_shard_of_health{ enum_type::treasure_shard_of_health };
inline constexpr item_type item_type::treasure_emblem_of_health{ enum_type::treasure_emblem_of_health };
inline constexpr item_type item_type::jungle_shard_of_strength{ enum_type::jungle_shard_of_strength };
inline constexpr item_type item_type::jungle_emblem_of_strength{ enum_type::jungle_emblem_of_strength };
inline constexpr item_type item_type::jungle_shard_of_focus{ enum_type::jungle_shard_of_focus };
inline constexpr item_type item_type::jungle_emblem_of_focus{ enum_type::jungle_emblem_of_focus };
inline constexpr item_type item_type::jungle_shard_of_health{ enum_type::jungle_shard_of_health };
inline constexpr item_type item_type::jungle_emblem_of_health{ enum_type::jungle_emblem_of_health };
inline constexpr item_type item_type::home_shard_of_strength{ enum_type::home_shard_of_strength };
inline constexpr item_type item_type::home_emblem_of_strength{ enum_type::home_emblem_of_strength };
inline constexpr item_type item_type::home_shard_of_focus{ enum_type::home_shard_of_focus };
inline constexpr item_type item_type::home_emblem_of_focus{ enum_type::home_emblem_of_focus };
inline constexpr item_type item_type::home_shard_of_health{ enum_type::home_shard_of_health };
inline constexpr item_type item_type::home_emblem_of_health{ enum_type::home_emblem_of_health };
inline constexpr item_type item_type::bruiser_shard_of_strength{ enum_type::bruiser_shard_of_strength };
inline constexpr item_type item_type::bruiser_emblem_of_strength{ enum_type::bruiser_emblem_of_strength };
inline constexpr item_type item_type::bruiser_shard_of_focus{ enum_type::bruiser_shard_of_focus };
inline constexpr item_type item_type::bruiser_emblem_of_focus{ enum_type::bruiser_emblem_of_focus };
inline constexpr item_type item_type::bruiser_shard_of_health{ enum_type::bruiser_shard_of_health };
inline constexpr item_type item_type::bruiser_emblem_of_health{ enum_type::bruiser_emblem_of_health };
inline constexpr item_type item_type::count{ enum_type::count };

/// Engine-provided consumable price and inventory carry limit.
struct consumable_info
{
	/// Purchase price in gold and maximum carried inventory count.
	std::uint32_t cost, carry_limit;
	/// Compare every field.
	[[nodiscard]] friend bool operator==(const consumable_info&, const consumable_info&) = default;
};

namespace detail
{
inline constexpr std::array<std::uint32_t, 2> raw_consumable_ids{ 0, 2 };
inline const std::array<cambc::consumable_info, 2> cached_consumables = []
{
	std::array<cambc::consumable_info, 2> result{};
	for (std::size_t i = 0; i < result.size(); ++i)
	{
		detail::consumable_info info{};
		detail::check(detail::sdk_consumable_info(raw_consumable_ids[i], &info));
		result[i] = { info.cost, info.carry_limit };
	}
	return result;
}();
} // namespace detail

/// Consumable inventory keys.
class consumable_type
{
public:
	/// Underlying engine enum; wrapper values convert to it implicitly.
	enum class enum_type : std::uint32_t
	{
		/// Teleport crystal: channel for three turn ends, then flash to a reserved tile within range 3.
		tp_crystal,
		/// Altar lock: placed on an altar within 2 tiles; blocks enemy tributes until destroyed by basic attacks.
		altar_lock,
		/// Number of consumable types; not a valid consumable.
		count
	};

	/// Known values and the end sentinel, exposed as wrapper constants.
	static const consumable_type tp_crystal;
	/// Altar lock: placed on an altar within 2 tiles; blocks enemy tributes until destroyed by basic attacks.
	static const consumable_type altar_lock;
	/// Number of consumable types; not a valid consumable.
	static const consumable_type count;

	static_assert(static_cast<std::size_t>(enum_type::count) == detail::cached_consumables.size());

	/// Construct the first enum value.
	constexpr consumable_type() noexcept = default;
	/// Wrap an underlying enum value without changing its identity.
	constexpr consumable_type(enum_type value) noexcept : value_{ value } {}
	/// Explicitly decode an ABI integer; metadata queries reject invalid values.
	explicit constexpr consumable_type(std::uint32_t value) noexcept : value_{ static_cast<enum_type>(value) } {}
	/// Preserve compatibility with functions expecting the underlying enum.
	[[nodiscard]] constexpr operator enum_type() const noexcept { return value_; }
	/// Explicit integer conversion for indexing and host ABI calls.
	template <class T> requires std::is_integral_v<T>
	[[nodiscard]] constexpr explicit operator T() const noexcept { return static_cast<T>(value_); }
	/// Get an owning metadata value; invalid kinds and host failures throw.
	[[nodiscard]] consumable_info info() const
	{
		const auto index = static_cast<std::size_t>(value_);
		if (index >= detail::cached_consumables.size())
			throw error{ error_code::invalid_action_buy_consumable_invalid_type, "invalid consumable type" };
		return detail::cached_consumables[index];
	}
	/// Get the purchase cost in gold.
	[[nodiscard]] std::uint32_t cost() const { return info().cost; }
	/// Get the maximum carried inventory count.
	[[nodiscard]] std::uint32_t carry_limit() const { return info().carry_limit; }
private:
	enum_type value_{};
};

inline constexpr consumable_type consumable_type::tp_crystal{ enum_type::tp_crystal };
inline constexpr consumable_type consumable_type::altar_lock{ enum_type::altar_lock };
inline constexpr consumable_type consumable_type::count{ enum_type::count };

/// Contiguous, zero-initialised values indexed by an enum or enum wrapper with a count sentinel.
template <class T, class Value = std::uint32_t>
	requires (std::is_enum_v<T> || requires { typename T::enum_type; requires std::is_enum_v<typename T::enum_type>; })
class enum_map
{
public:
	/// Access the value for key; throws std::out_of_range for an invalid key.
	[[nodiscard]] Value& operator[](T key) { return values_.at(static_cast<std::size_t>(key)); }

	/// Access the value for key; throws std::out_of_range for an invalid key.
	[[nodiscard]] const Value& operator[](T key) const { return values_.at(static_cast<std::size_t>(key)); }

	/// Get an iterator to the first enum-indexed value.
	[[nodiscard]] auto begin() noexcept { return values_.begin(); }

	/// Get the past-the-end iterator for enum-indexed values.
	[[nodiscard]] auto end() noexcept { return values_.end(); }

	/// Get a const iterator to the first enum-indexed value.
	[[nodiscard]] auto begin() const noexcept { return values_.begin(); }

	/// Get the const past-the-end iterator for enum-indexed values.
	[[nodiscard]] auto end() const noexcept { return values_.end(); }

	/// Get the number of entries in this container.
	[[nodiscard]] constexpr std::size_t size() const noexcept { return values_.size(); }

	/// Access contiguous backing storage; preserve its layout when passing it to the host.
	[[nodiscard]] Value* ptr() noexcept { return values_.data(); }
	/// Compare every value.
	[[nodiscard]] friend bool operator==(const enum_map&, const enum_map&) = default;

	/// Access contiguous backing storage; preserve its layout when passing it to the host.
	[[nodiscard]] const Value* ptr() const noexcept { return values_.data(); }

private:
	std::array<Value, static_cast<std::size_t>(T::count)> values_{};
};

/// Item-kind alias used by inventory entries.
using item = item_type;

/// Optional item passive tags; no passive is represented by an empty optional.
enum class item_passive : std::uint32_t { monster = 1, durability, treasure, jungle, home };

/// Format as "(x, y)".
/// @param value Value to format.
[[nodiscard]] inline std::string to_string(vec2 value) { return "(" + std::to_string(value.x) + ", " + std::to_string(value.y) + ")"; }

/// Get the enumerator name.
/// @param value Value to format.
[[nodiscard]] inline std::string to_string(team value)
{
	switch (value)
	{
	case team::ally: return "ally";
	case team::enemy: return "enemy";
	default: break;
	}
	return "team(" + std::to_string(static_cast<int>(value)) + ")";
}

/// Get the enumerator name.
/// @param value Value to format.
[[nodiscard]] inline std::string to_string(symmetry value)
{
	switch (value)
	{
	case symmetry::horizontal: return "horizontal";
	case symmetry::vertical: return "vertical";
	case symmetry::rotational180: return "rotational180";
	default: break;
	}
	return "symmetry(" + std::to_string(static_cast<std::int32_t>(value)) + ")";
}

/// Get the enumerator name.
/// @param value Value to format.
[[nodiscard]] inline std::string to_string(camp_type value)
{
	switch (value)
	{
	case camp_type::wolf: return "wolf";
	case camp_type::elk: return "elk";
	case camp_type::gem_cluster: return "gem_cluster";
	case camp_type::gold_cluster: return "gold_cluster";
	case camp_type::monolith: return "monolith";
	default: break;
	}
	return "camp_type(" + std::to_string(static_cast<std::int32_t>(value)) + ")";
}

/// Get the enumerator name.
/// @param value Value to format.
[[nodiscard]] inline std::string to_string(passive_type value)
{
	switch (value)
	{
	case passive_type::none: return "none";
	case passive_type::bonus_gold: return "bonus_gold";
	case passive_type::remote_shop: return "remote_shop";
	case passive_type::gem_focus: return "gem_focus";
	case passive_type::splash_attack: return "splash_attack";
	case passive_type::health_strength: return "health_strength";
	case passive_type::reset_cooldowns_on_kill: return "reset_cooldowns_on_kill";
	case passive_type::retaliation: return "retaliation";
	case passive_type::stun_immunity: return "stun_immunity";
	default: break;
	}
	return "passive_type(" + std::to_string(static_cast<int>(value)) + ")";
}

/// Get the enumerator name.
/// @param value Value to format.
[[nodiscard]] inline std::string to_string(champion_draft value)
{
	switch (value)
	{
	case champion_draft::ryth: return "ryth";
	case champion_draft::vesper_vale: return "vesper_vale";
	case champion_draft::vendetta: return "vendetta";
	case champion_draft::cassi_and_issac: return "cassi_and_issac";
	case champion_draft::stellar: return "stellar";
	case champion_draft::prism: return "prism";
	case champion_draft::marcella: return "marcella";
	case champion_draft::willow: return "willow";
	case champion_draft::ren: return "ren";
	case champion_draft::varg: return "varg";
	case champion_draft::tavi: return "tavi";
	default: break;
	}
	return "champion_draft(" + std::to_string(static_cast<std::int32_t>(value)) + ")";
}

/// Get the enumerator name.
/// @param value Value to format.
[[nodiscard]] inline std::string to_string(error_code value)
{
	switch (value)
	{
	case error_code::success: return "success";
	case error_code::invalid_handle: return "invalid_handle";
	case error_code::out_of_memory: return "out_of_memory";
	case error_code::file_not_found: return "file_not_found";
	case error_code::null_pointer: return "null_pointer";
	case error_code::invalid_argument: return "invalid_argument";
	case error_code::map_parse_error: return "map_parse_error";
	case error_code::invalid_map_file: return "invalid_map_file";
	case error_code::invalid_map_version: return "invalid_map_version";
	case error_code::replay_parse_error: return "replay_parse_error";
	case error_code::invalid_replay_file: return "invalid_replay_file";
	case error_code::invalid_replay_version: return "invalid_replay_version";
	case error_code::invalid_draft: return "invalid_draft";
	case error_code::draft_not_drafting: return "draft_not_drafting";
	case error_code::draft_duplicate_champion: return "draft_duplicate_champion";
	case error_code::draft_invalid_round: return "draft_invalid_round";
	case error_code::draft_unavailable: return "draft_unavailable";
	case error_code::already_initialized: return "already_initialized";
	case error_code::invalid_action: return "invalid_action";
	case error_code::invalid_action_memory_bad_range: return "invalid_action_memory_bad_range";
	case error_code::invalid_action_memory_draft_frozen: return "invalid_action_memory_draft_frozen";
	case error_code::invalid_action_move_cant_move: return "invalid_action_move_cant_move";
	case error_code::invalid_action_move_bad_destination: return "invalid_action_move_bad_destination";
	case error_code::invalid_action_spell_cast_invalid_source: return "invalid_action_spell_cast_invalid_source";
	case error_code::invalid_action_spell_cast_invalid_slot: return "invalid_action_spell_cast_invalid_slot";
	case error_code::invalid_action_spell_cast_no_ability: return "invalid_action_spell_cast_no_ability";
	case error_code::invalid_action_spell_cast_cant_cast: return "invalid_action_spell_cast_cant_cast";
	case error_code::invalid_action_spell_cast_already_cast: return "invalid_action_spell_cast_already_cast";
	case error_code::invalid_action_spell_cast_on_cooldown: return "invalid_action_spell_cast_on_cooldown";
	case error_code::invalid_action_spell_cast_no_charges: return "invalid_action_spell_cast_no_charges";
	case error_code::invalid_action_spell_cast_bad_target: return "invalid_action_spell_cast_bad_target";
	case error_code::invalid_action_spell_cast_out_of_range: return "invalid_action_spell_cast_out_of_range";
	case error_code::invalid_action_spell_cast_bad_destination: return "invalid_action_spell_cast_bad_destination";
	case error_code::invalid_action_cant_act: return "invalid_action_cant_act";
	case error_code::invalid_action_is_recalling: return "invalid_action_is_recalling";
	case error_code::invalid_action_is_submitting_gems: return "invalid_action_is_submitting_gems";
	case error_code::invalid_action_move_already_moved: return "invalid_action_move_already_moved";
	case error_code::invalid_action_move_out_of_range: return "invalid_action_move_out_of_range";
	case error_code::invalid_action_move_unreachable: return "invalid_action_move_unreachable";
	case error_code::invalid_action_attack_invalid_source: return "invalid_action_attack_invalid_source";
	case error_code::invalid_action_attack_cant_attack: return "invalid_action_attack_cant_attack";
	case error_code::invalid_action_attack_already_attacked: return "invalid_action_attack_already_attacked";
	case error_code::invalid_action_attack_bad_target: return "invalid_action_attack_bad_target";
	case error_code::invalid_action_attack_out_of_range: return "invalid_action_attack_out_of_range";
	case error_code::invalid_action_recall_cant_channel: return "invalid_action_recall_cant_channel";
	case error_code::invalid_action_submit_gems_cant_channel: return "invalid_action_submit_gems_cant_channel";
	case error_code::invalid_action_submit_gems_no_gems: return "invalid_action_submit_gems_no_gems";
	case error_code::invalid_action_submit_gems_bad_altar: return "invalid_action_submit_gems_bad_altar";
	case error_code::invalid_action_submit_gems_out_of_range: return "invalid_action_submit_gems_out_of_range";
	case error_code::invalid_action_buy_item_not_on_spawn: return "invalid_action_buy_item_not_on_spawn";
	case error_code::invalid_action_buy_item_invalid_item: return "invalid_action_buy_item_invalid_item";
	case error_code::invalid_action_buy_item_invalid_count: return "invalid_action_buy_item_invalid_count";
	case error_code::invalid_action_buy_item_not_enough_gold: return "invalid_action_buy_item_not_enough_gold";
	case error_code::invalid_action_buy_item_inventory_full: return "invalid_action_buy_item_inventory_full";
	case error_code::invalid_action_buy_item_stats_overflow: return "invalid_action_buy_item_stats_overflow";
	case error_code::invalid_action_buy_consumable_not_on_spawn: return "invalid_action_buy_consumable_not_on_spawn";
	case error_code::invalid_action_buy_consumable_invalid_type: return "invalid_action_buy_consumable_invalid_type";
	case error_code::invalid_action_buy_consumable_invalid_count: return "invalid_action_buy_consumable_invalid_count";
	case error_code::invalid_action_buy_consumable_inventory_full: return "invalid_action_buy_consumable_inventory_full";
	case error_code::invalid_action_buy_consumable_not_enough_gold: return "invalid_action_buy_consumable_not_enough_gold";
	case error_code::invalid_action_use_consumable_invalid_type: return "invalid_action_use_consumable_invalid_type";
	case error_code::invalid_action_use_consumable_not_owned: return "invalid_action_use_consumable_not_owned";
	case error_code::invalid_action_place_ward_no_charges: return "invalid_action_place_ward_no_charges";
	case error_code::invalid_action_place_ward_cant_place: return "invalid_action_place_ward_cant_place";
	case error_code::invalid_action_place_ward_bad_destination: return "invalid_action_place_ward_bad_destination";
	case error_code::invalid_action_place_ward_out_of_range: return "invalid_action_place_ward_out_of_range";
	case error_code::map_set_team_aspawn_area_bad_position: return "map_set_team_aspawn_area_bad_position";
	case error_code::map_add_bush_bad_position: return "map_add_bush_bad_position";
	case error_code::map_remove_bush_bad_position: return "map_remove_bush_bad_position";
	case error_code::map_add_wall_bad_position: return "map_add_wall_bad_position";
	case error_code::map_remove_wall_bad_position: return "map_remove_wall_bad_position";
	case error_code::map_add_hole_bad_position: return "map_add_hole_bad_position";
	case error_code::map_remove_hole_bad_position: return "map_remove_hole_bad_position";
	case error_code::map_add_altar_bad_position: return "map_add_altar_bad_position";
	case error_code::map_remove_altar_bad_position: return "map_remove_altar_bad_position";
	case error_code::map_get_bush_bad_position: return "map_get_bush_bad_position";
	case error_code::map_get_wall_bad_position: return "map_get_wall_bad_position";
	case error_code::map_get_hole_bad_position: return "map_get_hole_bad_position";
	case error_code::map_get_altar_bad_position: return "map_get_altar_bad_position";
	case error_code::map_get_camp_handle_at_bad_position: return "map_get_camp_handle_at_bad_position";
	case error_code::map_add_camp_bad_position: return "map_add_camp_bad_position";
	case error_code::map_set_camp_bad_position: return "map_set_camp_bad_position";
	case error_code::invalid_action_champion_teleport_bad_destination: return "invalid_action_champion_teleport_bad_destination";
	case error_code::invalid_action_respawn_group_blocked_spawn: return "invalid_action_respawn_group_blocked_spawn";
	case error_code::invalid_action_camp_teleport_bad_destination: return "invalid_action_camp_teleport_bad_destination";
	case error_code::timeline_bad_seek_position: return "timeline_bad_seek_position";
	case error_code::timeline_bad_seek_substep: return "timeline_bad_seek_substep";
	case error_code::api_no_map: return "api_no_map";
	case error_code::managed_resources_bad_position: return "managed_resources_bad_position";
	case error_code::managed_resources_bad_radius: return "managed_resources_bad_radius";
	case error_code::managed_resources_aliasing: return "managed_resources_aliasing";
	case error_code::managed_resources_metadata_mismatch: return "managed_resources_metadata_mismatch";
	case error_code::managed_resources_opaque_origin: return "managed_resources_opaque_origin";
	case error_code::api_invalid_team: return "api_invalid_team";
	case error_code::api_invalid_perspective: return "api_invalid_perspective";
	case error_code::api_not_initialized: return "api_not_initialized";
	case error_code::api_bad_position: return "api_bad_position";
	case error_code::api_not_in_vision: return "api_not_in_vision";
	case error_code::api_not_found: return "api_not_found";
	case error_code::api_invalid_camp_group: return "api_invalid_camp_group";
	case error_code::api_invalid_camp_info: return "api_invalid_camp_info";
	case error_code::api_invalid_camp: return "api_invalid_camp";
	case error_code::api_invalid_champion: return "api_invalid_champion";
	case error_code::api_invalid_ward: return "api_invalid_ward";
	case error_code::api_invalid_ability: return "api_invalid_ability";
	case error_code::api_not_teammate: return "api_not_teammate";
	case error_code::api_camp_group_not_waiting_to_respawn: return "api_camp_group_not_waiting_to_respawn";
	case error_code::api_champion_alive: return "api_champion_alive";
	case error_code::api_invalid_champion_type: return "api_invalid_champion_type";
	case error_code::api_invalid_camp_type: return "api_invalid_camp_type";
	case error_code::api_value_out_of_range: return "api_value_out_of_range";
	case error_code::api_buffer_too_small: return "api_buffer_too_small";
	case error_code::api_not_owner: return "api_not_owner";
	case error_code::invalid_action_recall_already_used: return "invalid_action_recall_already_used";
	case error_code::invalid_action_submit_gems_already_used: return "invalid_action_submit_gems_already_used";
	case error_code::invalid_action_cancel_channel_not_channeling: return "invalid_action_cancel_channel_not_channeling";
	case error_code::invalid_action_cancel_channel_already_used: return "invalid_action_cancel_channel_already_used";
	case error_code::invalid_action_place_ward_ally_ward_present: return "invalid_action_place_ward_ally_ward_present";
	case error_code::invalid_action_altar_lock_bad_altar: return "invalid_action_altar_lock_bad_altar";
	case error_code::invalid_action_altar_lock_out_of_range: return "invalid_action_altar_lock_out_of_range";
	case error_code::invalid_action_altar_lock_already_locked: return "invalid_action_altar_lock_already_locked";
	case error_code::invalid_action_is_teleporting: return "invalid_action_is_teleporting";
	case error_code::invalid_action_cancel_channel_not_cancellable: return "invalid_action_cancel_channel_not_cancellable";
	case error_code::invalid_action_submit_gems_altar_locked: return "invalid_action_submit_gems_altar_locked";
	case error_code::invalid_action_spell_cast_already_moved: return "invalid_action_spell_cast_already_moved";
	case error_code::invalid_action_move_exclusive_spell_cast: return "invalid_action_move_exclusive_spell_cast";
	case error_code::invalid_action_sell_not_tavi: return "invalid_action_sell_not_tavi";
	case error_code::invalid_action_sell_invalid_item: return "invalid_action_sell_invalid_item";
	case error_code::invalid_action_sell_not_owned: return "invalid_action_sell_not_owned";
	case error_code::invalid_action_sell_already_sold: return "invalid_action_sell_already_sold";
	case error_code::invalid_action_sell_gold_overflow: return "invalid_action_sell_gold_overflow";
	case error_code::invalid_action_sell_not_on_spawn: return "invalid_action_sell_not_on_spawn";
	default: break;
	}
	return "error_code(" + std::to_string(static_cast<std::int32_t>(value)) + ")";
}

/// Get the enumerator name.
/// @param value Value to format.
[[nodiscard]] inline std::string to_string(champion_type value)
{
	switch (value)
	{
	case champion_type::ryth: return "ryth";
	case champion_type::vesper_vale: return "vesper_vale";
	case champion_type::vendetta: return "vendetta";
	case champion_type::cassi: return "cassi";
	case champion_type::issac: return "issac";
	case champion_type::stellar: return "stellar";
	case champion_type::prism: return "prism";
	case champion_type::marcella: return "marcella";
	case champion_type::willow: return "willow";
	case champion_type::ren: return "ren";
	case champion_type::varg: return "varg";
	case champion_type::tavi: return "tavi";
	default: break;
	}
	return "champion_type(" + std::to_string(static_cast<std::int32_t>(value)) + ")";
}

/// Get the enumerator name.
/// @param value Value to format.
[[nodiscard]] inline std::string to_string(ability_kind value)
{
	switch (value)
	{
	case ability_kind::attack_reset: return "attack_reset";
	case ability_kind::damage: return "damage";
	case ability_kind::aoe_damage: return "aoe_damage";
	case ability_kind::cc: return "cc";
	case ability_kind::heal: return "heal";
	case ability_kind::dash: return "dash";
	case ability_kind::shield: return "shield";
	default: break;
	}
	return "ability_kind(" + std::to_string(static_cast<std::uint32_t>(value)) + ")";
}

/// Get the enumerator name.
/// @param value Value to format.
[[nodiscard]] inline std::string to_string(item_passive value)
{
	switch (value)
	{
	case item_passive::monster: return "monster";
	case item_passive::durability: return "durability";
	case item_passive::treasure: return "treasure";
	case item_passive::jungle: return "jungle";
	case item_passive::home: return "home";
	default: break;
	}
	return "item_passive(" + std::to_string(static_cast<std::uint32_t>(value)) + ")";
}

/// Get the enumerator name.
/// @param value Value to format.
[[nodiscard]] inline std::string to_string(item_type value)
{
	switch (static_cast<item_type::enum_type>(value))
	{
	case item_type::enum_type::monster_shard_of_strength: return "monster_shard_of_strength";
	case item_type::enum_type::monster_emblem_of_strength: return "monster_emblem_of_strength";
	case item_type::enum_type::monster_shard_of_focus: return "monster_shard_of_focus";
	case item_type::enum_type::monster_emblem_of_focus: return "monster_emblem_of_focus";
	case item_type::enum_type::monster_shard_of_health: return "monster_shard_of_health";
	case item_type::enum_type::monster_emblem_of_health: return "monster_emblem_of_health";
	case item_type::enum_type::power_shard_of_strength: return "power_shard_of_strength";
	case item_type::enum_type::power_emblem_of_strength: return "power_emblem_of_strength";
	case item_type::enum_type::power_shard_of_focus: return "power_shard_of_focus";
	case item_type::enum_type::power_emblem_of_focus: return "power_emblem_of_focus";
	case item_type::enum_type::power_shard_of_health: return "power_shard_of_health";
	case item_type::enum_type::power_emblem_of_health: return "power_emblem_of_health";
	case item_type::enum_type::durability_shard_of_strength: return "durability_shard_of_strength";
	case item_type::enum_type::durability_emblem_of_strength: return "durability_emblem_of_strength";
	case item_type::enum_type::durability_shard_of_focus: return "durability_shard_of_focus";
	case item_type::enum_type::durability_emblem_of_focus: return "durability_emblem_of_focus";
	case item_type::enum_type::durability_shard_of_health: return "durability_shard_of_health";
	case item_type::enum_type::durability_emblem_of_health: return "durability_emblem_of_health";
	case item_type::enum_type::treasure_shard_of_strength: return "treasure_shard_of_strength";
	case item_type::enum_type::treasure_emblem_of_strength: return "treasure_emblem_of_strength";
	case item_type::enum_type::treasure_shard_of_focus: return "treasure_shard_of_focus";
	case item_type::enum_type::treasure_emblem_of_focus: return "treasure_emblem_of_focus";
	case item_type::enum_type::treasure_shard_of_health: return "treasure_shard_of_health";
	case item_type::enum_type::treasure_emblem_of_health: return "treasure_emblem_of_health";
	case item_type::enum_type::jungle_shard_of_strength: return "jungle_shard_of_strength";
	case item_type::enum_type::jungle_emblem_of_strength: return "jungle_emblem_of_strength";
	case item_type::enum_type::jungle_shard_of_focus: return "jungle_shard_of_focus";
	case item_type::enum_type::jungle_emblem_of_focus: return "jungle_emblem_of_focus";
	case item_type::enum_type::jungle_shard_of_health: return "jungle_shard_of_health";
	case item_type::enum_type::jungle_emblem_of_health: return "jungle_emblem_of_health";
	case item_type::enum_type::home_shard_of_strength: return "home_shard_of_strength";
	case item_type::enum_type::home_emblem_of_strength: return "home_emblem_of_strength";
	case item_type::enum_type::home_shard_of_focus: return "home_shard_of_focus";
	case item_type::enum_type::home_emblem_of_focus: return "home_emblem_of_focus";
	case item_type::enum_type::home_shard_of_health: return "home_shard_of_health";
	case item_type::enum_type::home_emblem_of_health: return "home_emblem_of_health";
	case item_type::enum_type::bruiser_shard_of_strength: return "bruiser_shard_of_strength";
	case item_type::enum_type::bruiser_emblem_of_strength: return "bruiser_emblem_of_strength";
	case item_type::enum_type::bruiser_shard_of_focus: return "bruiser_shard_of_focus";
	case item_type::enum_type::bruiser_emblem_of_focus: return "bruiser_emblem_of_focus";
	case item_type::enum_type::bruiser_shard_of_health: return "bruiser_shard_of_health";
	case item_type::enum_type::bruiser_emblem_of_health: return "bruiser_emblem_of_health";
	default: break;
	}
	return "item_type(" + std::to_string(static_cast<std::uint32_t>(static_cast<item_type::enum_type>(value))) + ")";
}

/// Get the enumerator name.
/// @param value Value to format.
[[nodiscard]] inline std::string to_string(consumable_type value)
{
	switch (static_cast<consumable_type::enum_type>(value))
	{
	case consumable_type::enum_type::tp_crystal: return "tp_crystal";
	case consumable_type::enum_type::altar_lock: return "altar_lock";
	default: break;
	}
	return "consumable_type(" + std::to_string(static_cast<std::uint32_t>(static_cast<consumable_type::enum_type>(value))) + ")";
}

/// Owned item metadata queried from the engine; combinations consume ingredients.
struct item_info
{
	/// Full gold value (also Tavi's sale refund) and ingredient count needed to create the result item.
	std::uint32_t cost, combine_count;
	/// Combination result, absent when this item cannot be combined.
	std::optional<item_type> result;
	/// Health, strength and focus bonuses contributed by this item.
	std::int32_t health_points, strength_points, focus_points;
	/// Optional passive effect applied by this item.
	std::optional<item_passive> passive;
	/// Engine-provided parameter for the selected passive.
	float passive_magnitude;
	/// Whether the item can be bought directly; emblems must be combined.
	bool purchasable;
	/// Compare every field.
	[[nodiscard]] friend bool operator==(const item_info&, const item_info&) = default;
};

[[nodiscard]] inline item_info item_type::info() const
{
	detail::item_info data{};
	detail::check(detail::sdk_item_info(static_cast<std::uint32_t>(value_), &data));
	return { data.cost, data.combine_count,
		data.combine_count == 0 ? std::nullopt : std::optional{ static_cast<item_type>(data.result) },
		data.health_points, data.strength_points, data.focus_points,
		data.passive == 0 ? std::nullopt : std::optional{ static_cast<item_passive>(data.passive) },
		data.passive_magnitude, data.purchasable != 0 };
}

[[nodiscard]] inline std::uint32_t item_type::cost() const { return info().cost; }
[[nodiscard]] inline std::uint32_t item_type::combine_count() const { return info().combine_count; }
[[nodiscard]] inline std::optional<item_type> item_type::result() const { return info().result; }
[[nodiscard]] inline std::int32_t item_type::health_points() const { return info().health_points; }
[[nodiscard]] inline std::int32_t item_type::strength_points() const { return info().strength_points; }
[[nodiscard]] inline std::int32_t item_type::focus_points() const { return info().focus_points; }
[[nodiscard]] inline std::optional<item_passive> item_type::passive() const { return info().passive; }
[[nodiscard]] inline float item_type::passive_magnitude() const { return info().passive_magnitude; }
[[nodiscard]] inline bool item_type::is_purchasable() const { return info().purchasable; }

namespace detail
{
inline constexpr std::uint32_t ability_attribute_exclusive_with_movement = 1 << 0;

template <class Derived, class Underlying> class flag_set
{
public:
	/// Construct an empty mask.
	constexpr flag_set() noexcept = default;
	/// Decode raw engine bits without discarding unknown future flags.
	/// @param bits Raw flag bits, kept as is including unnamed bits.
	[[nodiscard]] static constexpr Derived from_bits_retain(Underlying bits) noexcept { return Derived{ bits }; }
	/// Decode raw engine bits, or return nullopt if any bit is not a known flag.
	/// @param bits Raw flag bits.
	[[nodiscard]] static constexpr std::optional<Derived> from_bits(Underlying bits) noexcept
	{
		if ((bits & static_cast<Underlying>(~Derived::known_bits)) != 0)
			return std::nullopt;
		return from_bits_retain(bits);
	}
	/// Decode raw engine bits, discarding unknown flags.
	/// @param bits Raw flag bits; unnamed bits are dropped.
	[[nodiscard]] static constexpr Derived from_bits_truncate(Underlying bits) noexcept
	{ return from_bits_retain(static_cast<Underlying>(bits & Derived::known_bits)); }
	/// Construct an empty mask.
	[[nodiscard]] static constexpr Derived empty() noexcept { return from_bits_retain(Underlying{}); }
	/// Construct a mask with every known flag set.
	[[nodiscard]] static constexpr Derived all() noexcept { return from_bits_retain(Derived::known_bits); }
	/// Return the underlying engine bit pattern, including unknown flags.
	[[nodiscard]] constexpr Underlying bits() const noexcept { return bits_; }
	/// Check that all selected bits are present; an empty mask is always contained.
	/// @param flags Flags to test.
	[[nodiscard]] constexpr bool contains(Derived flags) const noexcept { return (bits_ & flags.bits()) == flags.bits(); }
	/// Check whether any selected bit is present.
	/// @param flags Flags to test.
	[[nodiscard]] constexpr bool intersects(Derived flags) const noexcept { return (bits_ & flags.bits()) != 0; }
	/// Check whether no bits, known or unknown, are set.
	[[nodiscard]] constexpr bool is_empty() const noexcept { return bits_ == 0; }
	/// Check whether every known flag is set.
	[[nodiscard]] constexpr bool is_all() const noexcept { return (bits_ & Derived::known_bits) == Derived::known_bits; }
	/// Set selected flags.
	/// @param flags Flags to set.
	constexpr void insert(Derived flags) noexcept { bits_ = static_cast<Underlying>(bits_ | flags.bits()); }
	/// Clear selected flags.
	/// @param flags Flags to clear.
	constexpr void remove(Derived flags) noexcept { bits_ = static_cast<Underlying>(bits_ & ~flags.bits()); }
	/// Toggle selected flags.
	/// @param flags Flags to flip.
	constexpr void toggle(Derived flags) noexcept { bits_ = static_cast<Underlying>(bits_ ^ flags.bits()); }
	/// Insert selected flags when value is true, otherwise remove them.
	/// @param flags Flags to update.
	/// @param value `true` to set the flags, `false` to clear them.
	constexpr void set(Derived flags, bool value) noexcept
	{
		if (value)
			insert(flags);
		else
			remove(flags);
	}
	/// Compare every bit, including unknown flags.
	[[nodiscard]] friend constexpr bool operator==(Derived a, Derived b) noexcept { return a.bits() == b.bits(); }
	/// Compose bitmasks without discarding unknown flags.
	[[nodiscard]] friend constexpr Derived operator|(Derived a, Derived b) noexcept
	{ return from_bits_retain(static_cast<Underlying>(a.bits() | b.bits())); }
	/// Intersect bitmasks.
	[[nodiscard]] friend constexpr Derived operator&(Derived a, Derived b) noexcept
	{ return from_bits_retain(static_cast<Underlying>(a.bits() & b.bits())); }
	/// Toggle selected bits.
	[[nodiscard]] friend constexpr Derived operator^(Derived a, Derived b) noexcept
	{ return from_bits_retain(static_cast<Underlying>(a.bits() ^ b.bits())); }
	/// Complement known flags; unknown flags are discarded.
	[[nodiscard]] friend constexpr Derived operator~(Derived value) noexcept { return from_bits_truncate(static_cast<Underlying>(~value.bits())); }
	/// Return the flags in a that are not in b.
	[[nodiscard]] friend constexpr Derived operator-(Derived a, Derived b) noexcept
	{ return from_bits_retain(static_cast<Underlying>(a.bits() & ~b.bits())); }
	/// Insert selected flags.
	constexpr Derived& operator|=(Derived flags) noexcept
	{
		auto& self = static_cast<Derived&>(*this);
		self = self | flags;
		return self;
	}
	/// Retain only selected flags.
	constexpr Derived& operator&=(Derived flags) noexcept
	{
		auto& self = static_cast<Derived&>(*this);
		self = self & flags;
		return self;
	}
	/// Toggle selected flags.
	constexpr Derived& operator^=(Derived flags) noexcept
	{
		auto& self = static_cast<Derived&>(*this);
		self = self ^ flags;
		return self;
	}
	/// Remove selected flags.
	constexpr Derived& operator-=(Derived flags) noexcept
	{
		auto& self = static_cast<Derived&>(*this);
		self = self - flags;
		return self;
	}

	/// Forward iterator over set known flags, then any unknown bits as one final value.
	class iterator
	{
	public:
		using value_type = Derived;
		using difference_type = std::ptrdiff_t;
		using iterator_concept = std::forward_iterator_tag;
		constexpr iterator() noexcept = default;
		[[nodiscard]] constexpr Derived operator*() const noexcept { return from_bits_retain(current()); }
		constexpr iterator& operator++() noexcept
		{
			rest_ = static_cast<Underlying>(rest_ & ~current());
			return *this;
		}
		constexpr iterator operator++(int) noexcept
		{
			auto old = *this;
			++*this;
			return old;
		}
		[[nodiscard]] friend constexpr bool operator==(iterator, iterator) noexcept = default;

	private:
		friend class flag_set;
		explicit constexpr iterator(Underlying rest) noexcept : rest_{ rest } {}
		[[nodiscard]] constexpr Underlying current() const noexcept
		{
			const auto known = static_cast<Underlying>(rest_ & Derived::known_bits);
			return known != 0 ? static_cast<Underlying>(known & (~known + 1)) : rest_;
		}
		Underlying rest_{};
	};
	[[nodiscard]] constexpr iterator begin() const noexcept { return iterator{ bits_ }; }
	[[nodiscard]] constexpr iterator end() const noexcept { return iterator{}; }

protected:
	/// Construct from raw bits when defining named masks.
	explicit constexpr flag_set(Underlying bits) noexcept : bits_{ bits } {}

private:
	Underlying bits_{};
};
} // namespace detail

/// Current life, channel and action flags; contains(champion_state::alive) checks the life bit.
class champion_state : public detail::flag_set<champion_state, std::uint32_t>
{
	using base = detail::flag_set<champion_state, std::uint32_t>;
public:
	/// Construct a state with no flags set.
	constexpr champion_state() noexcept = default;
	/// Check whether the alive flag is clear.
	[[nodiscard]] constexpr bool is_dead() const noexcept { return (bits() & 1u) == 0; }
	/// Named state flags; see their definitions below.
	static const champion_state alive, recalling, submitting_gems, can_move, can_attack, completed_turn,
		recall_started, submit_gems_started, recall_cancelled, submit_gems_cancelled, can_cast, teleporting;
private:
	static constexpr std::uint32_t known_bits = 0x1BFFu;
	constexpr explicit champion_state(std::uint32_t bits) noexcept : base{ bits } {}
	friend class detail::flag_set<champion_state, std::uint32_t>;
};
/// Allowed ability target categories, not a geometric range mask.
class ability_target_mask : public detail::flag_set<ability_target_mask, std::uint32_t>
{
	using base = detail::flag_set<ability_target_mask, std::uint32_t>;
public:
	/// Construct a mask with no target categories.
	constexpr ability_target_mask() noexcept = default;
	/// Named target categories; see their definitions below.
	static const ability_target_mask camps, enemies, allies, self, ground;
private:
	static constexpr std::uint32_t known_bits = 0x1Fu;
	constexpr explicit ability_target_mask(std::uint32_t bits) noexcept : base{ bits } {}
	friend class detail::flag_set<ability_target_mask, std::uint32_t>;
};
/// Prohibited actions; contains(buff_restrictions::ward) checks ward placement.
class buff_restrictions : public detail::flag_set<buff_restrictions, std::uint8_t>
{
	using base = detail::flag_set<buff_restrictions, std::uint8_t>;
public:
	/// Construct a mask with no restrictions.
	constexpr buff_restrictions() noexcept = default;
	/// Named restrictions; see their definitions below.
	static const buff_restrictions move, attack, cast, shop, consumable, ward;
private:
	static constexpr std::uint8_t known_bits = 0x3Fu;
	constexpr explicit buff_restrictions(std::uint8_t bits) noexcept : base{ bits } {}
	friend class detail::flag_set<buff_restrictions, std::uint8_t>;
};

/// The champion is alive.
inline constexpr champion_state champion_state::alive{ std::uint32_t{ 1 } << 0 };
/// The champion is channelling a recall.
inline constexpr champion_state champion_state::recalling{ std::uint32_t{ 1 } << 1 };
/// The champion is channelling a tribute.
inline constexpr champion_state champion_state::submitting_gems{ std::uint32_t{ 1 } << 2 };
/// The champion can still move this turn.
inline constexpr champion_state champion_state::can_move{ std::uint32_t{ 1 } << 3 };
/// The champion can still basic attack this turn.
inline constexpr champion_state champion_state::can_attack{ std::uint32_t{ 1 } << 4 };
/// The champion has finished its turn this round.
inline constexpr champion_state champion_state::completed_turn{ std::uint32_t{ 1 } << 5 };
/// A recall was started this turn.
inline constexpr champion_state champion_state::recall_started{ std::uint32_t{ 1 } << 6 };
/// A tribute was started this turn.
inline constexpr champion_state champion_state::submit_gems_started{ std::uint32_t{ 1 } << 7 };
/// A recall was cancelled this turn.
inline constexpr champion_state champion_state::recall_cancelled{ std::uint32_t{ 1 } << 8 };
/// A tribute was cancelled this turn.
inline constexpr champion_state champion_state::submit_gems_cancelled{ std::uint32_t{ 1 } << 9 };
/// The champion can still cast an ability this turn.
inline constexpr champion_state champion_state::can_cast{ std::uint32_t{ 1 } << 11 };
/// The champion is channelling a teleport crystal.
inline constexpr champion_state champion_state::teleporting{ std::uint32_t{ 1 } << 12 };

/// Camp monsters.
inline constexpr ability_target_mask ability_target_mask::camps{ std::uint32_t{ 1 } << 0 };
/// Enemy champions.
inline constexpr ability_target_mask ability_target_mask::enemies{ std::uint32_t{ 1 } << 1 };
/// Allied champions.
inline constexpr ability_target_mask ability_target_mask::allies{ std::uint32_t{ 1 } << 2 };
/// The caster itself.
inline constexpr ability_target_mask ability_target_mask::self{ std::uint32_t{ 1 } << 3 };
/// A map tile.
inline constexpr ability_target_mask ability_target_mask::ground{ std::uint32_t{ 1 } << 4 };

/// Prevents moving.
inline constexpr buff_restrictions buff_restrictions::move{ std::uint8_t{ 1 } << 0 };
/// Prevents basic attacks.
inline constexpr buff_restrictions buff_restrictions::attack{ std::uint8_t{ 1 } << 1 };
/// Prevents casting abilities.
inline constexpr buff_restrictions buff_restrictions::cast{ std::uint8_t{ 1 } << 2 };
/// Prevents buying and selling.
inline constexpr buff_restrictions buff_restrictions::shop{ std::uint8_t{ 1 } << 3 };
/// Prevents using consumables.
inline constexpr buff_restrictions buff_restrictions::consumable{ std::uint8_t{ 1 } << 4 };
/// Prevents placing wards.
inline constexpr buff_restrictions buff_restrictions::ward{ std::uint8_t{ 1 } << 5 };

namespace detail
{
/// Entity types used when selecting action targets.
enum class entity_kind : std::uint32_t
{
	champion = 1,
	camp = 2,
	altar_lock = 6
};

/// Typed engine handle for an action target.
struct entity
{
	/// Host entity category that determines how the handle is interpreted.
	entity_kind	  kind;
	/// Engine identity within the selected category.
	std::uint32_t handle;

	/// Construct an action target with its exact engine kind and handle.
	constexpr entity(entity_kind kind, std::uint32_t handle) : kind{ kind }, handle{ handle } {}
};
} // namespace detail

namespace detail
{
/// Reject actions or private queries for a champion other than the authenticated self.
inline void require_self(std::uint32_t handle)
{
	const auto own = sdk_my_champion();
	if (own < 0 || handle != static_cast<std::uint32_t>(own))
		throw std::invalid_argument("action is only available for self");
}

/// Query an owning terrain mask selected by the engine map-mask tag.
inline bitboard map_mask(std::int32_t kind)
{
	bitboard result{ uninitialized };
	check(sdk_bc_map_mask(kind, result.ptr()));
	return result;
}
} // namespace detail

/// Properties shared by every ability payload.
struct ability_properties
{
	/// Configured ability cooldown in rounds, not the live remaining cooldown.
	std::int32_t  max_cooldown{};
	/// Allowed target categories; separate from the geometric casting range.
	ability_target_mask valid_targets{};
	/// Casting and normal movement are mutually exclusive in a round.
	bool exclusive_with_movement{};
	/// Compare every field.
	[[nodiscard]] friend bool operator==(const ability_properties&, const ability_properties&) = default;
};

/// Attack-reset metadata; this payload has no targeting range.
struct ability_attack_reset : ability_properties
{
	/// Payload kind tag.
	static constexpr ability_kind kind = ability_kind::attack_reset;
	/// Whether the reset grants an enhanced next attack.
	bool						  enhanced_attack{};
	/// Compare every field.
	[[nodiscard]] friend bool operator==(const ability_attack_reset&, const ability_attack_reset&) = default;
};

/// Cached direct-damage ability metadata.
struct ability_damage : ability_properties
{
	/// Payload kind tag.
	static constexpr ability_kind kind = ability_kind::damage;
	/// Cast range in tiles and engine effect-scaling modifier.
	float						  range{}, modifier{};
	/// Compare every field.
	[[nodiscard]] friend bool operator==(const ability_damage&, const ability_damage&) = default;
};

/// Cached area-damage metadata, including effect radius and duration.
struct ability_aoe_damage : ability_properties
{
	/// Payload kind tag.
	static constexpr ability_kind kind = ability_kind::aoe_damage;
	/// Cast range, effect-scaling modifier and area radius; distances are in tiles.
	float						  range{}, modifier{}, radius{};
	/// Effect duration or remaining buff time in engine rounds.
	std::int32_t				  duration{};
	/// Compare every field.
	[[nodiscard]] friend bool operator==(const ability_aoe_damage&, const ability_aoe_damage&) = default;
};

/// Cached crowd-control ability metadata.
struct ability_cc : ability_properties
{
	/// Payload kind tag.
	static constexpr ability_kind kind = ability_kind::cc;
	/// Cast range in tiles and engine effect-scaling modifier.
	float						  range{}, modifier{};
	/// Effect duration or remaining buff time in engine rounds.
	std::int32_t				  duration{};
	/// Compare every field.
	[[nodiscard]] friend bool operator==(const ability_cc&, const ability_cc&) = default;
};

/// Cached healing ability metadata.
struct ability_heal : ability_properties
{
	/// Payload kind tag.
	static constexpr ability_kind kind = ability_kind::heal;
	/// Cast range in tiles and engine effect-scaling modifier.
	float						  range{}, modifier{};
	/// Compare every field.
	[[nodiscard]] friend bool operator==(const ability_heal&, const ability_heal&) = default;
};

/// Cached dash ability metadata.
struct ability_dash : ability_properties
{
	/// Payload kind tag.
	static constexpr ability_kind kind = ability_kind::dash;
	/// Maximum dash distance in tiles.
	float						  range{};
	/// Compare every field.
	[[nodiscard]] friend bool operator==(const ability_dash&, const ability_dash&) = default;
};

/// Cached shield ability metadata.
struct ability_shield : ability_properties
{
	/// Payload kind tag.
	static constexpr ability_kind kind = ability_kind::shield;
	/// Cast range in tiles and engine effect-scaling modifier.
	float						  range{}, modifier{};
	/// Effect duration or remaining buff time in engine rounds.
	std::int32_t				  duration{};
	/// Compare every field.
	[[nodiscard]] friend bool operator==(const ability_shield&, const ability_shield&) = default;
};

/// Owned, tagged ability metadata; use ability_types tags to select a typed payload.
using ability_info = std::variant<ability_attack_reset, ability_damage, ability_aoe_damage, ability_cc, ability_heal,
								  ability_dash, ability_shield>;

namespace ability_types
{
/// Typed tag selecting the attack-reset payload.
inline constexpr type_tag_t<ability_attack_reset> attack_reset{};
/// Typed tag selecting direct-damage metadata.
inline constexpr type_tag_t<ability_damage>			   damage{};
/// Typed tag selecting area-damage metadata.
inline constexpr type_tag_t<ability_aoe_damage>		   aoe_damage{};
/// Typed tag selecting crowd-control metadata.
inline constexpr type_tag_t<ability_cc>				   cc{};
/// Typed tag selecting healing metadata.
inline constexpr type_tag_t<ability_heal>			   heal{};
/// Typed tag selecting dash metadata.
inline constexpr type_tag_t<ability_dash>			   dash{};
/// Typed tag selecting shield metadata.
inline constexpr type_tag_t<ability_shield>			   shield{};
} // namespace ability_types

/// Yield the authenticated bot's current turn to the host.
inline void end_turn();
class champion;
class camp;
class altar_lock;
namespace detail
{
struct handle_access;
}


/// A consumable inventory entry belonging to a champion.
class consumable
{
	friend class champion;
	friend struct detail::handle_access;
	consumable(handle_champion_t owner, consumable_type type) : owner_{ static_cast<std::uint32_t>(owner) }, type_{ type }
	{
	}

public:
	/// Create an inventory entry; live queries require a champion-bound entry.
	consumable() = default;

	/// Get the entity or inventory entry kind.
	[[nodiscard]] consumable_type kind() const noexcept { return type_; }

	/// Get the number currently owned.
	[[nodiscard]] std::uint32_t count() const
	{
		std::array<std::uint32_t, 4> counts{};
		detail::check(detail::sdk_champion_consumables(owner_, counts.data()));
		return counts[raw_id()];
	}

	/// Use the entry on a target or tile, if required; the host validates the self-only action.
	/// Returns false for a newly revealed hidden obstruction; known invalid destinations throw.
	/// @param destination Tile to use the consumable at.
	bool use(vec2 destination) const
	{
		detail::require_self(owner_);
		std::int32_t successful{};
		detail::check(detail::sdk_use_consumable(raw_id(), destination.x, destination.y, &successful));
		return successful != 0;
	}

	/// Use the entry on a target or tile, if required; the host validates the self-only action.
	/// Returns false for a newly revealed hidden obstruction; known invalid destinations throw.
	bool use() const { return use({ -1, -1 }); }

private:
	[[nodiscard]] std::uint32_t raw_id() const
	{
		const auto index = static_cast<std::size_t>(type_);
		if (index >= detail::raw_consumable_ids.size())
			throw std::out_of_range("invalid consumable type");
		return detail::raw_consumable_ids[index];
	}

	std::uint32_t	owner_{ std::numeric_limits<std::uint32_t>::max() };
	consumable_type type_{ consumable_type::tp_crystal };
};

/// Supported active buff kinds; count is an inventory-size sentinel.
enum class buff_types : std::uint32_t
{
	/// Absorbs incoming damage; the earliest-expiring shield absorbs first.
	shield,
	/// Prevents actions and interrupts channels.
	stun,
	/// The next basic attack is enhanced.
	enhanced_attack,
	/// Number of buff types; not a valid buff.
	count
};

/// Get the enumerator name.
/// @param value Value to format.
[[nodiscard]] inline std::string to_string(buff_types value)
{
	switch (value)
	{
	case buff_types::shield: return "shield";
	case buff_types::stun: return "stun";
	case buff_types::enhanced_attack: return "enhanced_attack";
	default: break;
	}
	return "buff_types(" + std::to_string(static_cast<std::uint32_t>(value)) + ")";
}

/// Buff timers are remaining engine rounds, including any application-round extension.
struct buff_stun
{
	/// Payload kind tag.
	static constexpr buff_types kind = buff_types::stun;
	/// Effect duration or remaining buff time in engine rounds.
	std::int32_t duration{};
	/// Compare every field.
	[[nodiscard]] friend bool operator==(const buff_stun&, const buff_stun&) = default;
};

/// Owned shield snapshot with remaining duration and shield value.
struct buff_shield
{
	/// Payload kind tag.
	static constexpr buff_types kind = buff_types::shield;
	/// Effect duration or remaining buff time in engine rounds.
	std::int32_t duration{};
	/// Shield value captured when the buff snapshot was queried.
	float		 value{};
	/// Compare every field.
	[[nodiscard]] friend bool operator==(const buff_shield&, const buff_shield&) = default;
};

/// Owned enhanced-attack snapshot with remaining duration.
struct buff_enhanced_attack
{
	/// Payload kind tag.
	static constexpr buff_types kind = buff_types::enhanced_attack;
	/// Effect duration or remaining buff time in engine rounds.
	std::int32_t duration{};
	/// Compare every field.
	[[nodiscard]] friend bool operator==(const buff_enhanced_attack&, const buff_enhanced_attack&) = default;
};

/// Owned, tagged active-buff snapshot; each alternative includes its remaining duration.
using buff_t = std::variant<buff_shield, buff_stun, buff_enhanced_attack>;

/// Buff values captured at construction. Call unit::buffs() for fresh values.
class buff_collection
{
	using storage_type = enum_map<buff_types, std::optional<buff_t>>;

public:
	/// Capture current buffs once; hidden units and champion designs throw cambc::error.

	/// Get the requested buff snapshot; absent buffs throw std::out_of_range.
	/// @param kind Buff kind to look up.
	[[nodiscard]] buff_t get(buff_types kind) const
	{
		const auto& value = values_[kind];
		if (!value)
			throw std::out_of_range("unit does not have this buff");
		return *value;
	}

	/// Get the requested buff snapshot; absent buffs throw std::out_of_range.
	template <class T> [[nodiscard]] T get() const { return std::get<T>(get(T::kind)); }

	/// Check the captured collection for this buff kind without querying the host.
	/// @param kind Buff kind to test for.
	[[nodiscard]] bool has(buff_types kind) const { return values_[kind].has_value(); }

	/// Check the captured collection for the selected buff payload type.
	template <class T> [[nodiscard]] bool has() const { return has(T::kind); }
	/// Check whether the captured collection has a shield buff.
	[[nodiscard]] bool has_shield() const { return has(buff_types::shield); }
	/// Check whether the captured collection has a stun buff.
	[[nodiscard]] bool has_stun() const { return has(buff_types::stun); }
	/// Check whether the captured collection has an enhanced-attack buff.
	[[nodiscard]] bool has_enhanced_attack() const { return has(buff_types::enhanced_attack); }

	/// Get the number of entries in this container.
	[[nodiscard]] std::size_t size() const
	{
		return static_cast<std::size_t>(
			std::count_if(values_.begin(), values_.end(), [](const auto& value) { return value.has_value(); }));
	}

	/// Check whether the captured collection has no active buffs.
	[[nodiscard]] bool is_empty() const { return size() == 0; }


	/// Forward iterator over captured buffs; references remain valid while the collection lives.
	class const_iterator
	{
	public:
		/// Iterated buff snapshot type.
		using value_type = buff_t;
		/// Iterator distance type.
		using difference_type = std::ptrdiff_t;
		/// Standard iterator category.
		using iterator_category = std::forward_iterator_tag;
		/// C++20 iterator concept.
		using iterator_concept = std::forward_iterator_tag;
		/// Reference to a captured buff.
		using reference = const buff_t&;
		/// Pointer to a captured buff.
		using pointer = const buff_t*;
		/// Construct an iterator that compares equal to the end sentinel.
		const_iterator() = default;

		/// Iterate the captured buffs in values.
		explicit const_iterator(const storage_type& values) : values_{ &values }
		{
			skip_empty();
		}

		/// Access the current buff.
		reference operator*() const { return *values_->ptr()[index_]; }

		/// Access the current buff.
		pointer operator->() const { return &**this; }

		/// Advance to the next captured buff.
		const_iterator& operator++()
		{
			++index_;
			skip_empty();
			return *this;
		}

		/// Advance to the next captured buff, returning the previous position.
		const_iterator operator++(int)
		{
			auto previous = *this;
			++*this;
			return previous;
		}

		/// Compare iterator positions.
		bool operator==(const const_iterator&) const = default;

		/// Check whether every captured buff has been visited.
		bool operator==(std::default_sentinel_t) const { return !values_ || index_ == values_->size(); }

	private:
		/// Advance the snapshot iterator past empty buff slots.
		void skip_empty()
		{
			while (index_ < values_->size() && !values_->ptr()[index_])
				++index_;
		}

		const storage_type* values_{};
		std::size_t								 index_{};
	};

	/// Iterate captured buffs without refreshing; the collection must outlive the iterator.
	[[nodiscard]] const_iterator begin() const { return const_iterator{ values_ }; }

	/// Get the past-the-end iterator for the stored values or owning snapshot.
	[[nodiscard]] std::default_sentinel_t end() const noexcept { return {}; }

	/// Compare every captured buff.
	[[nodiscard]] friend bool operator==(const buff_collection&, const buff_collection&) = default;

private:
	friend class unit;
	explicit buff_collection(detail::entity owner, bool design = false) : values_{ load(owner, design) } {}
	/// Query and decode one owning buff collection without shared mutable storage.
	[[nodiscard]] static storage_type load(detail::entity owner, bool design)
	{
		if (design)
			throw error{ error_code::api_not_in_vision, "design champion has no live buffs" };
		std::array<detail::buff_data, static_cast<std::size_t>(buff_types::count)> raw{};
		std::uint32_t count{};
		if (owner.kind == detail::entity_kind::camp)
		{
			std::array<detail::camp_buff_data, static_cast<std::size_t>(buff_types::count)> camps{};
			detail::check(detail::sdk_camp_buffs(owner.handle, camps.data(), camps.size(), &count));
			if (count > raw.size())
				throw std::runtime_error("invalid engine buff count");
			for (std::uint32_t i = 0; i < count; ++i)
			{
				raw[i].type = camps[i].type;
				if (raw[i].type == static_cast<std::uint32_t>(buff_types::shield))
					raw[i].data.shield = { camps[i].timer, camps[i].magnitude };
				else
					raw[i].data.stun.duration = camps[i].timer;
			}
		}
		else
			detail::check(detail::sdk_champion_buff_data(owner.handle, raw.data(), raw.size(), &count));
		if (count > raw.size())
			throw std::runtime_error("invalid engine buff count");
		storage_type values{};
		for (std::uint32_t i = 0; i < count; ++i)
		{
			switch (static_cast<buff_types>(raw[i].type))
			{
			case buff_types::shield:
				values[buff_types::shield] = buff_shield{ raw[i].data.shield.duration, raw[i].data.shield.value };
				break;
			case buff_types::stun:
				values[buff_types::stun] = buff_stun{ raw[i].data.stun.duration };
				break;
			case buff_types::enhanced_attack:
				values[buff_types::enhanced_attack] = buff_enhanced_attack{ raw[i].data.enhanced_attack.duration };
				break;
			default:
				throw std::runtime_error("unknown engine buff kind");
			}
		}
		return values;
	}

	const storage_type values_;
};

/// Common live combat queries for champions and camps.
class unit
{
protected:
	/// Bind common combat queries to a live entity or a design-only champion.
	explicit unit(detail::entity owner, bool design = false) : owner_{ owner }, design_{ design } {}
	/// Return this view's typed engine identity.
	[[nodiscard]] detail::entity entity() const noexcept { return owner_; }
	friend class ability;
	friend class champion;

public:
	/// Construct a fresh owning buff collection; previous collections retain their captured values.
	[[nodiscard]] buff_collection buffs() const { return buff_collection{ owner_, design_ }; }
	/// Destroy a derived unit view through the shared interface.
	virtual ~unit() = default;
	/// Query current health; live state requires a visible runtime identity.
	[[nodiscard]] virtual float health() const = 0;
	/// Query current maximum health; live state requires a visible runtime identity.
	[[nodiscard]] virtual float max_health() const = 0;
	/// Query current strength for this visible unit.
	[[nodiscard]] virtual float strength() const = 0;
	/// Query current focus for this visible unit.
	[[nodiscard]] virtual float focus() const = 0;
	/// Attack range in Chebyshev tiles; melee units have range 1.
	[[nodiscard]] virtual std::int32_t attack_range() const = 0;
	/// Build the live unit's Chebyshev attack-range mask; does not filter targets or attack availability.
	/// Hidden units and design-only champions propagate position/range query errors.
	[[nodiscard]] bitboard attack_tiles() const
	{
		bitboard result{ uninitialized };
		attack_tiles_into(result);
		return result;
	}

	/// Like `attack_tiles`, but writes into `out` instead of returning a new board.
	/// @param out Destination; its previous contents are overwritten.
	void attack_tiles_into(bitboard& out) const
	{
		out.clear();
		out.draw_chebyshev(position(), attack_range());
	}
	/// Get the tile position; live entity views query current engine state.
	[[nodiscard]] virtual vec2 position() const = 0;
	/// Check whether live state is available; hidden units and design-only views return false.
	[[nodiscard]] virtual bool is_visible() const = 0;

private:
	detail::entity owner_;
	bool design_{};
};

namespace map
{
class camp_group;
} // namespace map

/// A runtime camp, with public spawn metadata and visibility-checked combat queries.
class camp : public unit
{
	friend struct detail::handle_access;
	/// Bind a physical camp handle to its engine-defined group, type and spawn tile.
	camp(handle_camp_t handle, handle_camp_group_t group, camp_type type, vec2 position)
		: unit{ detail::entity{ detail::entity_kind::camp, static_cast<std::uint32_t>(handle) } },
		  handle_{ handle }, group_{ group }, type_{ type }, spawn_position_{ position } {}

public:

	/// Convert to the strong camp identity.
	[[nodiscard]] operator handle_camp_t() const noexcept { return handle_; }
	/// Compare camp identities.
	friend bool operator==(const camp& a, const camp& b) noexcept { return a.handle_ == b.handle_; }
	/// Get the entity or inventory entry kind.
	[[nodiscard]] camp_type kind() const noexcept { return type_; }
	/// Check whether this camp is a wolf.
	[[nodiscard]] bool is_wolf() const noexcept { return kind() == camp_type::wolf; }
	/// Check whether this camp is an elk.
	[[nodiscard]] bool is_elk() const noexcept { return kind() == camp_type::elk; }
	/// Check whether this camp is a gem cluster.
	[[nodiscard]] bool is_gem_cluster() const noexcept { return kind() == camp_type::gem_cluster; }
	/// Check whether this camp is a gold cluster.
	[[nodiscard]] bool is_gold_cluster() const noexcept { return kind() == camp_type::gold_cluster; }
	/// Check whether this camp is a monolith.
	[[nodiscard]] bool is_monolith() const noexcept { return kind() == camp_type::monolith; }
	/// Get the camp's original spawn tile, not its current live position.
	[[nodiscard]] vec2 spawn_position() const noexcept { return spawn_position_; }
	/// Query this camp's group from the engine on every call; invalid identities throw.
	[[nodiscard]] map::camp_group camp_group() const;
	/// Query current health; live state requires a visible runtime identity.
	[[nodiscard]] float health() const override { return info().health; }
	/// Query current maximum health; live state requires a visible runtime identity.
	[[nodiscard]] float max_health() const override { return info().max_health; }
	/// Get the tile position; live entity views query current engine state.
	[[nodiscard]] vec2 position() const override
	{
		const auto value = info();
		return { value.x, value.y };
	}
	/// Query current strength for this visible unit.
	[[nodiscard]] float strength() const override { return info().strength; }
	/// Query current focus for this visible unit.
	[[nodiscard]] float focus() const override { return info().focus; }
	/// Check whether live state is available; hidden units and design-only views return false.
	[[nodiscard]] bool is_visible() const override
	{
		detail::camp_state value{};
		const auto code = detail::sdk_camp(static_cast<std::uint32_t>(handle_), &value);
		if (code == static_cast<std::int32_t>(error_code::api_not_in_vision) ||
			code == static_cast<std::int32_t>(error_code::api_not_found))
			return false;
		detail::check(code);
		return true;
	}
	/// Get an owning snapshot of the camp's engine-provided design stats.
	[[nodiscard]] camp_stats base_stats() const
	{
		detail::camp_design value{};
		detail::check(detail::sdk_camp_design(static_cast<std::int32_t>(kind()), &value));
		return value.base_stats;
	}
	/// Get attack range from this camp's design; melee camps have range 1.
	[[nodiscard]] std::int32_t attack_range() const override { return base_stats().base_attack_range; }

private:
	/// Query a fresh visible camp state record; hidden or missing camps throw.
	[[nodiscard]] detail::camp_state info() const
	{
		detail::camp_state value{};
		detail::check(detail::sdk_camp(static_cast<std::uint32_t>(handle_), &value));
		return value;
	}

	handle_camp_t handle_;
	handle_camp_group_t group_;
	camp_type type_;
	vec2 spawn_position_;
};

/// An immutable observation of a visible altar lock.
class altar_lock
{
public:
	/// Copy a visible altar-lock observation; its fields remain a snapshot.
	explicit altar_lock(detail::altar_lock_info value) : value_{ value } {}
	/// Get the tile captured in this altar-lock observation.
	[[nodiscard]] vec2 position() const noexcept { return { value_.x, value_.y }; }
	/// Get the owning team relative to the authenticated bot.
	[[nodiscard]] team team() const { return value_.team == static_cast<std::uint32_t>(detail::sdk_my_team()) ? team::ally : team::enemy; }
	/// Check whether this altar lock belongs to the observing team.
	[[nodiscard]] bool is_ally() const { return team() == cambc::team::ally; }
	/// Check whether this altar lock belongs to the opposing team.
	[[nodiscard]] bool is_enemy() const { return team() == cambc::team::enemy; }
	/// Get the remaining hits from the captured altar-lock observation.
	[[nodiscard]] std::uint32_t hits_remaining() const noexcept { return value_.hits_remaining; }
	/// Compare every captured field.
	[[nodiscard]] friend bool operator==(const altar_lock&, const altar_lock&) = default;

private:
	friend class champion;
	[[nodiscard]] detail::entity entity() const noexcept { return { detail::entity_kind::altar_lock, value_.handle }; }
	detail::altar_lock_info value_;
};

/// A champion ability slot; actions require the owning champion to be self.
class ability
{
	friend class champion;
	friend struct detail::handle_access;
	ability(handle_champion_t owner, std::uint32_t slot) : owner_{ static_cast<std::uint32_t>(owner) }, slot_{ slot }
	{
		detail::ability_props props{};
		try
		{
			detail::check(detail::sdk_champion_ability_props(owner_, slot_, &props));
			if (props.kind == 0)
				return;
		}
		catch (const error& failure)
		{
			if (failure.code() != error_code::api_not_in_vision)
				throw;
			info_error_ = failure;
			return;
		}
		initialize(props);
	}

public:
	/// Copy immutable design metadata.
	explicit ability(const detail::ability_props& props, std::uint32_t slot)
		: owner_{ std::numeric_limits<std::uint32_t>::max() }, slot_{ slot }, design_{ true }
	{
		initialize(props);
	}

private:
	/// Decode the tagged engine payload into cached, typed ability metadata.
	void initialize(const detail::ability_props& props)
	{
		if (props.kind == 0)
			return;
		ability_properties common{ props.cooldown, ability_target_mask::from_bits_retain(props.targets),
			(props.attributes & detail::ability_attribute_exclusive_with_movement) != 0 };
		switch (static_cast<ability_kind>(props.kind))
		{
		case ability_kind::attack_reset:
			info_.emplace(ability_attack_reset{ common, props.payload.enhanced_attack != 0 });
			break;
		case ability_kind::damage:
			info_.emplace(ability_damage{ common, props.payload.damage.range, props.payload.damage.modifier });
			break;
		case ability_kind::aoe_damage:
			info_.emplace(ability_aoe_damage{ common, props.payload.aoe_damage.range, props.payload.aoe_damage.modifier,
										  props.payload.aoe_damage.radius, props.payload.aoe_damage.duration });
			break;
		case ability_kind::cc:
			info_.emplace(ability_cc{ common, props.payload.cc.range, props.payload.cc.modifier, props.payload.cc.duration });
			break;
		case ability_kind::heal:
			info_.emplace(ability_heal{ common, props.payload.heal.range, props.payload.heal.modifier });
			break;
		case ability_kind::dash:
			info_.emplace(ability_dash{ common, props.payload.dash_range });
			break;
		case ability_kind::shield:
			info_.emplace(ability_shield{ common, props.payload.shield.range, props.payload.shield.modifier, props.payload.shield.duration });
			break;
		default:
			throw std::runtime_error("unknown engine ability kind");
		}
	}

public:
	/// Return this ability's fixed champion slot index.
	[[nodiscard]] std::uint32_t slot() const noexcept { return slot_; }

	/// Get the remaining owner turn-end ticks; zero means ready.
	[[nodiscard]] std::int32_t cooldown() const
	{
		if (design_)
			throw error{ error_code::api_not_in_vision, "design champion has no live cooldown" };
		std::int32_t result{};
		detail::check(detail::sdk_champion_cooldown(owner_, slot_, &result));
		return result;
	}

	/// Query whether the live ability has a positive remaining cooldown.
	[[nodiscard]] bool is_on_cooldown() const { return cooldown() > 0; }

	/// Check whether this ability is ready to cast.
	[[nodiscard]] bool is_ready() const;

	/// Query the engine's live target-category flags.
	[[nodiscard]] ability_target_mask valid_targets() const
	{
		if (design_)
			return info_ ? std::visit([](const auto& value) { return value.valid_targets; }, *info_) : ability_target_mask{};
		detail::ability_props props{};
		detail::check(detail::sdk_champion_ability_props(owner_, slot_, &props));
		return ability_target_mask::from_bits_retain(props.targets);
	}

	/// Casting range only; does not filter visibility, occupancy or target categories.
	/// Throws for range-less or unused slots, hidden owners, and design-only abilities.
	[[nodiscard]] bitboard valid_tiles() const
	{
		bitboard result{ uninitialized };
		mask_into(result);
		return result;
	}

	/// Like `valid_tiles`, but writes into `out` instead of returning a new board.
	/// @param out Destination; its previous contents are overwritten.
	void mask_into(bitboard& out) const
	{
		const auto range = std::visit([](const auto& value) -> float {
			if constexpr (requires { value.range; })
				return value.range;
			else
				throw std::invalid_argument("ability has no casting range");
		}, info().value());
		if (design_)
			throw error{ error_code::api_not_in_vision, "design champion has no position" };
		detail::champion_info owner{};
		detail::check(detail::sdk_champion(owner_, &owner));
		out.clear();
		out.draw_chebyshev({ owner.x, owner.y }, range);
	}

	/// Check target categories, not cast readiness, range or visibility; allies/enemies exclude self.
	[[nodiscard]] bool can_target_camps() const { return valid_targets().contains(ability_target_mask::camps); }

	/// Check the allowed-target mask for enemy champions.
	[[nodiscard]] bool can_target_enemies() const { return valid_targets().contains(ability_target_mask::enemies); }

	/// Check the allowed-target mask for allied champions.
	[[nodiscard]] bool can_target_allies() const { return valid_targets().contains(ability_target_mask::allies); }

	/// Check the allowed-target mask for the casting champion.
	[[nodiscard]] bool can_target_self() const { return valid_targets().contains(ability_target_mask::self); }

	/// Check the allowed-target mask for a tile destination.
	[[nodiscard]] bool can_target_ground() const { return valid_targets().contains(ability_target_mask::ground); }

	/// Get constructor-cached ability properties; empty for an unused slot.
	/// Rethrow the cached visibility error if the owner could not be inspected at construction.
	[[nodiscard]] const std::optional<ability_info>& info() const
	{
		if (info_error_)
			throw *info_error_;
		return info_;
	}

	/// Extract a payload; throws std::bad_optional_access if absent, std::bad_variant_access for a different kind.
	template <class T> [[nodiscard]] T get() const { return std::get<T>(info().value()); }

	/// Get selected metadata; unavailable or mismatched payloads throw.
	/// @param kind Ability kind the slot must hold.
	[[nodiscard]] const ability_info& info(ability_kind kind) const
	{
		const auto& info = this->info().value();
		if (!is(kind))
			throw std::bad_variant_access{};
		return info;
	}

	/// Check the kind of constructor-cached ability metadata without querying the host.
	/// @param kind Ability kind to test for.
	[[nodiscard]] bool is(ability_kind kind) const noexcept
	{
		return info_ && std::visit([kind](const auto& value) { return value.kind == kind; }, *info_);
	}
	/// Check whether this is an attack-reset ability.
	[[nodiscard]] bool is_attack_reset() const noexcept { return is(ability_kind::attack_reset); }
	/// Check whether this is a damage ability.
	[[nodiscard]] bool is_damage() const noexcept { return is(ability_kind::damage); }
	/// Check whether this is an area-damage ability.
	[[nodiscard]] bool is_aoe_damage() const noexcept { return is(ability_kind::aoe_damage); }
	/// Check whether this is a crowd-control ability.
	[[nodiscard]] bool is_cc() const noexcept { return is(ability_kind::cc); }
	/// Check whether this is a healing ability.
	[[nodiscard]] bool is_heal() const noexcept { return is(ability_kind::heal); }
	/// Check whether this is a dash ability.
	[[nodiscard]] bool is_dash() const noexcept { return is(ability_kind::dash); }
	/// Check whether this is a shield ability.
	[[nodiscard]] bool is_shield() const noexcept { return is(ability_kind::shield); }

	/// Check whether constructor-cached metadata holds the selected payload type.
	template <class T> [[nodiscard]] bool is(type_tag_t<T>) const noexcept
	{
		return info_ && std::holds_alternative<T>(*info_);
	}

	/// Query damage before absorption/health limits; includes caster items and passives.
	[[nodiscard]] float damage_amount() const
	{
		require_runtime();
		float result{};
		detail::check(detail::sdk_champion_ability_damage(owner_, slot_, &result));
		return result;
	}

	/// Predict health lost after mitigation and shields, capped at current health. Target must be visible.
	/// @param target Visible champion that would be hit.
	[[nodiscard]] float damage_amount(const champion& target) const;

	/// Query heal before absorption/health limits; includes caster items and passives.
	[[nodiscard]] float heal_amount() const
	{
		require_runtime();
		float result{};
		detail::check(detail::sdk_champion_ability_heal(owner_, slot_, &result));
		return result;
	}

	/// Cap healing at the visible target's missing health. Does not cast or consume resources.
	/// @param target Visible champion that would be healed.
	[[nodiscard]] float heal_amount(const champion& target) const;

	/// Query shield before absorption/health limits; includes caster items and passives.
	[[nodiscard]] float shield_amount() const
	{
		require_runtime();
		float result{};
		detail::check(detail::sdk_champion_ability_shield(owner_, slot_, &result));
		return result;
	}

	/// Query full shield strength for a visible target. Does not cast or consume resources.
	/// @param target Visible champion that would be shielded.
	[[nodiscard]] float shield_amount(const champion& target) const;

	/// Cast without a target or destination.
	/// Returns false for a newly revealed hidden obstruction; known invalid destinations throw.
	bool use() const { return cast_entity(std::nullopt, { -1, -1 }); }

	/// Cast at a ground tile.
	/// Returns false for a newly revealed hidden obstruction; known invalid destinations throw.
	/// @param destination Ground tile to cast at.
	bool use(vec2 destination) const { return cast_entity(std::nullopt, destination); }

	/// Cast on a champion; unit casts land on the target's tile.
	/// Returns false for a newly revealed hidden obstruction; known invalid destinations throw.
	/// @param target Champion to cast on.
	bool use(const champion& target) const;
	/// Cast on a camp; the cast lands on the target's tile.
	/// Returns false for a newly revealed hidden obstruction; known invalid destinations throw.
	/// @param target Camp to cast on.
	bool use(const camp& target) const
	{
		return cast_entity(detail::entity{ detail::entity_kind::camp, static_cast<std::uint32_t>(static_cast<handle_camp_t>(target)) }, { -1, -1 });
	}

private:
	bool cast_entity(std::optional<detail::entity> target, vec2 destination) const
	{
		detail::require_self(owner_);
		std::int32_t successful{};
		detail::check(detail::sdk_cast(slot_, target ? static_cast<std::uint32_t>(target->kind) : 0, target ? target->handle : 0,
								  destination.x, destination.y, &successful));
		return successful != 0;
	}
	std::uint32_t owner_, slot_;
	/// Reject live queries on an ability that only describes a champion design.
	void require_runtime() const
	{
		if (design_)
			throw error{ error_code::api_not_in_vision, "design ability has no live owner" };
	}
	bool design_{};
	std::optional<ability_info> info_;
	std::optional<error> info_error_;
};

/// Champion handle with live queries and self-only actions.
class champion : public unit
{
	friend struct detail::handle_access;
	explicit champion(handle_champion_t handle)
		: unit{ detail::entity{ detail::entity_kind::champion, static_cast<std::uint32_t>(handle) } },
		  ability_slots{ ability{ handle, 0 }, ability{ handle, 1 } }, handle_{ static_cast<std::uint32_t>(handle) }
	{
		for (std::uint32_t i = 0; i < consumables_.size(); ++i)
		{
			const auto type = static_cast<consumable_type>(i);
			consumables_[type] = consumable{ handle, type };
		}
	}

public:
	/// A design-only champion: immutable properties are available, live queries require vision.
	explicit champion(champion_type type) : champion{ type, load_design(type) } {}

private:
	std::array<ability, 2>				  ability_slots;
	enum_map<consumable_type, consumable> consumables_;

public:
	/// Action wrappers by consumable type; each count is queried live.
	[[nodiscard]] const enum_map<consumable_type, consumable>& consumables() const noexcept
	{
		return consumables_;
	}

	/// Access the underlying engine handle.
	[[nodiscard]] operator handle_champion_t() const noexcept { return static_cast<handle_champion_t>(handle_); }

	/// Get current and maximum health.
	[[nodiscard]] float health() const override { return info().health; }

	/// Query current maximum health; live state requires a visible runtime identity.
	[[nodiscard]] float max_health() const override { return info().max_health; }

	/// Get the entity or inventory entry kind.
	[[nodiscard]] champion_type champion_name() const { return design_type_ ? *design_type_ : static_cast<champion_type>(info().type); }
	/// Get the passive effect tag from the champion's immutable design.
	[[nodiscard]] passive_type passive() const { return static_cast<passive_type>(load_design(champion_name()).definition.passive_type); }
	/// Get the owning team relative to the authenticated bot.
	[[nodiscard]] team team() const { return info().team == detail::sdk_my_team() ? team::ally : team::enemy; }
	/// Check whether this champion belongs to the observing team.
	[[nodiscard]] bool is_ally() const { return team() == cambc::team::ally; }
	/// Check whether this champion belongs to the opposing team.
	[[nodiscard]] bool is_enemy() const { return team() == cambc::team::enemy; }
	/// Query current strength for this visible unit.
	[[nodiscard]] float strength() const override { return info().strength; }
	/// Query current focus for this visible unit.
	[[nodiscard]] float focus() const override { return info().focus; }
	/// Get the unit's attack range in tiles; champion state is queried live.
	[[nodiscard]] std::int32_t attack_range() const override { return info().attack_range; }

	/// Get the tile position; live entity views query current engine state.
	[[nodiscard]] vec2 position() const override
	{
		const auto value = info();
		return { value.x, value.y };
	}

	/// Check whether the champion is alive.
	[[nodiscard]] bool is_alive() const
	{
		if (design_type_)
			throw error{ error_code::api_not_in_vision, "design champion has no live state" };
		std::int32_t alive{};
		detail::check(detail::sdk_champion_is_alive(handle_, &alive));
		return alive != 0;
	}

	/// Check whether the champion can be inspected from this perspective.
	[[nodiscard]] bool is_visible() const override
	{
		if (design_type_)
			return false;
		detail::champion_info result{};
		const auto			  code = detail::sdk_champion(handle_, &result);
		if (code == 97)
			return false; // api_notInVision
		detail::check(code);
		return result.x >= 0 && result.y >= 0;
	}

	/// Get current state and action availability.
	[[nodiscard]] champion_state state() const
	{
		return champion_state::from_bits_retain(info().state);
	}

	/// Check whether this champion can move.
	[[nodiscard]] bool can_move() const { return state().contains(champion_state::can_move); }
	/// Check whether this champion can attack.
	[[nodiscard]] bool can_attack() const { return state().contains(champion_state::can_attack); }
	/// Check whether this champion can cast.
	[[nodiscard]] bool can_cast() const { return state().contains(champion_state::can_cast); }
	/// Check whether this champion is recalling.
	[[nodiscard]] bool is_recalling() const { return state().contains(champion_state::recalling); }
	/// Check whether this champion is submitting gems.
	[[nodiscard]] bool is_submitting_gems() const { return state().contains(champion_state::submitting_gems); }
	/// Check whether this champion is teleporting.
	[[nodiscard]] bool is_teleporting() const { return state().contains(champion_state::teleporting); }
	/// Check whether this champion has completed its turn.
	[[nodiscard]] bool has_completed_turn() const { return state().contains(champion_state::completed_turn); }

	/// Union of active buff restrictions; requires visibility.
	[[nodiscard]] buff_restrictions restrictions() const
	{
		require_runtime();
		std::uint8_t result{};
		detail::check(detail::sdk_champion_restrictions(handle_, &result));
		return buff_restrictions::from_bits_retain(result);
	}

	/// Get tiles visible to this champion.
	[[nodiscard]] bitboard visible_tiles() const
	{
		bitboard result{ uninitialized };
		visible_tiles_into(result);
		return result;
	}

	/// Like `visible_tiles`, but writes into `out` instead of returning a new board.
	/// @param out Destination; its previous contents are overwritten.
	void visible_tiles_into(bitboard& out) const
	{
		if (design_type_)
			throw error{ error_code::api_not_in_vision, "design champion has no position" };
		detail::check(detail::sdk_bc_champion_vision(handle_, out.ptr()));
	}

	/// Vision at a hypothetical position, using this champion's design.
	/// @param position Hypothetical tile to see from.
	[[nodiscard]] bitboard visible_tiles_from(vec2 position) const
	{
		bitboard result{ uninitialized };
		visible_tiles_from_into(position, result);
		return result;
	}

	/// Like `visible_tiles_from`, but writes into `out` instead of returning a new board.
	/// @param position Hypothetical tile to see from.
	/// @param out Destination; its previous contents are overwritten.
	void visible_tiles_from_into(vec2 position, bitboard& out) const
	{
		detail::check(detail::sdk_hypothetical_vision(static_cast<std::int32_t>(champion_name()), position.x, position.y, out.ptr()));
	}

	/// Query the visible champion's remaining channel rounds; no active channel throws.
	[[nodiscard]] std::int32_t remaining_channel_time() const
	{
		(void)info();
		std::int32_t result{};
		detail::check(detail::sdk_champion_channel_time(handle_, &result));
		return result;
	}

	/// Query an owning list of wards placed by the authenticated self champion.
	[[nodiscard]] std::vector<vec2> ward_positions() const
	{
		detail::require_self(handle_);
		std::uint32_t count{};
		detail::check(detail::sdk_champion_ward_positions(handle_, nullptr, 0, &count));
		std::vector<detail::position> raw(count);
		detail::check(detail::sdk_champion_ward_positions(handle_, raw.data(), raw.size(), &count));
		std::vector<vec2> result;
		result.reserve(count);
		for (std::uint32_t i = 0; i < count; ++i)
			result.emplace_back(raw[i].x, raw[i].y);
		return result;
	}

	/// Query the authenticated champion's remaining ward recharge rounds.
	[[nodiscard]] std::int32_t ward_cooldown() const
	{
		detail::require_self(handle_);
		detail::ward_resources result{};
		detail::check(detail::sdk_champion_wards(handle_, &result));
		return result.cooldown;
	}

	/// Compute self-only ward placement range from current position and engine settings.
	[[nodiscard]] bitboard ward_placement_tiles() const
	{
		bitboard result{ uninitialized };
		ward_placement_tiles_into(result);
		return result;
	}

	/// Like `ward_placement_tiles`, but writes into `out` instead of returning a new board.
	/// @param out Destination; its previous contents are overwritten.
	void ward_placement_tiles_into(bitboard& out) const
	{
		detail::require_self(handle_);
		out.clear();
		out.draw_chebyshev(position(), constants::ward_placement_radius);
	}

	/// Check whether an item can be bought in the current state.
	/// @param item Item to buy.
	/// @param count Number of copies to buy.
	[[nodiscard]] bool can_buy(item_type item, std::uint32_t count = 1) const
	{
		detail::require_self(handle_);
		std::int32_t result{};
		detail::check(detail::sdk_champion_can_shop(handle_, false, &result));
		if (result == 0 || count == 0 || static_cast<std::uint32_t>(item) >= static_cast<std::uint32_t>(item_type::count))
			return false;
		const auto info = item.info();
		return info.purchasable && static_cast<std::uint64_t>(gold_count()) >=
			static_cast<std::uint64_t>(info.cost) * count;
	}

	/// Check whether a consumable can be bought in the current state.
	/// @param consumable Consumable to buy.
	[[nodiscard]] bool can_buy(consumable_type consumable) const
	{
		detail::require_self(handle_);
		std::int32_t result{};
		detail::check(detail::sdk_champion_can_shop(handle_, true, &result));
		if (result == 0 || static_cast<std::uint32_t>(consumable) >= static_cast<std::uint32_t>(consumable_type::count))
			return false;
		return gold_count() >= consumable.cost() &&
			consumables()[consumable].count() < consumable.carry_limit();
	}

	/// Check whether the authenticated self currently has permission to sell.
	[[nodiscard]] bool can_sell() const
	{
		detail::require_self(handle_);
		std::int32_t result{};
		detail::check(detail::sdk_champion_can_sell(handle_, &result));
		return result != 0;
	}

	/// Attack a champion or camp view; requires the authenticated self and host-validated action availability.
	/// @param target Visible champion or camp within attack range.
	void attack(const unit& target) const { attack_entity(target.entity()); }
	/// Attack an altar lock; requires the authenticated self and host-validated action availability.
	/// @param target Visible altar lock within attack range.
	void attack(const altar_lock& target) const { attack_entity(target.entity()); }

	/// Get the champion design base stats.
	[[nodiscard]] champion_stats base_stats() const
	{
		detail::champion_design result{};
		detail::check(detail::sdk_champion_design(static_cast<std::int32_t>(champion_name()), &result));
		return result.base_stats;
	}

	/// Return a copy of the two champion-bound ability views with immutable cached metadata.
	[[nodiscard]] std::array<ability, 2> abilities() const { return ability_slots; }

	/// Get item counts indexed by item type.
	[[nodiscard]] enum_map<item_type> item_count() const
	{
		require_runtime();
		enum_map<item_type> result;
		detail::check(detail::sdk_champion_items(handle_, result.ptr()));
		return result;
	}

	/// Get carried gems.
	[[nodiscard]] std::uint32_t gem_count() const
	{
		require_runtime();
		std::uint32_t result{};
		detail::check(detail::sdk_champion_gems(handle_, &result));
		return result;
	}

	/// Get gold; this query requires self.
	[[nodiscard]] std::uint32_t gold_count() const
	{
		detail::require_self(handle_);
		std::uint32_t result{};
		detail::check(detail::sdk_champion_gold(handle_, &result));
		return result;
	}

	/// Buy a consumable for self.
	/// @param type Consumable to buy.
	void buy(consumable_type type) const
	{
		detail::require_self(handle_);
		const auto index = static_cast<std::size_t>(type);
		if (index >= detail::raw_consumable_ids.size())
			throw error{ error_code::invalid_action_buy_consumable_invalid_type, "invalid consumable type" };
		detail::check(detail::sdk_buy_consumable(detail::raw_consumable_ids[index]));
	}

	/// Buy items for self.
	/// @param type Item to buy.
	/// @param count Number of copies to buy.
	void buy(item_type type, std::uint32_t count = 1) const
	{
		detail::require_self(handle_);
		detail::check(detail::sdk_buy_item(static_cast<std::uint32_t>(type), count));
	}

	/// Sell items for self; the engine validates champion and shop restrictions.
	/// @param type Item to sell.
	/// @param count Number of copies to sell.
	void sell(item_type type, std::uint32_t count = 1) const
	{
		detail::require_self(handle_);
		for (std::uint32_t i = 0; i < count; ++i)
			detail::check(detail::sdk_sell(static_cast<std::uint32_t>(type)));
	}

	/// Start recalling to spawn.
	void recall() const
	{
		detail::require_self(handle_);
		detail::check(detail::sdk_recall());
	}

	/// Cancel an active recall.
	void cancel_recall() const
	{
		detail::require_self(handle_);
		if (!state().contains(cambc::champion_state::recalling))
			throw std::invalid_argument("champion is not recalling");
		detail::check(detail::sdk_cancel_channel());
	}

	/// Place a ward at a destination tile.
	/// Returns false for a newly revealed hidden obstruction; known invalid destinations throw.
	/// @param destination Tile to place the ward on.
	bool place_ward(vec2 destination) const
	{
		detail::require_self(handle_);
		std::int32_t successful{};
		detail::check(detail::sdk_place_ward(destination.x, destination.y, &successful));
		return successful != 0;
	}

	/// Start submitting gems at an altar.
	/// @param altar Altar tile to tribute at.
	void submit_gems(vec2 altar) const
	{
		detail::require_self(handle_);
		detail::check(detail::sdk_submit_gems(altar.x, altar.y));
	}

	/// Cancel an active gem submission.
	void cancel_submit_gems() const
	{
		detail::require_self(handle_);
		if (!state().contains(cambc::champion_state::submitting_gems))
			throw std::invalid_argument("champion is not submitting gems");
		detail::check(detail::sdk_cancel_channel());
	}

	/// Attempt to move by a delta from the current position.
	/// Returns false for a newly revealed hidden obstruction; known invalid destinations throw.
	/// @param delta Displacement from the current position.
	bool move(vec2 delta) const
	{
		detail::require_self(handle_);
		const auto destination = position() + delta;
		std::int32_t successful{};
		detail::check(detail::sdk_move_to(destination.x, destination.y, &successful));
		return successful != 0;
	}

	/// Attempt to move to an absolute destination tile.
	/// Returns false for a newly revealed hidden obstruction; known invalid destinations throw.
	/// @param destination Map tile to move to.
	bool move_absolute(vec2 destination) const
	{
		detail::require_self(handle_);
		std::int32_t successful{};
		detail::check(detail::sdk_move_to(destination.x, destination.y, &successful));
		return successful != 0;
	}

	/// Move to the reachable tile on a shortest path to `target`, avoiding walls, holes, altars, the enemy spawn and
	/// visible units. Returns false without moving when already there, unable to move, or `target` is blocked or
	/// unreachable, and false for a newly revealed hidden obstruction.
	/// @param target Map tile to approach.
	bool move_towards(vec2 target) const;

	/// Set the scheduling priority for the next round.
	/// @param priority New scheduling priority; takes effect from the next round and persists.
	void set_priority(std::int32_t priority) const
	{
		detail::require_self(handle_);
		detail::check(detail::sdk_set_priority(priority));
	}

	/// Get the scheduling priority used for the current round.
	[[nodiscard]] std::int32_t prev_priority() const { return query_priority(false); }

	/// Get the scheduling priority for the next round.
	[[nodiscard]] std::int32_t priority() const { return query_priority(true); }

	/// Yield the current champion turn.
	void end_turn() const
	{
		detail::require_self(handle_);
		cambc::end_turn();
	}

	/// Compare champion identities.
	friend bool operator==(const champion& a, const champion& b) noexcept { return a.handle_ == b.handle_ && a.design_type_ == b.design_type_; }

private:
	/// Query and decode an immutable physical champion design.
	static detail::champion_design load_design(champion_type type)
	{
		detail::champion_design result{};
		detail::check(detail::sdk_champion_design(static_cast<std::int32_t>(type), &result));
		return result;
	}

	void attack_entity(detail::entity target) const
	{
		detail::require_self(handle_);
		detail::check(detail::sdk_attack(static_cast<std::uint32_t>(target.kind), target.handle));
	}
	/// Reject live queries on a champion view that only describes a design.
	void require_runtime() const
	{
		if (design_type_)
			throw error{ error_code::api_not_in_vision, "design champion has no live state" };
	}

	/// Construct a live champion view or immutable design; metadata errors propagate.
	champion(champion_type type, const detail::champion_design& design)
		: unit{ detail::entity{ detail::entity_kind::champion, std::numeric_limits<std::uint32_t>::max() }, true },
		  ability_slots{ ability{ design.definition.abilities[0], 0 }, ability{ design.definition.abilities[1], 1 } },
		  handle_{ std::numeric_limits<std::uint32_t>::max() }, design_type_{ type }
	{
		for (std::uint32_t i = 0; i < consumables_.size(); ++i)
		{
			const auto kind = static_cast<consumable_type>(i);
			consumables_[kind] = consumable{ static_cast<handle_champion_t>(handle_), kind };
		}
	}

	/// Query a visible champion's current or next-turn execution priority.
	[[nodiscard]] std::int32_t query_priority(bool next) const
	{
		if (design_type_)
			throw error{ error_code::api_not_in_vision, "design champion has no scheduling state" };
		std::int32_t result{};
		detail::check(detail::sdk_bc_champion_priority(handle_, next, &result));
		return result;
	}

	/// Query a fresh champion state record; designs and hidden champions throw.
	[[nodiscard]] detail::champion_info info() const
	{
		if (design_type_)
			throw error{ error_code::api_not_in_vision, "design champion has no live state" };
		detail::champion_info result{};
		detail::check(detail::sdk_champion(handle_, &result));
		return result;
	}

	std::uint32_t		   handle_;
	std::optional<champion_type> design_type_;
};

inline bool ability::use(const champion& target) const
{
	return cast_entity(target.entity(), { -1, -1 });
}

inline float ability::damage_amount(const champion& target) const
{
	require_runtime();
	float result{};
	detail::check(detail::sdk_champion_ability_damage_against(owner_, slot_, static_cast<std::uint32_t>(static_cast<handle_champion_t>(target)), &result));
	return result;
}

inline float ability::heal_amount(const champion& target) const
{
	require_runtime();
	float result{};
	detail::check(detail::sdk_champion_ability_heal_against(owner_, slot_, static_cast<std::uint32_t>(static_cast<handle_champion_t>(target)), &result));
	return result;
}

inline float ability::shield_amount(const champion& target) const
{
	require_runtime();
	float result{};
	detail::check(detail::sdk_champion_ability_shield_against(owner_, slot_, static_cast<std::uint32_t>(static_cast<handle_champion_t>(target)), &result));
	return result;
}

namespace detail
{
struct handle_access
{
	[[nodiscard]] static cambc::champion make_champion(handle_champion_t handle) { return cambc::champion{ handle }; }
	[[nodiscard]] static cambc::ability make_ability(handle_champion_t owner, std::uint32_t slot) { return cambc::ability{ owner, slot }; }
	[[nodiscard]] static cambc::consumable make_consumable(handle_champion_t owner, consumable_type type) { return cambc::consumable{ owner, type }; }
	[[nodiscard]] static cambc::camp make_camp(handle_camp_t handle, handle_camp_group_t group, camp_type type, vec2 position)
	{
		return cambc::camp{ handle, group, type, position };
	}
	[[nodiscard]] static map::camp_group make_camp_group(
		handle_camp_group_t handle, std::uint32_t gold, std::uint32_t gems, std::int32_t respawn, std::vector<cambc::camp> camps);
};
} // namespace detail

inline bool ability::is_ready() const
{
	if (!info())
		return false;
	return !is_on_cooldown() &&
		detail::handle_access::make_champion(static_cast<handle_champion_t>(owner_)).can_cast();
}





/// Immutable physical champion designs; abilities and base stats are available during draft, not live state.
inline const std::array<champion, 12> champion_designs = []<std::size_t... I>(std::index_sequence<I...>) {
	return std::array{ champion{ static_cast<champion_type>(I) }... };
}(std::make_index_sequence<12>{});

/// Whether this guest has a draft perspective, without querying the host.
[[nodiscard]] bool is_draft() noexcept;

/// Fixed-size team draft bytes captured at startup; assignment submits bytes without changing the snapshot.
class draft_memory_buffer
{
public:
	/// Load the current draft bytes into owning, read-only storage.
	draft_memory_buffer()
	{
		std::uint32_t size{};
		detail::check(detail::sdk_draft_memory_size(&size));
		bytes_.resize(size);
		if (size != 0)
			detail::check(detail::sdk_read_draft_memory(0, bytes_.data(), size));
	}

	/// Copy the captured bytes without querying or submitting to the engine.
	draft_memory_buffer(const draft_memory_buffer&) = default;

	/// Submit bytes immediately; errors throw and leave captured bytes unchanged.
	/// Oversized input is truncated to capacity; shorter input leaves the remaining engine bytes unchanged.
	draft_memory_buffer& operator=(std::span<const std::byte> bytes)
	{
		if (!cambc::is_draft())
			throw error{ error_code::invalid_action, "draft memory can only be submitted during the draft" };
		detail::check(detail::sdk_bc_submit_draft_memory(bytes.data(), bytes.size()));
		return *this;
	}

	/// Submit another buffer's captured bytes, including self-assignment.
	draft_memory_buffer& operator=(const draft_memory_buffer& other)
	{
		return *this = std::span<const std::byte>{ other.data(), other.size() };
	}

	/// Submit byte initialiser lists; an empty list submits no data.
	draft_memory_buffer& operator=(std::initializer_list<std::byte> bytes)
	{
		return *this = std::span<const std::byte>{ bytes.begin(), bytes.size() };
	}

	/// Submit contiguous arrays, vectors or spans of trivially copyable values using native layout.
	template <std::ranges::contiguous_range Range>
		requires (std::ranges::sized_range<Range> && std::is_trivially_copyable_v<std::ranges::range_value_t<Range>>)
	draft_memory_buffer& operator=(Range&& values)
	{
		return *this = std::as_bytes(std::span{ std::ranges::data(values), std::ranges::size(values) });
	}

	/// Get the captured byte count without querying the host.
	[[nodiscard]] std::size_t size() const { return bytes_.size(); }
	/// Check whether the captured buffer is empty.
	[[nodiscard]] bool is_empty() const { return bytes_.empty(); }
	/// Borrow bytes captured at startup; assignment does not change this snapshot.
	[[nodiscard]] const std::byte* data() const { return bytes_.data(); }
	/// Read one startup-snapshot byte; an out-of-range offset throws std::out_of_range.
	[[nodiscard]] const std::byte& operator[](std::size_t offset) const { return bytes_.at(offset); }
	/// Iterate over read-only captured bytes without querying the host.
	[[nodiscard]] auto begin() const { return bytes_.begin(); }
	/// Get the end of the read-only captured bytes.
	[[nodiscard]] auto end() const { return bytes_.end(); }

private:
	std::vector<std::byte> bytes_;
};

/// Team draft-memory snapshot captured at startup; assignments submit bytes without changing the snapshot.
inline draft_memory_buffer draft_memory{};


namespace detail
{
/// Authenticated champion view initialised at startup; absent in draft perspectives.
inline std::optional<champion> cached_self = []() -> std::optional<champion>
{
	const auto handle = detail::sdk_my_champion();
	if (handle == -1)
		return std::nullopt;
	return cambc::detail::handle_access::make_champion(static_cast<handle_champion_t>(handle));
}();
} // namespace detail

/// Return this guest's authenticated champion; throws std::bad_optional_access during drafting.
[[nodiscard]] inline cambc::champion& self() { return detail::cached_self.value(); }

/// Check whether this bot instance is running the draft rather than controlling a champion.
[[nodiscard]] inline bool is_draft() noexcept { return detail::sdk_my_champion() == -1; }

/// Get all champion handles on one side, including champions outside vision.
/// @param side Team to include.
[[nodiscard]] inline std::vector<cambc::champion> champions(team side)
{
	(void)detail::absolute_team(side);
	std::vector<cambc::champion> result;
	std::uint32_t count{};
	detail::check(side == team::ally ? detail::sdk_num_allies(&count) : detail::sdk_num_enemies(&count));
	std::vector<std::uint32_t> handles(count);
	detail::check(side == team::ally ? detail::sdk_allies(handles.data(), count, &count)
									 : detail::sdk_enemies(handles.data(), count, &count));
	result.reserve(count);
	for (std::uint32_t i = 0; i < count; ++i)
		result.push_back(detail::handle_access::make_champion(static_cast<handle_champion_t>(handles[i])));
	return result;
}

/// Get allied and enemy champion handles, including champions outside vision.
[[nodiscard]] inline std::vector<cambc::champion> champions()
{
	auto result = cambc::champions(team::ally);
	const auto enemies = cambc::champions(team::enemy);
	result.insert(result.end(), enemies.begin(), enemies.end());
	return result;
}

namespace detail
{
struct ward_access;
}

/// Immutable snapshot of a visible ward.
class ward
{
public:
	/// Get the ward's tile.
	[[nodiscard]] vec2 position() const noexcept { return position_; }
	/// Get the owning team relative to the authenticated bot.
	[[nodiscard]] cambc::team team() const noexcept { return team_; }
	/// Check whether this ward belongs to the observing team.
	[[nodiscard]] bool is_ally() const noexcept { return team() == cambc::team::ally; }
	/// Check whether this ward belongs to the opposing team.
	[[nodiscard]] bool is_enemy() const noexcept { return team() == cambc::team::enemy; }
	/// Get the remaining lifetime in rounds; empty for an unlimited lifetime.
	[[nodiscard]] std::optional<std::int32_t> remaining_time() const noexcept { return remaining_time_; }
	/// Query the mask for ordinary-ward vision at a placement tile.
	/// @param position Hypothetical ward tile.
	[[nodiscard]] static bitboard visible_tiles_from(vec2 position)
	{
		bitboard result{ uninitialized };
		visible_tiles_from_into(position, result);
		return result;
	}

	/// Like `visible_tiles_from`, but writes into `out` instead of returning a new board.
	/// @param position Hypothetical ward tile.
	/// @param out Destination; its previous contents are overwritten.
	static void visible_tiles_from_into(vec2 position, bitboard& out)
	{
		detail::check(detail::sdk_ward_vision(position.x, position.y, out.ptr()));
	}
	/// Hypothetical vision from this ward's position, ignoring current team visibility.
	[[nodiscard]] bitboard visible_tiles() const { return ward::visible_tiles_from(position_); }
	/// Like `visible_tiles`, but writes into `out`.
	/// @param out Destination; its previous contents are overwritten.
	void visible_tiles_into(bitboard& out) const { ward::visible_tiles_from_into(position_, out); }
	/// Compare every captured field.
	[[nodiscard]] friend bool operator==(const ward&, const ward&) = default;

private:
	friend struct detail::ward_access;

	explicit ward(const detail::ward_info& raw)
		: position_{ raw.x, raw.y },
		  team_{ raw.team == static_cast<std::uint32_t>(detail::sdk_my_team()) ? team::ally : team::enemy },
		  remaining_time_{ raw.timer < 0 ? std::nullopt : std::optional{ raw.timer } }
	{
	}
	vec2 position_;
	cambc::team team_;
	std::optional<std::int32_t> remaining_time_;
};

namespace detail
{
struct ward_access
{
	[[nodiscard]] static cambc::ward from_raw(const ward_info& value) { return cambc::ward{ value }; }
	[[nodiscard]] static cambc::ward from_handle(std::uint32_t handle)
	{
		ward_info value{};
		check(sdk_ward(handle, &value));
		return cambc::ward{ value };
	}
};
} // namespace detail

/// A visible teleport reservation snapshot, including the observer's own channel.
class teleport
{
public:
	/// Copy a visible teleport reservation; its metadata remains a snapshot.
	explicit teleport(detail::teleport_info value) : value_{ value } {}
	/// Construct a live view of the champion in the teleport reservation.
	[[nodiscard]] cambc::champion champion() const { return detail::handle_access::make_champion(static_cast<handle_champion_t>(value_.champion)); }
	/// Get the owning team relative to the authenticated bot.
	[[nodiscard]] team team() const { return value_.team == static_cast<std::uint32_t>(detail::sdk_my_team()) ? team::ally : team::enemy; }
	/// Check whether this teleport belongs to the observing team.
	[[nodiscard]] bool is_ally() const { return team() == cambc::team::ally; }
	/// Check whether this teleport belongs to the opposing team.
	[[nodiscard]] bool is_enemy() const { return team() == cambc::team::enemy; }
	/// Get the destination tile captured in this teleport reservation.
	[[nodiscard]] vec2 position() const noexcept { return { value_.destination.x, value_.destination.y }; }
	/// Get the remaining round count captured in this observation.
	[[nodiscard]] std::int32_t remaining_time() const noexcept { return value_.timer; }
	/// Hypothetical caster vision after arriving at the reserved destination, not current visibility.
	[[nodiscard]] bitboard visible_tiles() const
	{
		bitboard result{ uninitialized };
		visible_tiles_into(result);
		return result;
	}

	/// Like `visible_tiles`, but writes into `out` instead of returning a new board.
	/// @param out Destination; its previous contents are overwritten.
	void visible_tiles_into(bitboard& out) const
	{
		detail::check(detail::sdk_hypothetical_vision(value_.type, value_.destination.x, value_.destination.y, out.ptr()));
	}
	/// Compare every captured field.
	[[nodiscard]] friend bool operator==(const teleport&, const teleport&) = default;

private:
	detail::teleport_info value_;
};

/// Occupancy masks available from the current perspective.
namespace vision
{

/// Like `champion_tiles`, but writes into `out` instead of returning a new board.
/// @param side Team to include.
/// @param out Destination; its previous contents are overwritten.
inline void champion_tiles_into(team side, bitboard& out)
{
	if (side != team::ally && side != team::enemy)
		throw std::invalid_argument("invalid team");
	detail::check(side == team::ally ? detail::sdk_ally_occupancy(out.ptr()) : detail::sdk_enemy_occupancy(out.ptr()));
}

/// Get visible champion occupancy for one side.
/// @param side Team to include.
[[nodiscard]] inline bitboard champion_tiles(team side)
{
	bitboard result{ uninitialized };
	champion_tiles_into(side, result);
	return result;
}

/// Get visible champion occupancy for both sides.
[[nodiscard]] inline bitboard champion_tiles()
{
	return champion_tiles(team::ally) | champion_tiles(team::enemy);
}

/// Like `champion_tiles()`, but writes into `out`.
/// @param out Destination; its previous contents are overwritten.
inline void champion_tiles_into(bitboard& out)
{
	champion_tiles_into(team::ally, out);
	out |= champion_tiles(team::enemy);
}

/// Like `camp_tiles`, but writes into `out` instead of returning a new board.
/// @param out Destination; its previous contents are overwritten.
inline void camp_tiles_into(bitboard& out)
{
	detail::check(detail::sdk_camp_occupancy(out.ptr()));
}

/// Get camp occupancy from the current perspective.
[[nodiscard]] inline bitboard camp_tiles()
{
	bitboard result{ uninitialized };
	camp_tiles_into(result);
	return result;
}
} // namespace vision

/// Get the total gems submitted by a team.
/// @param side Team to count.
[[nodiscard]] inline std::uint32_t global_submitted_gems(team side)
{
	std::uint32_t result{};
	detail::check(detail::sdk_bc_submitted_gems(detail::absolute_team(side), &result));
	return result;
}

/// Get the current round number.
[[nodiscard]] inline std::uint32_t round_number()
{
	std::uint32_t result{};
	detail::check(detail::sdk_bc_round_number(&result));
	return result;
}

/// Map geometry and cached terrain views.
namespace map
{
/// Get the map width in tiles.
[[nodiscard]] inline std::int32_t width()
{
	return detail::layout().width;
}

/// Get the map height in tiles.
[[nodiscard]] inline std::int32_t height()
{
	return detail::layout().height;
}

/// Check whether a tile is within the map dimensions.
/// @param position Position to check.
[[nodiscard]] inline bool is_in_bounds(vec2 position)
{
	return position.x >= 0 && position.y >= 0 && position.x < width() && position.y < height();
}

/// Engine map symmetry captured during dynamic initialisation.
inline const symmetry cached_symmetry = []
{
	std::int32_t result{};
	detail::check(detail::sdk_bc_map_symmetry(&result));
	return static_cast<symmetry>(result);
}();

/// Get the map symmetry.
[[nodiscard]] inline symmetry symmetry()
{
	return cached_symmetry;
}

/// Reflect across the map horizontally, reversing tile x coordinates.
/// @param p Map position to reflect.
[[nodiscard]] inline vec2 reflect_horizontal(vec2 p)
{
	vec2 result{};
	detail::check(detail::sdk_Bitboard_ReflectHorizontalPosition(p.x, p.y, &result.x, &result.y));
	return result;
}

/// Reflect across the map vertically, reversing tile y coordinates.
/// @param p Map position to reflect.
[[nodiscard]] inline vec2 reflect_vertical(vec2 p)
{
	vec2 result{};
	detail::check(detail::sdk_Bitboard_ReflectVerticalPosition(p.x, p.y, &result.x, &result.y));
	return result;
}

/// Rotate tiles 180 degrees within the map dimensions.
/// @param p Map position to rotate.
[[nodiscard]] inline vec2 rotate_180(vec2 p)
{
	vec2 result{};
	detail::check(detail::sdk_Bitboard_Rotate180Position(p.x, p.y, &result.x, &result.y));
	return result;
}

/// Immutable terrain masks captured during dynamic initialisation.
inline const bitboard cached_walls = detail::map_mask(0);
/// Immutable bush occupancy for this map.
inline const bitboard cached_bushes = detail::map_mask(1);
/// Immutable hole occupancy for this map.
inline const bitboard cached_holes = detail::map_mask(2);
/// Immutable altar occupancy for this map.
inline const bitboard cached_altars = detail::map_mask(3);

/// Get cached terrain masks; references remain valid for the bot lifetime.
[[nodiscard]] inline const bitboard& walls()
{
	return cached_walls;
}

/// Get the immutable bush mask; the reference lasts for the bot lifetime.
[[nodiscard]] inline const bitboard& bushes()
{
	return cached_bushes;
}

/// Get the immutable hole mask; the reference lasts for the bot lifetime.
[[nodiscard]] inline const bitboard& holes()
{
	return cached_holes;
}

/// Get the immutable altar mask; the reference lasts for the bot lifetime.
[[nodiscard]] inline const bitboard& altars()
{
	return cached_altars;
}

/// Both relative-team spawn masks, captured from the engine during dynamic initialisation.
inline const std::array<bitboard, 2> cached_spawn_regions = []
{
	std::array<bitboard, 2> result;
	for (const auto side : { team::ally, team::enemy })
	{
		detail::aabb bounds{};
		detail::check(detail::sdk_spawn_region(detail::absolute_team(side), &bounds));
		auto& mask = result[static_cast<std::size_t>(side)];
		mask.clear();
		for (auto y = bounds.min_y; y < bounds.max_y; ++y)
			for (auto x = bounds.min_x; x < bounds.max_x; ++x)
				mask.set({ x, y });
	}
	return result;
}();

/// Get the cached spawn mask for a team.
/// @param value Team whose spawn region to return.
[[nodiscard]] inline const bitboard& spawn_region(team value)
{
	switch (value)
	{
	case team::ally:
		return cached_spawn_regions[0];
	case team::enemy:
		return cached_spawn_regions[1];
	}
	throw std::invalid_argument("invalid team");
}

/// The camp group still contains living camps.
struct camp_group_alive {};
/// All camps are defeated and the capture reward is available.
struct camp_group_reward_available {};
/// The group is waiting to respawn; timer is the remaining round count.
struct camp_group_respawning { std::int32_t timer; };
/// Current camp-group state: alive, reward available, or respawning with a timer.
using camp_group_state = std::variant<camp_group_alive, camp_group_reward_available, camp_group_respawning>;

/// A physical camp cluster and its rewards.
class camp_group
{
public:
	/// Convert to the strong camp-group identity.
	[[nodiscard]] operator handle_camp_group_t() const noexcept { return handle_; }
	/// Compare camp-group identities.
	friend bool operator==(const camp_group& a, const camp_group& b) noexcept { return a.handle_ == b.handle_; }

	/// Get the gold reward for capturing this camp group.
	[[nodiscard]] std::uint32_t gold_value() const noexcept { return gold_; }

	/// Get the gem reward for capturing this camp group.
	[[nodiscard]] std::uint32_t gem_value() const noexcept { return gems_; }

	/// Current lifecycle state; remembered state outside current vision is not exposed.
	[[nodiscard]] camp_group_state state() const
	{
		bitboard visible{ uninitialized };
		detail::check(detail::sdk_vision(visible.ptr()));
		if (std::ranges::none_of(camps_, [&](const auto& member) { return visible.is_set(member.spawn_position()); }))
			throw error{ error_code::api_not_in_vision, "camp group is not in vision" };
		std::uint32_t state{};
		detail::check(detail::sdk_camp_group_state(static_cast<std::uint32_t>(handle_), &state));
		switch (state)
		{
		case 1: return camp_group_alive{};
		case 2: return camp_group_reward_available{};
		case 3:
		{
			std::int32_t timer{};
			detail::check(detail::sdk_camp_group_cooldown(static_cast<std::uint32_t>(handle_), &timer));
			return camp_group_respawning{ timer };
		}
		case 0: throw error{ error_code::api_not_in_vision, "camp group state is unknown" };
		default: throw std::runtime_error("unknown engine camp group state");
		}
	}

	/// Get the configured camp-group respawn duration in rounds.
	[[nodiscard]] std::int32_t max_respawn_timer() const noexcept { return respawn_; }

	/// Get the camp members stored in this physical group.
	[[nodiscard]] std::span<const camp> camps() const noexcept { return camps_; }
	/// Iterate immutable camp members stored in this physical group.
	[[nodiscard]] auto begin() const noexcept { return camps_.begin(); }
	/// Get the past-the-end iterator for this group's camp members.
	[[nodiscard]] auto end() const noexcept { return camps_.end(); }

	/// Build the mask of original camp spawn tiles in this group; computed on every call.
	[[nodiscard]] bitboard spawn_tiles() const
	{
		bitboard result{ uninitialized };
		spawn_tiles_into(result);
		return result;
	}

	/// Like `spawn_tiles`, but writes into `out` instead of returning a new board.
	/// @param out Destination; its previous contents are overwritten.
	void spawn_tiles_into(bitboard& out) const
	{
		out.clear();
		for (const auto& member : camps_)
			out.set(member.spawn_position());
	}

private:
	friend struct detail::handle_access;
	camp_group(handle_camp_group_t handle, std::uint32_t gold, std::uint32_t gems, std::int32_t respawn, std::vector<cambc::camp> camps)
		: handle_{ handle }, gold_{ gold }, gems_{ gems }, respawn_{ respawn }, camps_{ std::move(camps) }
	{
	}

	handle_camp_group_t handle_;
	std::uint32_t gold_{}, gems_{};
	std::int32_t respawn_{};
	std::vector<cambc::camp> camps_;
};
} // namespace map

inline map::camp_group detail::handle_access::make_camp_group(
	handle_camp_group_t handle, std::uint32_t gold, std::uint32_t gems, std::int32_t respawn, std::vector<cambc::camp> camps)
{
	return map::camp_group{ handle, gold, gems, respawn, std::move(camps) };
}

namespace detail
{
/// Query one physical camp group and its members from the engine.
[[nodiscard]] inline map::camp_group load_camp_group(std::uint32_t group)
{
	camp_group_info info{};
	check(sdk_map_camp_group(group, &info));
	std::int32_t respawn{};
	check(sdk_bc_camp_group_max_respawn(group, &respawn));
	std::uint32_t members{};
	check(sdk_map_camp_group_num_camps(group, &members));
	std::vector<std::uint32_t> spawns(members);
	check(sdk_map_camp_group_camps(group, spawns.data(), members, &members));
	std::vector<cambc::camp> camps;
	camps.reserve(members);
	for (std::uint32_t member = 0; member < members; ++member)
	{
		camp_info spec{};
		check(sdk_map_camp_info(spawns[member], &spec));
		camps.push_back(handle_access::make_camp(static_cast<handle_camp_t>(spawns[member]), static_cast<handle_camp_group_t>(group),
			static_cast<camp_type>(spec.type), vec2{ spec.x, spec.y }));
	}
	return handle_access::make_camp_group(static_cast<handle_camp_group_t>(group), info.gold, info.gems, respawn, std::move(camps));
}

/// Query every static camp group from the engine in one batched call.
[[nodiscard]] inline std::vector<map::camp_group> load_camp_groups()
{
	std::vector<std::uint32_t> words(256);
	std::uint32_t count{};
	check(sdk_map_camp_layout(words.data(), static_cast<std::uint32_t>(words.size()), &count));
	if (count > words.size())
	{
		words.resize(count);
		check(sdk_map_camp_layout(words.data(), count, &count));
	}
	std::vector<map::camp_group> result;
	for (std::size_t i = 0; i + 5 <= count;)
	{
		const auto group = words[i];
		const auto gold = words[i + 1], gems = words[i + 2];
		const auto respawn = static_cast<std::int32_t>(words[i + 3]);
		const auto size = words[i + 4];
		i += 5;
		std::vector<cambc::camp> camps;
		camps.reserve(size);
		for (std::uint32_t m = 0; m < size && i + 4 <= count; ++m, i += 4)
			camps.push_back(handle_access::make_camp(static_cast<handle_camp_t>(words[i]), static_cast<handle_camp_group_t>(group),
				static_cast<camp_type>(words[i + 1]), vec2{ static_cast<std::int32_t>(words[i + 2]), static_cast<std::int32_t>(words[i + 3]) }));
		result.push_back(handle_access::make_camp_group(static_cast<handle_camp_group_t>(group), gold, gems, respawn, std::move(camps)));
	}
	return result;
}

/// Query one physical camp's spawn info and group from the engine.
[[nodiscard]] inline cambc::camp load_camp(std::uint32_t handle)
{
	camp_info spec{};
	check(sdk_map_camp_info(handle, &spec));
	std::uint32_t group{};
	check(sdk_map_camp_group_of(handle, &group));
	return handle_access::make_camp(static_cast<handle_camp_t>(handle), static_cast<handle_camp_group_t>(group),
		static_cast<camp_type>(spec.type), vec2{ spec.x, spec.y });
}
} // namespace detail

namespace map
{

/// Query physical camp groups from the engine on every call; also available in draft.
[[nodiscard]] inline std::vector<camp_group> camp_groups()
{
	return detail::load_camp_groups();
}

/// Find a camp by spawn tile, querying the engine on every call; throws if there is no camp.
/// @param position Spawn tile of the camp.
[[nodiscard]] inline cambc::camp camp(vec2 position)
{
	std::uint32_t handle{};
	const auto code = detail::sdk_map_camp_at(position.x, position.y, &handle);
	if (code == static_cast<std::int32_t>(error_code::api_not_found)) throw std::out_of_range("no camp on tile");
	detail::check(code);
	return detail::load_camp(handle);
}
} // namespace map

inline map::camp_group camp::camp_group() const
{
	return detail::load_camp_group(static_cast<std::uint32_t>(group_));
}

namespace vision
{
/// Like `ward_tiles`, but writes into `out` instead of returning a new board.
/// @param side Team to include.
/// @param out Destination; its previous contents are overwritten.
inline void ward_tiles_into(team side, bitboard& out)
{
	(void)detail::absolute_team(side);
	const auto code = side == team::ally ? detail::sdk_ally_ward_occupancy(out.ptr())
										: detail::sdk_enemy_ward_occupancy(out.ptr());
	detail::check(code);
}

/// Query visible ordinary-ward occupancy for one team.
/// @param side Team to include.
[[nodiscard]] inline bitboard ward_tiles(team side)
{
	bitboard result{ uninitialized };
	ward_tiles_into(side, result);
	return result;
}

/// Query visible ordinary-ward occupancy for both teams.
[[nodiscard]] inline bitboard ward_tiles()
{
	return ward_tiles(team::ally) | ward_tiles(team::enemy);
}

/// Like `ward_tiles()`, but writes into `out`.
/// @param out Destination; its previous contents are overwritten.
inline void ward_tiles_into(bitboard& out)
{
	ward_tiles_into(team::ally, out);
	out |= ward_tiles(team::enemy);
}

/// Fresh visible ordinary-ward observations; overlapping records remain distinct.
[[nodiscard]] inline std::vector<cambc::ward> wards()
{
	std::uint32_t count{};
	detail::check(detail::sdk_wards(nullptr, 0, &count));
	std::vector<detail::ward_info> raw(count);
	detail::check(detail::sdk_wards(raw.data(), raw.size(), &count));
	std::vector<cambc::ward> result;
	result.reserve(count);
	for (const auto& value : raw)
		result.push_back(detail::ward_access::from_raw(value));
	return result;
}

/// Get fresh ordinary-ward observations filtered by team.
/// @param side Team to include.
[[nodiscard]] inline std::vector<cambc::ward> wards(team side)
{
	(void)detail::absolute_team(side);
	auto result = wards();
	std::erase_if(result, [side](const auto& value) { return value.team() != side; });
	return result;
}

/// Get the ordinary ward at a tile.
/// @param position Tile to query.
[[nodiscard]] inline cambc::ward ward_at(vec2 position)
{
	std::uint32_t handle{};
	detail::check(detail::sdk_ward_at(position.x, position.y, &handle));
	return detail::ward_access::from_handle(handle);
}

/// Get the ordinary ward at a tile for one team.
/// @param position Tile to query.
/// @param side Team the ward must belong to.
[[nodiscard]] inline cambc::ward ward_at(vec2 position, team side)
{
	(void)detail::absolute_team(side);
	std::uint32_t handle{};
	auto code = side == team::ally ? detail::sdk_ally_ward_at(position.x, position.y, &handle)
								   : detail::sdk_enemy_ward_at(position.x, position.y, &handle);
	detail::check(code);
	return detail::ward_access::from_handle(handle);
}

/// Check ward occupancy at a tile; hidden or absent wards return false.
/// @param position Tile to query.
[[nodiscard]] inline bool has_ward(vec2 position)
{
	std::uint32_t handle{};
	const auto code = detail::sdk_ward_at(position.x, position.y, &handle);
	if (code == 0)
		return true;
	if (code == static_cast<std::int32_t>(error_code::api_not_found)
		|| code == static_cast<std::int32_t>(error_code::api_not_in_vision))
		return false;
	detail::check(code);
	return false;
}

/// Check ward occupancy for `side`; hidden or absent wards return false.
/// @param position Tile to query.
/// @param side Team the ward must belong to.
[[nodiscard]] inline bool has_ward(vec2 position, team side)
{
	(void)detail::absolute_team(side);
	std::uint32_t handle{};
	auto code = side == team::ally ? detail::sdk_ally_ward_at(position.x, position.y, &handle)
								   : detail::sdk_enemy_ward_at(position.x, position.y, &handle);
	if (code == 0)
		return true;
	if (code != static_cast<std::int32_t>(error_code::api_not_found)
		&& code != static_cast<std::int32_t>(error_code::api_not_in_vision))
		detail::check(code);
	if (code == static_cast<std::int32_t>(error_code::api_not_found)
		|| code == static_cast<std::int32_t>(error_code::api_not_in_vision))
		return false;
	detail::check(code);
	return false;
}

/// Like `teleport_tiles`, but writes into `out` instead of returning a new board.
/// @param out Destination; its previous contents are overwritten.
inline void teleport_tiles_into(bitboard& out)
{
	detail::check(detail::sdk_teleport_occupancy(out.ptr()));
}

/// Query current visible teleport-reservation occupancy.
[[nodiscard]] inline bitboard teleport_tiles()
{
	bitboard result{ uninitialized };
	teleport_tiles_into(result);
	return result;
}

/// Like `visible_tiles`, but writes into `out` instead of returning a new board.
/// @param out Destination; its previous contents are overwritten.
inline void visible_tiles_into(bitboard& out)
{
	detail::check(detail::sdk_vision(out.ptr()));
}

/// Query current visible tiles.
[[nodiscard]] inline bitboard visible_tiles()
{
	bitboard result{ uninitialized };
	visible_tiles_into(result);
	return result;
}

/// Check whether a tile is occupied by a visible teleport reservation.
/// @param position Tile to query.
[[nodiscard]] inline bool has_teleport(vec2 position)
{
	std::int32_t result{};
	const auto code = detail::sdk_has_teleport(position.x, position.y, &result);
	if (code == static_cast<std::int32_t>(error_code::api_not_in_vision))
		return false;
	detail::check(code);
	return result != 0;
}

/// Get fresh, owning visible teleport-reservation snapshots.
[[nodiscard]] inline std::vector<teleport> teleports()
{
	std::uint32_t count{};
	detail::check(detail::sdk_teleports(nullptr, 0, &count));
	std::vector<detail::teleport_info> raw(count);
	detail::check(detail::sdk_teleports(raw.data(), raw.size(), &count));
	std::vector<teleport> result;
	result.reserve(count);
	for (std::uint32_t i = 0; i < count; ++i)
		result.emplace_back(raw[i]);
	return result;
}

/// Get the visible teleport reservation on a tile.
/// @param position Tile to query.
[[nodiscard]] inline teleport teleport_at(vec2 position)
{
	if (has_teleport(position))
		for (const auto& value : teleports())
			if (value.position() == position)
				return value;
	throw error{ error_code::api_not_found, "no visible teleport at position" };
}

/// Like `altar_lock_tiles`, but writes into `out` instead of returning a new board.
/// @param side Team to include.
/// @param out Destination; its previous contents are overwritten.
inline void altar_lock_tiles_into(team side, bitboard& out)
{
	(void)detail::absolute_team(side);
	const auto code = side == team::ally ? detail::sdk_ally_altar_lock_occupancy(out.ptr())
										: detail::sdk_enemy_altar_lock_occupancy(out.ptr());
	detail::check(code);
}

/// Query current altar-lock occupancy for the selected team or both teams.
/// @param side Team to include.
[[nodiscard]] inline bitboard altar_lock_tiles(team side)
{
	bitboard result{ uninitialized };
	altar_lock_tiles_into(side, result);
	return result;
}

/// Query current altar-lock occupancy for the selected team or both teams.
[[nodiscard]] inline bitboard altar_lock_tiles()
{
	return altar_lock_tiles(team::ally) | altar_lock_tiles(team::enemy);
}

/// Like `altar_lock_tiles()`, but writes into `out`.
/// @param out Destination; its previous contents are overwritten.
inline void altar_lock_tiles_into(bitboard& out)
{
	altar_lock_tiles_into(team::ally, out);
	out |= altar_lock_tiles(team::enemy);
}

/// Prefer the allied lock if both teams have a lock on this altar.
/// @param position Tile to query.
[[nodiscard]] inline cambc::altar_lock altar_lock_at(vec2 position)
{
	detail::altar_lock_info result{};
	const auto code = detail::sdk_altar_lock(position.x, position.y, detail::absolute_team(team::ally), &result);
	if (code == static_cast<std::int32_t>(error_code::api_not_found))
	{
		detail::check(detail::sdk_altar_lock(position.x, position.y, detail::absolute_team(team::enemy), &result));
		return cambc::altar_lock{ result };
	}
	detail::check(code);
	return cambc::altar_lock{ result };
}

/// Check for a visible altar lock for either team; missing or hidden locks return false.
/// @param position Tile to query.
[[nodiscard]] inline bool has_altar_lock(vec2 position)
{
	detail::altar_lock_info result{};
	auto code = detail::sdk_altar_lock(position.x, position.y, detail::absolute_team(team::ally), &result);
	if (code == static_cast<std::int32_t>(error_code::api_not_found) || code == static_cast<std::int32_t>(error_code::api_not_in_vision))
	{
		code = detail::sdk_altar_lock(position.x, position.y, detail::absolute_team(team::enemy), &result);
		if (code == static_cast<std::int32_t>(error_code::api_not_found) || code == static_cast<std::int32_t>(error_code::api_not_in_vision))
			return false;
	}
	detail::check(code);
	return true;
}

/// Get fresh, owning visible altar-lock snapshots, optionally filtered by team.
[[nodiscard]] inline std::vector<cambc::altar_lock> altar_locks()
{
	std::uint32_t count{};
	detail::check(detail::sdk_altar_locks(nullptr, 0, &count));
	std::vector<detail::altar_lock_info> raw(count);
	detail::check(detail::sdk_altar_locks(raw.data(), raw.size(), &count));
	std::vector<cambc::altar_lock> result;
	result.reserve(count);
	for (std::uint32_t i = 0; i < count; ++i)
		result.emplace_back(raw[i]);
	return result;
}

/// Get fresh, owning visible altar-lock snapshots, optionally filtered by team.
/// @param side Team to include.
[[nodiscard]] inline std::vector<cambc::altar_lock> altar_locks(team side)
{
	(void)detail::absolute_team(side);
	auto result = altar_locks();
	std::erase_if(result, [side](const auto& lock) { return lock.team() != side; });
	return result;
}

/// Find a currently visible camp.
/// @param position Current tile of the camp.
[[nodiscard]] inline cambc::camp camp_at(vec2 position)
{
	std::uint32_t handle{};
	detail::check(detail::sdk_camp_at(position.x, position.y, &handle));
	return detail::load_camp(handle);
}

/// Create a fresh view of the currently visible champion at a position.
/// @param position Tile to query.
[[nodiscard]] inline cambc::champion champion_at(vec2 position)
{
	std::uint32_t handle{};
	detail::check(detail::sdk_champion_at(position.x, position.y, &handle));
	return cambc::detail::handle_access::make_champion(static_cast<handle_champion_t>(handle));
}

/// False on hidden or empty tiles; invalid positions retain the engine's error.
/// @param position Tile to query.
[[nodiscard]] inline bool has_camp(vec2 position)
{
	std::int32_t result{};
	const auto code = detail::sdk_has_camp(position.x, position.y, &result);
	if (code == static_cast<std::int32_t>(error_code::api_not_in_vision))
		return false;
	detail::check(code);
	return result != 0;
}

/// Check visible champion occupancy at a tile; invalid positions retain engine errors.
/// @param position Tile to query.
[[nodiscard]] inline bool has_champion(vec2 position)
{
	std::int32_t result{};
	const auto code = detail::sdk_has_champion(position.x, position.y, &result);
	if (code == static_cast<std::int32_t>(error_code::api_not_in_vision))
		return false;
	detail::check(code);
	return result != 0;
}

/// Get an owning list of currently visible champions, recomputed each call.
/// @param side Team to include.
[[nodiscard]] inline std::vector<cambc::champion> champions(team side)
{
	std::vector<cambc::champion> result;
	const auto positions = champion_tiles(side).positions();
	result.reserve(positions.size());
	for (const auto position : positions)
		result.push_back(champion_at(position));
	return result;
}

/// Get a fresh champion list for the current view and optional team filter.
[[nodiscard]] inline std::vector<cambc::champion> champions()
{
	std::vector<cambc::champion> result;
	const auto positions = champion_tiles().positions();
	result.reserve(positions.size());
	for (const auto position : positions)
		result.push_back(champion_at(position));
	return result;
}

/// Get an owning list of currently visible camps, recomputed each call.
[[nodiscard]] inline std::vector<cambc::camp> camps()
{
	std::vector<cambc::camp> result;
	const auto positions = camp_tiles().positions();
	result.reserve(positions.size());
	for (const auto position : positions)
		result.push_back(camp_at(position));
	return result;
}

/// A group is currently visible when any original spawn tile is in effective vision, even if empty.
[[nodiscard]] inline std::vector<map::camp_group> camp_groups()
{
	bitboard visible{ uninitialized };
	detail::check(detail::sdk_vision(visible.ptr()));
	std::vector<map::camp_group> result;
	for (auto& group : map::camp_groups())
		if (std::ranges::any_of(group, [&](const auto& member) { return visible.is_set(member.spawn_position()); }))
			result.push_back(std::move(group));
	return result;
}
} // namespace vision

/// Bounds-checked access to this team's shared bytes; reads and writes always reach the host.
class shared_memory_buffer
{
public:
	/// Get the shared-memory capacity in bytes.
	[[nodiscard]] static std::size_t size()
	{
		return detail::shared_memory_capacity;
	}

	/// Byte proxy that reads and writes through SDK imports.
	class reference
	{
	public:
		reference(const reference&) = default;

		/// Read the current byte at this proxy's checked offset.
		[[nodiscard]] operator std::uint8_t() const
		{
			std::uint8_t value{};
			detail::check(detail::sdk_read_shared_memory(static_cast<std::int32_t>(offset_), &value, 1));
			return value;
		}

		/// Write one byte at this proxy's checked offset.
		reference& operator=(std::uint8_t value)
		{
			detail::check(detail::sdk_write_shared_memory(static_cast<std::int32_t>(offset_), &value, 1));
			return *this;
		}

		reference& operator=(const reference& other) { return *this = static_cast<std::uint8_t>(other); }

		reference& operator=(std::byte value) { return *this = std::to_integer<std::uint8_t>(value); }

	private:
		friend class shared_memory_buffer;

		explicit reference(std::size_t offset) : offset_{ offset } {}

		std::size_t offset_;
	};

	/// Access one byte; throws if the offset is outside the buffer.
	[[nodiscard]] reference operator[](std::size_t offset) const
	{
		check_range(offset, 1);
		return reference{ offset };
	}

	/// Write bytes at an offset; the entire range must fit.
	/// @param offset Byte offset into shared memory.
	/// @param bytes Bytes to write.
	void write(std::size_t offset, std::span<const std::byte> bytes) const
	{
		check_range(offset, bytes.size());
		if (!bytes.empty())
			detail::check(
				detail::sdk_write_shared_memory(static_cast<std::int32_t>(offset), bytes.data(), static_cast<std::uint32_t>(bytes.size())));
	}

	/// Read a checked byte range into caller-owned storage.
	/// @param offset Byte offset into shared memory.
	/// @param bytes Receives the bytes; its size sets how many are read.
	void read(std::size_t offset, std::span<std::byte> bytes) const
	{
		check_range(offset, bytes.size());
		if (!bytes.empty())
			detail::check(detail::sdk_read_shared_memory(static_cast<std::int32_t>(offset), bytes.data(), static_cast<std::uint32_t>(bytes.size())));
	}

	/// Write the object representations of trivially copyable values to a checked byte range.
	template <class T, std::size_t Extent>
		requires std::is_trivially_copyable_v<T>
	void write(std::size_t offset, std::span<T, Extent> values) const
	{
		check_range(offset, 0);
		if (values.size() > (size() - offset) / sizeof(T))
			throw std::out_of_range("shared memory access outside buffer");
		write(offset, std::span<const std::byte>{ std::as_bytes(values) });
	}

	/// Read a checked byte range into trivially copyable values.
	/// @param offset Byte offset into shared memory.
	/// @param values Receives the values; its size sets how many are read.
	template <class T, std::size_t Extent> requires (std::is_trivially_copyable_v<T> && !std::is_const_v<T>)
	void read(std::size_t offset, std::span<T, Extent> values) const
	{
		check_range(offset, 0);
		if (values.size() > (size() - offset) / sizeof(T))
			throw std::out_of_range("shared memory access outside buffer");
		read(offset, std::span<std::byte>{ std::as_writable_bytes(values) });
	}

private:
	static void check_range(std::size_t offset, std::size_t length)
	{
		if (offset > size() || length > size() - offset || offset > static_cast<std::size_t>(std::numeric_limits<std::int32_t>::max()))
			throw std::out_of_range("shared memory access outside buffer");
	}
};

/// Shared memory available to this team.
inline constexpr shared_memory_buffer shared_memory{};

namespace draft
{
/// Query the number of champions per team; gameplay perspectives propagate engine errors.
[[nodiscard]] inline std::uint32_t team_size()
{
	std::uint8_t result{};
	detail::check(detail::sdk_draft_num_rounds(&result));
	return result;
}

/// Get an owning list of completed draft picks, recomputed each call.
/// @param side Team whose picks to return.
[[nodiscard]] inline std::vector<champion_draft> current_draft(team side)
{
	const auto absolute = detail::absolute_team(side);
	std::uint8_t rounds{};
	detail::check(detail::sdk_draft_num_rounds(&rounds));
	std::vector<champion_draft> result;
	result.reserve(rounds);
	for (std::uint32_t round = 0; round < rounds; ++round)
	{
		std::int32_t pick{};
		const auto	 code = detail::sdk_draft_get(absolute, round, &pick);
		if (code == 16)
			continue; // DraftUnavailable
		detail::check(code);
		result.push_back(static_cast<champion_draft>(pick));
	}
	return result;
}

/// Submit a champion pick for the current draft round.
/// @param pick Champion to pick.
inline void pick(champion_draft pick)
{
	detail::check(detail::sdk_draft_champion(static_cast<std::int32_t>(pick)));
}

} // namespace draft

namespace debug
{
/// Emit a debug dot on a tile using a packed 0xRRGGBB colour.
/// @param position Tile to mark.
/// @param rgb Packed `0xRRGGBB` colour.
inline void draw_dot(vec2 position, std::uint32_t rgb)
{
	detail::check(detail::sdk_debug_dot(position.x, position.y, rgb));
}

/// Emit a debug line between tiles using a packed 0xRRGGBB colour.
/// @param from Start tile.
/// @param to End tile.
/// @param rgb Packed `0xRRGGBB` colour.
inline void draw_line(vec2 from, vec2 to, std::uint32_t rgb)
{
	detail::check(detail::sdk_debug_line(from.x, from.y, to.x, to.y, rgb));
}
} // namespace debug

/// Get the fuel left in the current turn's budget; the first draft turn gets 10x and each champion's first turn gets 2x.
[[nodiscard]] inline std::uint64_t fuel_remaining()
{
	const auto remaining = detail::sdk_fuel_remaining();
	if (remaining < 0)
		throw std::runtime_error("fuel_remaining ABI mismatch: negative fuel");
	return static_cast<std::uint64_t>(remaining);
}

/// Flush buffered output and yield the current turn.
inline void end_turn()
{
	std::fflush(nullptr);
	detail::sdk_yield();
}

/// Resign the game. A successful resign never returns, so buffered output is flushed first.
inline void resign()
{
	std::fflush(nullptr);
	detail::check(detail::sdk_resign());
}

inline bool champion::move_towards(vec2 target) const
{
	detail::require_self(handle_);
	const vec2 origin{ position() };
	if (origin == target || !state().contains(champion_state::can_move)) return false;
	bitboard blocked{ map::walls() | map::holes() | map::altars() | map::spawn_region(team::enemy) | vision::champion_tiles()
		| vision::camp_tiles() | vision::teleport_tiles() };
	blocked.set(origin, false);
	if (blocked.is_set(target)) return false;
	bitboard roots;
	roots.set(target, true);
	bitboard destinations;
	destinations.set(origin, true);
	bitboard visited;
	distance_field distances;
	distances.compute_to(blocked, roots, destinations, visited);
	vec2 best{ origin };
	std::uint16_t best_distance{ distances.get(origin) };
	for (vec2 tile : blocked.reachable_tiles(origin, static_cast<std::uint16_t>(base_stats().base_move_speed)).positions())
	{
		if (distances.get(tile) < best_distance)
		{
			best = tile;
			best_distance = distances.get(tile);
		}
	}
	return best != origin && move_absolute(best);
}

} // namespace cambc

/// Hashes both coordinates; consistent with vec2 equality.
template <>
struct std::hash<cambc::vec2>
{
	/// Hash a tile position.
	[[nodiscard]] std::size_t operator()(cambc::vec2 v) const noexcept
	{
		return std::hash<std::uint64_t>{}(static_cast<std::uint64_t>(static_cast<std::uint32_t>(v.x)) << 32 | static_cast<std::uint32_t>(v.y));
	}
};

/// Hashes the engine handle; consistent with champion equality.
template <>
struct std::hash<cambc::champion>
{
	/// Hash a champion identity.
	[[nodiscard]] std::size_t operator()(const cambc::champion& c) const noexcept { return std::hash<std::uint32_t>{}(static_cast<std::uint32_t>(static_cast<cambc::handle_champion_t>(c))); }
};

/// Hashes the engine handle; consistent with camp equality.
template <>
struct std::hash<cambc::camp>
{
	/// Hash a camp identity.
	[[nodiscard]] std::size_t operator()(const cambc::camp& c) const noexcept { return std::hash<std::uint32_t>{}(static_cast<std::uint32_t>(static_cast<cambc::handle_camp_t>(c))); }
};

/// Hashes the engine handle; consistent with camp-group equality.
template <>
struct std::hash<cambc::map::camp_group>
{
	/// Hash a camp-group identity.
	[[nodiscard]] std::size_t operator()(const cambc::map::camp_group& g) const noexcept { return std::hash<std::uint32_t>{}(static_cast<std::uint32_t>(static_cast<cambc::handle_camp_group_t>(g))); }
};
