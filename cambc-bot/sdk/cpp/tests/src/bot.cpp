#include <cambc.h>
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <cassert>
#include <cstdio>
#include <cstdlib>

extern "C" {
__attribute__((import_module("env"), import_name("allies"))) std::int32_t allies(std::uint32_t*, std::uint32_t, std::uint32_t*);
__attribute__((import_module("env"), import_name("board_vision"))) std::int32_t vision(std::uint64_t*);
__attribute__((import_module("env"), import_name("board_vision_mask"))) std::int32_t visible_tiles(std::int32_t, std::int32_t, std::int32_t, std::uint64_t*);
__attribute__((import_module("env"), import_name("my_champion"))) std::int32_t my_champion();
__attribute__((import_module("env"), import_name("draft_current_round"))) std::int32_t current_round(std::uint8_t*);
__attribute__((import_module("env"), import_name("draft_memory_size"))) std::int32_t memory_size(std::uint32_t*);
__attribute__((import_module("env"), import_name("read_draft_memory"))) std::int32_t read_memory(std::int32_t, void*, std::uint32_t);
}

template<class Exception, class F>
void throws(F test) {
    bool caught = false;
    try { test(); } catch (const Exception&) { caught = true; }
    assert(caught);
}

void test_primitives() {
    const cambc::vec2 p{1, 1}, q{2, 1};
    cambc::bitboard a{};
    a.clear();
    assert(a.width() == cambc::map::width() && a.height() == cambc::map::height());
    assert(reinterpret_cast<std::uintptr_t>(a.ptr()) % 16 == 0);
    assert(a.is_empty() && a.positions().empty());
    assert(cambc::map::is_in_bounds(p) && !cambc::map::is_in_bounds({-1, 0}));
    a.set(p);
    assert(a.is_set(p) && a.count() == 1);
    auto b = a;
    b.set(q);
    assert(!a.is_set(q) && b.count() == 2 && a.ptr() != b.ptr());
    cambc::bitboard c{};
    c = b;
    c.toggle(p);
    assert(c.is_set(q) && !c.is_set(p) && b.is_set(p));
    auto* alias = &c;
    c = *alias;
    assert(c.is_set(q));
    c = std::move(*alias);
    assert(c.is_set(q));
    cambc::bitboard moved{std::move(c)};
    c = a;
    assert(moved.is_set(q) && c.is_set(p));

    cambc::bitboard one, zero;
    one.fill(); zero.clear();
    std::fprintf(stderr, "checking bitwise\n");
    assert(one.count() == std::uint32_t(a.width() * a.height()));
    assert(~one == zero && ~~a == a);
    assert((a | b) == b && (a & b) == a && (a ^ b).is_set(q));
    c = a; c |= c; assert(c == a);
    c &= c; assert(c == a);
    c ^= c; assert(c.is_empty());
    c.fill(); assert(c == one); c.clear(); assert(c.is_empty());
    assert(a.shift_right(1).is_set(q));
    assert(a.shift_right(-1).is_set({0, 1}));
    assert(a.shift_up(1).is_set({1, 2}));
    assert(a.shift_up(-1).is_set({1, 0}));
    assert(a.shift_right(a.width()).is_empty());
    assert(a.shift_up(a.height()).is_empty());
    assert(a.reflect_horizontal().is_set(cambc::map::reflect_horizontal(p)));
    assert(a.reflect_vertical().is_set(cambc::map::reflect_vertical(p)));
    assert(a.rotate_180().is_set(cambc::map::rotate_180(p)));
    assert(a.reflect(cambc::map::symmetry()).reflect(cambc::map::symmetry()) == a);

    b.import_raw_buffer(a.export_raw_buffer()); assert(b == a);
    std::fprintf(stderr, "checking transfers\n");
    b.import_linear_buffer(a.export_linear_buffer()); assert(b == a);
    auto positions = b.positions();
    assert(positions.size() == 1 && positions[0] == p);
    throws<cambc::error>([&] { b.import_linear_buffer({}); });
    throws<cambc::error>([&] { b.export_raw_buffer(std::span<std::uint8_t>{}); });
    throws<cambc::error>([&] { a.set({-1, 0}); });
    throws<cambc::error>([&] { (void)a.is_set({a.width(), 0}); });
    throws<cambc::error>([&] { a.toggle({0, a.height()}); });
    a.clear(); a.draw_circle(p, 1.0f); assert(a.is_set(p) && a.is_set(q));
    a.clear(); a.draw_chebyshev(p, 1.0f); assert(a.is_set({2, 2}));
    try {
        a.draw_circle(p, -1.0f);
        assert(false);
    } catch (const cambc::error& e) {
        assert(e.code() == cambc::error_code::invalid_argument);
        assert(!e.message().empty());
        assert(e.message() == cambc::detail::get_last_error());
        assert(std::string(e.what()).find(e.message()) != std::string::npos);
        assert(std::string(e.what()).find(std::to_string(static_cast<std::int32_t>(e.code()))) != std::string::npos);
    }

    cambc::bitboard walls;
    walls.clear();
    std::fprintf(stderr, "checking reachability\n");
    assert(walls.reachable_tiles(p, 0).count() == 1);
    cambc::bitboard frontier{}, scratch{};
    assert(walls.reachable_tiles(p, 3) == walls.reachable_tiles(p, 3, frontier, scratch));
    assert(walls.shadow_cast(p, 2).is_set(q));
    throws<cambc::error>([&] { (void)walls.reachable_tiles(p, 3, walls, scratch); });

    cambc::distance_field field{};
    std::fprintf(stderr, "checking fields\n");
    assert(field.width() == a.width() && field.height() == a.height());
    assert(reinterpret_cast<std::uintptr_t>(field.ptr()) % 2 == 0);
    field.set(p, 42); assert(field.get(p) == 42);
    auto copied_field = field; copied_field.set(p, 7);
    assert(field.get(p) == 42 && copied_field.ptr() != field.ptr());
    copied_field = field; copied_field.set(p, 9); assert(field.get(p) == 42);
    cambc::distance_field moved_field{std::move(copied_field)};
    assert(moved_field.get(p) == 9);
    cambc::distance_field restored{};
    restored.import_raw_buffer(field.export_raw_buffer()); assert(restored.get(p) == 42);
    restored.import_linear_buffer(field.export_linear_buffer()); assert(restored.get(p) == 42);
    throws<cambc::error>([&] { field.import_linear_buffer({}); });
    throws<cambc::error>([&] { (void)field.get({-1, 0}); });
    throws<cambc::error>([&] { field.set({0, field.height()}, 0); });
    cambc::bitboard roots{}, visited{}, destinations{};
    roots.clear(); destinations.clear();
    roots.set(p); destinations.set(q);
    auto expansions = field.compute_with_expansion_count(walls, roots, visited, 3);
    std::fprintf(stderr, "checking field computations\n");
    assert(expansions == 3 && field.get(p) == 0 && field.get(q) == 1);
    assert(field.get({field.width() - 1, field.height() - 1}) == cambc::distance_field::unreachable);
    field.compute(walls, roots, visited, 0);
    assert(field.get(p) == 0 && field.get(q) == cambc::distance_field::unreachable);
    expansions = field.compute_to_with_expansion_count(walls, roots, destinations, visited, 3);
    assert(expansions == 1 && field.get(q) == 1);
    field.compute_to(walls, roots, destinations, visited);
    assert(field.get(q) == 1);
    // Invalid guest pointers must be rejected before touching an engine buffer.
    assert(cambc::detail::sdk_Bitboard_Zero(reinterpret_cast<std::uint64_t*>(1)) == -1);
    assert(cambc::detail::sdk_DistanceField_Get(reinterpret_cast<const std::uint16_t*>(1), 0, 0, field.ptr()) == -1);
    std::fprintf(stderr, "primitives PASS\n");
}

void test_map() {
    assert(cambc::map::is_in_bounds({0, 0}));
    assert(cambc::map::is_in_bounds({cambc::map::width() - 1, cambc::map::height() - 1}));
    for (const auto p : {cambc::vec2{-1, 0}, cambc::vec2{0, -1}, cambc::vec2{cambc::map::width(), 0},
                         cambc::vec2{0, cambc::map::height()}, cambc::vec2{std::numeric_limits<std::int32_t>::min(), 0},
                         cambc::vec2{0, std::numeric_limits<std::int32_t>::max()}})
        assert(!cambc::map::is_in_bounds(p));
    const auto& walls = cambc::map::walls();
    assert(&walls == &cambc::map::walls());
    assert(&cambc::map::bushes() == &cambc::map::bushes());
    assert(&cambc::map::holes() == &cambc::map::holes());
    assert(&cambc::map::altars() == &cambc::map::altars());
    auto changed = walls;
    changed.fill();
    assert(&cambc::map::spawn_region(cambc::team::ally) == &cambc::map::spawn_region(cambc::team::ally));
    assert(!cambc::map::spawn_region(cambc::team::ally).is_empty());
    assert(!cambc::map::spawn_region(cambc::team::enemy).is_empty());
    assert(cambc::map::reflect_horizontal(cambc::map::reflect_horizontal({1, 1})) == cambc::vec2(1, 1));
    assert(cambc::map::rotate_180(cambc::map::rotate_180({1, 1})) == cambc::vec2(1, 1));
    if (!cambc::is_draft()) {
    cambc::bitboard occupied;
    occupied.clear();
    const auto groups = cambc::map::camp_groups();
    std::uint32_t group_count{};
    cambc::detail::check(cambc::detail::sdk_map_num_camp_groups(&group_count));
    assert(groups.size() == group_count);
    for (const auto& group : groups) {
        assert(group.spawn_tiles().count() == group.camps().size());
        assert((occupied & group.spawn_tiles()).is_empty());
        occupied |= group.spawn_tiles();
        std::uint32_t count{};
        cambc::detail::check(cambc::detail::sdk_map_camp_group_num_camps(static_cast<std::uint32_t>(static_cast<cambc::handle_camp_group_t>(group)), &count));
        assert(count == group.camps().size());
        assert(group.max_respawn_timer() > 0);
        for (const auto& camp : group) {
            const auto position = camp.spawn_position();
            assert(group.spawn_tiles().is_set(position));
            assert(static_cast<cambc::handle_camp_t>(cambc::map::camp(position)) == static_cast<cambc::handle_camp_t>(camp));
            assert(static_cast<cambc::handle_camp_group_t>(camp.camp_group()) == static_cast<cambc::handle_camp_group_t>(group));
            cambc::detail::camp_info info{};
            cambc::detail::check(cambc::detail::sdk_map_camp_info(static_cast<std::uint32_t>(static_cast<cambc::handle_camp_t>(camp)), &info));
            assert(position == cambc::vec2(info.x, info.y));
            assert(camp.kind() == static_cast<cambc::camp_type>(info.type));
        }
    }
    throws<std::out_of_range>([] { (void)cambc::map::camp({-1, 0}); });
    } else {
        try { (void)cambc::map::camp_groups(); assert(false); }
        catch (const cambc::error& error) { assert(error.code() == cambc::error_code::api_not_initialized); }
    }
    const cambc::vec2 a{3, 4}, b{1, 2};
    assert(a + b == cambc::vec2(4, 6) && a - b == cambc::vec2(2, 2));
    assert(a * 2 == cambc::vec2(6, 8) && a / 2 == cambc::vec2(1, 2));
    assert(a.dot(b) == 11 && a.cross(b) == 2 && a.length_squared() == 25);
    assert(a.chebyshev_distance(b) == 2);
    throws<std::invalid_argument>([&] { (void)(a / 0); });
    std::fprintf(stderr, "map PASS\n");
}

void test_vision() {
    const auto champions = cambc::vision::champions();
    const auto positions = cambc::vision::champion_tiles().positions();
    assert(champions.size() == positions.size());
    for (std::size_t i = 0; i < positions.size(); ++i)
        assert(champions[i].position() == positions[i]);
    const auto self = cambc::vision::champion_at(cambc::self().position());
    assert(self.team() == cambc::team::ally);
    for (const auto side : {cambc::team::ally, cambc::team::enemy}) {
        const auto filtered = cambc::vision::champions(side);
        assert(filtered.size() == cambc::vision::champion_tiles(side).count());
        for (const auto& member : filtered) assert(member.team() == side);
        const auto roster = cambc::champions(side);
        std::uint32_t count{};
        cambc::detail::check(side == cambc::team::ally ? cambc::detail::sdk_num_allies(&count) : cambc::detail::sdk_num_enemies(&count));
        assert(roster.size() == count);
    }
    for (const auto& champion : champions) {
        const auto position = champion.position();
        assert(cambc::vision::has_champion(position));
        assert(static_cast<cambc::handle_champion_t>(cambc::vision::champion_at(position)) == static_cast<cambc::handle_champion_t>(champion));
        assert(champion.is_visible());
        cambc::detail::champion_design design{};
        cambc::detail::check(cambc::detail::sdk_champion_design(static_cast<std::int32_t>(champion.champion_name()), &design));
        assert(champion.passive() == static_cast<cambc::passive_type>(design.definition.passive_type));
        const cambc::unit& unit = champion;
        assert(unit.attack_range() == champion.base_stats().base_attack_range);
        if (design.definition.attack_type == 0) assert(unit.attack_range() == 1);
        std::uint8_t restrictions{};
        cambc::detail::check(cambc::detail::sdk_champion_restrictions(static_cast<std::uint32_t>(static_cast<cambc::handle_champion_t>(champion)), &restrictions));
        assert(champion.restrictions() == cambc::buff_restrictions::from_bits_retain(restrictions));
    }
    assert(static_cast<cambc::handle_champion_t>(self) == static_cast<cambc::handle_champion_t>(cambc::vision::champion_at(cambc::self().position())));
    const auto camps = cambc::vision::camps();
    assert(camps.size() == cambc::vision::camp_tiles().count());
    for (const auto& camp : camps) {
        const auto position = camp.position();
        assert(camp.attack_tiles().is_set(position));
        assert(cambc::vision::has_camp(position));
        assert(&cambc::vision::camp_at(position) == &cambc::vision::camp_at(position));
        assert(camp.is_visible() && camp.health() > 0 && camp.max_health() >= camp.health());
        cambc::detail::camp_design design{};
        cambc::detail::check(cambc::detail::sdk_camp_design(static_cast<std::int32_t>(camp.kind()), &design));
        const cambc::unit& unit = camp;
        assert(unit.attack_range() == design.base_stats.base_attack_range);
        if (design.definition.attack_type == 0) assert(unit.attack_range() == 1);
        std::array<cambc::detail::camp_buff_data, 3> raw_buffs{};
        std::uint32_t num_buffs{};
        cambc::detail::check(cambc::detail::sdk_camp_buffs(static_cast<std::uint32_t>(static_cast<cambc::handle_camp_t>(camp)),
                                                  raw_buffs.data(), raw_buffs.size(), &num_buffs));
        assert(camp.buffs().size() == num_buffs);
    }
    cambc::bitboard visible;
    cambc::detail::check(vision(visible.ptr()));
    const auto groups = cambc::vision::camp_groups();
    std::size_t expected{};
    for (const auto& group : cambc::map::camp_groups())
        expected += !(group.spawn_tiles() & visible).is_empty();
    assert(groups.size() == expected);
    for (const auto& group : groups) {
        assert(!(group.spawn_tiles() & visible).is_empty());
        (void)group.state();
    }
    for (const auto position : visible.positions()) {
        if (!cambc::vision::has_camp(position)) throws<cambc::error>([&] { (void)cambc::vision::camp_at(position); });
        if (!cambc::vision::has_champion(position)) throws<cambc::error>([&] { (void)cambc::vision::champion_at(position); });
    }
    for (const auto position : {cambc::vec2{-1, 0}, cambc::vec2{cambc::map::width(), 0}}) {
        throws<cambc::error>([&] { (void)cambc::vision::camp_at(position); });
        throws<cambc::error>([&] { (void)cambc::vision::champion_at(position); });
        throws<cambc::error>([&] { (void)cambc::vision::has_camp(position); });
        throws<cambc::error>([&] { (void)cambc::vision::has_champion(position); });
    }
    std::fprintf(stderr, "vision PASS\n");
}

void test_champions() {
    static_assert(cambc::vec2{2, 3}.rotate_cw() == cambc::vec2{3, -2});
    static_assert(cambc::vec2{2, 3}.rotate_ccw() == cambc::vec2{-3, 2});
    static_assert(cambc::vec2{2, 3}.rotate_cw().rotate_ccw() == cambc::vec2{2, 3});
    assert(!cambc::is_draft() && static_cast<cambc::handle_champion_t>(cambc::self()) == static_cast<cambc::handle_champion_t>(my_champion()));
    const auto& self = cambc::self();
    const cambc::handle_champion_t self_handle = self;
    cambc::detail::champion_info raw{};
    cambc::detail::check(cambc::detail::sdk_champion(static_cast<std::uint32_t>(self_handle), &raw));
    assert(self.health() == raw.health && self.max_health() == raw.max_health);
    assert(self.position() == cambc::vec2(raw.x, raw.y));
    assert(self.champion_name() == static_cast<cambc::champion_type>(raw.type));
    assert(self.strength() == raw.strength);
    assert(self.focus() == raw.focus);
    const auto attack_tiles = self.attack_tiles();
    const auto attack_position = self.position();
    const auto attack_range = self.attack_range();
    assert(attack_tiles.is_set(attack_position));
    for (auto y = 0; y < cambc::map::height(); ++y)
        for (auto x = 0; x < cambc::map::width(); ++x)
            assert(attack_tiles.is_set({x, y}) == (cambc::vec2{x, y}.chebyshev_distance(attack_position) <= attack_range));
    assert(self.attack_range() == raw.attack_range);
    assert(self.base_stats().base_health > 0);
    std::uint32_t gold{}, gems{};
    cambc::detail::check(cambc::detail::sdk_champion_gold(static_cast<std::uint32_t>(self_handle), &gold));
    cambc::detail::check(cambc::detail::sdk_champion_gems(static_cast<std::uint32_t>(self_handle), &gems));
    assert(self.gold_count() == gold && self.gem_count() == gems);
    auto items = self.item_count();
    const auto& consumables = self.consumables();
    assert(items.size() == static_cast<std::size_t>(cambc::item::count) && consumables.size() == 2);
    const auto original = items[cambc::item::monster_shard_of_strength];
    items[cambc::item::monster_shard_of_strength] = original + 1;
    assert(self.item_count()[cambc::item::monster_shard_of_strength] == original);
    throws<std::out_of_range>([&] { (void)items[cambc::item::count]; });
    for (std::uint32_t slot = 0; slot < 2; ++slot) {
        const auto abilities = self.abilities();
        assert(abilities[slot].slot() == slot);
        cambc::detail::ability_props props{};
        cambc::detail::check(cambc::detail::sdk_champion_ability_props(static_cast<std::uint32_t>(self_handle), slot, &props));
        assert(abilities[slot].valid_targets().bits() == props.targets);
        assert(abilities[slot].can_target_camps() == ((props.targets & 1) != 0));
        assert(abilities[slot].can_target_enemies() == ((props.targets & 2) != 0));
        assert(abilities[slot].can_target_allies() == ((props.targets & 4) != 0));
        assert(abilities[slot].can_target_self() == ((props.targets & 8) != 0));
        assert(abilities[slot].can_target_ground() == ((props.targets & 16) != 0));
        const auto info = abilities[slot].info();
        assert(info.has_value() == (props.kind != 0));
        assert(abilities[slot].is(static_cast<cambc::ability_kind>(props.kind)) == (props.kind != 0));
        if (!info) continue;
        const auto ability = std::visit([](const auto& value) { return static_cast<const cambc::ability_properties&>(value); }, *info);
        assert(std::visit([](const auto& value) { return value.kind; }, *info) == static_cast<cambc::ability_kind>(props.kind));
        assert(ability.max_cooldown == props.cooldown);
        assert(ability.valid_targets.bits() == props.targets);
        assert(ability.exclusive_with_movement == bool(props.attributes & cambc::detail::ability_attribute_exclusive_with_movement));
        std::int32_t cooldown{};
        cambc::detail::check(cambc::detail::sdk_champion_cooldown(static_cast<std::uint32_t>(self_handle), slot, &cooldown));
        assert(abilities[slot].cooldown() == cooldown);
        assert(abilities[slot].is_on_cooldown() == (cooldown > 0));
        if (abilities[slot].is(cambc::ability_kind::attack_reset))
            throws<std::invalid_argument>([&] { (void)self.abilities()[slot].valid_tiles(); });
        else {
            const auto range = std::visit([](const auto& payload) -> float {
                if constexpr (requires { payload.range; }) return payload.range;
                else return 0;
            }, *info);
            cambc::bitboard range_mask;
            range_mask.clear(); range_mask.draw_chebyshev(self.position(), range);
            assert(self.abilities()[slot].valid_tiles() == range_mask);
        }
    }
    cambc::bitboard expected;
    cambc::detail::check(vision(expected.ptr()));
    assert(self.visible_tiles() == expected);
    assert(cambc::vision::visible_tiles().is_set(self.position()));
    cambc::detail::check(visible_tiles(static_cast<std::int32_t>(self.champion_name()), raw.x, raw.y, expected.ptr()));
        assert(self.visible_tiles_from({raw.x, raw.y}) == expected);
    std::array<std::uint32_t, 16> handles{};
    std::uint32_t count{};
    cambc::detail::check(allies(handles.data(), handles.size(), &count));
    bool seen_other = false;
    for (auto handle : std::span(handles).first(count)) {
        if (handle == static_cast<std::uint32_t>(self_handle)) continue;
        const cambc::champion other = cambc::detail::handle_access::make_champion(static_cast<cambc::handle_champion_t>(handle));
        throws<std::invalid_argument>([&] { (void)other.gold_count(); });
        try {
            const auto p = other.position();
            cambc::detail::check(visible_tiles(static_cast<std::int32_t>(other.champion_name()), p.x, p.y, expected.ptr()));
            assert(other.visible_tiles() == expected);
            assert(other.health() >= 0 && other.max_health() > 0);
            (void)other.base_stats(); (void)other.abilities();
            (void)other.item_count(); (void)other.consumables()[cambc::consumable_type::tp_crystal].count(); (void)other.gem_count();
            seen_other = true;
        } catch (const cambc::error& e) { assert(e.code() == cambc::error_code::api_not_in_vision); }
    }
    assert(seen_other);
    std::fprintf(stderr, "champions PASS\n");
}

void test_actions() {
    const auto& self = cambc::self();
    assert(self.is_visible() && self.is_alive());
    assert(self.state().contains(cambc::champion_state::alive) && !self.state().is_dead());
    const cambc::handle_champion_t self_handle = self;
    assert(self_handle == static_cast<cambc::handle_champion_t>(my_champion()));
    (void)self.consumables()[cambc::consumable_type::tp_crystal].count();
    throws<std::out_of_range>([&] { (void)self.consumables()[cambc::consumable_type::count]; });
    throws<cambc::error>([&] { (void)cambc::detail::handle_access::make_ability(self, 2); });
    const cambc::consumable invalid_consumable = cambc::detail::handle_access::make_consumable(self, cambc::consumable_type::count);
    throws<cambc::error>([&] { invalid_consumable.use(); });
    throws<std::out_of_range>([&] { (void)invalid_consumable.count(); });
    throws<cambc::error>([&] { self.buy(cambc::consumable_type::count); });
    const auto roster = cambc::champions();
    assert(std::find(roster.begin(), roster.end(), self) != roster.end());
    cambc::bitboard visible;
    visible.clear();
    bool hidden = false;
    for (const auto& other : roster) {
        const bool can_inspect = other.is_visible();
        if (can_inspect) visible.set(other.position());
        else if (other.is_alive()) {
            hidden = true;
            throws<cambc::error>([&] { (void)other.health(); });
            throws<cambc::error>([&] { (void)other.state(); });
            throws<cambc::error>([&] { (void)other.restrictions(); });
            throws<cambc::error>([&] { (void)other.attack_range(); });
            throws<cambc::error>([&] { (void)other.abilities()[0].info(); });
            assert(!other.abilities()[0].is(cambc::ability_kind::damage));
            assert(!other.abilities()[0].is(cambc::ability_types::damage));
            throws<cambc::error>([&] { (void)other.consumables()[cambc::consumable_type::tp_crystal].count(); });
            throws<cambc::error>([&] { (void)other.prev_priority(); });
            throws<cambc::error>([&] { (void)other.priority(); });
        }
        if (other == self) continue;
        throws<std::invalid_argument>([&] { other.move({-1, -1}); });
        throws<std::invalid_argument>([&] { other.move_absolute({-1, -1}); });
        throws<std::invalid_argument>([&] { other.set_priority(2); });
        throws<std::invalid_argument>([&] { other.end_turn(); });
        throws<std::invalid_argument>([&] { other.abilities()[0].use(); });
        throws<std::invalid_argument>([&] { other.abilities()[0].use(self); });
        throws<std::invalid_argument>([&] { other.abilities()[0].use(self); });
        throws<std::invalid_argument>([&] { other.consumables()[cambc::consumable_type::tp_crystal].use(); });
        throws<std::invalid_argument>([&] { other.buy(cambc::consumable_type::tp_crystal); });
        throws<std::invalid_argument>([&] { other.buy(cambc::item_type::monster_shard_of_strength); });
        throws<std::invalid_argument>([&] { other.sell(cambc::item_type::monster_shard_of_strength); });
        throws<std::invalid_argument>([&] { other.recall(); });
        throws<std::invalid_argument>([&] { other.cancel_recall(); });
        throws<std::invalid_argument>([&] { other.submit_gems({0, 0}); });
        throws<std::invalid_argument>([&] { other.cancel_submit_gems(); });
        throws<std::invalid_argument>([&] { other.place_ward({0, 0}); });
        throws<std::invalid_argument>([&] { other.attack(self); });
        throws<std::invalid_argument>([&] { (void)other.can_buy(cambc::item_type::monster_shard_of_strength); });
        throws<std::invalid_argument>([&] { (void)other.can_sell(); });
        throws<std::invalid_argument>([&] { (void)other.ward_positions(); });
        throws<std::invalid_argument>([&] { (void)other.ward_cooldown(); });
        throws<std::invalid_argument>([&] { (void)other.ward_placement_tiles(); });
    }
    assert(hidden);
    assert(cambc::vision::champion_tiles() == visible);
    assert(cambc::vision::champion_tiles() == (cambc::vision::champion_tiles(cambc::team::ally) | cambc::vision::champion_tiles(cambc::team::enemy)));
    cambc::bitboard expected;
    cambc::detail::check(cambc::detail::sdk_camp_occupancy(expected.ptr()));
    assert(cambc::vision::camp_tiles() == expected);
    for (const auto side : {cambc::team::ally, cambc::team::enemy}) {
        std::uint32_t gems{};
        cambc::detail::check(cambc::detail::sdk_bc_submitted_gems(cambc::detail::absolute_team(side), &gems));
        assert(cambc::global_submitted_gems(side) == gems);
    }
    throws<std::invalid_argument>([] { (void)cambc::global_submitted_gems(static_cast<cambc::team>(2)); });
    throws<std::invalid_argument>([&] { self.cancel_recall(); });
    throws<std::invalid_argument>([&] { self.cancel_submit_gems(); });
    throws<cambc::error>([&] { self.place_ward({-1, -1}); });
    throws<cambc::error>([&] { self.submit_gems({-1, -1}); });
    throws<cambc::error>([&] { self.consumables()[cambc::consumable_type::tp_crystal].use({-1, -1}); });
    throws<cambc::error>([&] { self.sell(cambc::item_type::monster_shard_of_strength); });
    throws<cambc::error>([&] { self.attack(self); });
    throws<cambc::error>([&] { (void)self.remaining_channel_time(); });
    std::int32_t item_shop_allowed{}, can_sell{};
    cambc::detail::check(cambc::detail::sdk_champion_can_shop(static_cast<std::uint32_t>(self_handle), 0, &item_shop_allowed));
    cambc::detail::check(cambc::detail::sdk_champion_can_sell(static_cast<std::uint32_t>(self_handle), &can_sell));
    assert(item_shop_allowed != 0 || !self.can_buy(cambc::item_type::monster_shard_of_strength));
    (void)self.can_buy(cambc::item_type::monster_shard_of_strength);
    assert(self.can_sell() == (can_sell != 0));
    assert(self.ward_positions().empty());
    const auto ward_position = self.position();
    assert(self.ward_placement_tiles().is_set(ward_position));
    for (const auto tile : self.ward_placement_tiles().positions())
        assert(tile.chebyshev_distance(ward_position) <= cambc::constants::ward_placement_radius);
    self.place_ward(ward_position);
    assert((self.ward_positions() == std::vector{ward_position}));
    assert(cambc::vision::ward_tiles(cambc::team::ally).is_set(ward_position));
    const auto ward = cambc::vision::ward_at(ward_position);
    assert(ward.position() == ward_position && ward.team() == cambc::team::ally);
    if (cambc::constants::ward_lifetime < 0)
        assert(!ward.remaining_time());
    else
        assert(ward.remaining_time() == cambc::constants::ward_lifetime);
    assert(cambc::vision::has_ward(ward_position));
    assert(cambc::vision::ward_tiles(cambc::team::ally).is_set(ward_position));
    const auto wards = cambc::vision::wards();
    assert(!wards.empty());
    for (const auto& value : wards) {
        assert(cambc::vision::ward_tiles(value.team()).is_set(value.position()));
        assert(value.visible_tiles() == cambc::ward::visible_tiles_from(value.position()));
    }
    for (const auto side : {cambc::team::ally, cambc::team::enemy})
        for (const auto& value : cambc::vision::wards(side)) assert(value.team() == side);
    cambc::bitboard hypothetical;
    cambc::detail::check(cambc::detail::sdk_ward_vision(ward_position.x, ward_position.y, hypothetical.ptr()));
    assert(cambc::ward::visible_tiles_from(ward_position) == hypothetical);
    assert(ward.visible_tiles() == hypothetical);
    const auto teleports = cambc::vision::teleports();
    assert(teleports.size() == cambc::vision::teleport_tiles().count());
    for (const auto& tp : teleports) {
        assert(cambc::vision::has_teleport(tp.position()) && tp.remaining_time() > 0);
        assert(tp.visible_tiles().is_set(tp.position()));
    }
    assert(!cambc::vision::has_teleport(ward_position));
    cambc::detail::ward_resources resources{};
    cambc::detail::check(cambc::detail::sdk_champion_wards(static_cast<std::uint32_t>(self_handle), &resources));
    assert(self.ward_cooldown() == resources.cooldown);
    assert((cambc::vision::ward_tiles(cambc::team::enemy) & ~self.visible_tiles()).is_empty());
    for (const auto& lock : cambc::vision::altar_locks()) {
        assert(cambc::vision::has_altar_lock(lock.position()));
        assert(cambc::vision::altar_lock_at(lock.position()).position() == lock.position());
    }
    assert(cambc::vision::altar_lock_tiles() == (cambc::vision::altar_lock_tiles(cambc::team::ally) | cambc::vision::altar_lock_tiles(cambc::team::enemy)));
    for (const auto side : {cambc::team::ally, cambc::team::enemy}) {
        const auto locks = cambc::vision::altar_locks(side);
        assert(locks.size() == cambc::vision::altar_lock_tiles(side).count());
        for (const auto& lock : locks) assert(lock.team() == side);
    }
    cambc::debug::draw_dot(ward_position, 0xff0000);
    cambc::debug::draw_line(ward_position, ward_position + cambc::vec2{1, 1}, 0x00ff00);
    self.recall();
    assert(self.state().contains(cambc::champion_state::recalling));
    assert(self.remaining_channel_time() == cambc::constants::recall_time);
    throws<std::invalid_argument>([&] { self.cancel_submit_gems(); });
    self.cancel_recall();
    assert(!self.state().contains(cambc::champion_state::recalling) && self.state().contains(cambc::champion_state::recall_cancelled));
    const auto pos = self.position();
    bool moved = false;
    for (int dx = -1; dx <= 1 && !moved; ++dx) {
        for (int dy = -1; dy <= 1 && !moved; ++dy) {
            const auto destination = pos + cambc::vec2{dx, dy};
            if ((!dx && !dy) || !cambc::map::is_in_bounds(destination) ||
                cambc::map::walls().is_set(destination) || cambc::map::holes().is_set(destination) ||
                visible.is_set(destination)) continue;
            if (static_cast<std::uint32_t>(self_handle) % 2)
                self.move({dx, dy});
            else
                self.move_absolute(destination);
            assert(self.position() == destination && !self.state().contains(cambc::champion_state::can_move));
            moved = true;
        }
    }
    assert(moved);
    assert(self.prev_priority() == 0 && self.priority() == 0);
    self.set_priority(-123);
    assert(self.prev_priority() == 0 && self.priority() == -123);
    const auto round = cambc::round_number();
    self.end_turn();
    assert(cambc::round_number() == round + 1);
    assert(self.prev_priority() == -123 && self.priority() == -123);
    for (const auto& ability : self.abilities()) {
        if (ability.is(cambc::ability_types::aoe_damage)) {
            ability.use(self.position());
            assert(ability.is_on_cooldown());
            std::fprintf(stderr, "casting PASS\n");
            break;
        }
    }
    std::fprintf(stderr, "actions PASS\n");
}

int run_tests() {
    std::fprintf(stderr, "SDK tests starting\n");
    assert(cambc::champion_designs.size() == 13 && cambc::draft::team_size() > 0);
    for (std::size_t i = 0; i < cambc::champion_designs.size(); ++i) {
        const auto& design = cambc::champion_designs[i];
        assert(design.champion_name() == static_cast<cambc::champion_type>(i) && !design.is_visible());
        assert(design.base_stats().base_health > 0);
        cambc::detail::champion_design raw{};
        cambc::detail::check(cambc::detail::sdk_champion_design(static_cast<std::int32_t>(i), &raw));
        assert(design.passive() == static_cast<cambc::passive_type>(raw.definition.passive_type));
        (void)design.abilities()[0].info();
        throws<cambc::error>([&] { (void)design.strength(); });
        throws<cambc::error>([&] { (void)design.attack_range(); });
        throws<cambc::error>([&] { (void)design.restrictions(); });
        throws<cambc::error>([&] { (void)design.position(); });
        throws<cambc::error>([&] { (void)design.buffs(); });
    }
    test_primitives();
    test_map();
    for (std::uint32_t i = 0; i < static_cast<std::uint32_t>(cambc::item_type::count); ++i) {
        cambc::detail::item_info raw{};
        cambc::detail::check(cambc::detail::sdk_item_info(i, &raw));
        const auto item = static_cast<cambc::item_type>(i).info();
        assert(item.cost == raw.cost && item.combine_count == raw.combine_count);
        assert(item.purchasable == (raw.purchasable != 0) && item.combine_count == raw.combine_count);
        assert(item.health_points == raw.health_points && item.strength_points == raw.strength_points && item.focus_points == raw.focus_points);
        assert(item.passive_magnitude == raw.passive_magnitude);
        assert(item.result.has_value() == (raw.combine_count != 0));
        if (item.result) assert(static_cast<std::uint32_t>(*item.result) == raw.result);
        assert(item.passive.has_value() == (raw.passive != 0));
        if (item.passive) assert(static_cast<std::uint32_t>(*item.passive) == raw.passive);
    }
    for (std::uint32_t i = 0; i < static_cast<std::uint32_t>(cambc::consumable_type::count); ++i) {
        const auto item = static_cast<cambc::consumable_type>(i).info();
        assert(item.cost > 0 && item.carry_limit > 0);
    }
    throws<cambc::error>([] { (void)cambc::item_type::count.info(); });
    throws<cambc::error>([] { (void)cambc::consumable_type::count.info(); });
    std::fprintf(stderr, "shop metadata PASS\n");
    const auto shared_size = cambc::shared_memory.size();
    assert(shared_size >= 4);
    const std::array<std::uint8_t, 4> bytes{1, 2, 0, 255};
    cambc::shared_memory.write(shared_size - bytes.size(), std::span{bytes});
    for (std::size_t i = 0; i < bytes.size(); ++i)
        assert(cambc::shared_memory[shared_size - bytes.size() + i] == bytes[i]);
    cambc::shared_memory[shared_size - 1] = std::byte{42};
    cambc::shared_memory[shared_size - 2] = cambc::shared_memory[shared_size - 1];
    assert(cambc::shared_memory[shared_size - 2] == 42);
    const auto before = std::uint8_t(cambc::shared_memory[shared_size - 1]);
    std::array<std::uint8_t, 2> restored{};
    cambc::shared_memory.read(shared_size - restored.size(), std::span{restored});
    assert(restored[0] == 42 && restored[1] == 42);
    throws<std::out_of_range>([&] { (void)cambc::shared_memory[shared_size]; });
    throws<std::out_of_range>([] { (void)cambc::shared_memory[std::size_t(-1)]; });
    throws<std::out_of_range>([&] { cambc::shared_memory.write(shared_size - 1, std::span{bytes}); });
    throws<std::out_of_range>([&] { cambc::shared_memory.write(std::size_t(-1), std::span{bytes}); });
    cambc::shared_memory.write(shared_size, std::span<const std::byte>{});
    throws<std::out_of_range>([&] { cambc::shared_memory.write(shared_size + 1, std::span<const std::byte>{}); });
    throws<std::out_of_range>([&] { cambc::shared_memory.read(shared_size - 1, std::span{restored}); });
    assert(cambc::shared_memory[shared_size - 1] == before);
    std::fprintf(stderr, "shared memory PASS\n");
    if (my_champion() == -1) {
        assert(cambc::is_draft());
        throws<std::bad_optional_access>([] { (void)cambc::self(); });
        for (;;) {
        auto before = cambc::draft::current_draft(cambc::team::ally);
        const auto original = std::vector(before.begin(), before.end());
        if (!before.empty()) before[0] = cambc::champion_draft::willow;
        const auto refreshed = cambc::draft::current_draft(cambc::team::ally);
        assert(std::equal(original.begin(), original.end(), refreshed.begin(), refreshed.end()));
        (void)cambc::draft::current_draft(cambc::team::enemy);
        if (!before.empty()) assert(before[0] == cambc::champion_draft::willow);

        std::uint32_t required{};
        cambc::detail::check(memory_size(&required));
        std::vector<std::uint8_t> bytes(required + 17, 0xa5), restored(required);
        const std::vector<std::byte> snapshot(cambc::draft_memory.begin(), cambc::draft_memory.end());
        const std::array<std::uint8_t, 4> short_memory{1, 2, 3, 4};
        cambc::draft_memory = short_memory;
        assert(cambc::draft_memory.size() == required);
        assert(std::equal(snapshot.begin(), snapshot.end(), cambc::draft_memory.begin(), cambc::draft_memory.end()));
        cambc::draft_memory = bytes;
        assert(cambc::draft_memory.size() == required);
        assert(std::equal(snapshot.begin(), snapshot.end(), cambc::draft_memory.begin(), cambc::draft_memory.end()));
        cambc::detail::check(read_memory(0, restored.data(), required));
        assert(std::all_of(restored.begin(), restored.end(), [](auto v) { return v == 0xa5; }));
        // A declared tail extending past memory is irrelevant after host truncation.
        cambc::detail::check(cambc::detail::sdk_bc_submit_draft_memory(bytes.data(), 0x7fffffff));
        cambc::draft_memory = {};
        assert(std::equal(snapshot.begin(), snapshot.end(), cambc::draft_memory.begin(), cambc::draft_memory.end()));
        std::uint8_t round{};
        cambc::detail::check(current_round(&round));
        constexpr std::array picks{cambc::champion_draft::ryth, cambc::champion_draft::cassi_and_issac,
            cambc::champion_draft::tavi, cambc::champion_draft::vesper_vale, cambc::champion_draft::vendetta,
            cambc::champion_draft::stellar, cambc::champion_draft::prism, cambc::champion_draft::marcella,
            cambc::champion_draft::willow, cambc::champion_draft::ren,
            cambc::champion_draft::varg};
        cambc::draft::pick(picks.at(round));
        std::fprintf(stderr, "draft PASS\n");
        cambc::end_turn();
        }
    } else {
        test_champions();
        test_vision();
        std::uint32_t required{};
        cambc::detail::check(memory_size(&required));
        std::vector<std::uint8_t> restored(required);
        cambc::detail::check(read_memory(0, restored.data(), required));
        assert(std::all_of(restored.begin(), restored.end(), [](auto v) { return v == 0xa5; }));
        std::fprintf(stderr, "gameplay draft-memory PASS\n");
        assert(cambc::draft_memory.size() == restored.size());
        assert(std::all_of(cambc::draft_memory.begin(), cambc::draft_memory.end(), [](auto v) { return v == std::byte{0xa5}; }));
        bool draft_memory_rejected = false;
        try {
            cambc::draft_memory = {std::byte{0}};
        } catch (const cambc::error& e) {
            draft_memory_rejected = e.code() == cambc::error_code::invalid_action;
        }
        assert(draft_memory_rejected);
        assert(std::all_of(cambc::draft_memory.begin(), cambc::draft_memory.end(), [](auto v) { return v == std::byte{0xa5}; }));
        test_actions();
    }
    for (;;) cambc::self().end_turn();
}

int main() {
#ifdef BC_TEST_RESIGN
    cambc::resign();
    std::fprintf(stderr, "resign PASS\n");
    cambc::end_turn();
    std::abort();
#endif
    try { return run_tests(); }
    catch (const std::exception& e) {
        std::fprintf(stderr, "SDK test exception: %s\n", e.what());
        std::abort();
    }
}
