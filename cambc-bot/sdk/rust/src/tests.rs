use crate::*;
use crate::{fake, sys};
fn setup() {
    fake::reset();
}
#[test]
fn abi_layouts_match_cpp() {
    assert_eq!(std::mem::size_of::<Vec2>(), 8);
    assert_eq!(std::mem::size_of::<Constants>(), 120);
    assert_eq!(std::mem::align_of::<Constants>(), 8);
    assert_eq!(
        std::mem::offset_of!(Constants, submit_gem_duration_multiplier),
        16
    );
    assert_eq!(std::mem::offset_of!(Constants, tp_scroll_time), 108);
    assert_eq!(std::mem::offset_of!(Constants, tp_crystal_time), 112);
    assert_eq!(std::mem::size_of::<sys::GameItemInfo>(), 40);
    assert_eq!(
        std::mem::offset_of!(sys::GameItemInfo, passive_magnitude),
        24
    );
    assert_eq!(std::mem::offset_of!(sys::GameItemInfo, combine_cost), 36);
    assert_eq!(std::mem::size_of::<ConsumableInfo>(), 8);
    assert_eq!(std::mem::align_of::<ConsumableInfo>(), 4);
    assert_eq!(std::mem::offset_of!(ConsumableInfo, carry_limit), 4);
    assert_eq!(std::mem::size_of::<sys::Metadata>(), 28);
    assert_eq!(std::mem::size_of::<sys::AbilityProps>(), 36);
    assert_eq!(std::mem::size_of::<sys::UnitDefinition>(), 88);
    assert_eq!(std::mem::size_of::<sys::ChampionDesign>(), 128);
    assert_eq!(std::mem::size_of::<sys::CampDesign>(), 124);
    assert_eq!(std::mem::size_of::<sys::ChampionInfo>(), 44);
    assert_eq!(std::mem::size_of::<sys::BuffData>(), 12);
    assert_eq!(std::mem::size_of::<sys::CampBuffData>(), 12);
    assert_eq!(std::mem::size_of::<sys::WardInfo>(), 20);
    assert_eq!(std::mem::size_of::<sys::ControlWardInfo>(), 16);
    assert_eq!(std::mem::size_of::<sys::AltarLockInfo>(), 24);
    assert_eq!(std::mem::size_of::<sys::TeleportInfo>(), 28);
    assert_eq!(std::mem::offset_of!(sys::AbilityProps, payload), 20);
    assert_eq!(std::mem::offset_of!(sys::UnitDefinition, abilities), 12);
    assert_eq!(std::mem::offset_of!(sys::UnitDefinition, attack_type), 84);
    assert_eq!(std::mem::offset_of!(sys::BuffData, value), 8);
}

#[test]
fn ward_restriction_preserves_engine_bit() {
    assert_eq!(BuffRestrictions::WARD.bits(), 1 << 5);
    let restrictions = BuffRestrictions::WARD | BuffRestrictions::MOVE;
    assert!(restrictions.contains(BuffRestrictions::WARD));
    assert!(!restrictions.contains(BuffRestrictions::CONSUMABLE));
}

#[test]
fn logical_masks_preserve_named_bits_combinations_and_unknown_bits() {
    macro_rules! check_flags {
        ($mask:ty, $bits:ty, [$($flag:ident),+]) => {{
            let flags = [$(<$mask>::$flag),+];
            let empty = <$mask>::empty();
            let unknown: $bits = 1 << (<$bits>::BITS - 1);
            let future = <$mask>::from_bits_retain(unknown);
            assert_eq!(empty.bits(), 0);
            assert!(empty.is_empty() && empty.contains(empty));
            assert_eq!(future.bits(), unknown);
            assert_ne!(future, empty);
            let mut combined = future;
            let mut bits = unknown;
            for flag in flags {
                let expected = flag.bits();
                assert_ne!(expected, 0);
                assert_eq!(expected.count_ones(), 1);
                assert_eq!(combined.bits() & expected, 0);
                assert!((!empty).contains(flag) && (!future).contains(flag));
                assert!(flag.contains(flag) && flag.contains(empty));
                assert!(!empty.contains(flag) && !future.contains(flag));
                combined |= flag;
                bits |= expected;
                assert_eq!(combined.bits(), bits);
                assert!(combined.contains(flag) && combined.contains(future));
                assert_eq!(combined & flag, flag);
                assert_eq!(future | flag, flag | future);
                assert_eq!(combined ^ combined, empty);
            }
            assert_eq!(combined, <$mask>::from_bits_retain(bits));
            let mut changed = combined;
            changed ^= future;
            assert_eq!(changed.bits(), bits ^ unknown);
            changed &= future;
            assert_eq!(changed, empty);
        }};
    }
    check_flags!(
        ChampionState,
        u32,
        [
            ALIVE,
            RECALLING,
            SUBMITTING_GEMS,
            CAN_MOVE,
            CAN_ATTACK,
            COMPLETED_TURN,
            RECALL_STARTED,
            SUBMIT_GEMS_STARTED,
            RECALL_CANCELLED,
            SUBMIT_GEMS_CANCELLED,
            TELEPORTING,
            CAN_CAST
        ]
    );
    check_flags!(
        AbilityTargetMask,
        u32,
        [CAMPS, ENEMIES, ALLIES, SELF, GROUND]
    );
    check_flags!(
        BuffRestrictions,
        u8,
        [MOVE, ATTACK, CAST, SHOP, CONSUMABLE, WARD]
    );
    assert!(ChampionState::empty().is_dead());
    assert!(!ChampionState::ALIVE.is_dead());
}

#[test]
fn typed_masks_decode_host_values_without_changing_raw_records() {
    setup();
    let targets = AbilityTargetMask::CAMPS | AbilityTargetMask::SELF;
    fake::hook(move |name, args| match name {
        "champion_ability_props" => {
            unsafe {
                (args[2] as *mut sys::AbilityProps).write(sys::AbilityProps {
                    kind: AbilityKind::Damage as u32,
                    targets: targets.bits() | 0x8000_0000,
                    attributes: 1 | 0x8000_0000,
                    ..Default::default()
                });
            }
            Some(0)
        }
        "champion_restrictions" => {
            unsafe {
                (args[1] as *mut u8).write(BuffRestrictions::WARD.bits() | 0x80);
            }
            Some(0)
        }
        _ => None,
    });
    crate::init().unwrap();
    let champion = crate::self_champion().unwrap().unwrap();
    let ability = &champion.abilities()[0];
    let info = ability.info().unwrap().unwrap().as_dyn();
    assert_eq!(info.valid_targets().bits(), targets.bits() | 0x8000_0000);
    assert_eq!(ability.valid_targets().unwrap(), info.valid_targets());
    assert!(info.is_exclusive_with_movement());
    assert!(ability.can_target_camps().unwrap() && ability.can_target_self().unwrap());
    assert!(!ability.can_target_ground().unwrap());
    let restrictions = champion.restrictions().unwrap();
    assert_eq!(restrictions.bits(), BuffRestrictions::WARD.bits() | 0x80);
    assert!(restrictions.contains(BuffRestrictions::WARD));
    assert_eq!(core::mem::size_of::<ChampionState>(), 4);
    assert_eq!(core::mem::size_of::<AbilityTargetMask>(), 4);
    assert_eq!(core::mem::size_of::<BuffRestrictions>(), 1);
    assert_eq!(core::mem::offset_of!(sys::AbilityProps, targets), 12);
    assert_eq!(core::mem::offset_of!(sys::AbilityProps, attributes), 16);
    assert_eq!(core::mem::offset_of!(sys::ChampionInfo, state), 40);
}
#[test]
fn single_ability_attribute_is_a_boolean() {
    for (attributes, expected) in [
        (0, false),
        (1, true),
        (0x8000_0000, false),
        (0x8000_0001, true),
    ] {
        let ability = Ability::design(
            sys::AbilityProps {
                kind: AbilityKind::Damage as u32,
                attributes,
                ..Default::default()
            },
            0,
        )
        .unwrap();
        let info = ability.info().unwrap().unwrap().as_dyn();
        let value: bool = info.properties().exclusive_with_movement;
        assert_eq!(value, expected);
        assert_eq!(info.is_exclusive_with_movement(), expected);
    }
}

#[test]
fn action_entities_preserve_engine_ids_without_spell() {
    let entities = [
        (Entity(crate::EntityTarget::Champion(ChampionHandle(7))), 1),
        (Entity(crate::EntityTarget::Camp(CampHandle(7))), 2),
        (Entity(crate::EntityTarget::AltarLock(7)), 6),
    ];
    assert_eq!(EntityKind::ALL.len(), entities.len());
    assert!(EntityKind::try_from(3).is_err());
    assert!(EntityKind::try_from(4).is_err());
    assert!(EntityKind::try_from(5).is_err());
    for (entity, id) in entities {
        assert_eq!(entity.kind() as u32, id);
        assert_eq!(EntityKind::try_from(id).unwrap(), entity.kind());
        assert_eq!(entity.handle(), 7);
    }
}

#[test]
fn metadata_rejects_inconsistent_dimensions_and_storage() {
    let mut m = sys::Metadata {
        width: 9,
        height: 9,
        chunked_width: 2,
        chunked_height: 2,
        chunk_stride: 2,
        board_bytes: 32,
        field_bytes: 512,
    };
    crate::board::validate_metadata(&m).unwrap();
    m.field_bytes = 128;
    assert!(crate::board::validate_metadata(&m).is_err());
    m.field_bytes = 512;
    m.width = 0;
    assert!(crate::board::validate_metadata(&m).is_err());
    m.width = 9;
    m.chunk_stride = 3;
    assert!(crate::board::validate_metadata(&m).is_err());
}
#[test]
fn count_only_rosters_are_not_inferred_from_zero_capacity_reads() {
    setup();
    crate::init().unwrap();
    let allies = crate::champions(Some(Team::Ally)).unwrap();
    assert_eq!(allies.len(), 2);
    assert_eq!(crate::champions(None).unwrap().len(), 3);
    let handles = crate::list(|p, n, out| unsafe { sys::allies(p, n, out) }).unwrap();
    assert!(handles.is_empty());
    fake::hook(|name, a| {
        if name == "enemies" {
            unsafe {
                (a[2] as *mut u32).write(2);
            }
            Some(0)
        } else {
            None
        }
    });
    assert!(crate::champions(Some(Team::Enemy)).is_err());
}
#[test]
fn integer_vectors_and_validated_enums() {
    assert_eq!(Vec2::new(3, 4).length_squared(), 25);
    assert_eq!(Vec2::new(3, 4).rotate_cw(), Vec2::new(4, -3));
    assert_eq!(Vec2::NORTH, Vec2::new(0, 1));
    assert_eq!(Vec2::SOUTH_WEST, Vec2::new(-1, -1));
    assert_eq!(Vec2::NORTH.rotate_cw(), Vec2::EAST);
    assert_eq!(Vec2::NORTH_EAST.rotate_cw(), Vec2::SOUTH_EAST);
    assert_eq!(Vec2::NORTH + Vec2::WEST, Vec2::NORTH_WEST);
    assert_eq!(Vec2::new(3, 4).cross(Vec2::new(2, 1)), -5);
    assert_eq!(
        Vec2::new(i32::MIN, 0).chebyshev_distance(Vec2::new(i32::MAX, 0)),
        4294967295
    );
    assert!(Vec2::new(1, 2).checked_div(0).is_err());
    assert!(AbilityKind::try_from(0).is_err());
    assert!(BuffKind::try_from(3).is_err());
    assert_eq!(PassiveType::try_from(0).unwrap(), PassiveType::None);
    assert!(ChampionType::try_from(12).is_err());
}
#[test]
fn local_validation_uses_engine_codes_and_preserves_diagnostics() {
    let division = Vec2::new(1, 2).checked_div(0).unwrap_err();
    assert_eq!(division.code(), Some(ErrorCode::InvalidArgument));
    assert_eq!(division.raw_code(), Some(5));
    assert!(division.to_string().contains("invalid integer division"));

    let kind = AbilityKind::try_from(99).unwrap_err();
    assert_eq!(kind.code(), Some(ErrorCode::ApiValueOutOfRange));
    assert_eq!(kind.raw_code(), Some(110));
    assert!(kind.to_string().contains("AbilityKind"));
    assert!(kind.to_string().contains("99"));

    let oversized = crate::counted_list::<u32>(1, |_, _, out| {
        unsafe { out.write(2) };
        0
    })
    .unwrap_err();
    assert_eq!(oversized.code(), Some(ErrorCode::InvalidArgument));
    assert_eq!(oversized.raw_code(), Some(5));
    assert!(oversized
        .to_string()
        .contains("host returned oversized list"));
}
#[test]
fn dynamic_ability_metadata_is_usable_and_immutable() {
    setup();
    let a = Champion::new(ChampionHandle(0)).unwrap();
    let ability = &a.abilities()[0];
    let kind = AbilityKind::Damage;
    let info: &dyn AbilityMetadata = ability.info().unwrap().unwrap().as_dyn();
    assert_eq!(info.kind(), kind);
    assert_eq!(info.max_cooldown(), 10);
    assert_eq!(info.range().unwrap(), 3.);
    assert_eq!(info.valid_targets().bits(), 31);
    assert!(matches!(
        ability.info().unwrap(),
        Some(AbilityInfo::Damage(_))
    ));
    assert!(!ability.is(AbilityKind::Shield));
    assert!(ability.is_ready().unwrap());
    fake::edit(|s| s.cooldown = 7);
    assert_eq!(ability.cooldown().unwrap(), 7);
    assert!(!ability.is_ready().unwrap());
    assert_eq!(info.max_cooldown(), 10);
    assert!(ability.is(kind));
    assert!(!ability.is(AbilityKind::Heal));
    fake::edit(|s| s.cooldown = 0);
    fake::hook(|name, args| {
        if name != "champion" {
            return None;
        }
        unsafe {
            (args[1] as *mut sys::ChampionInfo).write(sys::ChampionInfo {
                state: ChampionState::CAN_MOVE.bits() as i32,
                ..Default::default()
            });
        }
        Some(0)
    });
    assert!(!ability.is_ready().unwrap());
}

#[test]
fn ability_health_predictions_forward_targets_and_preserve_errors() {
    setup();
    crate::init().unwrap();
    let champion = Champion::new(ChampionHandle(0)).unwrap();
    let ability = &champion.abilities()[0];
    fake::hook(|name, args| match name {
        "champion_ability_damage_against" => {
            assert_eq!(&args[..3], &[0, 0, 7]);
            unsafe { (args[3] as *mut f32).write(17.5) };
            Some(0)
        }
        "champion_ability_heal_against" => {
            assert_eq!(&args[..3], &[0, 0, 8]);
            unsafe { (args[3] as *mut f32).write(8.5) };
            Some(0)
        }
        _ => None,
    });
    assert_eq!(
        ability
            .damage_amount_against(&Champion::new(ChampionHandle(7)).unwrap())
            .unwrap(),
        17.5
    );
    assert_eq!(
        ability
            .heal_amount_against(&Champion::new(ChampionHandle(8)).unwrap())
            .unwrap(),
        8.5
    );

    fake::hook(|name, _| {
        matches!(
            name,
            "champion_ability_damage_against" | "champion_ability_heal_against"
        )
        .then_some(12_345)
    });
    assert_eq!(
        ability
            .damage_amount_against(&Champion::new(ChampionHandle(7)).unwrap())
            .unwrap_err()
            .raw_code(),
        Some(12_345)
    );
    assert_eq!(
        ability
            .heal_amount_against(&Champion::new(ChampionHandle(8)).unwrap())
            .unwrap_err()
            .raw_code(),
        Some(12_345)
    );
}

#[test]
fn all_ability_payloads_decode_without_transmute() {
    setup();
    for kind in AbilityKind::ALL {
        let a = Ability::design(fake::props(*kind as u32), 0).unwrap();
        assert_eq!(a.info().unwrap().unwrap().as_dyn().kind(), *kind);
        assert_eq!(a.is_attack_reset(), a.is(AbilityKind::AttackReset));
        assert_eq!(a.is_damage(), a.is(AbilityKind::Damage));
        assert_eq!(a.is_aoe_damage(), a.is(AbilityKind::AoeDamage));
        assert_eq!(a.is_cc(), a.is(AbilityKind::Cc));
        assert_eq!(a.is_heal(), a.is(AbilityKind::Heal));
        assert_eq!(a.is_dash(), a.is(AbilityKind::Dash));
        assert_eq!(a.is_shield(), a.is(AbilityKind::Shield));
        if *kind == AbilityKind::AttackReset {
            assert_eq!(
                a.info().unwrap().unwrap().as_dyn().range(),
                Err(Error::NoRange)
            );
        }
    }
    let unused = Ability::design(fake::props(0), 0).unwrap();
    assert!(!unused.is(AbilityKind::Damage));
    assert!(unused.info().unwrap().is_none());
    let error = unused.valid_tiles().unwrap_err();
    assert_eq!(
        error.code(),
        Some(ErrorCode::InvalidActionSpellCastNoAbility)
    );
    assert_eq!(error.raw_code(), Some(25));
    assert!(Ability::design(fake::props(99), 0).is_err());
    let raw = sys::AbilityProps {
        payload: [2f32.to_bits(), 3f32.to_bits(), 4f32.to_bits(), 5],
        ..fake::props(3)
    };
    let a = Ability::design(raw, 0).unwrap();
    let Some(AbilityInfo::AoeDamage(p)) = a.info().unwrap() else {
        panic!()
    };
    assert_eq!((p.range, p.radius, p.modifier, p.duration), (2., 3., 4., 5));
}
#[test]
fn hidden_roster_keeps_cached_visibility_error_and_has_never_throws() {
    setup();
    crate::init().unwrap();
    fake::edit(|s| s.hidden = true);
    let enemies = crate::champions(Some(Team::Enemy)).unwrap();
    assert_eq!(enemies.len(), 1);
    assert!(!enemies[0].is_visible().unwrap());
    assert!(!enemies[0].abilities()[0].is(AbilityKind::Damage));
    assert_eq!(
        enemies[0].abilities()[0].info().err().unwrap().code(),
        Some(ErrorCode::ApiNotInVision)
    );
    fake::edit(|s| s.hidden = false);
    assert!(enemies[0].is_visible().unwrap());
    assert!(!enemies[0].abilities()[0].is(AbilityKind::Damage));
}
#[test]
fn design_champions_are_draft_safe_but_not_live() {
    setup();
    fake::edit(|s| s.draft = true);
    crate::init().unwrap();
    assert!(crate::self_champion().unwrap().is_none());
    fake::HOST.with(|host| {
        assert!(!host
            .borrow()
            .calls
            .iter()
            .any(|name| name.starts_with("map_camp")));
    });
    let designs = crate::champion_designs().unwrap();
    assert_eq!(designs.len(), 12);
    for (i, c) in designs.iter().enumerate() {
        assert_eq!(c.champion_name().unwrap() as usize, i);
        assert_eq!(c.base_stats().unwrap().base_attack_range, 1);
        assert_eq!(c.abilities()[0].slot(), 0);
        assert_eq!(c.abilities()[1].slot(), 1);
        assert_eq!(c.passive().unwrap(), PassiveType::None);
        assert!(!c.is_visible().unwrap());
        assert!(c.strength().is_err());
        assert!(c.focus().is_err());
        assert!(c.is_alive().is_err());
        assert!(c.position().is_err());
        assert!(c.attack_tiles().is_err());
        assert!(c.buffs().is_err());
        assert!(c
            .can_buy(ItemType::MonsterShardOfStrength, 1)
            .is_err());
        assert!(c
            .can_buy_consumable(ConsumableType::TpCrystal)
            .is_err());
        assert!(c.visible_tiles().is_err());
        assert!(c.abilities()[0].cooldown().is_err());
        assert!(c.abilities()[0].valid_tiles().is_err());
        assert!(c.abilities()[0].cast_untargeted().is_err());
        assert!(c.consumables()[ConsumableType::TpCrystal].count().is_err());
        assert!(c
            .attack(&Entity(crate::EntityTarget::Camp(CampHandle(10))))
            .is_err());
    }
    assert_eq!(crate::draft::team_size().unwrap(), 3);
    assert_eq!(
        crate::draft::current_draft(Team::Enemy).unwrap(),
        vec![ChampionDraft::Ryth, ChampionDraft::VesperVale]
    );
    assert_eq!(crate::draft_memory().bytes(), b"rust");
    assert!(crate::map::camp_groups().is_ok());
    crate::draft_memory().submit(b"data").unwrap();
    assert_eq!(crate::draft_memory().bytes(), b"rust");
    crate::draft::pick(ChampionDraft::Tavi).unwrap();
}
#[test]
fn draft_memory_transfer_preserves_snapshot_and_host_errors() {
    setup();
    fake::edit(|s| s.draft = true);
    crate::init().unwrap();
    let memory = crate::draft_memory().bytes();
    let mut submissions = 0;
    fake::hook(move |name, args| {
        if name != "bc_submit_draft_memory" {
            return None;
        }
        assert_eq!(args[1], 4);
        let bytes = unsafe { core::slice::from_raw_parts(args[0] as *const u8, args[1]) };
        assert_eq!(bytes, b"data");
        submissions += 1;
        Some(if submissions == 1 { 0 } else { 12_345 })
    });
    crate::draft_memory().submit(b"data").unwrap();
    assert_eq!(
        crate::draft_memory()
            .submit(b"data")
            .unwrap_err()
            .raw_code(),
        Some(12_345)
    );
    fake::edit(|host| host.calls.clear());
    assert_eq!(memory, b"rust");
    assert!(core::ptr::eq(memory, crate::draft_memory().bytes()));
    fake::HOST.with(|host| assert!(host.borrow().calls.is_empty()));
}
#[test]
fn draft_memory_submission_is_rejected_in_gameplay_without_host_call() {
    setup();
    crate::init().unwrap();
    fake::HOST.with(|host| host.borrow_mut().calls.clear());
    assert_eq!(
        crate::draft_memory().submit(b"data").unwrap_err().code(),
        Some(ErrorCode::InvalidAction)
    );
    fake::HOST.with(|host| {
        assert!(!host
            .borrow()
            .calls
            .iter()
            .any(|name| name == "bc_submit_draft_memory"));
    });
    assert_eq!(crate::draft_memory().bytes(), b"rust");
}
#[test]
fn live_unit_queries_are_fresh_and_shared_via_deref() {
    setup();
    crate::init().unwrap();
    let c = Champion::new(ChampionHandle(0)).unwrap();
    assert_eq!(
        c.consumables()
            .iter()
            .map(|consumable| consumable.kind())
            .collect::<Vec<_>>(),
        ConsumableType::ALL
    );
    assert_eq!(c.health().unwrap(), 100.);
    fake::edit(|s| s.health = 80.);
    assert_eq!(c.health().unwrap(), 80.);
    let camp = crate::map::camp_groups().unwrap()[0].camps()[0].clone();
    assert_eq!(camp.health().unwrap(), 80.);
    assert_eq!(camp.attack_range().unwrap(), 1);
    assert_eq!(camp.camp_group().unwrap().handle(), CampGroupHandle(5));
    assert!(camp.buffs().unwrap().has(BuffKind::Stun));
}
#[test]
fn attack_tiles_use_live_positions_and_unit_ranges() {
    setup();
    crate::init().unwrap();
    let champion = Champion::new(ChampionHandle(0)).unwrap();
    let camp = &crate::map::camp_groups().unwrap()[0].camps()[0].clone();
    let draws = std::rc::Rc::new(std::cell::Cell::new(0));
    let calls = draws.clone();
    fake::hook(move |name, args| match name {
        "champion" => {
            unsafe {
                (args[1] as *mut sys::ChampionInfo).write(sys::ChampionInfo {
                    x: 2,
                    y: 2,
                    attack_range: 2,
                    ..Default::default()
                });
            }
            Some(0)
        }
        "bc_Bitboard_DrawChebyshev" => {
            let (position, range) = if calls.get() < 2 {
                (Vec2::new(2, 2), 2.0f32)
            } else {
                (Vec2::new(3, 3), 1.0f32)
            };
            assert_eq!(args[1] as i32, position.x);
            assert_eq!(args[2] as i32, position.y);
            assert_eq!(args[3] as u32, range.to_bits());
            calls.set(calls.get() + 1);
            Some(0)
        }
        _ => None,
    });
    champion.attack_tiles().unwrap();
    champion.attack_tiles().unwrap();
    camp.attack_tiles().unwrap();
    camp.attack_tiles().unwrap();
    assert_eq!(draws.get(), 4);
    fake::hook(|name, args| match name {
        "champion" => {
            unsafe {
                (args[1] as *mut sys::ChampionInfo).write(sys::ChampionInfo {
                    x: 0,
                    y: 0,
                    attack_range: 3,
                    ..Default::default()
                });
            }
            Some(0)
        }
        "bc_Bitboard_DrawChebyshev" => {
            assert_eq!((args[1], args[2]), (0, 0));
            assert_eq!(args[3] as u32, 3.0f32.to_bits());
            Some(0)
        }
        _ => None,
    });
    champion.attack_tiles().unwrap();
    fake::hook(|name, _| match name {
        "champion" | "camp" => Some(97),
        _ => None,
    });
    assert_eq!(champion.attack_tiles().unwrap_err().raw_code(), Some(97));
    assert_eq!(camp.attack_tiles().unwrap_err().raw_code(), Some(97));
    fake::hook(|name, _| match name {
        "bc_Bitboard_DrawChebyshev" => Some(12345),
        _ => None,
    });
    assert_eq!(champion.attack_tiles().unwrap_err().raw_code(), Some(12345));
}
#[test]
fn buffs_are_captured_at_construction_with_typed_and_dyn_views() {
    setup();
    fake::edit(|s| {
        s.buffs = vec![
            sys::BuffData {
                kind: 0,
                duration: 2,
                value: 75.,
            },
            sys::BuffData {
                kind: 1,
                duration: 4,
                value: 0.,
            },
        ]
    });
    let c = Champion::new(ChampionHandle(0)).unwrap();
    let buffs = c.buffs().unwrap();
    let calls_after_construction = fake::HOST.with(|host| host.borrow().calls.len());
    let buff = buffs.get(BuffKind::Shield).unwrap();
    let info: &dyn BuffInfo = buff.as_dyn();
    assert_eq!(info.kind(), BuffKind::Shield);
    assert_eq!(info.duration(), 2);
    assert_eq!(info.shield_value(), Some(75.));
    assert!(matches!(
        buff,
        BuffPayload::Shield {
            duration: 2,
            value: 75.
        }
    ));
    let boxed = buffs.dyn_info(BuffKind::Shield).unwrap();
    let iter = buffs.iter();
    assert!(buffs.has(BuffKind::Stun));
    assert!(buffs.has_shield() && buffs.has_stun());
    assert!(!buffs.has_enhanced_attack());
    assert_eq!(buffs.has_shield(), buffs.has(BuffKind::Shield));
    assert_eq!(buffs.has_stun(), buffs.has(BuffKind::Stun));
    assert_eq!(buffs.has_enhanced_attack(), buffs.has(BuffKind::EnhancedAttack));
    assert_eq!(buffs.len(), 2);
    assert!(!buffs.is_empty());
    fake::HOST.with(|host| assert_eq!(host.borrow().calls.len(), calls_after_construction));
    assert_eq!(c.restrictions().unwrap(), BuffRestrictions::MOVE);
    fake::edit(|s| s.buffs.clear());
    assert_eq!(boxed.shield_value(), Some(75.));
    assert_eq!(info.duration(), 2);
    assert_eq!(iter.count(), 2);
    assert_eq!(buffs.len(), 2);
    let fresh_buffs = c.buffs().unwrap();
    assert!(fresh_buffs.is_empty());
    assert!(!fresh_buffs.has_shield() && !fresh_buffs.has_stun() && !fresh_buffs.has_enhanced_attack());
    assert!(fresh_buffs.get(BuffKind::Shield).is_none());
    fake::hook(|name, _| (name == "champion_buff_data").then_some(97));
    assert_eq!(buffs.len(), 2);
    assert_eq!(
        c.buffs().unwrap_err().code(),
        Some(ErrorCode::ApiNotInVision)
    );
}
#[test]
fn invalid_buff_tags_and_oversized_lists_are_errors() {
    setup();
    let c = Champion::new(ChampionHandle(0)).unwrap();
    fake::edit(|s| {
        s.buffs = vec![sys::BuffData {
            kind: 99,
            ..Default::default()
        }]
    });
    assert_eq!(
        c.buffs().unwrap_err().code(),
        Some(ErrorCode::ApiValueOutOfRange)
    );
    fake::hook(|name, a| {
        if name == "champion_buff_data" {
            unsafe {
                (a[3] as *mut u32).write(if a[2] == 0 { 1 } else { 2 });
            }
            Some(0)
        } else {
            None
        }
    });
    assert!(c.buffs().is_err());
}
#[test]
fn nonself_actions_and_private_queries_are_results() {
    setup();
    crate::init().unwrap();
    let c = Champion::new(ChampionHandle(1)).unwrap();
    assert_eq!(
        c.r#move(Vec2::new(1, 0)).unwrap_err().code(),
        Some(ErrorCode::ApiNotOwner)
    );
    assert!(c
        .attack(&Entity(crate::EntityTarget::Camp(CampHandle(10))))
        .is_err());
    assert!(c.ward_positions().is_err());
    assert!(c
        .can_buy(ItemType::MonsterShardOfStrength, 1)
        .is_err());
    assert!(c
        .can_buy_consumable(ConsumableType::TpCrystal)
        .is_err());
    assert!(c.can_sell().is_err());
    assert!(c.ward_placement_tiles().is_err());
    assert!(c.buy(ItemType::MonsterShardOfStrength).is_err());
    assert!(c.sell(ItemType::MonsterShardOfStrength).is_err());
    assert!(c.abilities()[0]
        .cast_on(&Entity(crate::EntityTarget::Camp(CampHandle(10))))
        .is_err());
    assert!(c.consumables()[ConsumableType::TpCrystal]
        .use_untargeted()
        .is_err());
}
#[test]
fn raw_and_typed_engine_errors_are_preserved() {
    setup();
    crate::init().unwrap();
    fake::hook(|name, _| {
        if name == "place_ward" {
            Some(65)
        } else if name == "resign" {
            Some(12345)
        } else {
            None
        }
    });
    let c = Champion::new(ChampionHandle(0)).unwrap();
    let e = c.place_ward(Vec2::new(7, 7)).unwrap_err();
    assert_eq!(e.raw_code(), Some(65));
    assert_eq!(e.code(), Some(ErrorCode::InvalidActionPlaceWardOutOfRange));
    assert!(e.to_string().contains("fake diagnostic"));
    let e = crate::resign().unwrap_err();
    assert_eq!(e.raw_code(), Some(12345));
    assert_eq!(e.code(), None);
}
#[test]
fn overlapping_wards_remain_distinct_observations() {
    setup();
    crate::init().unwrap();
    fake::edit(|s| {
        s.wards = vec![
            sys::WardInfo {
                team: 0,
                x: 2,
                y: 2,
                timer: -1,
                disabled: 0,
            },
            sys::WardInfo {
                team: 1,
                x: 2,
                y: 2,
                timer: 3,
                disabled: 1,
            },
        ];
    });
    let wards = crate::vision::wards(None).unwrap();
    assert_eq!(wards.len(), 2);
    for w in &wards {
        assert_eq!(w.position(), Vec2::new(2, 2));
    }
    assert_eq!(wards[0].remaining_time(), None);
    assert_eq!(crate::vision::wards(Some(Team::Enemy)).unwrap().len(), 1);
    let live = crate::vision::ward_at(Vec2::new(2, 2), None).unwrap();
    assert_eq!(live.position(), Vec2::new(2, 2));
    fake::edit(|s| s.wards.clear());
    assert_eq!(wards.len(), 2);
    assert_eq!(crate::vision::wards(None).unwrap().len(), 0);
}
#[test]
fn position_lookups_distinguish_absence_and_invalid_positions() {
    setup();
    crate::init().unwrap();
    assert_eq!(
        crate::vision::ward_at(Vec2::new(0, 0), None)
            .unwrap_err()
            .code(),
        Some(ErrorCode::ApiNotFound)
    );
    assert!(!crate::vision::has_ward(Vec2::new(7, 0), None).unwrap());
    assert_eq!(
        crate::vision::ward_at(Vec2::new(7, 0), None)
            .unwrap_err()
            .code(),
        Some(ErrorCode::ApiNotInVision)
    );
    assert_eq!(
        crate::vision::ward_at(Vec2::new(-1, 0), None)
            .unwrap_err()
            .code(),
        Some(ErrorCode::ApiBadPosition)
    );
    assert!(!crate::vision::has_champion(Vec2::new(7, 0)).unwrap());
    assert_eq!(
        crate::vision::champion_at(Vec2::new(0, 0))
            .unwrap_err()
            .code(),
        Some(ErrorCode::ApiNotFound)
    );
    assert_eq!(
        crate::vision::champion_at(Vec2::new(7, 0))
            .unwrap_err()
            .code(),
        Some(ErrorCode::ApiNotInVision)
    );
    assert_eq!(
        crate::vision::camp_at(Vec2::new(0, 0)).unwrap_err().code(),
        Some(ErrorCode::ApiNotFound)
    );
    assert_eq!(
        crate::vision::camp_at(Vec2::new(7, 0)).unwrap_err().code(),
        Some(ErrorCode::ApiNotInVision)
    );
    assert_eq!(
        crate::vision::altar_lock_at(Vec2::new(7, 0))
            .unwrap_err()
            .code(),
        Some(ErrorCode::ApiNotInVision)
    );
    assert!(!crate::vision::has_altar_lock(Vec2::new(7, 0)).unwrap());
    assert!(!crate::vision::has_altar_lock(Vec2::new(0, 0)).unwrap());
    assert!(crate::vision::has_altar_lock(Vec2::new(1, 2)).unwrap());
    assert_eq!(
        crate::vision::has_altar_lock(Vec2::new(8, 0))
            .unwrap_err()
            .code(),
        Some(ErrorCode::ApiBadPosition)
    );
    assert_eq!(
        crate::vision::has_champion(Vec2::new(8, 0))
            .unwrap_err()
            .code(),
        Some(ErrorCode::ApiBadPosition)
    );
    assert!(!crate::vision::has_camp(Vec2::new(0, 0)).unwrap());
    assert!(crate::vision::champion_at(Vec2::new(2, 2)).is_ok());
}

#[test]
fn team_filtered_ward_lookups_use_only_ordinary_wards() {
    setup();
    crate::init().unwrap();
    fake::hook(|name, args| match name {
        "ally_ward_at" => {
            unsafe { (args[2] as *mut u32).write(11) };
            Some(0)
        }
        "enemy_ward_at" => {
            unsafe { (args[2] as *mut u32).write(12) };
            Some(0)
        }
        "ward" => {
            let team = if args[0] == 11 { 0 } else { 1 };
            unsafe {
                (args[1] as *mut sys::WardInfo).write(sys::WardInfo {
                    team,
                    ..Default::default()
                });
            }
            Some(0)
        }
        _ => None,
    });
    assert_eq!(
        crate::vision::ward_at(Vec2::new(2, 2), Some(Team::Ally))
            .unwrap()
            .team(),
        Team::Ally
    );
    assert!(crate::vision::has_ward(Vec2::new(2, 2), Some(Team::Ally)).unwrap());
    assert_eq!(
        crate::vision::ward_at(Vec2::new(2, 2), Some(Team::Enemy))
            .unwrap()
            .team(),
        Team::Enemy
    );

    fake::hook(|name, _| (name == "ally_ward_at").then_some(ErrorCode::ApiNotFound as i64));
    assert_eq!(
        crate::vision::ward_at(Vec2::new(2, 2), Some(Team::Ally))
            .unwrap_err()
            .code(),
        Some(ErrorCode::ApiNotFound)
    );
    assert!(!crate::vision::has_ward(Vec2::new(2, 2), Some(Team::Ally)).unwrap());

    fake::hook(|name, _| (name == "ally_ward_at").then_some(ErrorCode::ApiNotInVision as i64));
    assert!(!crate::vision::has_ward(Vec2::new(2, 2), Some(Team::Ally)).unwrap());

    fake::hook(|name, _| (name == "ally_ward_at").then_some(12_345));
    assert_eq!(
        crate::vision::has_ward(Vec2::new(2, 2), Some(Team::Ally))
            .unwrap_err()
            .raw_code(),
        Some(12_345)
    );
}

#[test]
fn teleports_and_altar_locks_keep_metadata_and_filtering() {
    setup();
    crate::init().unwrap();
    fake::edit(|s| {
        s.teleports = vec![sys::TeleportInfo {
            champion: 0,
            kind: 0,
            team: 1,
            destination: Vec2::new(2, 2),
            timer: 3,
            channel_type: 4,
        }];
        s.locks = vec![
            sys::AltarLockInfo {
                kind: 6,
                handle: 3,
                team: 0,
                x: 2,
                y: 2,
                hits_remaining: 3,
            },
            sys::AltarLockInfo {
                kind: 6,
                handle: 4,
                team: 1,
                x: 2,
                y: 2,
                hits_remaining: 2,
            },
        ];
    });
    let t = crate::vision::teleports().unwrap();
    assert_eq!(t[0].position(), Vec2::new(2, 2));
    assert_eq!(t[0].team(), Team::Enemy);
    assert!(!t[0].is_ally() && t[0].is_enemy());
    assert_eq!(t[0].visible_tiles().unwrap().count().unwrap(), 64);
    let mut into = crate::Bitboard::new().unwrap();
    t[0].visible_tiles_into(&mut into).unwrap();
    assert_eq!(into.count().unwrap(), 64);
    assert!(crate::vision::has_teleport(Vec2::new(2, 2)).unwrap());
    assert!(!crate::vision::has_teleport(Vec2::new(0, 0)).unwrap());
    assert_eq!(
        crate::vision::teleport_at(Vec2::new(2, 2))
            .unwrap()
            .position(),
        Vec2::new(2, 2)
    );
    assert_eq!(
        crate::vision::teleport_at(Vec2::new(0, 0))
            .unwrap_err()
            .code(),
        Some(ErrorCode::ApiNotFound)
    );
    let locks = crate::vision::altar_locks(None).unwrap();
    assert_eq!(locks.len(), 2);
    assert_eq!(
        crate::vision::altar_locks(Some(Team::Enemy)).unwrap()[0].hits_remaining(),
        2
    );
    assert_eq!(locks[0].entity(), Entity(crate::EntityTarget::AltarLock(3)));
    assert_eq!(
        crate::vision::altar_lock_at(Vec2::new(2, 2))
            .unwrap()
            .team(),
        Team::Ally
    );
    let ally_lock = crate::vision::altar_lock_at(Vec2::new(2, 2)).unwrap();
    assert!(ally_lock.is_ally() && !ally_lock.is_enemy());
    assert_eq!(
        crate::vision::altar_lock_at(Vec2::new(0, 0))
            .unwrap_err()
            .code(),
        Some(ErrorCode::ApiNotFound)
    );
    let enemy_lock = crate::vision::altar_lock_at(Vec2::new(1, 2)).unwrap();
    assert_eq!(enemy_lock.team(), Team::Enemy);
    assert!(!enemy_lock.is_ally() && enemy_lock.is_enemy());
}

#[test]
fn camp_identity_is_handle_only_and_private_type_drives_live_queries() {
    setup();
    let camp = Camp::new(
        CampHandle(9),
        CampGroupHandle(2),
        CampType::Monolith,
        Vec2::new(2, 3),
    );
    assert_eq!(camp.entity().unwrap(), Entity::from(CampHandle(9)));
    assert_eq!(
        camp.entity().unwrap(),
        Entity(crate::EntityTarget::Camp(CampHandle(9)))
    );
    assert_eq!(camp.kind(), CampType::Monolith);
    for kind in CampType::ALL {
        let camp = Camp::new(CampHandle(10), CampGroupHandle(2), *kind, Vec2::new(0, 0));
        assert_eq!(camp.is_wolf(), *kind == CampType::Wolf);
        assert_eq!(camp.is_elk(), *kind == CampType::Elk);
        assert_eq!(camp.is_gem_cluster(), *kind == CampType::GemCluster);
        assert_eq!(camp.is_gold_cluster(), *kind == CampType::GoldCluster);
        assert_eq!(camp.is_monolith(), *kind == CampType::Monolith);
    }
    fake::hook(|name, args| {
        match name {
            "camp" => {
                assert_eq!(args[0], 9);
                unsafe {
                    (args[1] as *mut sys::CampState).write(sys::CampState {
                        health: 35.0,
                        strength: 12.0,
                        focus: 4.0,
                        ..Default::default()
                    });
                }
            }
            "camp_design" => {
                assert_eq!(args[0], CampType::Monolith as usize);
                unsafe {
                    (args[1] as *mut sys::CampDesign).write(sys::CampDesign {
                        base_stats: CampStats {
                            base_attack_range: 7,
                            base_move_speed: 3,
                            ..Default::default()
                        },
                        ..Default::default()
                    });
                }
            }
            _ => return None,
        }
        Some(0)
    });
    assert_eq!(camp.health().unwrap(), 35.0);
    assert_eq!(camp.strength().unwrap(), 12.0);
    assert_eq!(camp.focus().unwrap(), 4.0);
    assert_eq!(camp.attack_range().unwrap(), 7);
}

#[test]
fn champion_state_masks_preserve_engine_bits_and_unknown_values() {
    setup();
    let champion = Champion::new(ChampionHandle(0)).unwrap();
    fn mock_state(raw: u32) {
        fake::hook(move |name, args| {
            if name != "champion" {
                return None;
            }
            assert_eq!(args[0], 0);
            unsafe {
                (args[1] as *mut sys::ChampionInfo).write(sys::ChampionInfo {
                    state: raw as i32,
                    ..Default::default()
                });
            }
            Some(0)
        });
    }
    mock_state(0);
    assert_eq!(champion.is_ally().unwrap(), champion.team().unwrap() == Team::Ally);
    assert_eq!(champion.is_enemy().unwrap(), champion.team().unwrap() == Team::Enemy);
    let flags = [
        (1 << 0, ChampionState::ALIVE),
        (1 << 1, ChampionState::RECALLING),
        (1 << 2, ChampionState::SUBMITTING_GEMS),
        (1 << 3, ChampionState::CAN_MOVE),
        (1 << 4, ChampionState::CAN_ATTACK),
        (1 << 5, ChampionState::COMPLETED_TURN),
        (1 << 6, ChampionState::RECALL_STARTED),
        (1 << 7, ChampionState::SUBMIT_GEMS_STARTED),
        (1 << 8, ChampionState::RECALL_CANCELLED),
        (1 << 9, ChampionState::SUBMIT_GEMS_CANCELLED),
        (1 << 11, ChampionState::CAN_CAST),
        (1 << 12, ChampionState::TELEPORTING),
    ];
    for (bit, flag) in flags {
        assert_eq!(flag.bits(), bit);
        mock_state(bit);
        let state = champion.state().unwrap();
        assert_eq!(state, flag);
        assert!(state.contains(flag));
        assert_eq!(state.is_dead(), flag != ChampionState::ALIVE);
        assert_eq!(champion.can_move().unwrap(), state.contains(ChampionState::CAN_MOVE));
        assert_eq!(champion.can_attack().unwrap(), state.contains(ChampionState::CAN_ATTACK));
        assert_eq!(champion.can_cast().unwrap(), state.contains(ChampionState::CAN_CAST));
        assert_eq!(champion.is_recalling().unwrap(), state.contains(ChampionState::RECALLING));
        assert_eq!(
            champion.is_submitting_gems().unwrap(),
            state.contains(ChampionState::SUBMITTING_GEMS)
        );
        assert_eq!(champion.is_teleporting().unwrap(), state.contains(ChampionState::TELEPORTING));
        assert_eq!(
            champion.has_completed_turn().unwrap(),
            state.contains(ChampionState::COMPLETED_TURN)
        );
    }
    let raw = ChampionState::ALIVE.bits()
        | ChampionState::TELEPORTING.bits()
        | (1 << 10)
        | (1 << 31);
    mock_state(raw);
    let state = champion.state().unwrap();
    assert_eq!(state.bits(), raw);
    assert!(state.contains(ChampionState::ALIVE | ChampionState::TELEPORTING));
    assert_eq!(ChampionState::all().bits() & (1 << 10), 0);
    assert!(!state.contains(ChampionState::RECALLING));
    assert!(!state.is_dead());
    assert!(ChampionState::empty().is_dead());
}
#[test]
fn camp_groups_use_host_membership_and_state_variants() {
    setup();
    crate::init().unwrap();
    let groups = crate::map::camp_groups().unwrap();
    assert_eq!(groups.len(), 1);
    let g = &groups[0];
    assert_eq!(g.iter().count(), 2);
    assert_eq!(
        (g.gold_value(), g.gem_value(), g.max_respawn_timer()),
        (10, 4, 30)
    );
    assert_eq!(g.spawn_tiles().unwrap().count().unwrap(), 2);
    assert_eq!(g.state().unwrap(), CampGroupState::Alive);
    fake::edit(|s| s.group_state = 2);
    assert_eq!(g.state().unwrap(), CampGroupState::RewardAvailable);
    fake::edit(|s| s.group_state = 3);
    assert_eq!(g.state().unwrap(), CampGroupState::Respawning(30));
    fake::edit(|s| s.group_state = 99);
    assert_eq!(
        g.state().unwrap_err().code(),
        Some(ErrorCode::ApiValueOutOfRange)
    );
    fake::hook(|name, a| {
        if name == "board_vision" {
            unsafe {
                (a[0] as *mut u64).write(0);
                (a[0] as *mut u64).add(1).write(0);
            }
            Some(0)
        } else {
            None
        }
    });
    assert_eq!(
        g.state().unwrap_err().code(),
        Some(ErrorCode::ApiNotInVision)
    );
}
#[test]
fn aligned_boards_and_fields_clone_own_their_storage() {
    setup();
    crate::init().unwrap();
    let mut b = Bitboard::new().unwrap();
    assert_eq!(b.ptr() as usize % 16, 0);
    b.set(Vec2::new(3, 3), true).unwrap();
    let clone = b.clone();
    b.clear();
    assert!(clone.is_set(Vec2::new(3, 3)).unwrap());
    assert!(b.is_empty().unwrap());
    assert_eq!(clone.positions().unwrap(), vec![Vec2::new(3, 3)]);
    assert_eq!(clone.union(&b).unwrap().count().unwrap(), 1);
    fake::hook(|name, _| match name {
        "bc_Bitboard_IsEqual"
        | "bc_Bitboard_Union"
        | "bc_Bitboard_Intersect"
        | "bc_Bitboard_Xor" => Some(5),
        _ => None,
    });
    assert!(clone.is_equal(&clone).unwrap());
    assert!(clone
        .union(&clone)
        .unwrap()
        .is_set(Vec2::new(3, 3))
        .unwrap());
    assert!(clone
        .intersect(&clone)
        .unwrap()
        .is_set(Vec2::new(3, 3))
        .unwrap());
    assert!(clone.xor(&clone).unwrap().is_empty().unwrap());
    fake::edit(|s| s.hook = None);
    assert_eq!(clone.complement().unwrap().count().unwrap(), 63);
    assert!(clone.is_set(Vec2::new(8, 0)).is_err());
    assert_eq!(clone.export_raw_buffer().unwrap().len(), 16);
    assert!(clone.export_raw_buffer_into(&mut [0; 1]).is_err());
    let mut field = DistanceField::new().unwrap();
    assert_eq!(field.ptr() as usize % 16, 0);
    field.set(Vec2::new(1, 1), 7).unwrap();
    let f = field.clone();
    field.set(Vec2::new(1, 1), 8).unwrap();
    assert_eq!(f.get(Vec2::new(1, 1)).unwrap(), 7);
    assert_eq!(f.export_raw_buffer().unwrap().len(), 128);
}
#[test]
fn distance_field_next_moves() {
    setup();
    let mut field = DistanceField::new().unwrap();
    for x in 0..field.width() {
        for y in 0..field.height() {
            let p = Vec2::new(x, y);
            let d = p.chebyshev_distance(Vec2::new(3, 3)) as u16;
            field.set(p, d).unwrap();
        }
    }
    field
        .set(Vec2::new(7, 7), DistanceField::UNREACHABLE)
        .unwrap();
    let from = Vec2::new(5, 3);
    assert_eq!(field.next_move_from(from).unwrap(), Some(Vec2::SOUTH_WEST));
    assert_eq!(
        field.next_moves_from(from).unwrap(),
        vec![Vec2::SOUTH_WEST, Vec2::WEST, Vec2::NORTH_WEST]
    );
    assert_eq!(
        field.next_move_from(Vec2::new(0, 0)).unwrap(),
        Some(Vec2::NORTH_EAST)
    );
    assert_eq!(field.next_move_from(Vec2::new(3, 3)).unwrap(), None);
    assert!(field.next_moves_from(Vec2::new(7, 7)).unwrap().is_empty());
    assert!(field.next_move_from(Vec2::new(-1, 0)).is_err());
}
#[test]
fn shared_memory_bounds_and_fresh_inventory() {
    setup();
    crate::init().unwrap();
    let memory = crate::shared_memory();
    memory.write(0, &[1, 2, 3, 4]).unwrap();
    assert_eq!(memory.get(2).unwrap(), 3);
    assert!(memory.write(3, &[1, 2]).is_err());
    assert!(memory.read(usize::MAX, &mut []).is_err());
    memory.write(4, &[]).unwrap();
    let c = Champion::new(ChampionHandle(0)).unwrap();
    assert_eq!(c.item_count().unwrap()[ItemType::PowerShardOfStrength], 6);
    assert_eq!(
        c.consumables()[ConsumableType::TpCrystal].count().unwrap(),
        0
    );
    assert_eq!(
        c.consumables()[ConsumableType::AltarLock].count().unwrap(),
        2
    );
    assert_eq!(ConsumableType::TpCrystal as u32, 0);
    assert_eq!(ConsumableType::AltarLock as u32, 1);
    assert_eq!(ConsumableType::TpCrystal.raw(), 0);
    assert_eq!(ConsumableType::AltarLock.raw(), 2);
    assert!(ConsumableType::try_from(2).is_err());
    assert_eq!(
        ItemType::MonsterShardOfStrength.info().unwrap().result,
        Some(ItemType::MonsterEmblemOfStrength)
    );
    assert_eq!(ConsumableType::TpCrystal.info().unwrap().carry_limit, 1);
}
#[test]
fn shared_memory_caches_capacity_and_matches_cpp_bounds() {
    setup();
    crate::init().unwrap();
    let memory: &'static SharedMemory = crate::shared_memory();
    assert_eq!(memory.size(), 4);
    fake::edit(|host| host.calls.clear());
    fake::hook(|name, _| (name == "shared_memory_size").then_some(12_345));
    assert!(core::ptr::eq(memory, crate::shared_memory()));
    assert_eq!(memory.size(), 4);
    memory.read(4, &mut []).unwrap();
    memory.write(4, &[]).unwrap();
    for offset in [4, 5, usize::MAX] {
        assert_eq!(
            memory.get(offset).unwrap_err().code(),
            Some(ErrorCode::InvalidArgument)
        );
        assert!(memory.set(offset, 1).is_err());
    }
    for offset in [5, usize::MAX] {
        assert!(memory.read(offset, &mut []).is_err());
        assert!(memory.write(offset, &[]).is_err());
    }
    let mut bytes = [77; 4];
    assert!(memory.read(1, &mut bytes).is_err());
    assert_eq!(bytes, [77; 4]);
    assert!(memory.write(1, &[1, 2, 3, 4]).is_err());
    fake::HOST.with(|host| assert!(host.borrow().calls.is_empty()));
    memory.write(0, &[1, 2, 3, 4]).unwrap();
    memory.read(0, &mut bytes).unwrap();
    assert_eq!(bytes, [1, 2, 3, 4]);
    memory.set(0, 42).unwrap();
    memory.set(1, memory.get(0).unwrap()).unwrap();
    assert_eq!(memory.get(1).unwrap(), 42);
    fake::edit(|host| host.memory[1] = 99);
    assert_eq!(memory.get(1).unwrap(), 99);
}

#[test]
fn shared_memory_typed_buffers_use_byte_offsets_and_checked_ranges() {
    setup();
    crate::init().unwrap();
    let memory = crate::shared_memory();
    let values = [0x1234u16, 0xabcd];
    memory.write_values(0, &values).unwrap();
    let mut restored = [0u16; 2];
    memory.read_values(0, &mut restored).unwrap();
    assert_eq!(restored, values);
    fake::HOST.with(|host| {
        assert_eq!(host.borrow().memory, bytemuck::cast_slice(&values));
    });
    memory.write_values(1, &[0x4567u16]).unwrap();
    memory.read_values(1, &mut restored[..1]).unwrap();
    assert_eq!(restored[0], 0x4567);
    fake::edit(|host| host.calls.clear());
    assert!(memory.read_values(1, &mut restored).is_err());
    assert!(memory.write_values(1, &values).is_err());
    memory.read_values::<u32>(4, &mut []).unwrap();
    memory.write_values::<u32>(4, &[]).unwrap();
    assert!(memory.read_values::<u32>(usize::MAX, &mut []).is_err());
    assert!(memory.write_values::<u32>(usize::MAX, &[]).is_err());
    fake::HOST.with(|host| assert!(host.borrow().calls.is_empty()));
}

#[test]
fn shared_memory_transfers_preserve_engine_errors() {
    setup();
    crate::init().unwrap();
    let memory = crate::shared_memory();
    fake::hook(|name, _| {
        matches!(name, "read_shared_memory" | "write_shared_memory").then_some(12_345)
    });
    let mut bytes = [77; 4];
    let mut values = [0u32];
    for error in [
        memory.get(0).unwrap_err(),
        memory.set(0, 1).unwrap_err(),
        memory.read(0, &mut bytes).unwrap_err(),
        memory.write(0, &[1, 2, 3, 4]).unwrap_err(),
        memory.read_values(0, &mut values).unwrap_err(),
        memory.write_values(0, &[1u32]).unwrap_err(),
    ] {
        assert_eq!(error.raw_code(), Some(12_345));
        assert_eq!(error.code(), None);
    }
    assert_eq!(bytes, [77; 4]);
    fake::HOST.with(|host| assert_eq!(host.borrow().memory, [0; 4]));
}

#[test]
fn shared_memory_rejects_offsets_outside_signed_import_range() {
    setup();
    fake::hook(|name, args| {
        if name != "shared_memory_size" {
            return None;
        }
        unsafe { (args[0] as *mut u32).write(u32::MAX) };
        Some(0)
    });
    crate::init().unwrap();
    let memory = crate::shared_memory();
    assert_eq!(memory.size(), u32::MAX as usize);
    fake::edit(|host| host.calls.clear());
    let maximum = i32::MAX as usize;
    memory.read(maximum, &mut []).unwrap();
    memory.write(maximum, &[]).unwrap();
    assert!(memory.read(maximum + 1, &mut []).is_err());
    assert!(memory.write(maximum + 1, &[]).is_err());
    fake::HOST.with(|host| assert!(host.borrow().calls.is_empty()));
}

#[test]
fn constants_are_owned_runtime_queries_and_preserve_errors() {
    setup();
    let mut calls = 0;
    fake::hook(move |name, args| {
        if name != "game_settings" {
            return None;
        }
        assert_eq!(args[0] % 8, 0);
        calls += 1;
        unsafe {
            (args[0] as *mut Constants).write(Constants {
                capture_time: calls,
                submit_gem_duration_multiplier: 0.375,
                ward_charges: 65538,
                ward_placement_radius: 2.5,
                tp_scroll_time: 17,
                tp_crystal_time: 19,
                reserved: 0,
                ..Default::default()
            });
        }
        Some(0)
    });
    crate::init().unwrap();
    let first: &'static Constants = crate::constants();
    assert_eq!(first.capture_time, 1);
    assert_eq!(first.submit_gem_duration_multiplier, 0.375);
    assert_eq!(first.ward_charges, 65538);
    assert_eq!(first.ward_placement_radius, 2.5);
    assert_eq!(first.tp_scroll_time, 17);
    assert_eq!(first.tp_crystal_time, 19);
    fake::edit(|host| host.calls.clear());
    fake::hook(|name, _| (name == "game_settings").then_some(12345));
    crate::init().unwrap();
    assert!(core::ptr::eq(first, crate::constants()));
    fake::HOST.with(|host| assert!(host.borrow().calls.is_empty()));
    setup();
    fake::hook(|name, _| (name == "game_settings").then_some(12345));
    let error = crate::init().unwrap_err();
    assert_eq!(error.raw_code(), Some(12345));
    assert_eq!(error.code(), None);
}

#[test]
fn init_populates_all_caches_and_accessors_never_query_or_initialize() {
    setup();
    assert!(std::panic::catch_unwind(crate::constants).is_err());
    assert!(crate::self_champion().is_err());
    assert!(crate::champion_designs().is_err());
    assert!(std::panic::catch_unwind(crate::shared_memory).is_err());
    assert!(crate::map::walls().is_err());
    assert!(crate::map::camp_groups().is_err());
    fake::HOST.with(|host| assert!(host.borrow().calls.is_empty()));
    crate::init().unwrap();
    fake::HOST.with(|host| {
        let host = host.borrow();
        for (name, count) in [
            ("bc_metadata", 1),
            ("game_settings", 1),
            ("shared_memory_size", 1),
            ("champion_design", 12),
            ("bc_map_mask", 4),
        ] {
            assert_eq!(
                host.calls
                    .iter()
                    .filter(|call| call.as_str() == name)
                    .count(),
                count
            );
        }
    });
    fake::edit(|host| host.calls.clear());
    assert_eq!(crate::constants().max_rounds, 2000);
    assert_eq!(crate::map::width().unwrap(), 8);
    assert_eq!(crate::map::height().unwrap(), 8);
    assert!(crate::map::is_in_bounds(Vec2::new(7, 7)).unwrap());
    crate::map::symmetry().unwrap();
    crate::map::walls().unwrap();
    crate::map::bushes().unwrap();
    crate::map::holes().unwrap();
    crate::map::altars().unwrap();
    crate::map::spawn_region(Team::Ally).unwrap();
    crate::map::spawn_region(Team::Enemy).unwrap();
    assert_eq!(crate::shared_memory().size(), 4);
    assert_eq!(crate::draft_memory().bytes(), b"rust");
    assert_eq!(crate::draft_memory().len(), 4);
    assert_eq!(crate::draft_memory().get(0).unwrap(), b'r');
    assert!(crate::draft_memory().get(4).is_err());
    assert_eq!(crate::champion_designs().unwrap().len(), 12);
    crate::init().unwrap();
    fake::HOST.with(|host| assert!(host.borrow().calls.is_empty()));
    assert_eq!(crate::map::camp_groups().unwrap().len(), 1);
    fake::edit(|host| host.health = 27.);
    assert_eq!(
        crate::self_champion().unwrap().unwrap().health().unwrap(),
        27.
    );
}

#[test]
fn board_and_field_constructors_query_layout_without_caching_it() {
    setup();
    crate::init().unwrap();
    let board = Bitboard::new().unwrap();
    assert_eq!(board.complement().unwrap().count().unwrap(), 64);
    assert_eq!(board.xor(&board).unwrap().count().unwrap(), 0);
    assert_eq!(board.export_raw_buffer().unwrap().len(), 16);
    assert_eq!(
        DistanceField::new()
            .unwrap()
            .export_raw_buffer()
            .unwrap()
            .len(),
        128
    );
    fake::hook(|name, args| {
        if name != "bc_metadata" {
            return None;
        }
        unsafe {
            (args[0] as *mut sys::Metadata).write(sys::Metadata {
                width: 16,
                height: 8,
                chunked_width: 2,
                chunked_height: 1,
                chunk_stride: 2,
                board_bytes: 16,
                field_bytes: 256,
            });
        }
        Some(0)
    });
    let board = Bitboard::new().unwrap();
    assert_eq!(board.width(), 16);
    assert_eq!(board.height(), 8);
    assert_eq!(DistanceField::new().unwrap().raw_buffer_size(), 256);
    fake::hook(|name, _| (name == "bc_metadata").then_some(12_345));
    assert_eq!(Bitboard::new().unwrap_err().raw_code(), Some(12_345));
    assert_eq!(DistanceField::new().unwrap_err().raw_code(), Some(12_345));
    crate::init().unwrap();
}

#[test]
fn init_failures_publish_no_cache_and_can_be_retried() {
    for name in [
        "bc_metadata",
        "game_settings",
        "champion_design",
        "shared_memory_size",
        "draft_memory_size",
        "read_draft_memory",
        "bc_map_symmetry",
        "bc_map_mask",
        "spawn_region",
    ] {
        setup();
        fake::hook(move |called, _| (called == name).then_some(12_345));
        assert_eq!(
            crate::init().err().unwrap().raw_code(),
            Some(12_345),
            "{name}"
        );
        assert!(
            std::panic::catch_unwind(crate::constants).is_err(),
            "{name}"
        );
        assert!(
            std::panic::catch_unwind(crate::shared_memory).is_err(),
            "{name}"
        );
        assert!(crate::map::camp_groups().is_err(), "{name}");
        fake::hook(|_, _| None);
        crate::init().unwrap();
        assert!(crate::map::camp_groups().is_ok(), "{name}");
    }
}

#[test]
fn item_metadata_is_engine_backed_and_decodes_public_fields() {
    setup();
    crate::init().unwrap();
    fake::hook(|name, args| {
        if name != "item_info" {
            return None;
        }
        assert_eq!(args[0], ItemType::MonsterEmblemOfStrength as usize);
        assert_eq!(args[1] % 4, 0);
        unsafe {
            (args[1] as *mut sys::GameItemInfo).write(sys::GameItemInfo {
                cost: 71,
                purchasable: 1,
                health_points: 8,
                strength_points: 9,
                focus_points: 10,
                passive: 2,
                passive_magnitude: 0.125,
                combine_count: 3,
                result: ItemType::PowerEmblemOfFocus as u32,
                combine_cost: 213,
            });
        }
        Some(0)
    });
    let first = ItemType::MonsterEmblemOfStrength.info().unwrap();
    assert_eq!(
        first,
        ItemInfo {
            cost: 71,
            purchasable: true,
            health_points: 8,
            strength_points: 9,
            focus_points: 10,
            passive: Some(ItemPassive::Durability),
            passive_magnitude: 0.125,
            combine_count: 3,
            result: Some(ItemType::PowerEmblemOfFocus),
        }
    );
    fake::hook(|name, args| {
        if name != "item_info" {
            return None;
        }
        unsafe {
            (args[1] as *mut sys::GameItemInfo).write(sys::GameItemInfo {
                cost: 99,
                result: 0xff,
                ..Default::default()
            });
        }
        Some(0)
    });
    let second = ItemType::MonsterEmblemOfStrength.info().unwrap();
    assert_eq!(second.cost, 99);
    assert_eq!(second.result, None);
    assert_eq!(second.passive, None);
    assert_eq!(first.cost, 71);
}

#[test]
fn typed_position_and_ward_queries_preserve_host_arguments_and_errors() {
    setup();
    fake::hook(|name, args| {
        let value = match name {
            "bc_Bitboard_ReflectHorizontalPosition" => 10,
            "bc_Bitboard_ReflectVerticalPosition" => 20,
            "bc_Bitboard_Rotate180Position" => 30,
            "board_ward_vision_mask" => {
                assert_eq!(&args[..2], &[2, 3]);
                assert_eq!(args[2] % 16, 0);
                unsafe { (args[2] as *mut u64).write(1) };
                return Some(0);
            }
            _ => return None,
        };
        assert_eq!(&args[..2], &[2, 3]);
        unsafe {
            (args[2] as *mut i32).write(value);
            (args[3] as *mut i32).write(value + 1);
        }
        Some(0)
    });
    let position = Vec2::new(2, 3);
    assert_eq!(position.reflect_horizontal(), Vec2::new(-2, 3));
    assert_eq!(position.reflect_vertical(), Vec2::new(2, -3));
    assert_eq!(position.rotate_180(), Vec2::new(-2, -3));
    assert_eq!(
        crate::map::reflect_horizontal(position).unwrap(),
        Vec2::new(10, 11)
    );
    assert_eq!(
        crate::map::reflect_vertical(position).unwrap(),
        Vec2::new(20, 21)
    );
    assert_eq!(crate::map::rotate_180(position).unwrap(), Vec2::new(30, 31));
    let mask = Ward::visible_tiles_from(position).unwrap();
    assert_eq!(mask.raw_buffer_size(), 16);
    assert!(mask.is_set(Vec2::new(0, 0)).unwrap());
    let ward = Ward {
        position,
        team: Team::Ally,
        remaining_time: None,
    };
    assert!(ward.is_ally() && !ward.is_enemy());
    assert_eq!(ward.visible_tiles().unwrap().raw_buffer_size(), 16);
    let mut out = crate::Bitboard::new().unwrap();
    ward.visible_tiles_into(&mut out).unwrap();
    assert_eq!(out, ward.visible_tiles().unwrap());

    fake::hook(|_, _| Some(12_345));
    assert_eq!(
        crate::map::reflect_horizontal(position)
            .unwrap_err()
            .raw_code(),
        Some(12_345)
    );
    assert_eq!(
        crate::map::reflect_vertical(position)
            .unwrap_err()
            .raw_code(),
        Some(12_345)
    );
    assert_eq!(
        crate::map::rotate_180(position).unwrap_err().raw_code(),
        Some(12_345)
    );
    assert_eq!(
        Ward::visible_tiles_from(position).unwrap_err().raw_code(),
        Some(12_345)
    );
    assert_eq!(ward.visible_tiles().unwrap_err().raw_code(), Some(12_345));
}

#[test]
fn ward_placement_tiles_use_engine_settings() {
    setup();
    let c = Champion::new(ChampionHandle(0)).unwrap();
    fake::hook(|name, args| match name {
        "game_settings" => {
            unsafe {
                (args[0] as *mut Constants).write(Constants {
                    ward_placement_radius: 4.5,
                    ..Default::default()
                });
            }
            Some(0)
        }
        "bc_Bitboard_DrawChebyshev" => {
            assert_eq!(args[3] as u32, 4.5f32.to_bits());
            Some(0)
        }
        _ => None,
    });
    crate::init().unwrap();
    c.ward_placement_tiles().unwrap();
    fake::HOST.with(|host| {
        let calls = &host.borrow().calls;
        assert!(calls.iter().any(|name| name == "game_settings"));
        assert!(calls.iter().any(|name| name == "bc_Bitboard_DrawChebyshev"));
    });
}

#[test]
fn item_metadata_forwards_all_kinds_and_validates_runtime_tags() {
    setup();
    crate::init().unwrap();
    for &kind in ItemType::ALL {
        fake::hook(move |name, args| {
            if name != "item_info" {
                return None;
            }
            assert_eq!(args[0], kind as usize);
            unsafe {
                (args[1] as *mut sys::GameItemInfo).write(sys::GameItemInfo {
                    cost: kind as u32 + 100,
                    result: 0xff,
                    ..Default::default()
                });
            }
            Some(0)
        });
        let info = kind.info().unwrap();
        assert_eq!(info.cost, kind as u32 + 100);
        assert_eq!(info.result, None);
    }
    for (passive, result) in [(6, 0xff), (0, 42), (0, 43)] {
        fake::hook(move |name, args| {
            if name != "item_info" {
                return None;
            }
            unsafe {
                (args[1] as *mut sys::GameItemInfo).write(sys::GameItemInfo {
                    passive,
                    result,
                    ..Default::default()
                });
            }
            Some(0)
        });
        assert_eq!(
            ItemType::MonsterShardOfStrength.info().unwrap_err().code(),
            Some(ErrorCode::ApiValueOutOfRange)
        );
    }
    fake::hook(|name, _| {
        (name == "item_info").then_some(ErrorCode::InvalidActionBuyItemInvalidItem as i64)
    });
    let error = ItemType::MonsterShardOfStrength.info().unwrap_err();
    assert_eq!(
        error.code(),
        Some(ErrorCode::InvalidActionBuyItemInvalidItem)
    );
    assert_eq!(
        error.raw_code(),
        Some(ErrorCode::InvalidActionBuyItemInvalidItem as i32)
    );
}

#[test]
fn consumable_metadata_queries_all_kinds_and_returns_fresh_owned_records() {
    setup();
    crate::init().unwrap();
    fake::edit(|s| s.calls.clear());
    let mut calls = 0;
    fake::hook(move |name, args| {
        if name != "consumable_info" {
            return None;
        }
        let raw = if calls % ConsumableType::ALL.len() == 0 {
            0
        } else {
            2
        };
        assert_eq!(args[0], raw);
        calls += 1;
        unsafe {
            (args[1] as *mut ConsumableInfo).write(ConsumableInfo {
                cost: calls as u32 * 19,
                carry_limit: 65_536 + calls as u32,
            });
        }
        Some(0)
    });
    let mut snapshots = Vec::new();
    for &kind in ConsumableType::ALL {
        snapshots.push(kind.info().unwrap());
    }
    assert_eq!(ConsumableType::TpCrystal.info().unwrap().cost, 57);
    for (index, snapshot) in snapshots.iter().enumerate() {
        assert_eq!(snapshot.cost, (index as u32 + 1) * 19);
        assert_eq!(snapshot.carry_limit, 65_537 + index as u32);
    }
    fake::HOST.with(|host| assert_eq!(host.borrow().calls.len(), 3));
}

#[test]
fn can_buy_checks_item_and_consumable_purchase_constraints() {
    use std::cell::Cell;
    use std::rc::Rc;

    setup();
    crate::init().unwrap();
    let gold = Rc::new(Cell::new(100u32));
    let carried = Rc::new(Cell::new(false));
    let hook_gold = gold.clone();
    let hook_carried = carried.clone();
    fake::hook(move |name, args| match name {
        "champion_can_shop" => {
            unsafe { (args[2] as *mut i32).write(1) };
            Some(0)
        }
        "champion_gold" => {
            unsafe { (args[1] as *mut u32).write(hook_gold.get()) };
            Some(0)
        }
        "champion_consumables" => {
            unsafe {
                let counts = args[1] as *mut u32;
                counts.write(u32::from(hook_carried.get()));
                counts.add(1).write(0);
                counts.add(2).write(0);
                counts.add(3).write(0);
            }
            Some(0)
        }
        "item_info" => {
            unsafe {
                (args[1] as *mut sys::GameItemInfo).write(sys::GameItemInfo {
                    cost: 33,
                    purchasable: u32::from(args[0] != ItemType::MonsterEmblemOfStrength as usize),
                    result: 0xff,
                    ..Default::default()
                });
            }
            Some(0)
        }
        "consumable_info" => {
            unsafe {
                (args[1] as *mut ConsumableInfo).write(ConsumableInfo {
                    cost: 50,
                    carry_limit: 1,
                });
            }
            Some(0)
        }
        _ => None,
    });
    let c = crate::self_champion().unwrap().unwrap();
    let shard = ItemType::MonsterShardOfStrength;
    assert!(!c.can_buy(shard, 0).unwrap());
    gold.set(32);
    assert!(!c.can_buy(shard, 1).unwrap());
    gold.set(100);
    assert!(c.can_buy(shard, 2).unwrap());
    assert!(!c.can_buy(ItemType::MonsterEmblemOfStrength, 1).unwrap());
    gold.set(49);
    assert!(!c.can_buy_consumable(ConsumableType::TpCrystal).unwrap());
    gold.set(100);
    assert!(c.can_buy_consumable(ConsumableType::TpCrystal).unwrap());
    carried.set(true);
    assert!(!c.can_buy_consumable(ConsumableType::TpCrystal).unwrap());
}

#[test]
fn item_and_consumable_info_getters_forward_metadata_fields() {
    setup();
    crate::init().unwrap();
    fake::hook(|name, args| match name {
        "item_info" => {
            unsafe {
                (args[1] as *mut sys::GameItemInfo).write(sys::GameItemInfo {
                    cost: 73,
                    combine_count: 3,
                    result: ItemType::PowerEmblemOfFocus as u32,
                    health_points: 4,
                    strength_points: 5,
                    focus_points: 6,
                    passive: ItemPassive::Durability as u32,
                    passive_magnitude: 0.75,
                    purchasable: 1,
                    ..Default::default()
                });
            }
            Some(0)
        }
        "consumable_info" => {
            unsafe {
                (args[1] as *mut ConsumableInfo).write(ConsumableInfo {
                    cost: 91,
                    carry_limit: 7,
                });
            }
            Some(0)
        }
        _ => None,
    });
    let item = ItemType::MonsterEmblemOfStrength.info().unwrap();
    let consumable = ConsumableType::TpCrystal.info().unwrap();
    assert_eq!(ItemType::MonsterEmblemOfStrength.cost().unwrap(), item.cost);
    assert_eq!(
        ItemType::MonsterEmblemOfStrength.combine_count().unwrap(),
        item.combine_count
    );
    assert_eq!(
        ItemType::MonsterEmblemOfStrength.result().unwrap(),
        item.result
    );
    assert_eq!(
        ItemType::MonsterEmblemOfStrength.health_points().unwrap(),
        item.health_points
    );
    assert_eq!(
        ItemType::MonsterEmblemOfStrength.strength_points().unwrap(),
        item.strength_points
    );
    assert_eq!(
        ItemType::MonsterEmblemOfStrength.focus_points().unwrap(),
        item.focus_points
    );
    assert_eq!(
        ItemType::MonsterEmblemOfStrength.passive().unwrap(),
        item.passive
    );
    assert_eq!(
        ItemType::MonsterEmblemOfStrength
            .passive_magnitude()
            .unwrap(),
        item.passive_magnitude
    );
    assert_eq!(
        ItemType::MonsterEmblemOfStrength.is_purchasable().unwrap(),
        item.purchasable
    );
    assert_eq!(ConsumableType::TpCrystal.cost().unwrap(), consumable.cost);
    assert_eq!(
        ConsumableType::TpCrystal.carry_limit().unwrap(),
        consumable.carry_limit
    );
}

#[test]
fn consumable_metadata_preserves_engine_and_unknown_errors() {
    setup();
    crate::init().unwrap();
    for raw in [
        ErrorCode::InvalidActionBuyConsumableInvalidType as i32,
        12_345,
    ] {
        fake::hook(move |name, _| (name == "consumable_info").then_some(raw as i64));
        let error = ConsumableType::TpCrystal.info().unwrap_err();
        assert_eq!(error.raw_code(), Some(raw));
        assert_eq!(error.code(), ErrorCode::try_from(raw).ok());
    }
}

#[test]
fn action_targeting_counts_and_channels_preserve_host_arguments() {
    setup();
    crate::init().unwrap();
    let c = Champion::new(ChampionHandle(0)).unwrap();
    fake::hook(|name, a| {
        match name {
            "attack" => assert_eq!(a, &[2, 10]),
            "cast" => assert_eq!(&a[..5], &[0, 2, 10, -1_i32 as usize, -1_i32 as usize]),
            "buy_item" => assert_eq!(a, &[0, 2]),
            "sell" => assert_eq!(a, &[0]),
            "move_to" => assert_eq!(&a[..2], &[3, 2]),
            _ => {}
        }
        None
    });
    c.attack(&Entity(crate::EntityTarget::Camp(CampHandle(10))))
        .unwrap();
    c.abilities()[0]
        .cast_on(&Entity(crate::EntityTarget::Camp(CampHandle(10))))
        .unwrap();
    c.buy_count(ItemType::MonsterShardOfStrength, 2).unwrap();
    c.sell_count(ItemType::MonsterShardOfStrength, 2).unwrap();
    c.r#move(Vec2::new(1, 0)).unwrap();
    assert_eq!(c.remaining_channel_time().unwrap(), 3);
    assert_eq!(c.priority().unwrap(), 3);
    assert_eq!(c.prev_priority().unwrap(), 3);
    crate::debug::draw_dot(Vec2::new(1, 1), 0xffffff).unwrap();
    crate::debug::draw_line(Vec2::new(1, 1), Vec2::new(2, 2), 0).unwrap();
    assert_eq!(crate::round_number().unwrap(), 9);
    assert_eq!(crate::global_submitted_gems(Team::Enemy).unwrap(), 20);
}

#[test]
fn unit_attack_targets_convert_by_value_and_reference() {
    setup();
    crate::init().unwrap();
    let attacker = Champion::new(ChampionHandle(0)).unwrap();
    let target = Champion::new(ChampionHandle(11)).unwrap();
    let target_unit: &Unit = &target;
    fake::hook(|name, args| {
        if name == "attack" {
            assert_eq!(args, &[1, 11]);
        }
        None
    });
    attacker.attack(target_unit).unwrap();
    attacker.attack(target_unit).unwrap();
}

#[test]
fn display_and_opponent() {
    assert_eq!(Vec2::new(3, -4).to_string(), "(3, -4)");
    let mut v = Vec2::new(7, -5) / 2;
    assert_eq!(v, Vec2::new(3, -2));
    v /= 3;
    assert_eq!(v, Vec2::new(1, 0));
    assert_eq!(Team::Ally.opponent(), Team::Enemy);
    assert_eq!(Team::Enemy.opponent().to_string(), "Ally");
    assert_eq!(
        ItemType::BruiserEmblemOfHealth.to_string(),
        "BruiserEmblemOfHealth"
    );
    assert_eq!(ErrorCode::ApiNotFound.to_string(), "ApiNotFound");
    assert_eq!(ConsumableType::AltarLock.to_string(), "AltarLock");
}

#[test]
fn value_traits() {
    fn copy_hash<T: Copy + Eq + core::hash::Hash>() {}
    fn hash<T: Eq + core::hash::Hash>() {}
    fn copy_eq<T: Copy + PartialEq>() {}
    copy_hash::<Team>();
    copy_hash::<ItemCounts>();
    copy_hash::<Ward>();
    copy_hash::<Teleport>();
    copy_hash::<AltarLock>();
    copy_hash::<CampGroupState>();
    copy_hash::<AbilityAttackReset>();
    hash::<Unit>();
    hash::<Champion>();
    hash::<Camp>();
    hash::<CampGroup>();
    hash::<Ability>();
    hash::<Error>();
    copy_eq::<AbilityInfo>();
    copy_eq::<BuffCollection>();
    copy_eq::<ChampionStats>();
    assert_eq!(ItemCounts::default(), ItemCounts([0; 42]));
}

#[test]
fn destination_actions_distinguish_success_hidden_blocking_and_errors() {
    setup();
    crate::init().unwrap();
    let c = Champion::new(ChampionHandle(0)).unwrap();
    let actions: [&dyn Fn() -> crate::Result<bool>; 5] = [
        &|| c.move_absolute(Vec2::new(3, 2)),
        &|| c.r#move(Vec2::new(1, 0)),
        &|| c.place_ward(Vec2::new(3, 2)),
        &|| c.abilities()[0].cast_at(Vec2::new(3, 2)),
        &|| c.consumables()[ConsumableType::TpCrystal].use_at(Vec2::new(3, 2)),
    ];
    for action in actions {
        assert!(action().unwrap());
        fake::hook(|name, args| match name {
            "move_to" | "cast" | "use_consumable" | "place_ward" => {
                unsafe { (*args.last().unwrap() as *mut i32).write(0); }
                Some(0)
            }
            _ => None,
        });
        assert!(!action().unwrap());
        fake::hook(|name, _| match name {
            "move_to" => Some(22),
            "cast" => Some(32),
            "use_consumable" => Some(122),
            "place_ward" => Some(117),
            _ => None,
        });
        assert!(action().is_err());
        fake::hook(|_, _| None);
    }
}
