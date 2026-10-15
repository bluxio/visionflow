#include <cambc.h>
#include "draft.h"
#include <algorithm>
#include <array>

namespace bc = cambc;

// Picks are revealed simultaneously and duplicates across teams are allowed, so a fixed order is safe.
void play_draft() {
    using enum bc::champion_draft;
    constexpr std::array priority{
        varg, vendetta, stellar, ryth, willow, vesper_vale, ren, tavi, marcella, prism, cassi_and_issac,
    };

    const auto chosen = bc::draft::current_draft(bc::team::ally);
    for (auto pick : priority) {
        if (std::find(chosen.begin(), chosen.end(), pick) == chosen.end()) {
            bc::draft::pick(pick);
            return;
        }
    }
}
