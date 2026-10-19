#pragma once
#include <cambc.h>
#include <array>
#include <cstdint>

// Strategy knobs; scripts/variant.sh rewrites single lines here to build experiment bots.
namespace tuning {
using enum cambc::champion_draft;
// Picks are revealed simultaneously and duplicates across teams are allowed, so a fixed order is safe.
inline constexpr std::array draft_priority = { cassi_and_issac, stellar, ren, vendetta, varg, tavi, ryth, willow, vesper_vale, marcella, prism };
// Bought until the emblem completes, then each champion buys its main stat.
inline constexpr cambc::item_type opening_shard = cambc::item_type::durability_shard_of_health;
inline constexpr cambc::item_type opening_emblem = cambc::item_type::durability_emblem_of_health;
inline constexpr cambc::item_type strength_shard = cambc::item_type::power_shard_of_strength;
inline constexpr cambc::item_type focus_shard = cambc::item_type::power_shard_of_focus;
inline constexpr cambc::item_type health_shard = cambc::item_type::power_shard_of_health;
inline constexpr float recall_health_ratio = 0.4f;
inline constexpr float flee_health_ratio = 0.55f;
inline constexpr float leave_spawn_health_ratio = 0.95f;
inline constexpr float heal_ally_ratio = 0.65f;
inline constexpr float shield_health_ratio = 0.8f;
inline constexpr std::uint32_t bank_gem_threshold = 40;
inline constexpr std::uint32_t cautious_gem_count = 20;
inline constexpr int nearby_altar_distance = 6;
inline constexpr std::uint32_t endgame_rounds = 150;
inline constexpr float squad_damage_per_round = 150.0f;
inline constexpr int camp_threat_range = 3;
inline constexpr int enemy_threat_range = 4;
inline constexpr int skirmish_range = 7;
inline constexpr int ally_support_range = 6;
inline constexpr int max_chase_distance = 8;
inline constexpr int enemy_spawn_buffer = 4;
inline constexpr float engage_power_ratio = 0.8f;
inline constexpr float cautious_engage_power_ratio = 1.4f;
inline constexpr std::uint64_t movement_fuel_reserve = 18000;
inline constexpr int max_squads = 1;
inline constexpr int min_squad_size = 2;
inline constexpr bool log_activity = false;
inline constexpr std::uint32_t monolith_from_round = 400;
inline constexpr std::uint32_t monolith_gem_fallback = 80;
inline constexpr float danger_weight = 0.01f;
inline constexpr float altar_weight = 0.0f;
inline constexpr float cluster_bonus = 1.5f;
inline constexpr float travel_weight = 2.5f;
inline constexpr float nearest_camp_bonus = 1.25f;
inline constexpr int nearest_camp_bonus_range = 8;
inline constexpr bool flee_home = true;
inline constexpr std::size_t dead_mask_offset = 3;
inline constexpr int regroup_dead_count = 100;
inline constexpr float enemy_half_factor = 1.0f;
inline constexpr int recall_walk_distance = 6;
inline constexpr float hopeless_power_ratio = 2.0f;
inline constexpr int dive_melee_range = 3;
inline constexpr float dive_engage_power_ratio = 1.35f;
inline constexpr float dive_focus_bonus = 0.55f;
inline constexpr std::uint64_t path_fuel_reserve = 50000;
inline constexpr bool enable_path_precompute = false;
inline constexpr std::uint64_t panic_fuel_reserve = 40000;
inline constexpr std::uint64_t dash_fuel_reserve = 45000;
inline constexpr std::uint64_t observe_fuel_reserve = 50000;
inline constexpr int max_group_polls = 2;
inline constexpr float linked_focus_bonus = 0.6f;
inline constexpr int partner_leash = 0;
inline constexpr int detour_altar_distance = 0;
inline constexpr int fight_priority = 10;
inline constexpr int farm_priority = 0;
inline constexpr bool buy_altar_locks = false;
inline constexpr std::uint32_t lock_gold_reserve = 50;
inline constexpr std::size_t beacon_offset = 4;
inline constexpr std::uint8_t beacon_lifetime = 3;
inline constexpr int beacon_radius = 14;
inline constexpr float beacon_min_vitality = 0.6f;
// Nonzero seeds jitter camp scores so benchmarks can sample several games per map.
inline constexpr std::uint32_t seed = 3;
inline constexpr float seed_jitter = 0.15f;
inline constexpr int stall_patience = 4;
inline constexpr std::uint32_t stall_cooldown = 15;
} // namespace tuning
