#pragma once
#include <cambc.h>
#include <array>
#include <cstdint>
#include <optional>
#include <vector>

// Per-champion state survives between turns; teammates coordinate only through shared memory.
class Bot {
    struct Group {
        cambc::map::camp_group group;
        cambc::bitboard tiles;
        std::vector<cambc::vec2> spawns;
        cambc::vec2 center;
        float health = 0;
        float strength = 0;
        bool monolith = false;
        std::uint32_t respawn_until = 0;
        bool reward_ready = false;
        // Walking distance over static terrain, filled in lazily as spare fuel allows.
        std::optional<cambc::distance_field> path;
    };

    // Live units seen this turn, gathered once to save fuel.
    struct View {
        std::vector<cambc::camp> camps;
        std::vector<cambc::champion> enemies;
        std::vector<cambc::champion> allies;
    };

    enum class Activity { dead, recalling, banking, healing, capturing, fighting, attacking, travelling, waiting, idle, count };

    std::array<std::uint32_t, static_cast<std::size_t>(Activity::count)> activity_counts{};
    bool initialised = false;
    bool melee = true;
    int squad = 0;
    int self_index = 0;
    int dead_allies = 0;
    int squad_count = 1;
    int stalled_turns = 0;
    std::optional<cambc::champion> partner;
    std::vector<Group> groups;
    std::vector<std::int16_t> group_at;
    std::vector<cambc::vec2> altars;
    cambc::bitboard altar_approach;
    std::optional<cambc::distance_field> altar_path;
    std::optional<cambc::distance_field> home_path;
    cambc::vec2 home_center;
    cambc::vec2 enemy_center;
    cambc::bitboard terrain;
    std::uint32_t safe_gem_value = 0;

    void init();
    void precompute_paths();
    int group_of(cambc::vec2 tile) const;
    int travel_to(int group, cambc::vec2 from) const;
    int altar_distance(cambc::vec2 from) const;
    void observe_groups();
    bool target_is_valid(int target) const;
    bool monolith_allowed(const Group& info) const;
    int choose_target(const std::vector<int>& taken) const;
    float vitality() const;
    int current_target(const View& view);

    void shop() const;
    cambc::item_type next_item() const;

    bool skirmish(const View& view);
    float focus_priority(const cambc::champion& enemy) const;
    void raise_beacon(cambc::vec2 where) const;
    bool answer_beacon();
    bool stick_with_partner();
    bool should_bank(int target) const;
    void bank();
    void retreat_or_recall(const View& view);
    Activity farm(int target, const View& view);
    Activity decide();

public:
    // Priorities: shop and heal, recall when low, win or flee fights, bank gems, then farm the squad's camp.
    void play_turn();
};
