#include <cambc.h>
#include "bot.h"
#include <algorithm>
#include <array>
#include <cstdint>
#include <exception>
#include <optional>
#include <span>
#include <type_traits>
#include <variant>

namespace bc = cambc;
using bc::team;
using bc::vec2;

namespace {

constexpr float recall_health_ratio = 0.3f;
constexpr float leave_spawn_health_ratio = 0.95f;
constexpr float heal_ally_ratio = 0.65f;
constexpr float shield_health_ratio = 0.8f;
constexpr std::uint32_t bank_gem_threshold = 50;
constexpr int nearby_altar_distance = 6;
constexpr std::uint32_t endgame_rounds = 150;
constexpr float squad_damage_per_round = 120.0f;
constexpr int camp_threat_range = 3;
constexpr int enemy_threat_range = 4;

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

bool is_alive_unit(const bc::unit& unit) {
    return attempt([&] { return unit.is_visible() && unit.health() > 0; });
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

// Walk one step (after an optional dash) towards the nearest goal tile.
bool advance_towards(bc::bitboard goals, bool allow_dash = true) {
    auto& me = bc::self();
    const auto here = me.position();
    if (goals.is_set(here)) return false;

    const auto dash = allow_dash ? ready_ability(&bc::ability::is_dash) : std::nullopt;
    if (!me.can_move() && !dash) return false;

    const auto blocked = blocked_tiles();
    goals &= ~blocked | bc::vision::camp_tiles();
    if (goals.is_empty()) return false;

    const auto path_walls = blocked & ~goals;
    bc::distance_field field;
    bc::bitboard visited;
    field.compute_to(path_walls, goals, tiles_around(here, 0), visited);

    if (dash && field.get(here) > 2) {
        const auto dash_walls = blocked & ~bc::map::holes();
        auto landing = here;
        for (auto tile : dash_walls.reachable_tiles(here, 2).positions())
            if (!blocked.is_set(tile) && field.get(tile) < field.get(landing)) landing = tile;
        if (field.get(landing) + 2 <= field.get(here) && attempt([&] { return dash->use(landing); })) {
            advance_towards(goals, false);
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

void use_support() {
    auto& me = bc::self();
    if (auto heal = ready_ability(&bc::ability::is_heal)) {
        std::optional<bc::champion> patient;
        float worst = heal_ally_ratio;
        for (const auto& ally : bc::vision::champions(team::ally)) {
            if (ally == me || !is_alive_unit(ally)) continue;
            const float ratio = ally.health() / ally.max_health();
            if (ratio < worst && in_cast_range(*heal, ally.position())) {
                worst = ratio;
                patient = ally;
            }
        }
        if (patient && attempt([&] { return heal->use(*patient); })) return;
    }

    if (me.health() >= me.max_health() * shield_health_ratio) return;
    if (auto shield = ready_ability(&bc::ability::is_shield)) {
        bool threatened = false;
        for (const auto& camp : bc::vision::camps())
            threatened |= distance(camp.position(), me.position()) <= camp_threat_range && camp.strength() > 0;
        for (const auto& enemy : bc::vision::champions(team::enemy))
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
        Group info{group, group.spawn_tiles(), {}, 0, 0, false, 0, false};
        vec2 sum{};
        for (const auto& camp : group.camps()) {
            const auto tile = camp.spawn_position();
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

    // Split into two squads on larger teams; sort by handle so every teammate agrees.
    auto allies = bc::champions(team::ally);
    std::sort(allies.begin(), allies.end(), [](const auto& a, const auto& b) {
        return static_cast<std::uint32_t>(static_cast<bc::handle_champion_t>(a))
            < static_cast<std::uint32_t>(static_cast<bc::handle_champion_t>(b));
    });
    squad_count = allies.size() >= 4 ? 2 : 1;
    const auto self_index = std::find(allies.begin(), allies.end(), me) - allies.begin();
    squad = static_cast<int>(self_index % squad_count);

    initialised = true;
}

int Bot::group_of(vec2 tile) const {
    return group_at[static_cast<std::size_t>(tile.y * bc::map::width() + tile.x)];
}

void Bot::observe_groups() {
    const auto round = bc::round_number();
    for (const auto& group : bc::vision::camp_groups()) {
        if (group.camps().empty()) continue;
        const int index = group_of(group.camps()[0].spawn_position());
        if (index < 0) continue;
        attempt([&] {
            const auto state = group.state();
            auto& info = groups[static_cast<std::size_t>(index)];
            if (const auto* respawning = std::get_if<bc::map::camp_group_respawning>(&state)) {
                info.respawn_until = round + static_cast<std::uint32_t>(std::max(respawning->timer, 0));
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
    return !info.monolith && info.respawn_until <= bc::round_number();
}

int Bot::choose_target(int avoid) const {
    const auto here = bc::self().position();
    const auto round = bc::round_number();
    int best = -1;
    float best_score = 0;
    for (int i = 0; i < static_cast<int>(groups.size()); ++i) {
        const auto& info = groups[static_cast<std::size_t>(i)];
        if (info.monolith || (i == avoid && groups.size() > 1)) continue;
        const int travel = distance(here, info.center);
        if (info.respawn_until > round + static_cast<std::uint32_t>(travel)) continue;

        const float clear_time = info.health / squad_damage_per_round;
        const float danger = info.strength * clear_time / 100.0f;
        const float value = info.group.gem_value() + 0.3f * info.group.gold_value();
        const float score = value / (travel + clear_time + danger + bc::constants::capture_time + 5.0f);
        if (score > best_score) {
            best_score = score;
            best = i;
        }
    }
    return best;
}

// Shared memory bytes [2s, 2s+1] hold squad s's target group index plus one.
int Bot::current_target() {
    std::uint16_t stored = 0;
    bc::shared_memory.read(static_cast<std::size_t>(2 * squad), std::span{&stored, 1});
    int target = static_cast<int>(stored) - 1;
    if (target_is_valid(target)) return target;

    int other = -1;
    if (squad_count > 1) {
        std::uint16_t other_stored = 0;
        bc::shared_memory.read(static_cast<std::size_t>(2 * (1 - squad)), std::span{&other_stored, 1});
        other = static_cast<int>(other_stored) - 1;
    }
    target = choose_target(other);
    stored = static_cast<std::uint16_t>(target + 1);
    bc::shared_memory.write(static_cast<std::size_t>(2 * squad), std::span{&stored, 1});
    return target;
}

bc::item_type Bot::next_item() const {
    auto& me = bc::self();
    const auto counts = me.item_count();
    if (counts[bc::item_type::jungle_emblem_of_health] == 0) return bc::item_type::jungle_shard_of_health;

    switch (me.champion_name()) {
    case bc::champion_type::stellar:
    case bc::champion_type::prism:
    case bc::champion_type::cassi:
        return bc::item_type::power_shard_of_focus;
    case bc::champion_type::ryth:
        return bc::item_type::power_shard_of_health;
    default:
        return bc::item_type::power_shard_of_strength;
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

// Hit the weakest enemy champion in reach; stay put while trading blows.
bool Bot::fight_enemies() {
    auto& me = bc::self();
    std::optional<bc::champion> victim;
    for (const auto& enemy : bc::vision::champions(team::enemy)) {
        if (!is_alive_unit(enemy) || !me.attack_tiles().is_set(enemy.position())) continue;
        if (!victim || enemy.health() < victim->health()) victim = enemy;
    }
    if (!victim) return false;
    const bool hit = attack_with_reset(*victim);
    const bool cast = cast_offensive(*victim, true);
    return hit || cast;
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

// Step out of camp and enemy reach first, because damage cancels the recall channel.
void Bot::retreat_or_recall() {
    auto& me = bc::self();
    auto danger = empty_board();
    for (const auto& camp : bc::vision::camps())
        if (is_alive_unit(camp) && camp.strength() > 0) danger |= tiles_around(camp.position(), camp_threat_range);
    for (const auto& enemy : bc::vision::champions(team::enemy))
        if (is_alive_unit(enemy)) danger |= tiles_around(enemy.position(), enemy_threat_range);

    if (!danger.is_set(me.position())) {
        attempt([&] { me.recall(); return true; });
        return;
    }
    auto safe = ~danger;
    safe &= tiles_around(me.position(), 6);
    if (!advance_towards(safe))
        for (const auto& enemy : bc::vision::champions(team::enemy))
            if (attack_with_reset(enemy)) break;
}

void Bot::farm(int target) {
    auto& me = bc::self();
    if (target < 0) return;
    const auto& info = groups[static_cast<std::size_t>(target)];

    // Only hit camps in our target group, or harmless clusters whose whole group is harmless.
    auto pick_victim = [&]() -> std::optional<bc::camp> {
        std::optional<bc::camp> victim;
        const auto reach = me.attack_tiles();
        for (const auto& camp : bc::vision::camps()) {
            if (!is_alive_unit(camp) || !reach.is_set(camp.position())) continue;
            const int group = group_of(camp.spawn_position());
            const bool wanted = group == target
                || (group >= 0 && groups[static_cast<std::size_t>(group)].strength <= 0);
            if (wanted && (!victim || camp.health() < victim->health())) victim = camp;
        }
        return victim;
    };

    if (auto victim = pick_victim()) {
        attack_with_reset(*victim);
        cast_offensive(*victim, true);
        return;
    }

    // Approach: melee closes on the camps, ranged stops at attack range; empty groups mean capture.
    auto goals = empty_board();
    for (const auto& camp : bc::vision::camps()) {
        if (!is_alive_unit(camp) || group_of(camp.spawn_position()) != target) continue;
        if (melee) goals.set(camp.position());
        else goals |= tiles_around(camp.position(), me.attack_range());
    }
    if (goals.is_empty()) goals = info.tiles;

    const bool moved = advance_towards(goals);
    if (auto victim = pick_victim()) {
        attack_with_reset(*victim);
        cast_offensive(*victim, !moved);
    }
}

void Bot::play_turn() {
    if (!initialised) init();
    auto& me = bc::self();

    if (!me.is_alive()) {
        shop();
        return;
    }
    if (me.is_recalling() || me.is_submitting_gems() || me.is_teleporting()) return;

    observe_groups();
    const bool at_spawn = bc::map::spawn_region(team::ally).is_set(me.position());
    if (at_spawn) shop();

    use_support();

    if (at_spawn && me.health() < me.max_health() * leave_spawn_health_ratio) {
        fight_enemies();
        return;
    }
    if (!at_spawn && me.health() < me.max_health() * recall_health_ratio) {
        retreat_or_recall();
        return;
    }
    if (fight_enemies()) return;

    const int target = current_target();
    if (should_bank(target)) {
        bank();
        return;
    }
    farm(target);
}
