#pragma once
#include <cambc.h>
#include <cstdint>
#include <vector>

// Per-champion state survives between turns; teammates coordinate only through shared memory.
class Bot {
    struct Group {
        cambc::map::camp_group group;
        cambc::bitboard tiles;
        cambc::vec2 center;
        float health = 0;
        float strength = 0;
        bool monolith = false;
        std::uint32_t respawn_until = 0;
        bool reward_ready = false;
    };

    bool initialised = false;
    bool melee = true;
    int squad = 0;
    int squad_count = 1;
    std::vector<Group> groups;
    std::vector<std::int16_t> group_at;
    std::vector<cambc::vec2> altars;
    cambc::bitboard altar_approach;

    void init();
    int group_of(cambc::vec2 tile) const;
    void observe_groups();
    bool target_is_valid(int target) const;
    int choose_target(int avoid) const;
    int current_target();

    void shop() const;
    cambc::item_type next_item() const;

    bool fight_enemies();
    bool should_bank(int target) const;
    void bank();
    void retreat_or_recall();
    void farm(int target);

public:
    // Priorities: shop and heal, recall when low, fight enemies in range, bank gems, then farm the squad's camp.
    void play_turn();
};
