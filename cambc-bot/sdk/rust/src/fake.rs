use crate::sys;
use std::cell::RefCell;
pub type Hook = Box<dyn FnMut(&str, &[usize]) -> Option<i64>>;
#[derive(Default)]
pub struct State {
    pub draft: bool,
    pub hidden: bool,
    pub health: f32,
    pub cooldown: i32,
    pub buffs: Vec<sys::BuffData>,
    pub wards: Vec<sys::WardInfo>,
    pub locks: Vec<sys::AltarLockInfo>,
    pub teleports: Vec<sys::TeleportInfo>,
    pub memory: [u8; 4],
    pub group_state: u32,
    pub hook: Option<Hook>,
    pub calls: Vec<String>,
}
thread_local! {pub static HOST:RefCell<State>=RefCell::new(State::default());}
pub fn reset() {
    crate::world::reset_for_tests();
    HOST.with(|h| {
        *h.borrow_mut() = State {
            health: 100.,
            group_state: 1,
            ..State::default()
        }
    });
}
pub fn edit(f: impl FnOnce(&mut State)) {
    HOST.with(|h| f(&mut h.borrow_mut()));
}
pub fn hook(f: impl FnMut(&str, &[usize]) -> Option<i64> + 'static) {
    edit(|s| s.hook = Some(Box::new(f)));
}
unsafe fn put<T>(p: usize, v: T) {
    unsafe { (p as *mut T).write(v) }
}
unsafe fn write_list<T: Copy>(args: &[usize], values: &[T]) -> i64 {
    let [p, n, out] = args else {
        panic!("bad list arguments");
    };
    unsafe { put(*out, values.len() as u32) };
    if *n == 0 {
        return 0;
    }
    if *n < values.len() {
        return 111;
    }
    unsafe { std::ptr::copy_nonoverlapping(values.as_ptr(), *p as *mut T, values.len()) };
    0
}
unsafe fn write_truncated<T: Copy>(args: &[usize], values: &[T]) -> i64 {
    let [p, n, out] = args else {
        panic!("bad list arguments")
    };
    let length = (*n).min(values.len());
    unsafe {
        put(*out, length as u32);
        if length != 0 {
            std::ptr::copy_nonoverlapping(values.as_ptr(), *p as *mut T, length);
        }
    }
    0
}
pub fn props(kind: u32) -> sys::AbilityProps {
    sys::AbilityProps {
        kind,
        cooldown: 10,
        targets: 31,
        attributes: 0,
        payload: [3.0f32.to_bits(), 0.5f32.to_bits(), 5, 0],
        ..Default::default()
    }
}
pub fn call(name: &str, a: &[usize]) -> i64 {
    HOST.with(|host| {
        let mut s = host.borrow_mut();
        s.calls.push(name.to_owned());
        if let Some(hook) = s.hook.as_mut() {
            if let Some(code) = hook(name, a) {
                return code;
            }
        }
        unsafe {
            match name {
                "game_settings" => {
                    put(
                        a[0],
                        crate::Constants {
                            capture_time: 5,
                            camp_sleep_time: 2,
                            respawn_time: 60,
                            recall_time: 5,
                            submit_gem_duration_multiplier: 0.5,
                            submit_gem_base_duration: 0,
                            submit_gem_minimum_duration: 1,
                            submit_gems_range: 1,
                            max_rounds: 2000,
                            winning_gems: 1000,
                            kill_gold: 50,
                            assist_gold: 10,
                            passive_gold: 0,
                            spawn_healing: 1.0,
                            active_wards: 1,
                            ward_charges: 1,
                            shared_memory_bytes: 4,
                            draft_memory_bytes: 1024,
                            tile_reveal_rounds: 1,
                            ward_cooldown: 4,
                            ward_lifetime: -1,
                            ward_vision_radius: 6.0,
                            ward_placement_radius: 2.0,
                            control_ward_vision_radius: 6.0,
                            control_ward_placement_radius: 2.0,
                            altar_lock_hits: 3,
                            tp_scroll_time: 3,
                            tp_crystal_time: 0,
                            reserved: 0,
                        },
                    );
                    0
                }
                "consumable_info" => {
                    put(
                        a[1],
                        crate::shop::ConsumableInfo {
                            cost: 50,
                            carry_limit: 1,
                        },
                    );
                    0
                }
                "item_info" => {
                    put(
                        a[1],
                        sys::GameItemInfo {
                            cost: 33,
                            purchasable: 1,
                            strength_points: 1,
                            combine_count: 5,
                            result: a[0] as u32 + 1,
                            combine_cost: 165,
                            ..Default::default()
                        },
                    );
                    0
                }
                "bc_metadata" => {
                    put(
                        a[0],
                        sys::Metadata {
                            width: 8,
                            height: 8,
                            chunked_width: 1,
                            chunked_height: 1,
                            chunk_stride: 2,
                            board_bytes: 16,
                            field_bytes: 128,
                        },
                    );
                    0
                }
                "my_team" => 0,
                "my_champion" => {
                    if s.draft {
                        -1
                    } else {
                        0
                    }
                }
                "bc_get_last_error" => {
                    let msg = b"fake diagnostic";
                    if a[1] > 0 {
                        std::ptr::copy_nonoverlapping(
                            msg.as_ptr(),
                            a[0] as *mut u8,
                            a[1].min(msg.len()),
                        );
                    }
                    msg.len() as i64
                }
                "champion_ability_props" => {
                    if s.hidden && a[0] == 2 {
                        return 97;
                    }
                    put(a[2], props(if a[1] == 0 { 2 } else { 7 }));
                    0
                }
                "champion_design" => {
                    if a[0] > 12 {
                        return 108;
                    }
                    put(
                        a[1],
                        sys::ChampionDesign {
                            definition: sys::UnitDefinition {
                                kind: a[0] as i32,
                                abilities: [props(2), props(7)],
                                ..Default::default()
                            },
                            base_stats: sys::ChampionStats {
                                base_attack_range: 1,
                                ..Default::default()
                            },
                        },
                    );
                    0
                }
                "camp_design" => {
                    put(
                        a[1],
                        sys::CampDesign {
                            base_stats: sys::CampStats {
                                base_attack_range: 1,
                                ..Default::default()
                            },
                            ..Default::default()
                        },
                    );
                    0
                }
                "champion" => {
                    if s.hidden && a[0] == 2 {
                        return 97;
                    }
                    put(
                        a[1],
                        sys::ChampionInfo {
                            team: if a[0] == 2 { 1 } else { 0 },
                            kind: 0,
                            x: 2,
                            y: 2,
                            health: s.health,
                            max_health: 100.,
                            strength: 20.,
                            focus: 40.,
                            move_speed: 3,
                            attack_range: 1,
                            state: 1 | 8 | 16 | 2048,
                        },
                    );
                    0
                }
                "champion_is_alive" => {
                    put(a[1], 1i32);
                    0
                }
                "champion_cooldown" => {
                    put(a[2], s.cooldown);
                    0
                }
                "champion_ability_damage" | "champion_ability_heal" | "champion_ability_shield" => {
                    put(a[2], 40f32);
                    0
                }
                "champion_ability_damage_against"
                | "champion_ability_heal_against"
                | "champion_ability_shield_against" => {
                    put(a[3], 35f32);
                    0
                }
                "champion_buff_data" => write_list(&a[1..], &s.buffs),
                "camp_buffs" => write_truncated(
                    &a[1..],
                    &[sys::CampBuffData {
                        kind: 1,
                        magnitude: 0.,
                        timer: 2,
                    }],
                ),
                "allies" => write_truncated(a, &[0u32, 1]),
                "enemies" => write_truncated(a, &[2u32]),
                "num_allies" | "num_enemies" => {
                    put(a[0], if name == "num_allies" { 2u32 } else { 1 });
                    0
                }
                "champion_restrictions" => {
                    put(a[1], 1u8);
                    0
                }
                "champion_wards" => {
                    put(
                        a[1],
                        sys::WardResources {
                            charges: 1,
                            cooldown: 0,
                        },
                    );
                    0
                }
                "bc_champion_ward_positions" => write_list(&a[1..], &[crate::Vec2::new(2, 3)]),
                "bc_champion_channel_time"
                | "bc_champion_priority"
                | "champion_can_shop"
                | "champion_can_sell" => {
                    put(*a.last().unwrap(), 3i32);
                    0
                }
                "champion_items" | "champion_consumables" => {
                    let n = if name == "champion_items" { 42 } else { 4 };
                    let p = a[1] as *mut u32;
                    for i in 0..n {
                        p.add(i).write(i as u32);
                    }
                    0
                }
                "champion_gems" | "champion_gold" => {
                    put(a[1], 10u32);
                    0
                }
                "wards" => write_list(a, &s.wards),
                "teleports" => write_list(a, &s.teleports),
                "altar_locks" => write_list(a, &s.locks),
                "ward_at" | "ally_ward_at" | "enemy_ward_at" | "camp_at" | "champion_at" => {
                    if a[0] >= 8 || a[1] >= 8 {
                        return 96;
                    }
                    if a[0] == 7 {
                        return 97;
                    }
                    if a[0] == 0 {
                        return 98;
                    }
                    put(a[2], if name == "camp_at" { 10u32 } else { 0 });
                    0
                }
                "ward" => {
                    put(
                        a[1],
                        sys::WardInfo {
                            team: 0,
                            x: 2,
                            y: 2,
                            timer: -1,
                            disabled: 1,
                        },
                    );
                    0
                }
                "altar_lock" => {
                    if a[0] >= 8 || a[1] >= 8 {
                        return 96;
                    }
                    if a[0] == 0 {
                        return 98;
                    }
                    if a[0] == 7 {
                        return 97;
                    }
                    if a[0] == 1 && a[1] == 2 && a[2] == 0 {
                        return 98;
                    }
                    put(
                        a[3],
                        sys::AltarLockInfo {
                            kind: 6,
                            handle: 4,
                            team: a[2] as u32,
                            x: a[0] as i32,
                            y: a[1] as i32,
                            hits_remaining: 3,
                        },
                    );
                    0
                }
                "is_occupied_by_teleport" => {
                    if a[0] >= 8 || a[1] >= 8 {
                        return 96;
                    }
                    put(a[2], i32::from(a[0] == 2));
                    0
                }
                "has_camp" | "has_champion" => {
                    if a[0] >= 8 || a[1] >= 8 {
                        return 96;
                    }
                    if a[0] == 7 {
                        return 97;
                    }
                    put(a[2], i32::from(a[0] == 2));
                    0
                }
                "camp" => {
                    put(
                        a[1],
                        sys::CampState {
                            x: 3,
                            y: 3,
                            health: s.health,
                            max_health: 100.,
                            strength: 10.,
                            ..Default::default()
                        },
                    );
                    0
                }
                "map_num_camp_groups" => {
                    put(a[0], 1u32);
                    0
                }
                "map_camp_groups" => write_truncated(a, &[5u32]),
                "map_camp_group" => {
                    put(a[1], sys::CampGroupInfo { gems: 4, gold: 10 });
                    0
                }
                "bc_camp_group_max_respawn" | "camp_group_cooldown" => {
                    put(a[1], 30i32);
                    0
                }
                "map_camp_group_num_camps" => {
                    put(a[1], 2u32);
                    0
                }
                "map_camp_group_camps" => write_truncated(&a[1..], &[10u32, 11]),
                "map_camp_info" => {
                    put(
                        a[1],
                        sys::CampInfo {
                            x: if a[0] == 10 { 3 } else { 4 },
                            y: 3,
                            kind: 2,
                        },
                    );
                    0
                }
                "map_camp_layout" => write_truncated(a, &[5u32, 10, 4, 30, 2, 10, 2, 3, 3, 11, 2, 4, 3]),
                "map_camp_group_of" => {
                    put(a[1], 5u32);
                    0
                }
                "map_camp_at" => match (a[0], a[1]) {
                    (3, 3) | (4, 3) => {
                        put(a[2], if a[0] == 3 { 10u32 } else { 11 });
                        0
                    }
                    _ => 98,
                },
                "camp_group_state" => {
                    put(a[1], s.group_state);
                    0
                }
                "draft_num_rounds" => {
                    put(a[0], 3u8);
                    0
                }
                "draft_get" => {
                    if a[1] == 2 {
                        return 16;
                    }
                    put(a[2], a[1] as i32);
                    0
                }
                "draft_memory_size" => {
                    put(a[0], 4u32);
                    0
                }
                "read_draft_memory" => {
                    std::ptr::copy_nonoverlapping(b"rust".as_ptr(), a[1] as *mut u8, a[2]);
                    0
                }
                "shared_memory_size" => {
                    put(a[0], 4u32);
                    0
                }
                "read_shared_memory" => {
                    std::ptr::copy_nonoverlapping(s.memory[a[0]..].as_ptr(), a[1] as *mut u8, a[2]);
                    0
                }
                "write_shared_memory" => {
                    std::ptr::copy_nonoverlapping(
                        a[1] as *const u8,
                        s.memory[a[0]..].as_mut_ptr(),
                        a[2],
                    );
                    0
                }
                "bc_round_number" => {
                    put(a[0], 9u32);
                    0
                }
                "bc_submitted_gems" => {
                    put(a[1], 20u32);
                    0
                }
                "bc_map_symmetry" => {
                    put(a[0], 0i32);
                    0
                }
                "spawn_region" => {
                    put(
                        a[1],
                        sys::Aabb {
                            min_x: 0,
                            min_y: if a[0] == 0 { 0 } else { 6 },
                            max_x: 2,
                            max_y: if a[0] == 0 { 2 } else { 8 },
                        },
                    );
                    0
                }
                "bc_map_mask" => {
                    put(a[1], 0u64);
                    put(a[1] + 8, 0u64);
                    0
                }
                "board_vision"
                | "bc_champion_vision"
                | "board_vision_mask"
                | "board_ward_vision_mask" => {
                    put(*a.last().unwrap(), u64::MAX);
                    put(*a.last().unwrap() + 8, 0u64);
                    0
                }
                "board_ally_occupancy"
                | "board_enemy_occupancy"
                | "board_camp_occupancy"
                | "board_ally_ward_occupancy"
                | "board_enemy_ward_occupancy"
                | "board_teleport_occupancy"
                | "board_ally_altar_lock_occupancy"
                | "board_enemy_altar_lock_occupancy" => {
                    put(a[0], 1u64 << 18);
                    put(a[0] + 8, 0u64);
                    0
                }
                "fuel_remaining" => 10000,
                "cast" | "use_consumable" | "move_to" | "place_ward" => {
                    put(*a.last().unwrap(), 1i32);
                    0
                },
                "attack"
                | "buy_consumable"
                | "buy_item"
                | "sell"
                | "recall"
                | "cancel_channel"
                | "submit_gems"
                | "move"
                | "move_absolute"
                | "set_priority"
                | "draft_champion"
                | "bc_submit_draft_memory"
                | "debug_dot"
                | "debug_line"
                | "resign"
                | "yield" => 0,
                "bc_Bitboard_One" => {
                    put(a[0], u64::MAX);
                    put(a[0] + 8, 0u64);
                    0
                }
                "bc_Bitboard_Test" | "bc_Bitboard_Set" | "bc_Bitboard_Toggle" => {
                    let x = a[1] as i32;
                    let y = a[2] as i32;
                    if !(0..8).contains(&x) || !(0..8).contains(&y) {
                        return 88;
                    }
                    let bit = 1u64 << (y * 8 + x);
                    let p = a[0] as *mut u64;
                    match name {
                        "bc_Bitboard_Test" => put(a[3], i32::from(*p & bit != 0)),
                        "bc_Bitboard_Set" => *p = if a[3] != 0 { *p | bit } else { *p & !bit },
                        _ => *p ^= bit,
                    }
                    0
                }
                "bc_Bitboard_Zero" => {
                    put(a[0], 0u64);
                    put(a[0] + 8, 0u64);
                    0
                }
                "bc_Bitboard_IsZero" => {
                    put(a[1], i32::from(*(a[0] as *const u64) == 0));
                    0
                }
                "bc_Bitboard_NumSetBits" => {
                    put(a[1], (*(a[0] as *const u64)).count_ones());
                    0
                }
                "bc_Bitboard_GetSetBits" => {
                    let value = *(a[0] as *const u64);
                    let mut n = 0;
                    for bit in 0..64 {
                        if value & (1u64 << bit) != 0 {
                            (a[1] as *mut crate::Vec2)
                                .add(n)
                                .write(crate::Vec2::new(bit % 8, bit / 8));
                            n += 1;
                        }
                    }
                    put(a[3], n as u32);
                    0
                }
                "bc_Bitboard_Union" | "bc_Bitboard_Intersect" | "bc_Bitboard_Xor" => {
                    let x = *(a[0] as *const u64);
                    let y = *(a[1] as *const u64);
                    put(
                        a[2],
                        match name {
                            "bc_Bitboard_Union" => x | y,
                            "bc_Bitboard_Intersect" => x & y,
                            _ => x ^ y,
                        },
                    );
                    put(a[2] + 8, 0u64);
                    0
                }
                "bc_Bitboard_Not" => {
                    put(a[1], !*(a[0] as *const u64));
                    put(a[1] + 8, 0u64);
                    0
                }
                "bc_Bitboard_IsEqual" => {
                    put(
                        a[2],
                        i32::from(*(a[0] as *const u64) == *(a[1] as *const u64)),
                    );
                    0
                }
                "bc_Bitboard_ExportRawBuffer"
                | "bc_Bitboard_ImportRawBuffer"
                | "bc_DistanceField_ExportRawBuffer"
                | "bc_DistanceField_ImportRawBuffer" => {
                    let required = if name.contains("DistanceField") {
                        128
                    } else {
                        16
                    };
                    if a[2] != required {
                        return 5;
                    }
                    let (src, dst) = if name.contains("Import") {
                        (a[1], a[0])
                    } else {
                        (a[0], a[1])
                    };
                    std::ptr::copy_nonoverlapping(src as *const u8, dst as *mut u8, a[2]);
                    0
                }
                "bc_DistanceField_Get" | "bc_DistanceField_Set" => {
                    if a[1] >= 8 || a[2] >= 8 {
                        return 88;
                    }
                    let p = (a[0] as *mut u16).add(a[2] * 8 + a[1]);
                    if name.ends_with("Get") {
                        put(a[3], *p)
                    } else {
                        *p = a[3] as u16;
                    }
                    0
                }
                _ => panic!("unhandled fake host import {name}"),
            }
        }
    })
}
