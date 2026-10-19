#include <cambc.h>
#include "draft.h"
#include <algorithm>
#include <array>
#include <iostream>

namespace bc = cambc;

// Picks are revealed simultaneously and duplicates across teams are allowed, so a fixed order is safe.
void play_draft() {
    using enum bc::champion_draft;
    constexpr std::array priority{
        cassi_and_issac, stellar, varg, ren, tavi, vendetta, ryth, willow, vesper_vale, marcella, prism,
    };

    const auto chosen = bc::draft::current_draft(bc::team::ally);
    if (chosen.empty()) {
        std::cout << "MAP " << bc::map::width() << "x" << bc::map::height() << " team " << bc::draft::team_size()
                  << " spawn " << bc::to_string(bc::map::spawn_region(bc::team::ally).positions()[0])
                  << " enemy " << bc::to_string(bc::map::spawn_region(bc::team::enemy).positions()[0]) << "\n";
        for (auto altar : bc::map::altars().positions()) std::cout << "ALTAR " << bc::to_string(altar) << "\n";
        for (const auto& group : bc::map::camp_groups()) {
            std::cout << "GROUP gems " << group.gem_value() << " gold " << group.gold_value() << " respawn "
                      << group.max_respawn_timer() << " at " << bc::to_string(group.camps()[0].spawn_position()) << " :";
            for (const auto& camp : group.camps()) std::cout << ' ' << bc::to_string(camp.kind());
            std::cout << "\n";
        }
    }
    for (auto pick : priority) {
        if (std::find(chosen.begin(), chosen.end(), pick) == chosen.end()) {
            bc::draft::pick(pick);
            return;
        }
    }
}
