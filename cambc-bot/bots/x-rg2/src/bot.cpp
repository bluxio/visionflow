#include <cambc.h>
#include "bot.h"
#include "tuning.h"
#include <algorithm>
#include <array>
#include <bit>
#include <cstdint>
#include <exception>
#include <iostream>
#include <optional>
#include <span>
#include <string>
#include <type_traits>
#include <variant>

namespace bc = cambc;
using bc::team;
using bc::vec2;

using namespace tuning;

namespace {

int distance(vec2 a, vec2 b) { return static_cast<int>(a.chebyshev_distance(b)); }

bc::bitboard empty_board() {
    bc::bitboard board;
    board.clear();
    return board;
}

bc::bitboard tiles_around(vec2 position, int radius) {
    auto tiles = empty_board();
    tiles.draw_chebyshev(position, static_cast<float>(radius));
    return tiles;
}

bc::bitboard blocked_tiles() {
    auto blocked = bc::map::walls() | bc::map::holes() | bc::map::altars()
        | bc::map::spawn_region(team::enemy) | bc::vision::champion_tiles()
        | bc::vision::camp_tiles() | bc::vision::teleport_tiles();
    blocked.set(bc::self().position(), false);
    return blocked;
}

template <class Action>
bool attempt(Action&& action) {
    try {
        return action();
    } catch (const std::exception&) {
        return false;
    }
}

template <class T, class Query>
T query_or(T fallback, Query&& query) {
    try {
        return query();
    } catch (const std::exception&) {
        return fallback;
    }
}

bool is_alive_unit(const bc::unit& unit) {
    return attempt([&] { return unit.is_visible() && unit.health() > 0; });
}

float health_ratio(const bc::unit& unit) {
    return query_or(1.0f, [&] { return unit.health() / unit.max_health(); });
}

// Rough fight value: how long it survives times how hard it hits.
float combat_power(const bc::champion& champion) {
    return query_or(0.0f, [&] { return champion.health() * (champion.strength() + 0.25f * champion.focus()); });
}

bool is_channeling(const bc::champion& champion) {
    return attempt([&] { return champion.is_recalling() || champion.is_submitting_gems() || champion.is_teleporting(); });
}

bool exclusive_with_movement(const bc::ability& ability) {
    return std::visit([](const auto& payload) { return payload.exclusive_with_movement; }, *ability.info());
}

float effect_modifier(const bc::ability& ability) {
    return std::visit([](const auto& payload) -> float {
        if constexpr (requires { payload.modifier; }) return payload.modifier;
        else return 0.0f;
    }, *ability.info());
}

bool in_cast_range(const bc::ability& ability, vec2 tile) {
    return attempt([&] { return ability.valid_tiles().is_set(tile); });
}

std::optional<bc::ability> ready_ability(bool (bc::ability::*kind)() const noexcept) {
    auto& me = bc::self();
    if (!me.can_cast()) return std::nullopt;
    for (const auto& ability : me.abilities())
        if (ability.info() && (ability.*kind)() && attempt([&] { return ability.is_ready(); }))
            return ability;
    return std::nullopt;
}

// Walk one step (after an optional dash) towards the nearest goal tile, routing around `avoid` when possible.
bool advance_towards(bc::bitboard goals, bool allow_dash = true, const bc::bitboard* avoid = nullptr) {
    auto& me = bc::self();
    const auto here = me.position();
    if (goals.is_set(here) || bc::fuel_remaining() < movement_fuel_reserve) return false;

    const auto dash = allow_dash ? ready_ability(&bc::ability::is_dash) : std::nullopt;
    if (!me.can_move() && !dash) return false;

    const auto blocked = blocked_tiles();
    goals &= ~blocked | bc::vision::camp_tiles();
    if (goals.is_empty()) return false;

    auto path_walls = blocked & ~goals;
    bc::distance_field field;
    bc::bitboard visited;
    if (avoid) {
        auto detour = path_walls | (*avoid & ~goals);
        detour.set(here, false);
        field.compute_to(detour, goals, tiles_around(here, 0), visited);
    }
    if (!avoid || field.get(here) == bc::distance_field::unreachable)
        field.compute_to(path_walls, goals, tiles_around(here, 0), visited);

    if (dash && field.get(here) > 2) {
        const auto dash_walls = blocked & ~bc::map::holes();
        auto landing = here;
        for (auto tile : dash_walls.reachable_tiles(here, 2).positions())
            if (!blocked.is_set(tile) && field.get(tile) < field.get(landing)) landing = tile;
        if (field.get(landing) + 2 <= field.get(here) && attempt([&] { return dash->use(landing); })) {
            advance_towards(goals, false, avoid);
            return true;
        }
    }

    if (!me.can_move()) return false;
    auto step = here;
    for (auto tile : blocked.reachable_tiles(here, me.base_stats().base_move_speed).positions())
        if (field.get(tile) < field.get(step)) step = tile;
    return step != here && attempt([&] { return me.move_absolute(step); });
}

// Basic attack, then spend an attack reset for a second hit (or a first hit after a ranged walk).
template <class Target>
bool attack_with_reset(const Target& target) {
    auto& me = bc::self();
    if (!is_alive_unit(target) || !me.attack_tiles().is_set(target.position())) return false;

    bool hit = false;
    if (me.can_attack()) hit = attempt([&] { me.attack(target); return true; });
    if (!me.can_attack() && is_alive_unit(target)) {
        if (auto reset = ready_ability(&bc::ability::is_attack_reset))
            if (attempt([&] { return reset->use(); }) && me.can_attack())
                hit = attempt([&] { me.attack(target); return true; }) || hit;
    }
    return hit;
}

// Cast the first ready damage or stun spell that can reach the target.
template <class Target>
bool cast_offensive(const Target& target, bool may_root) {
    auto& me = bc::self();
    if (!me.can_cast() || !is_alive_unit(target)) return false;
    constexpr bool on_champion = std::is_same_v<Target, bc::champion>;
    const auto tile = target.position();
    const bool harmless = !on_champion && target.strength() <= 0;

    for (const auto& ability : me.abilities()) {
        if (!ability.info() || !(ability.is_damage() || ability.is_aoe_damage() || ability.is_cc())) continue;
        if (!attempt([&] { return ability.is_ready(); })) continue;
        if (exclusive_with_movement(ability) && !may_root) continue;
        if (harmless && ability.is_cc() && effect_modifier(ability) <= 0) continue;
        if (!in_cast_range(ability, tile)) continue;

        const bool unit_target = on_champion ? ability.can_target_enemies() : ability.can_target_camps();
        if (unit_target && attempt([&] { return ability.use(target); })) return true;
        if (!unit_target && ability.can_target_ground() && attempt([&] { return ability.use(tile); })) return true;
    }
    return false;
}

bool cast_at(const bc::ability& ability, const bc::champion& enemy) {
    if (ability.can_target_enemies()) return attempt([&] { return ability.use(enemy); });
    return ability.can_target_ground() && attempt([&] { return ability.use(enemy.position()); });
}

// Spend a damage spell when it kills outright: kills pay gold and the victim's whole gem bag.
bool secure_kill(const std::vector<const bc::champion*>& enemies, bool may_root) {
    auto& me = bc::self();
    if (!me.can_cast()) return false;
    for (const auto& ability : me.abilities()) {
        if (!ability.info() || !(ability.is_damage() || ability.is_aoe_damage() || ability.is_cc())) continue;
        if (effect_modifier(ability) <= 0 || (exclusive_with_movement(ability) && !may_root)) continue;
        if (!attempt([&] { return ability.is_ready(); })) continue;
        for (const auto* enemy : enemies) {
            if (!in_cast_range(ability, enemy->position())) continue;
            const float damage = query_or(0.0f, [&] { return ability.damage_amount(*enemy); });
            if (damage + 0.5f >= query_or(1e9f, [&] { return enemy->health(); }) && cast_at(ability, *enemy)) return true;
        }
    }
    return false;
}

// Stun the preferred enemy if it can be reached, otherwise the hardest hitter that is not already stunned.
bool stun_threat(const std::vector<const bc::champion*>& enemies, const bc::champion& preferred, bool may_root) {
    auto& me = bc::self();
    if (!me.can_cast()) return false;
    const auto stunnable = [](const bc::champion& enemy) {
        return query_or(false, [&] { return enemy.passive() != bc::passive_type::stun_immunity && !enemy.buffs().has_stun(); });
    };
    for (const auto& ability : me.abilities()) {
        if (!ability.info() || !ability.is_cc() || (exclusive_with_movement(ability) && !may_root)) continue;
        if (!attempt([&] { return ability.is_ready(); })) continue;
        const bc::champion* victim = nullptr;
        if (stunnable(preferred) && in_cast_range(ability, preferred.position())) victim = &preferred;
        if (!victim) {
            float best = -1;
            for (const auto* enemy : enemies) {
                const float threat = query_or(0.0f, [&] { return enemy->strength() + 0.5f * enemy->focus(); });
                if (threat > best && stunnable(*enemy) && in_cast_range(ability, enemy->position())) {
                    best = threat;
                    victim = enemy;
                }
            }
        }
        if (victim && cast_at(ability, *victim)) return true;
    }
    return false;
}

void use_support(const std::vector<bc::champion>& allies, const std::vector<bc::champion>& enemies,
    const std::vector<bc::camp>& camps) {
    auto& me = bc::self();
    if (auto heal = ready_ability(&bc::ability::is_heal)) {
        const bc::champion* patient = nullptr;
        float worst = heal_ally_ratio;
        for (const auto& ally : allies) {
            const float ratio = health_ratio(ally);
            if (ratio < worst && in_cast_range(*heal, ally.position())) {
                worst = ratio;
                patient = &ally;
            }
        }
        if (patient && attempt([&] { return heal->use(*patient); })) return;
    }

    if (me.health() >= me.max_health() * shield_health_ratio) return;
    if (auto shield = ready_ability(&bc::ability::is_shield)) {
        bool threatened = false;
        for (const auto& camp : camps)
            threatened |= distance(camp.position(), me.position()) <= camp_threat_range && camp.strength() > 0;
        for (const auto& enemy : enemies)
            threatened |= distance(enemy.position(), me.position()) <= enemy_threat_range;
        if (threatened) attempt([&] { return shield->use(); });
    }
}

} // namespace

void Bot::init() {
    auto& me = bc::self();
    melee = me.attack_range() <= 1;

    const int width = bc::map::width();
    group_at.assign(static_cast<std::size_t>(width * bc::map::height()), -1);
    for (const auto& group : bc::map::camp_groups()) {
        Group info{group, group.spawn_tiles(), {}, {}, 0, 0, false, 0, false, {}};
        vec2 sum{};
        for (const auto& camp : group.camps()) {
            const auto tile = camp.spawn_position();
            info.spawns.push_back(tile);
            sum += tile;
            group_at[static_cast<std::size_t>(tile.y * width + tile.x)] = static_cast<std::int16_t>(groups.size());
            switch (camp.kind()) {
            case bc::camp_type::wolf: info.health += 1350; info.strength += 20; break;
            case bc::camp_type::elk: info.health += 2400; info.strength += 60; break;
            case bc::camp_type::monolith: info.health += 20000; info.strength += 200; info.monolith = true; break;
            default: info.health += 500; break;
            }
        }
        const auto members = static_cast<std::int32_t>(std::max<std::size_t>(group.camps().size(), 1));
        info.center = sum / members;
        groups.push_back(std::move(info));
    }

    altars = bc::map::altars().positions();
    altar_approach = empty_board();
    for (auto altar : altars) altar_approach |= tiles_around(altar, bc::constants::submit_gems_range);

    terrain = bc::map::walls() | bc::map::holes() | bc::map::altars() | bc::map::spawn_region(team::enemy);
    {
        const auto spawn = bc::map::spawn_region(team::ally).positions();
        vec2 sum{};
        for (auto tile : spawn) sum += tile;
        home_center = sum / static_cast<std::int32_t>(std::max<std::size_t>(spawn.size(), 1));
        const auto theirs = bc::map::spawn_region(team::enemy).positions();
        vec2 total{};
        for (auto tile : theirs) total += tile;
        enemy_center = total / static_cast<std::int32_t>(std::max<std::size_t>(theirs.size(), 1));
    }

    // Contiguous squads in handle order keep Cassi and Issac (drafted together) side by side.
    auto allies = bc::champions(team::ally);
    std::sort(allies.begin(), allies.end(), [](const auto& a, const auto& b) {
        return static_cast<std::uint32_t>(static_cast<bc::handle_champion_t>(a))
            < static_cast<std::uint32_t>(static_cast<bc::handle_champion_t>(b));
    });
    const int team_size = static_cast<int>(allies.size());
    squad_count = std::clamp(team_size / min_squad_size, 1, max_squads);
    self_index = static_cast<int>(std::find(allies.begin(), allies.end(), me) - allies.begin());
    squad = self_index * squad_count / team_size;
    for (const auto& ally : allies) {
        const auto type = ally.champion_name();
        const auto mine = me.champion_name();
        if ((mine == bc::champion_type::cassi && type == bc::champion_type::issac)
            || (mine == bc::champion_type::issac && type == bc::champion_type::cassi))
            partner = ally;
    }

    initialised = true;
}

int Bot::group_of(vec2 tile) const {
    return group_at[static_cast<std::size_t>(tile.y * bc::map::width() + tile.x)];
}

namespace {
int path_or(const std::optional<bc::distance_field>& path, vec2 from, int fallback) {
    if (!path) return fallback;
    const auto steps = query_or(bc::distance_field::unreachable, [&] { return path->get(from); });
    return steps == bc::distance_field::unreachable ? 2 * fallback : static_cast<int>(steps);
}
}

// One field per turn at most, so the expensive search never crowds out the turn's real work.
void Bot::precompute_paths() {
    if (bc::fuel_remaining() < path_fuel_reserve) return;
    auto compute = [&](std::optional<bc::distance_field>& path, const bc::bitboard& roots) {
        bc::bitboard visited;
        path.emplace();
        attempt([&] { path->compute(terrain & ~roots, roots, visited); return true; });
    };
    if (!home_path) return compute(home_path, bc::map::spawn_region(team::ally));
    if (!altar_path) return compute(altar_path, altar_approach & ~terrain);
    for (auto& info : groups)
        if (!info.path) return compute(info.path, info.tiles);
}

int Bot::travel_to(int group, vec2 from) const {
    const auto& info = groups[static_cast<std::size_t>(group)];
    return path_or(info.path, from, distance(from, info.center));
}

int Bot::altar_distance(vec2 from) const {
    int nearest = 1000;
    for (auto altar : altars) nearest = std::min(nearest, distance(from, altar));
    return path_or(altar_path, from, nearest);
}

// vision::camp_groups() reloads the whole map layout every call, so test our cached groups instead.
void Bot::observe_groups() {
    const auto round = bc::round_number();
    const auto visible = bc::vision::visible_tiles();
    for (auto& info : groups) {
        if (std::ranges::none_of(info.spawns, [&](vec2 tile) { return visible.is_set(tile); })) continue;
        attempt([&] {
            const auto state = info.group.state();
            // Standing on a spawn tile freezes the timer at zero, so always look at least a round ahead.
            if (const auto* respawning = std::get_if<bc::map::camp_group_respawning>(&state)) {
                info.respawn_until = round + static_cast<std::uint32_t>(std::max(respawning->timer, 1));
                info.reward_ready = false;
            } else {
                info.respawn_until = 0;
                info.reward_ready = std::holds_alternative<bc::map::camp_group_reward_available>(state);
            }
            return true;
        });
    }
}

bool Bot::target_is_valid(int target) const {
    if (target < 0 || target >= static_cast<int>(groups.size())) return false;
    const auto& info = groups[static_cast<std::size_t>(target)];
    return monolith_allowed(info) && info.respawn_until < bc::round_number();
}

bool Bot::monolith_allowed(const Group& info) const {
    return !info.monolith || bc::round_number() >= monolith_from_round;
}

int Bot::choose_target(const std::vector<int>& taken) const {
    const auto here = bc::self().position();
    const auto round = bc::round_number();
    int best = -1;
    float best_score = 0;
    for (int i = 0; i < static_cast<int>(groups.size()); ++i) {
        const auto& info = groups[static_cast<std::size_t>(i)];
        if (!monolith_allowed(info) || std::ranges::find(taken, i) != taken.end()) continue;
        const int travel = travel_to(i, here);
        if (info.respawn_until >= round + static_cast<std::uint32_t>(travel)) continue;

        const float clear_time = info.health / squad_damage_per_round;
        const float danger = info.strength * clear_time * danger_weight;
        const float banking = altar_weight * static_cast<float>(info.group.gem_value()) / static_cast<float>(bank_gem_threshold)
            * static_cast<float>(altar_distance(info.center));
        const float value = info.group.gem_value() + 0.3f * info.group.gold_value();
        float score = value / (travel + clear_time + danger + banking + bc::constants::capture_time + 5.0f);
        if (distance(info.center, enemy_center) < distance(info.center, home_center)) score *= enemy_half_factor;
        if constexpr (seed != 0) {
            std::uint64_t mix = (static_cast<std::uint64_t>(seed) << 32) ^ (static_cast<std::uint64_t>(i) << 16) ^ (round / 25);
            mix = (mix ^ (mix >> 30)) * 0xbf58476d1ce4e5b9ULL;
            mix = (mix ^ (mix >> 27)) * 0x94d049bb133111ebULL;
            mix ^= mix >> 31;
            score *= 1.0f + seed_jitter * (static_cast<float>(mix % 1000) / 1000.0f - 0.5f);
        }
        if (score > best_score) {
            best_score = score;
            best = i;
        }
    }
    return best < 0 && !taken.empty() ? choose_target({}) : best;
}

// Cassi and Issac share one life, so each judges danger by the weaker of the pair.
float Bot::vitality() const {
    float ratio = health_ratio(bc::self());
    if (partner && is_alive_unit(*partner)) ratio = std::min(ratio, health_ratio(*partner));
    return ratio;
}

// Shared memory bytes [2s, 2s+1] hold squad s's target group index plus one.
int Bot::current_target(const View& view) {
    std::uint16_t stored = 0;
    bc::shared_memory.read(static_cast<std::size_t>(2 * squad), std::span{&stored, 1});
    int target = static_cast<int>(stored) - 1;

    // One ally is enough to capture a cleared group, so the rest of the squad moves on.
    if (target_is_valid(target)) {
        auto& info = groups[static_cast<std::size_t>(target)];
        const bool capturer_present = info.reward_ready && std::ranges::any_of(view.allies, [&](const auto& ally) {
            return info.tiles.is_set(ally.position());
        });
        if (!capturer_present) return target;
        info.respawn_until = bc::round_number() + static_cast<std::uint32_t>(bc::constants::capture_time)
            + static_cast<std::uint32_t>(info.group.max_respawn_timer());
    }

    std::vector<int> taken;
    for (int other = 0; other < squad_count; ++other) {
        if (other == squad) continue;
        std::uint16_t other_stored = 0;
        bc::shared_memory.read(static_cast<std::size_t>(2 * other), std::span{&other_stored, 1});
        taken.push_back(static_cast<int>(other_stored) - 1);
    }
    target = choose_target(taken);
    stored = static_cast<std::uint16_t>(target + 1);
    bc::shared_memory.write(static_cast<std::size_t>(2 * squad), std::span{&stored, 1});
    return target;
}

bc::item_type Bot::next_item() const {
    auto& me = bc::self();
    const auto counts = me.item_count();
    if (opening_emblem != opening_shard && counts[opening_emblem] == 0) return opening_shard;

    switch (me.champion_name()) {
    case bc::champion_type::stellar:
    case bc::champion_type::prism:
    case bc::champion_type::cassi:
        return focus_shard;
    case bc::champion_type::ryth:
        return health_shard;
    default:
        return strength_shard;
    }
}

void Bot::shop() const {
    auto& me = bc::self();
    for (int purchases = 0; purchases < 20; ++purchases) {
        const auto item = next_item();
        if (!attempt([&] { return me.can_buy(item); })) return;
        if (!attempt([&] { me.buy(item); return true; })) return;
    }
}

// Fight when the local matchup favours us or running is hopeless; otherwise back off.
// Returns true if the turn was spent.
bool Bot::skirmish(const View& view) {
    auto& me = bc::self();
    const auto here = me.position();

    std::vector<const bc::champion*> nearby;
    float theirs = 0;
    for (const auto& enemy : view.enemies) {
        if (distance(enemy.position(), here) > skirmish_range) continue;
        nearby.push_back(&enemy);
        theirs += combat_power(enemy);
    }
    if (nearby.empty()) return false;

    float ours = combat_power(me);
    for (const auto& ally : view.allies)
        if (distance(ally.position(), here) <= ally_support_range) ours += combat_power(ally);

    const bool carrying = me.gem_count() >= cautious_gem_count;
    const float required = carrying ? cautious_engage_power_ratio : engage_power_ratio;
    const bool favourable = ours >= theirs * required && vitality() >= flee_health_ratio;
    // Walking away from a melee champion just hands it free hits.
    const bool cornered = ours * hopeless_power_ratio >= theirs && std::ranges::any_of(nearby, [&](const auto* enemy) {
        return enemy->attack_range() <= 1 && distance(enemy->position(), here) <= 2;
    });

    const bc::champion* target = nullptr;
    for (const auto* enemy : nearby)
        if (!target || focus_priority(*enemy) < focus_priority(*target)) target = enemy;

    const auto reach = me.attack_tiles();
    const bc::champion* in_reach = reach.is_set(target->position()) ? target : nullptr;
    if (!in_reach)
        for (const auto* enemy : nearby)
            if (reach.is_set(enemy->position()) && (!in_reach || focus_priority(*enemy) < focus_priority(*in_reach)))
                in_reach = enemy;

    if (!favourable && !cornered) {
        stun_threat(nearby, *target, false);
        auto danger = empty_board();
        auto reach = empty_board();
        for (const auto* enemy : nearby) {
            danger |= tiles_around(enemy->position(), std::max(enemy->attack_range(), 1) + 2);
            reach |= tiles_around(enemy->position(), std::max(enemy->attack_range(), 1) + 1);
        }
        if (!danger.is_set(here)) return false;
        if (flee_home) {
            if (advance_towards(bc::map::spawn_region(team::ally), true, &reach)) return true;
        } else {
            auto safe = ~danger;
            safe &= tiles_around(here, 6);
            if (advance_towards(safe)) return true;
        }
    }

    raise_beacon(here);
    if (secure_kill(nearby, true)) {
        if (in_reach) attack_with_reset(*in_reach);
        return true;
    }

    if (in_reach) {
        stun_threat(nearby, *in_reach, true);
        attack_with_reset(*in_reach);
        cast_offensive(*in_reach, true);
        return true;
    }

    // Chase only short distances and never into the enemy's healing spawn.
    const bool near_their_spawn
        = !(tiles_around(target->position(), enemy_spawn_buffer) & bc::map::spawn_region(team::enemy)).is_empty();
    if (!favourable || distance(target->position(), here) > max_chase_distance || near_their_spawn) {
        stun_threat(nearby, *target, false);
        return false;
    }

    stun_threat(nearby, *target, false);
    const auto goals = tiles_around(target->position(), me.attack_range());
    const bool moved = advance_towards(goals);
    if (secure_kill(nearby, !moved)) return true;
    attack_with_reset(*target);
    cast_offensive(*target, !moved);
    return true;
}

// Lower is better: low health, linked Cassi/Issac (one kill takes both), channelers and gem carriers.
float Bot::focus_priority(const bc::champion& enemy) const {
    float score = query_or(1e9f, [&] { return enemy.health(); });
    const auto type = query_or(bc::champion_type::ryth, [&] { return enemy.champion_name(); });
    if (type == bc::champion_type::cassi || type == bc::champion_type::issac) score *= linked_focus_bonus;
    score -= 10.0f * static_cast<float>(query_or(0u, [&] { return enemy.gem_count(); }));
    if (is_channeling(enemy)) score -= 300.0f;
    return score;
}

// Shared memory bytes 4..7: x, y, low byte of the round, and a nonzero marker.
void Bot::raise_beacon(vec2 where) const {
    const std::array<std::uint8_t, 4> beacon{static_cast<std::uint8_t>(where.x), static_cast<std::uint8_t>(where.y),
        static_cast<std::uint8_t>(bc::round_number() & 0xFF), 1};
    bc::shared_memory.write(beacon_offset, std::span{beacon});
}

bool Bot::answer_beacon() {
    auto& me = bc::self();
    std::array<std::uint8_t, 4> beacon{};
    bc::shared_memory.read(beacon_offset, std::span{beacon});
    if (beacon[3] == 0) return false;
    const auto age = static_cast<std::uint8_t>((bc::round_number() & 0xFF) - beacon[2]);
    if (age > beacon_lifetime) return false;

    const vec2 where{beacon[0], beacon[1]};
    const int gap = distance(me.position(), where);
    if (gap <= 2 || gap > beacon_radius) return false;
    if (vitality() < beacon_min_vitality || me.gem_count() >= cautious_gem_count) return false;
    return advance_towards(tiles_around(where, 2));
}

bool Bot::should_bank(int target) const {
    auto& me = bc::self();
    const auto gems = me.gem_count();
    if (gems == 0) return false;

    // Never walk off a capture in progress.
    const auto here = me.position();
    if (target >= 0) {
        const auto& info = groups[static_cast<std::size_t>(target)];
        if (info.reward_ready && info.tiles.is_set(here)) return false;
    }

    if (gems >= bank_gem_threshold) return true;
    if (bc::round_number() + endgame_rounds >= bc::constants::max_rounds) return true;
    return std::ranges::any_of(altars, [&](vec2 altar) { return distance(here, altar) <= nearby_altar_distance; });
}

void Bot::bank() {
    auto& me = bc::self();
    auto goals = altar_approach;
    for (auto altar : altars) {
        if (bc::vision::has_altar_lock(altar) && bc::vision::altar_lock_at(altar).is_enemy()) {
            goals &= ~tiles_around(altar, bc::constants::submit_gems_range);
            continue;
        }
        if (distance(me.position(), altar) <= bc::constants::submit_gems_range
            && attempt([&] { me.submit_gems(altar); return true; }))
            return;
    }
    advance_towards(goals);
}

// Run for our own spawn, which enemies cannot enter, routing around anything that can hit us.
// Recall instead when nothing is close enough to interrupt it and home is a long walk.
void Bot::retreat_or_recall(const View& view) {
    auto& me = bc::self();
    const auto here = me.position();
    const auto home = bc::map::spawn_region(team::ally);
    auto danger = empty_board();
    auto reach = empty_board();
    for (const auto& camp : view.camps) {
        if (camp.strength() <= 0) continue;
        danger |= tiles_around(camp.position(), camp_threat_range);
        reach |= tiles_around(camp.position(), camp_threat_range);
    }
    for (const auto& enemy : view.enemies) {
        danger |= tiles_around(enemy.position(), enemy_threat_range);
        reach |= tiles_around(enemy.position(), std::max(enemy.attack_range(), 1) + 1);
    }

    const int walk_home = path_or(home_path, here, distance(here, home_center));
    if (!danger.is_set(here) && walk_home > recall_walk_distance) {
        attempt([&] { me.recall(); return true; });
        return;
    }
    if (flee_home) {
        if (advance_towards(home, true, &reach)) return;
    } else {
        auto safe = ~danger;
        safe &= tiles_around(here, 6);
        if (advance_towards(safe)) return;
    }
    for (const auto& enemy : view.enemies)
        if (attack_with_reset(enemy)) break;
}

Bot::Activity Bot::farm(int target, const View& view) {
    auto& me = bc::self();
    if (target < 0) return Activity::idle;
    const auto& info = groups[static_cast<std::size_t>(target)];

    // Only hit camps in our target group, or harmless clusters whose whole group is harmless.
    auto pick_victim = [&]() -> const bc::camp* {
        const bc::camp* victim = nullptr;
        const auto reach = me.attack_tiles();
        for (const auto& camp : view.camps) {
            if (!is_alive_unit(camp) || !reach.is_set(camp.position())) continue;
            const int group = group_of(camp.spawn_position());
            const bool wanted = group == target
                || (group >= 0 && groups[static_cast<std::size_t>(group)].strength <= 0);
            if (wanted && (!victim || camp.health() < victim->health())) victim = &camp;
        }
        return victim;
    };

    if (const auto* victim = pick_victim()) {
        attack_with_reset(*victim);
        cast_offensive(*victim, true);
        return Activity::attacking;
    }

    // Approach: melee closes on the camps, ranged stops at attack range; empty groups mean capture.
    auto goals = empty_board();
    for (const auto& camp : view.camps) {
        if (group_of(camp.spawn_position()) != target) continue;
        if (melee) goals.set(camp.position());
        else goals |= tiles_around(camp.position(), me.attack_range());
    }
    if (goals.is_empty()) goals = info.tiles;

    const bool moved = advance_towards(goals);
    if (const auto* victim = pick_victim()) {
        attack_with_reset(*victim);
        cast_offensive(*victim, !moved);
        return Activity::attacking;
    }
    if (moved) {
        stalled_turns = 0;
        return Activity::travelling;
    }

    // Blocked or contested targets (hidden enemies, bushes, occupied tiles) are abandoned for a while,
    // but never while the group's camps are still standing and being fought.
    const bool camps_standing = std::ranges::any_of(view.camps, [&](const auto& camp) {
        return group_of(camp.spawn_position()) == target;
    });
    if (!camps_standing && ++stalled_turns >= stall_patience) {
        stalled_turns = 0;
        groups[static_cast<std::size_t>(target)].respawn_until = bc::round_number() + stall_cooldown;
        const std::uint16_t cleared = 0;
        bc::shared_memory.write(static_cast<std::size_t>(2 * squad), std::span{&cleared, 1});
    }
    return Activity::waiting;
}

void Bot::play_turn() {
    if (!initialised) init();
    const auto activity = decide();
    precompute_paths();
    if constexpr (log_activity) {
        static Activity previous = Activity::idle;
        static std::string context;
        if (activity == Activity::dead && previous != Activity::dead)
            std::cout << "DIED r" << bc::round_number() << ' ' << bc::to_string(bc::self().champion_name()) << " after " << context << std::endl;
        previous = activity;
        if (activity != Activity::dead) {
            const auto& me = bc::self();
            context = "act" + std::to_string(static_cast<int>(activity)) + " hp" + std::to_string(static_cast<int>(me.health()))
                + " gems" + std::to_string(me.gem_count()) + " foes" + std::to_string(bc::vision::champions(team::enemy).size())
                + " pals" + std::to_string(bc::vision::champions(team::ally).size()) + " at" + bc::to_string(me.position());
        }
        ++activity_counts[static_cast<std::size_t>(activity)];
        if (bc::round_number() % 100 == 99) {
            std::cout << "ACT " << bc::round_number() << ' ' << bc::to_string(bc::self().champion_name());
            for (auto count : activity_counts) std::cout << ' ' << count;
            std::cout << '\n';
        }
    }
}

Bot::Activity Bot::decide() {
    auto& me = bc::self();

    const bool alive = me.is_alive();
    std::uint8_t dead_mask = 0;
    bc::shared_memory.read(dead_mask_offset, std::span{&dead_mask, 1});
    const auto my_bit = static_cast<std::uint8_t>(1u << (self_index & 7));
    const auto updated = static_cast<std::uint8_t>(alive ? dead_mask & ~my_bit : dead_mask | my_bit);
    if (updated != dead_mask) bc::shared_memory.write(dead_mask_offset, std::span{&updated, 1});
    dead_allies = std::popcount(static_cast<unsigned>(updated & ~my_bit));

    if (!alive) {
        shop();
        return Activity::dead;
    }
    if (me.is_recalling()) return Activity::recalling;
    if (me.is_submitting_gems()) return Activity::banking;
    if (me.is_teleporting()) return Activity::recalling;

    observe_groups();
    const auto here = me.position();
    const bool at_spawn = bc::map::spawn_region(team::ally).is_set(here);
    if (at_spawn) shop();

    View view;
    for (auto& camp : bc::vision::camps())
        if (is_alive_unit(camp)) view.camps.push_back(std::move(camp));
    for (auto& enemy : bc::vision::champions(team::enemy))
        if (is_alive_unit(enemy)) view.enemies.push_back(std::move(enemy));
    for (auto& ally : bc::vision::champions(team::ally))
        if (!(ally == me) && is_alive_unit(ally)) view.allies.push_back(std::move(ally));

    use_support(view.allies, view.enemies, view.camps);

    if (at_spawn && (me.health() < me.max_health() * leave_spawn_health_ratio || dead_allies >= regroup_dead_count)) {
        skirmish(view);
        return Activity::healing;
    }

    // Hold a capture in progress unless an enemy forces a fight.
    const int standing_on = group_of(here);
    if (standing_on >= 0 && groups[static_cast<std::size_t>(standing_on)].reward_ready) {
        skirmish(view);
        return Activity::capturing;
    }

    if (!at_spawn && me.health() < me.max_health() * recall_health_ratio) {
        retreat_or_recall(view);
        return Activity::recalling;
    }
    if (skirmish(view)) return Activity::fighting;

    const int target = current_target(view);
    if (should_bank(target)) {
        bank();
        return Activity::banking;
    }
    if (answer_beacon()) return Activity::fighting;
    return farm(target, view);
}
