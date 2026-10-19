#pragma once
#include <cambc.h>
#include <array>
#include <cstdint>

// Strategy knobs; scripts/variant.sh rewrites single lines here to build experiment bots.
namespace tuning {
using enum cambc::champion_draft;
// Picks are revealed simultaneously and duplicates across teams are allowed, so a fixed order is safe.
inline constexpr std::array draft_priority = { cassi_and_issac, stellar, varg, ren, tavi, vendetta, ryth, willow, vesper_vale, marcella, prism };
// Bought until the emblem completes, then each champion buys its main stat.
inline constexpr cambc::item_type opening_shard = cambc::item_type::durability_shard_of_health;
inline constexpr cambc::item_type opening_emblem = cambc::item_type::durability_emblem_of_health;
inline constexpr cambc::item_type strength_shard = cambc::item_type::power_shard_of_strength;
inline constexpr cambc::item_type focus_shard = cambc::item_type::power_shard_of_focus;
inline constexpr cambc::item_type health_shard = cambc::item_type::power_shard_of_health;
inline constexpr float recall_health_ratio = 0.3f;
inline constexpr float flee_health_ratio = 0.45f;
inline constexpr float leave_spawn_health_ratio = 0.95f;
inline constexpr float heal_ally_ratio = 0.65f;
inline constexpr float shield_health_ratio = 0.8f;
inline constexpr std::uint32_t bank_gem_threshold = 50;
inline constexpr std::uint32_t cautious_gem_count = 20;
inline constexpr int nearby_altar_distance = 6;
inline constexpr std::uint32_t endgame_rounds = 150;
inline constexpr float squad_damage_per_round = 120.0f;
inline constexpr int camp_threat_range = 3;
inline constexpr int enemy_threat_range = 4;
inline constexpr int skirmish_range = 7;
inline constexpr int ally_support_range = 6;
inline constexpr int max_chase_distance = 5;
inline constexpr int enemy_spawn_buffer = 4;
inline constexpr float engage_power_ratio = 1.0f;
inline constexpr float cautious_engage_power_ratio = 1.4f;
inline constexpr std::uint64_t movement_fuel_reserve = 12000;
inline constexpr int max_squads = 1;
inline constexpr int min_squad_size = 2;
inline constexpr bool log_activity = false;
inline constexpr std::uint32_t monolith_from_round = 100000;
inline constexpr float danger_weight = 0.01f;
inline constexpr float altar_weight = 0.0f;
inline constexpr bool flee_home = true;
inline constexpr int recall_walk_distance = 6;
inline constexpr float hopeless_power_ratio = 1000.0f;
inline constexpr std::uint64_t path_fuel_reserve = 40000;
inline constexpr float linked_focus_bonus = 0.6f;
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
