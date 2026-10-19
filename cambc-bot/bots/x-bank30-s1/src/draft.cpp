#include <cambc.h>
#include "draft.h"
#include "tuning.h"
#include <algorithm>
#include <iostream>

namespace bc = cambc;

void play_draft() {
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
    for (auto pick : tuning::draft_priority) {
        if (std::find(chosen.begin(), chosen.end(), pick) == chosen.end()) {
            bc::draft::pick(pick);
            return;
        }
    }
}
