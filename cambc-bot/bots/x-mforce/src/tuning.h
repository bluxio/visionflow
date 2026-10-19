#pragma once
#include <cstdint>

// Strategy knobs; scripts/variant.sh rewrites single lines here to build experiment bots.
namespace tuning {
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
inline constexpr bool log_activity = true;
inline constexpr std::uint32_t monolith_from_round = 0;
inline constexpr float danger_weight = 0.01f;
inline constexpr int stall_patience = 4;
inline constexpr std::uint32_t stall_cooldown = 15;
} // namespace tuning
