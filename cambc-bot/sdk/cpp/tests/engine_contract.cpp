// Compile-only contract between the bot SDK and the bundled engine.
// Keep explicit mappings here when adding SDK constants, enums or ABI fields.
#include "cambc.h"
#include "Engine/Balance/GameSettings.h"
#include "Engine/Balance/ChampionPassives.h"
#include "EngineC/Game/API.h"
#include "Engine/Balance/Items.h"
#include "Engine/Balance/ItemPassives.h"
#include "EngineC/Map.h"
#include "EngineC/Items.h"

#define BC_VALUE(sdk, engine) static_assert((sdk) == (engine), "SDK drift: " #sdk " != " #engine)
#define BC_ENUM(sdk, engine) BC_VALUE(static_cast<std::int64_t>(sdk), static_cast<std::int64_t>(engine))
#define BC_LAYOUT(sdk, engine) \
 static_assert(sizeof(sdk) == sizeof(engine), "SDK size drift: " #sdk); \
 static_assert(alignof(sdk) == alignof(engine), "SDK alignment drift: " #sdk)
#define BC_FIELD(sdk, field, engine, member) \
 static_assert(offsetof(sdk, field) == offsetof(engine, member), "SDK offset drift: " #sdk "::" #field); \
 static_assert(sizeof(((sdk*)nullptr)->field) == sizeof(((engine*)nullptr)->member), "SDK field size drift: " #sdk "::" #field)

static_assert(std::is_same_v<decltype(&cambc::detail::sdk_has_teleport),
 std::int32_t(*)(std::int32_t, std::int32_t, cbool*)>);
static_assert(std::is_same_v<decltype(&cambc::detail::sdk_ally_ward_at),
 std::int32_t(*)(std::int32_t, std::int32_t, std::uint32_t*)>);
static_assert(std::is_same_v<decltype(&cambc::detail::sdk_enemy_ward_at),
 std::int32_t(*)(std::int32_t, std::int32_t, std::uint32_t*)>);

BC_LAYOUT(cambc::detail::game_settings, Game_Settings);
BC_FIELD(cambc::detail::game_settings, capture_time, Game_Settings, m_captureTime);
BC_FIELD(cambc::detail::game_settings, camp_sleep_time, Game_Settings, m_campSleepTime);
BC_FIELD(cambc::detail::game_settings, respawn_time, Game_Settings, m_respawnTime);
BC_FIELD(cambc::detail::game_settings, recall_time, Game_Settings, m_recallTime);
BC_FIELD(cambc::detail::game_settings, submit_gem_duration_multiplier, Game_Settings, m_submitGemDurationMultiplier);
BC_FIELD(cambc::detail::game_settings, submit_gem_base_duration, Game_Settings, m_submitGemBaseDuration);
BC_FIELD(cambc::detail::game_settings, submit_gem_minimum_duration, Game_Settings, m_submitGemMinimumDuration);
BC_FIELD(cambc::detail::game_settings, submit_gems_range, Game_Settings, m_submitGemsRange);
BC_FIELD(cambc::detail::game_settings, max_rounds, Game_Settings, m_maxRounds);
BC_FIELD(cambc::detail::game_settings, winning_gems, Game_Settings, m_winningGems);
BC_FIELD(cambc::detail::game_settings, kill_gold, Game_Settings, m_killGold);
BC_FIELD(cambc::detail::game_settings, assist_gold, Game_Settings, m_assistGold);
BC_FIELD(cambc::detail::game_settings, passive_gold, Game_Settings, m_passiveGold);
BC_FIELD(cambc::detail::game_settings, spawn_healing, Game_Settings, m_spawnHealing);
BC_FIELD(cambc::detail::game_settings, active_wards, Game_Settings, m_activeWards);
BC_FIELD(cambc::detail::game_settings, ward_charges, Game_Settings, m_wardCharges);
BC_FIELD(cambc::detail::game_settings, shared_memory_bytes, Game_Settings, m_sharedMemoryBytes);
BC_FIELD(cambc::detail::game_settings, draft_memory_bytes, Game_Settings, m_draftMemoryBytes);
BC_FIELD(cambc::detail::game_settings, tile_reveal_rounds, Game_Settings, m_tileRevealRounds);
BC_FIELD(cambc::detail::game_settings, ward_cooldown, Game_Settings, m_wardCooldown);
BC_FIELD(cambc::detail::game_settings, ward_lifetime, Game_Settings, m_wardLifetime);
BC_FIELD(cambc::detail::game_settings, ward_vision_radius, Game_Settings, m_wardVisionRadius);
BC_FIELD(cambc::detail::game_settings, ward_placement_radius, Game_Settings, m_wardPlacementRadius);
BC_FIELD(cambc::detail::game_settings, control_ward_vision_radius, Game_Settings, m_controlWardVisionRadius);
BC_FIELD(cambc::detail::game_settings, control_ward_placement_radius, Game_Settings, m_controlWardPlacementRadius);
BC_FIELD(cambc::detail::game_settings, altar_lock_hits, Game_Settings, m_altarLockHits);
BC_FIELD(cambc::detail::game_settings, tp_scroll_time, Game_Settings, m_tpScrollTime);
BC_FIELD(cambc::detail::game_settings, tp_crystal_time, Game_Settings, m_tpCrystalTime);
BC_FIELD(cambc::detail::game_settings, reserved, Game_Settings, m_reserved);

BC_LAYOUT(cambc::detail::item_info, Game_ItemInfo);
BC_FIELD(cambc::detail::item_info, cost, Game_ItemInfo, m_cost);
BC_FIELD(cambc::detail::item_info, purchasable, Game_ItemInfo, m_purchasable);
BC_FIELD(cambc::detail::item_info, health_points, Game_ItemInfo, m_healthPoints);
BC_FIELD(cambc::detail::item_info, strength_points, Game_ItemInfo, m_strengthPoints);
BC_FIELD(cambc::detail::item_info, focus_points, Game_ItemInfo, m_focusPoints);
BC_FIELD(cambc::detail::item_info, passive, Game_ItemInfo, m_passive);
BC_FIELD(cambc::detail::item_info, passive_magnitude, Game_ItemInfo, m_passiveMagnitude);
BC_FIELD(cambc::detail::item_info, combine_count, Game_ItemInfo, m_combineCount);
BC_FIELD(cambc::detail::item_info, result, Game_ItemInfo, m_result);
BC_FIELD(cambc::detail::item_info, combine_cost, Game_ItemInfo, m_combineCost);
BC_LAYOUT(cambc::detail::consumable_info, Game_ConsumableInfo);
BC_FIELD(cambc::detail::consumable_info, cost, Game_ConsumableInfo, m_cost);
BC_FIELD(cambc::detail::consumable_info, carry_limit, Game_ConsumableInfo, m_carryLimit);
BC_VALUE(cambc::constants::passive_abilities::prism_focus_per_gem, Engine::ChampionPassiveBalance::k_focusPerGem);
BC_VALUE(cambc::constants::passive_abilities::varg_splash_radius, Engine::ChampionPassiveBalance::k_splashRadius);
BC_VALUE(cambc::constants::passive_abilities::vesper_vale_bonus_gold_percent, Engine::ChampionPassiveBalance::k_bonusGoldPercent);
BC_VALUE(cambc::constants::passive_abilities::ryth_health_strength_ratio, Engine::ChampionPassiveBalance::k_healthStrengthRatio);
BC_VALUE(cambc::constants::passive_abilities::ren_retaliation_focus_ratio, Engine::ChampionPassiveBalance::k_retaliationFocusRatio);

BC_ENUM(cambc::passive_type::none, e_Game_Passive_Type_none);
BC_LAYOUT(cambc::passive_type, Game_Passive_Type);
BC_ENUM(cambc::passive_type::bonus_gold, e_Game_Passive_Type_bonusGold);
BC_ENUM(cambc::passive_type::remote_shop, e_Game_Passive_Type_remoteShop);
BC_ENUM(cambc::passive_type::gem_focus, e_Game_Passive_Type_gemFocus);
BC_ENUM(cambc::passive_type::splash_attack, e_Game_Passive_Type_splashAttack);
BC_ENUM(cambc::passive_type::health_strength, e_Game_Passive_Type_healthStrength);
BC_ENUM(cambc::passive_type::reset_cooldowns_on_kill, e_Game_Passive_Type_resetCooldownsOnKill);
BC_ENUM(cambc::passive_type::retaliation, e_Game_Passive_Type_retaliation);
BC_ENUM(cambc::passive_type::stun_immunity, e_Game_Passive_Type_stunImmunity);
// Omnivamp and Heart of Ruin are unreleased and hidden from the SDK; they must stay the last engine values.
BC_VALUE(static_cast<int>(cambc::passive_type::stun_immunity) + 1, e_Game_Passive_Type_omnivamp);
BC_VALUE(e_Game_Passive_Type_omnivamp + 1, e_Game_Passive_Type_numTypes);
BC_VALUE(cambc::buff_restrictions::empty().bits(), e_BuffRestriction_none);
BC_LAYOUT(cambc::buff_restrictions, BuffRestriction);
BC_VALUE(cambc::buff_restrictions::move.bits(), e_BuffRestriction_move);
BC_VALUE(cambc::buff_restrictions::attack.bits(), e_BuffRestriction_attack);
BC_VALUE(cambc::buff_restrictions::cast.bits(), e_BuffRestriction_cast);
BC_VALUE(cambc::buff_restrictions::shop.bits(), e_BuffRestriction_shop);
BC_VALUE(cambc::buff_restrictions::consumable.bits(), e_BuffRestriction_consumable);
BC_VALUE(cambc::buff_restrictions::ward.bits(), e_BuffRestriction_trinket);

// symmetry
BC_ENUM(cambc::symmetry::horizontal, e_Symmetry_horizontal);
BC_ENUM(cambc::symmetry::vertical, e_Symmetry_vertical);
BC_ENUM(cambc::symmetry::rotational180, e_Symmetry_rotational180);

// camp
BC_ENUM(cambc::camp_type::wolf, e_Camp_wolf);
BC_ENUM(cambc::camp_type::elk, e_Camp_elk);
BC_ENUM(cambc::camp_type::gem_cluster, e_Camp_gemCluster);
BC_ENUM(cambc::camp_type::gold_cluster, e_Camp_goldCluster);
BC_ENUM(cambc::camp_type::monolith, e_Camp_monolith);

// champion_draft
BC_ENUM(cambc::champion_draft::ryth, e_ChampionDraft_ryth);
BC_ENUM(cambc::champion_draft::vesper_vale, e_ChampionDraft_vesperVale);
BC_ENUM(cambc::champion_draft::vendetta, e_ChampionDraft_vendetta);
BC_ENUM(cambc::champion_draft::cassi_and_issac, e_ChampionDraft_cassi_and_issac);
BC_ENUM(cambc::champion_draft::stellar, e_ChampionDraft_stellar);
BC_ENUM(cambc::champion_draft::prism, e_ChampionDraft_prism);
BC_ENUM(cambc::champion_draft::marcella, e_ChampionDraft_marcella);
BC_ENUM(cambc::champion_draft::willow, e_ChampionDraft_willow);
BC_ENUM(cambc::champion_draft::ren, e_ChampionDraft_ren);
BC_ENUM(cambc::champion_draft::varg, e_ChampionDraft_varg);
BC_ENUM(cambc::champion_draft::tavi, e_ChampionDraft_tavi);

// champion_type
BC_ENUM(cambc::champion_type::ryth, e_Champion_ryth);
BC_ENUM(cambc::champion_type::vesper_vale, e_Champion_vesperVale);
BC_ENUM(cambc::champion_type::vendetta, e_Champion_vendetta);
BC_ENUM(cambc::champion_type::cassi, e_Champion_cassi);
BC_ENUM(cambc::champion_type::issac, e_Champion_issac);
BC_ENUM(cambc::champion_type::stellar, e_Champion_stellar);
BC_ENUM(cambc::champion_type::prism, e_Champion_prism);
BC_ENUM(cambc::champion_type::marcella, e_Champion_marcella);
BC_ENUM(cambc::champion_type::willow, e_Champion_willow);
BC_ENUM(cambc::champion_type::ren, e_Champion_ren);
BC_ENUM(cambc::champion_type::varg, e_Champion_varg);
BC_ENUM(cambc::champion_type::tavi, e_Champion_tavi);

// ability_kind
BC_ENUM(cambc::ability_kind::attack_reset, e_Game_Ability_Type_attackReset);
BC_ENUM(cambc::ability_kind::damage, e_Game_Ability_Type_damage);
BC_ENUM(cambc::ability_kind::aoe_damage, e_Game_Ability_Type_aoeDamage);
BC_ENUM(cambc::ability_kind::cc, e_Game_Ability_Type_cc);
BC_ENUM(cambc::ability_kind::heal, e_Game_Ability_Type_heal);
BC_ENUM(cambc::ability_kind::dash, e_Game_Ability_Type_dash);
BC_ENUM(cambc::ability_kind::shield, e_Game_Ability_Type_shield);

// item_type
BC_ENUM(cambc::item_type::monster_shard_of_strength, e_Item_monsterShardOfStrength);
BC_ENUM(cambc::item_type::monster_emblem_of_strength, e_Item_monsterEmblemOfStrength);
BC_ENUM(cambc::item_type::monster_shard_of_focus, e_Item_monsterShardOfFocus);
BC_ENUM(cambc::item_type::monster_emblem_of_focus, e_Item_monsterEmblemOfFocus);
BC_ENUM(cambc::item_type::monster_shard_of_health, e_Item_monsterShardOfHealth);
BC_ENUM(cambc::item_type::monster_emblem_of_health, e_Item_monsterEmblemOfHealth);
BC_ENUM(cambc::item_type::power_shard_of_strength, e_Item_powerShardOfStrength);
BC_ENUM(cambc::item_type::power_emblem_of_strength, e_Item_powerEmblemOfStrength);
BC_ENUM(cambc::item_type::power_shard_of_focus, e_Item_powerShardOfFocus);
BC_ENUM(cambc::item_type::power_emblem_of_focus, e_Item_powerEmblemOfFocus);
BC_ENUM(cambc::item_type::power_shard_of_health, e_Item_powerShardOfHealth);
BC_ENUM(cambc::item_type::power_emblem_of_health, e_Item_powerEmblemOfHealth);
BC_ENUM(cambc::item_type::durability_shard_of_strength, e_Item_durabilityShardOfStrength);
BC_ENUM(cambc::item_type::durability_emblem_of_strength, e_Item_durabilityEmblemOfStrength);
BC_ENUM(cambc::item_type::durability_shard_of_focus, e_Item_durabilityShardOfFocus);
BC_ENUM(cambc::item_type::durability_emblem_of_focus, e_Item_durabilityEmblemOfFocus);
BC_ENUM(cambc::item_type::durability_shard_of_health, e_Item_durabilityShardOfHealth);
BC_ENUM(cambc::item_type::durability_emblem_of_health, e_Item_durabilityEmblemOfHealth);
BC_ENUM(cambc::item_type::treasure_shard_of_strength, e_Item_treasureShardOfStrength);
BC_ENUM(cambc::item_type::treasure_emblem_of_strength, e_Item_treasureEmblemOfStrength);
BC_ENUM(cambc::item_type::treasure_shard_of_focus, e_Item_treasureShardOfFocus);
BC_ENUM(cambc::item_type::treasure_emblem_of_focus, e_Item_treasureEmblemOfFocus);
BC_ENUM(cambc::item_type::treasure_shard_of_health, e_Item_treasureShardOfHealth);
BC_ENUM(cambc::item_type::treasure_emblem_of_health, e_Item_treasureEmblemOfHealth);
BC_ENUM(cambc::item_type::jungle_shard_of_strength, e_Item_jungleShardOfStrength);
BC_ENUM(cambc::item_type::jungle_emblem_of_strength, e_Item_jungleEmblemOfStrength);
BC_ENUM(cambc::item_type::jungle_shard_of_focus, e_Item_jungleShardOfFocus);
BC_ENUM(cambc::item_type::jungle_emblem_of_focus, e_Item_jungleEmblemOfFocus);
BC_ENUM(cambc::item_type::jungle_shard_of_health, e_Item_jungleShardOfHealth);
BC_ENUM(cambc::item_type::jungle_emblem_of_health, e_Item_jungleEmblemOfHealth);
BC_ENUM(cambc::item_type::home_shard_of_strength, e_Item_homeShardOfStrength);
BC_ENUM(cambc::item_type::home_emblem_of_strength, e_Item_homeEmblemOfStrength);
BC_ENUM(cambc::item_type::home_shard_of_focus, e_Item_homeShardOfFocus);
BC_ENUM(cambc::item_type::home_emblem_of_focus, e_Item_homeEmblemOfFocus);
BC_ENUM(cambc::item_type::home_shard_of_health, e_Item_homeShardOfHealth);
BC_ENUM(cambc::item_type::home_emblem_of_health, e_Item_homeEmblemOfHealth);
BC_ENUM(cambc::item_type::bruiser_shard_of_strength, e_Item_bruiserShardOfStrength);
BC_ENUM(cambc::item_type::bruiser_emblem_of_strength, e_Item_bruiserEmblemOfStrength);
BC_ENUM(cambc::item_type::bruiser_shard_of_focus, e_Item_bruiserShardOfFocus);
BC_ENUM(cambc::item_type::bruiser_emblem_of_focus, e_Item_bruiserEmblemOfFocus);
BC_ENUM(cambc::item_type::bruiser_shard_of_health, e_Item_bruiserShardOfHealth);
BC_ENUM(cambc::item_type::bruiser_emblem_of_health, e_Item_bruiserEmblemOfHealth);
BC_ENUM(cambc::item_type::count, e_Item_numTypes);

// consumable_type
static_assert(cambc::detail::raw_consumable_ids[0] == e_Game_Consumable_tpCrystal);
static_assert(cambc::detail::raw_consumable_ids[1] == e_Game_Consumable_altarLock);

// entity_kind
BC_ENUM(cambc::detail::entity_kind::champion, e_Game_EntityKind_champion);
BC_ENUM(cambc::detail::entity_kind::camp, e_Game_EntityKind_camp);
BC_ENUM(cambc::detail::entity_kind::altar_lock, e_Game_EntityKind_altarLock);

// error_code
BC_ENUM(cambc::error_code::success, e_ErrorCode_success);
BC_ENUM(cambc::error_code::invalid_handle, e_ErrorCode_invalidHandle);
BC_ENUM(cambc::error_code::out_of_memory, e_ErrorCode_outOfMemory);
BC_ENUM(cambc::error_code::file_not_found, e_ErrorCode_fileNotFound);
BC_ENUM(cambc::error_code::null_pointer, e_ErrorCode_nullptr);
BC_ENUM(cambc::error_code::invalid_argument, e_ErrorCode_invalidArgument);
BC_ENUM(cambc::error_code::map_parse_error, e_ErrorCode_mapParseError);
BC_ENUM(cambc::error_code::invalid_map_file, e_ErrorCode_invalidMapFile);
BC_ENUM(cambc::error_code::invalid_map_version, e_ErrorCode_invalidMapVersion);
BC_ENUM(cambc::error_code::replay_parse_error, e_ErrorCode_replayParseError);
BC_ENUM(cambc::error_code::invalid_replay_file, e_ErrorCode_invalidReplayFile);
BC_ENUM(cambc::error_code::invalid_replay_version, e_ErrorCode_invalidReplayVersion);
BC_ENUM(cambc::error_code::invalid_draft, e_ErrorCode_invalidDraft);
BC_ENUM(cambc::error_code::draft_not_drafting, e_ErrorCode_draft_notDrafting);
BC_ENUM(cambc::error_code::draft_duplicate_champion, e_ErrorCode_draft_duplicateChampion);
BC_ENUM(cambc::error_code::draft_invalid_round, e_ErrorCode_draft_invalidRound);
BC_ENUM(cambc::error_code::draft_unavailable, e_ErrorCode_draft_unavailable);
BC_ENUM(cambc::error_code::already_initialized, e_ErrorCode_alreadyInitialized);
BC_ENUM(cambc::error_code::invalid_action, e_ErrorCode_invalidAction);
BC_ENUM(cambc::error_code::invalid_action_memory_bad_range, e_ErrorCode_invalidAction_memory_badRange);
BC_ENUM(cambc::error_code::invalid_action_memory_draft_frozen, e_ErrorCode_invalidAction_memory_draftFrozen);
BC_ENUM(cambc::error_code::invalid_action_move_cant_move, e_ErrorCode_invalidAction_move_cantMove);
BC_ENUM(cambc::error_code::invalid_action_move_bad_destination, e_ErrorCode_invalidAction_move_badDestination);
BC_ENUM(cambc::error_code::invalid_action_spell_cast_invalid_source, e_ErrorCode_invalidAction_spellCast_invalidSource);
BC_ENUM(cambc::error_code::invalid_action_spell_cast_invalid_slot, e_ErrorCode_invalidAction_spellCast_invalidSlot);
BC_ENUM(cambc::error_code::invalid_action_spell_cast_no_ability, e_ErrorCode_invalidAction_spellCast_noAbility);
BC_ENUM(cambc::error_code::invalid_action_spell_cast_cant_cast, e_ErrorCode_invalidAction_spellCast_cantCast);
BC_ENUM(cambc::error_code::invalid_action_spell_cast_already_cast, e_ErrorCode_invalidAction_spellCast_alreadyCast);
BC_ENUM(cambc::error_code::invalid_action_spell_cast_on_cooldown, e_ErrorCode_invalidAction_spellCast_onCooldown);
BC_ENUM(cambc::error_code::invalid_action_spell_cast_no_charges, e_ErrorCode_invalidAction_spellCast_noCharges);
BC_ENUM(cambc::error_code::invalid_action_spell_cast_bad_target, e_ErrorCode_invalidAction_spellCast_badTarget);
BC_ENUM(cambc::error_code::invalid_action_spell_cast_out_of_range, e_ErrorCode_invalidAction_spellCast_outOfRange);
BC_ENUM(cambc::error_code::invalid_action_spell_cast_bad_destination, e_ErrorCode_invalidAction_spellCast_badDestination);
BC_ENUM(cambc::error_code::invalid_action_cant_act, e_ErrorCode_invalidAction_cantAct);
BC_ENUM(cambc::error_code::invalid_action_is_recalling, e_ErrorCode_invalidAction_isRecalling);
BC_ENUM(cambc::error_code::invalid_action_is_submitting_gems, e_ErrorCode_invalidAction_isSubmittingGems);
BC_ENUM(cambc::error_code::invalid_action_move_already_moved, e_ErrorCode_invalidAction_move_alreadyMoved);
BC_ENUM(cambc::error_code::invalid_action_move_out_of_range, e_ErrorCode_invalidAction_move_outOfRange);
BC_ENUM(cambc::error_code::invalid_action_move_unreachable, e_ErrorCode_invalidAction_move_unreachable);
BC_ENUM(cambc::error_code::invalid_action_attack_invalid_source, e_ErrorCode_invalidAction_attack_invalidSource);
BC_ENUM(cambc::error_code::invalid_action_attack_cant_attack, e_ErrorCode_invalidAction_attack_cantAttack);
BC_ENUM(cambc::error_code::invalid_action_attack_already_attacked, e_ErrorCode_invalidAction_attack_alreadyAttacked);
BC_ENUM(cambc::error_code::invalid_action_attack_bad_target, e_ErrorCode_invalidAction_attack_badTarget);
BC_ENUM(cambc::error_code::invalid_action_attack_out_of_range, e_ErrorCode_invalidAction_attack_outOfRange);
BC_ENUM(cambc::error_code::invalid_action_recall_cant_channel, e_ErrorCode_invalidAction_recall_cantChannel);
BC_ENUM(cambc::error_code::invalid_action_submit_gems_cant_channel, e_ErrorCode_invalidAction_submitGems_cantChannel);
BC_ENUM(cambc::error_code::invalid_action_submit_gems_no_gems, e_ErrorCode_invalidAction_submitGems_noGems);
BC_ENUM(cambc::error_code::invalid_action_submit_gems_bad_altar, e_ErrorCode_invalidAction_submitGems_badAltar);
BC_ENUM(cambc::error_code::invalid_action_submit_gems_out_of_range, e_ErrorCode_invalidAction_submitGems_outOfRange);
BC_ENUM(cambc::error_code::invalid_action_buy_item_not_on_spawn, e_ErrorCode_invalidAction_buyItem_notOnSpawn);
BC_ENUM(cambc::error_code::invalid_action_buy_item_invalid_item, e_ErrorCode_invalidAction_buyItem_invalidItem);
BC_ENUM(cambc::error_code::invalid_action_buy_item_invalid_count, e_ErrorCode_invalidAction_buyItem_invalidCount);
BC_ENUM(cambc::error_code::invalid_action_buy_item_not_enough_gold, e_ErrorCode_invalidAction_buyItem_notEnoughGold);
BC_ENUM(cambc::error_code::invalid_action_buy_item_inventory_full, e_ErrorCode_invalidAction_buyItem_inventoryFull);
BC_ENUM(cambc::error_code::invalid_action_buy_item_stats_overflow, e_ErrorCode_invalidAction_buyItem_statsOverflow);
BC_ENUM(cambc::error_code::invalid_action_buy_consumable_not_on_spawn, e_ErrorCode_invalidAction_buyConsumable_notOnSpawn);
BC_ENUM(cambc::error_code::invalid_action_buy_consumable_invalid_type, e_ErrorCode_invalidAction_buyConsumable_invalidType);
BC_ENUM(cambc::error_code::invalid_action_buy_consumable_invalid_count, e_ErrorCode_invalidAction_buyConsumable_invalidCount);
BC_ENUM(cambc::error_code::invalid_action_buy_consumable_inventory_full, e_ErrorCode_invalidAction_buyConsumable_inventoryFull);
BC_ENUM(cambc::error_code::invalid_action_buy_consumable_not_enough_gold, e_ErrorCode_invalidAction_buyConsumable_notEnoughGold);
BC_ENUM(cambc::error_code::invalid_action_use_consumable_invalid_type, e_ErrorCode_invalidAction_useConsumable_invalidType);
BC_ENUM(cambc::error_code::invalid_action_use_consumable_not_owned, e_ErrorCode_invalidAction_useConsumable_notOwned);
BC_ENUM(cambc::error_code::invalid_action_place_ward_no_charges, e_ErrorCode_invalidAction_placeWard_noCharges);
BC_ENUM(cambc::error_code::invalid_action_place_ward_cant_place, e_ErrorCode_invalidAction_placeWard_cantPlace);
BC_ENUM(cambc::error_code::invalid_action_place_ward_bad_destination, e_ErrorCode_invalidAction_placeWard_badDestination);
BC_ENUM(cambc::error_code::invalid_action_place_ward_out_of_range, e_ErrorCode_invalidAction_placeWard_outOfRange);
BC_ENUM(cambc::error_code::map_set_team_aspawn_area_bad_position, e_ErrorCode_map_setTeamASpawnArea_badPosition);
BC_ENUM(cambc::error_code::map_add_bush_bad_position, e_ErrorCode_map_addBush_badPosition);
BC_ENUM(cambc::error_code::map_remove_bush_bad_position, e_ErrorCode_map_removeBush_badPosition);
BC_ENUM(cambc::error_code::map_add_wall_bad_position, e_ErrorCode_map_addWall_badPosition);
BC_ENUM(cambc::error_code::map_remove_wall_bad_position, e_ErrorCode_map_removeWall_badPosition);
BC_ENUM(cambc::error_code::map_add_hole_bad_position, e_ErrorCode_map_addHole_badPosition);
BC_ENUM(cambc::error_code::map_remove_hole_bad_position, e_ErrorCode_map_removeHole_badPosition);
BC_ENUM(cambc::error_code::map_add_altar_bad_position, e_ErrorCode_map_addAltar_badPosition);
BC_ENUM(cambc::error_code::map_remove_altar_bad_position, e_ErrorCode_map_removeAltar_badPosition);
BC_ENUM(cambc::error_code::map_get_bush_bad_position, e_ErrorCode_map_getBush_badPosition);
BC_ENUM(cambc::error_code::map_get_wall_bad_position, e_ErrorCode_map_getWall_badPosition);
BC_ENUM(cambc::error_code::map_get_hole_bad_position, e_ErrorCode_map_getHole_badPosition);
BC_ENUM(cambc::error_code::map_get_altar_bad_position, e_ErrorCode_map_getAltar_badPosition);
BC_ENUM(cambc::error_code::map_get_camp_handle_at_bad_position, e_ErrorCode_map_getCampHandleAt_badPosition);
BC_ENUM(cambc::error_code::map_add_camp_bad_position, e_ErrorCode_map_addCamp_badPosition);
BC_ENUM(cambc::error_code::map_set_camp_bad_position, e_ErrorCode_map_setCamp_badPosition);
BC_ENUM(cambc::error_code::invalid_action_champion_teleport_bad_destination, e_ErrorCode_invalidAction_championTeleport_badDestination);
BC_ENUM(cambc::error_code::invalid_action_respawn_group_blocked_spawn, e_ErrorCode_invalidAction_respawnGroup_blockedSpawn);
BC_ENUM(cambc::error_code::invalid_action_camp_teleport_bad_destination, e_ErrorCode_invalidAction_campTeleport_badDestination);
BC_ENUM(cambc::error_code::timeline_bad_seek_position, e_ErrorCode_timeline_badSeekPosition);
BC_ENUM(cambc::error_code::timeline_bad_seek_substep, e_ErrorCode_timeline_badSeekSubstep);
BC_ENUM(cambc::error_code::api_no_map, e_ErrorCode_api_noMap);
BC_ENUM(cambc::error_code::managed_resources_bad_position, e_ErrorCode_managedResources_badPosition);
BC_ENUM(cambc::error_code::managed_resources_bad_radius, e_ErrorCode_managedResources_badRadius);
BC_ENUM(cambc::error_code::managed_resources_aliasing, e_ErrorCode_managedResources_aliasing);
BC_ENUM(cambc::error_code::managed_resources_metadata_mismatch, e_ErrorCode_managedResources_metadataMismatch);
BC_ENUM(cambc::error_code::managed_resources_opaque_origin, e_ErrorCode_managedResources_opaqueOrigin);
BC_ENUM(cambc::error_code::api_invalid_team, e_ErrorCode_api_invalidTeam);
BC_ENUM(cambc::error_code::api_invalid_perspective, e_ErrorCode_api_invalidPerspective);
BC_ENUM(cambc::error_code::api_not_initialized, e_ErrorCode_api_notInitialized);
BC_ENUM(cambc::error_code::api_bad_position, e_ErrorCode_api_badPosition);
BC_ENUM(cambc::error_code::api_not_in_vision, e_ErrorCode_api_notInVision);
BC_ENUM(cambc::error_code::api_not_found, e_ErrorCode_api_notFound);
BC_ENUM(cambc::error_code::api_invalid_camp_group, e_ErrorCode_api_invalidCampGroup);
BC_ENUM(cambc::error_code::api_invalid_camp_info, e_ErrorCode_api_invalidCampInfo);
BC_ENUM(cambc::error_code::api_invalid_camp, e_ErrorCode_api_invalidCamp);
BC_ENUM(cambc::error_code::api_invalid_champion, e_ErrorCode_api_invalidChampion);
BC_ENUM(cambc::error_code::api_invalid_ward, e_ErrorCode_api_invalidWard);
BC_ENUM(cambc::error_code::api_invalid_ability, e_ErrorCode_api_invalidAbility);
BC_ENUM(cambc::error_code::api_not_teammate, e_ErrorCode_api_notTeammate);
BC_ENUM(cambc::error_code::api_camp_group_not_waiting_to_respawn, e_ErrorCode_api_campGroupNotWaitingToRespawn);
BC_ENUM(cambc::error_code::api_champion_alive, e_ErrorCode_api_championAlive);
BC_ENUM(cambc::error_code::api_invalid_champion_type, e_ErrorCode_api_invalidChampionType);
BC_ENUM(cambc::error_code::api_invalid_camp_type, e_ErrorCode_api_invalidCampType);
BC_ENUM(cambc::error_code::api_value_out_of_range, e_ErrorCode_api_valueOutOfRange);
BC_ENUM(cambc::error_code::api_buffer_too_small, e_ErrorCode_api_bufferTooSmall);
BC_ENUM(cambc::error_code::api_not_owner, e_ErrorCode_api_notOwner);
BC_ENUM(cambc::error_code::invalid_action_recall_already_used, e_ErrorCode_invalidAction_recall_alreadyUsed);
BC_ENUM(cambc::error_code::invalid_action_submit_gems_already_used, e_ErrorCode_invalidAction_submitGems_alreadyUsed);
BC_ENUM(cambc::error_code::invalid_action_cancel_channel_not_channeling, e_ErrorCode_invalidAction_cancelChannel_notChanneling);
BC_ENUM(cambc::error_code::invalid_action_cancel_channel_already_used, e_ErrorCode_invalidAction_cancelChannel_alreadyUsed);
BC_ENUM(cambc::error_code::invalid_action_place_ward_ally_ward_present, e_ErrorCode_invalidAction_placeWard_allyWardPresent);
BC_ENUM(cambc::error_code::invalid_action_altar_lock_bad_altar, e_ErrorCode_invalidAction_altarLock_badAltar);
BC_ENUM(cambc::error_code::invalid_action_altar_lock_out_of_range, e_ErrorCode_invalidAction_altarLock_outOfRange);
BC_ENUM(cambc::error_code::invalid_action_altar_lock_already_locked, e_ErrorCode_invalidAction_altarLock_alreadyLocked);
BC_ENUM(cambc::error_code::invalid_action_is_teleporting, e_ErrorCode_invalidAction_isTeleporting);
BC_ENUM(cambc::error_code::invalid_action_cancel_channel_not_cancellable, e_ErrorCode_invalidAction_cancelChannel_notCancellable);
BC_ENUM(cambc::error_code::invalid_action_submit_gems_altar_locked, e_ErrorCode_invalidAction_submitGems_altarLocked);
BC_ENUM(cambc::error_code::invalid_action_spell_cast_already_moved, e_ErrorCode_invalidAction_spellCast_alreadyMoved);
BC_ENUM(cambc::error_code::invalid_action_move_exclusive_spell_cast, e_ErrorCode_invalidAction_move_exclusiveSpellCast);
BC_ENUM(cambc::error_code::invalid_action_sell_not_tavi, e_ErrorCode_invalidAction_sell_notTavi);
BC_ENUM(cambc::error_code::invalid_action_sell_invalid_item, e_ErrorCode_invalidAction_sell_invalidItem);
BC_ENUM(cambc::error_code::invalid_action_sell_not_owned, e_ErrorCode_invalidAction_sell_notOwned);
BC_ENUM(cambc::error_code::invalid_action_sell_already_sold, e_ErrorCode_invalidAction_sell_alreadySold);
BC_ENUM(cambc::error_code::invalid_action_sell_gold_overflow, e_ErrorCode_invalidAction_sell_goldOverflow);
BC_ENUM(cambc::error_code::invalid_action_sell_not_on_spawn, e_ErrorCode_invalidAction_sell_notOnSpawn);

BC_LAYOUT(cambc::detail::aabb, Aabb);
BC_FIELD(cambc::detail::aabb, min_x, Aabb, m_minX);
BC_FIELD(cambc::detail::aabb, min_y, Aabb, m_minY);
BC_FIELD(cambc::detail::aabb, max_x, Aabb, m_maxX);
BC_FIELD(cambc::detail::aabb, max_y, Aabb, m_maxY);

BC_LAYOUT(cambc::detail::camp_group_info, Game_CampGroup);
BC_FIELD(cambc::detail::camp_group_info, gems, Game_CampGroup, m_gemReward);
BC_FIELD(cambc::detail::camp_group_info, gold, Game_CampGroup, m_goldReward);

BC_LAYOUT(cambc::detail::camp_info, Game_CampInfo);
BC_FIELD(cambc::detail::camp_info, type, Game_CampInfo, m_type);
BC_FIELD(cambc::detail::camp_info, x, Game_CampInfo, m_x);
BC_FIELD(cambc::detail::camp_info, y, Game_CampInfo, m_y);

BC_LAYOUT(cambc::detail::camp_state, Game_Camp);
BC_FIELD(cambc::detail::camp_state, x, Game_Camp, m_x);
BC_FIELD(cambc::detail::camp_state, y, Game_Camp, m_y);
BC_FIELD(cambc::detail::camp_state, health, Game_Camp, m_health);
BC_FIELD(cambc::detail::camp_state, max_health, Game_Camp, m_maxHealth);
BC_FIELD(cambc::detail::camp_state, strength, Game_Camp, m_strength);
BC_FIELD(cambc::detail::camp_state, focus, Game_Camp, m_focus);
static_assert(offsetof(cambc::detail::camp_state, cooldowns) == offsetof(Game_Camp, m_ability0Cooldown));
static_assert(offsetof(cambc::detail::camp_state, cooldowns) + sizeof(std::int32_t) == offsetof(Game_Camp, m_ability1Cooldown));

BC_LAYOUT(cambc::detail::camp_base_stats, Game_CampBaseStats);
BC_FIELD(cambc::detail::camp_base_stats, vision_radius, Game_CampBaseStats, m_visionRadius);
BC_FIELD(cambc::detail::camp_base_stats, base_move_speed, Game_CampBaseStats, m_baseMoveSpeed);
BC_FIELD(cambc::detail::camp_base_stats, base_attack_range, Game_CampBaseStats, m_baseAttackRange);
BC_FIELD(cambc::detail::camp_base_stats, base_health, Game_CampBaseStats, m_baseHealth);
BC_FIELD(cambc::detail::camp_base_stats, health_growth, Game_CampBaseStats, m_healthGrowth);
BC_FIELD(cambc::detail::camp_base_stats, base_strength, Game_CampBaseStats, m_baseStrength);
BC_FIELD(cambc::detail::camp_base_stats, strength_growth, Game_CampBaseStats, m_strengthGrowth);
BC_FIELD(cambc::detail::camp_base_stats, base_focus, Game_CampBaseStats, m_baseFocus);
BC_FIELD(cambc::detail::camp_base_stats, focus_growth, Game_CampBaseStats, m_focusGrowth);
BC_LAYOUT(cambc::detail::camp_design, Game_CampDesign);
BC_FIELD(cambc::detail::camp_design, definition, Game_CampDesign, m_definition);
BC_FIELD(cambc::detail::camp_design, base_stats, Game_CampDesign, m_baseStats);

BC_LAYOUT(cambc::detail::champion_info, Game_Champion);
BC_FIELD(cambc::detail::champion_info, team, Game_Champion, m_team);
BC_FIELD(cambc::detail::champion_info, type, Game_Champion, m_champion);
BC_FIELD(cambc::detail::champion_info, x, Game_Champion, m_x);
BC_FIELD(cambc::detail::champion_info, y, Game_Champion, m_y);
BC_FIELD(cambc::detail::champion_info, health, Game_Champion, m_health);
BC_FIELD(cambc::detail::champion_info, max_health, Game_Champion, m_maxHealth);
BC_FIELD(cambc::detail::champion_info, strength, Game_Champion, m_strength);
BC_FIELD(cambc::detail::champion_info, focus, Game_Champion, m_focus);
BC_FIELD(cambc::detail::champion_info, move_speed, Game_Champion, m_moveSpeed);
BC_FIELD(cambc::detail::champion_info, attack_range, Game_Champion, m_attackRange);
BC_FIELD(cambc::detail::champion_info, state, Game_Champion, m_state);

BC_LAYOUT(cambc::detail::champion_base_stats, Game_ChampionBaseStats);
BC_FIELD(cambc::detail::champion_base_stats, vision_radius, Game_ChampionBaseStats, m_visionRadius);
BC_FIELD(cambc::detail::champion_base_stats, base_move_speed, Game_ChampionBaseStats, m_baseMoveSpeed);
BC_FIELD(cambc::detail::champion_base_stats, base_attack_range, Game_ChampionBaseStats, m_baseAttackRange);
BC_FIELD(cambc::detail::champion_base_stats, base_health, Game_ChampionBaseStats, m_baseHealth);
BC_FIELD(cambc::detail::champion_base_stats, health_growth, Game_ChampionBaseStats, m_healthGrowth);
BC_FIELD(cambc::detail::champion_base_stats, base_strength, Game_ChampionBaseStats, m_baseStrength);
BC_FIELD(cambc::detail::champion_base_stats, strength_growth, Game_ChampionBaseStats, m_strengthGrowth);
BC_FIELD(cambc::detail::champion_base_stats, base_focus, Game_ChampionBaseStats, m_baseFocus);
BC_FIELD(cambc::detail::champion_base_stats, focus_growth, Game_ChampionBaseStats, m_focusGrowth);
BC_FIELD(cambc::detail::champion_base_stats, camp_damage_multiplier, Game_ChampionBaseStats, m_campDamageMultiplier);

BC_LAYOUT(cambc::detail::champion_design, Game_ChampionDesign);
BC_FIELD(cambc::detail::champion_design, definition, Game_ChampionDesign, m_definition);
BC_FIELD(cambc::detail::champion_design, base_stats, Game_ChampionDesign, m_baseStats);

BC_LAYOUT(cambc::detail::unit_definition, Game_ChampionDefinition);
BC_FIELD(cambc::detail::unit_definition, type, Game_ChampionDefinition, m_champion);
BC_FIELD(cambc::detail::unit_definition, vision_type, Game_ChampionDefinition, m_vision);
BC_FIELD(cambc::detail::unit_definition, passive_type, Game_ChampionDefinition, m_passive);
BC_FIELD(cambc::detail::unit_definition, abilities, Game_ChampionDefinition, m_abilities);
BC_FIELD(cambc::detail::unit_definition, attack_type, Game_ChampionDefinition, m_attackType);
BC_LAYOUT(cambc::detail::unit_definition, Game_CampDefinition);

BC_LAYOUT(cambc::detail::position, Game_Position);
BC_FIELD(cambc::detail::position, x, Game_Position, m_x);
BC_FIELD(cambc::detail::position, y, Game_Position, m_y);
BC_LAYOUT(cambc::detail::ward_resources, Game_TrinketState);
BC_FIELD(cambc::detail::ward_resources, charges, Game_TrinketState, m_charges);
BC_FIELD(cambc::detail::ward_resources, cooldown, Game_TrinketState, m_rechargeTimer);
BC_LAYOUT(cambc::detail::ward_info, Game_Ward);
BC_FIELD(cambc::detail::ward_info, team, Game_Ward, m_team);
BC_FIELD(cambc::detail::ward_info, x, Game_Ward, m_x);
BC_FIELD(cambc::detail::ward_info, y, Game_Ward, m_y);
BC_FIELD(cambc::detail::ward_info, timer, Game_Ward, m_timer);
BC_LAYOUT(cambc::detail::control_ward_info, Game_ControlWard);
BC_FIELD(cambc::detail::control_ward_info, team, Game_ControlWard, m_team);
BC_FIELD(cambc::detail::control_ward_info, x, Game_ControlWard, m_x);
BC_FIELD(cambc::detail::control_ward_info, y, Game_ControlWard, m_y);
BC_FIELD(cambc::detail::control_ward_info, timer, Game_ControlWard, m_timer);
BC_LAYOUT(cambc::detail::teleport_info, Game_Teleport);
BC_FIELD(cambc::detail::teleport_info, champion, Game_Teleport, m_champion);
BC_FIELD(cambc::detail::teleport_info, type, Game_Teleport, m_type);
BC_FIELD(cambc::detail::teleport_info, team, Game_Teleport, m_team);
BC_FIELD(cambc::detail::teleport_info, destination, Game_Teleport, m_destination);
BC_FIELD(cambc::detail::teleport_info, timer, Game_Teleport, m_timer);
BC_FIELD(cambc::detail::teleport_info, channel_type, Game_Teleport, m_channelType);
BC_ENUM(cambc::item_passive::monster, e_Game_ItemPassive_monster);
BC_ENUM(cambc::item_passive::durability, e_Game_ItemPassive_durability);
BC_ENUM(cambc::item_passive::treasure, e_Game_ItemPassive_treasure);
BC_ENUM(cambc::item_passive::jungle, e_Game_ItemPassive_jungle);
BC_ENUM(cambc::item_passive::home, e_Game_ItemPassive_home);
BC_VALUE(static_cast<int>(cambc::item_passive::home) + 1, e_Game_ItemPassive_numTypes);
BC_LAYOUT(cambc::detail::altar_lock_info, Game_AltarLock);
static_assert(offsetof(cambc::detail::altar_lock_info, kind) == offsetof(Game_AltarLock, m_entity));
static_assert(offsetof(cambc::detail::altar_lock_info, handle) == offsetof(Game_AltarLock, m_entity) + offsetof(Game_EntityRef, m_index));
BC_FIELD(cambc::detail::altar_lock_info, team, Game_AltarLock, m_team);
static_assert(offsetof(cambc::detail::altar_lock_info, x) == offsetof(Game_AltarLock, m_position));
static_assert(offsetof(cambc::detail::altar_lock_info, y) == offsetof(Game_AltarLock, m_position) + offsetof(Game_Position, m_y));
BC_FIELD(cambc::detail::altar_lock_info, hits_remaining, Game_AltarLock, m_hitsRemaining);
BC_LAYOUT(cambc::detail::camp_buff_data, Game_Buff);
BC_FIELD(cambc::detail::camp_buff_data, type, Game_Buff, m_type);
BC_FIELD(cambc::detail::camp_buff_data, magnitude, Game_Buff, m_magnitude);
BC_FIELD(cambc::detail::camp_buff_data, timer, Game_Buff, m_timer);
static_assert(e_Game_CampGroupState_hasCamps == 1);
static_assert(e_Game_CampGroupState_waitingForRewardClaim == 2);
static_assert(e_Game_CampGroupState_waitingToRespawn == 3);

BC_LAYOUT(cambc::detail::buff_data, Game_BuffData);
BC_FIELD(cambc::detail::buff_data, type, Game_BuffData, m_type);
BC_FIELD(cambc::detail::buff_data, data, Game_BuffData, m_data);
BC_FIELD(cambc::detail::buff_data, data.shield.duration, Game_BuffData, m_data.m_shield.m_duration);
BC_FIELD(cambc::detail::buff_data, data.shield.value, Game_BuffData, m_data.m_shield.m_value);
BC_FIELD(cambc::detail::buff_data, data.stun.duration, Game_BuffData, m_data.m_stun.m_duration);
BC_FIELD(cambc::detail::buff_data, data.enhanced_attack.duration, Game_BuffData, m_data.m_enhancedAuto.m_duration);

BC_VALUE(cambc::detail::ability_target_camps, e_Game_Ability_Target_camps);
BC_VALUE(cambc::detail::ability_target_enemies, e_Game_Ability_Target_enemies);
BC_VALUE(cambc::detail::ability_target_allies, e_Game_Ability_Target_allies);
BC_VALUE(cambc::detail::ability_target_self, e_Game_Ability_Target_self);
BC_VALUE(cambc::detail::ability_target_ground, e_Game_Ability_Target_ground);
BC_VALUE(cambc::ability_target_mask::camps.bits(), e_Game_Ability_Target_camps);
BC_VALUE(cambc::ability_target_mask::enemies.bits(), e_Game_Ability_Target_enemies);
BC_VALUE(cambc::ability_target_mask::allies.bits(), e_Game_Ability_Target_allies);
BC_VALUE(cambc::ability_target_mask::self.bits(), e_Game_Ability_Target_self);
BC_VALUE(cambc::ability_target_mask::ground.bits(), e_Game_Ability_Target_ground);
BC_VALUE(cambc::detail::ability_attribute_exclusive_with_movement, e_Game_Ability_Attributes_exclusiveWithMovement);
BC_VALUE(cambc::champion_state::alive.bits(), e_Champion_State_alive);
BC_VALUE(cambc::champion_state::recalling.bits(), e_Champion_State_isRecalling);
BC_VALUE(cambc::champion_state::submitting_gems.bits(), e_Champion_State_isSubmittingGems);
BC_VALUE(cambc::champion_state::can_move.bits(), e_Champion_State_canMove);
BC_VALUE(cambc::champion_state::can_attack.bits(), e_Champion_State_canAttack);
BC_VALUE(cambc::champion_state::completed_turn.bits(), e_Champion_State_hasCompletedTurn);
BC_VALUE(cambc::champion_state::recall_started.bits(), e_Champion_State_recallStarted);
BC_VALUE(cambc::champion_state::submit_gems_started.bits(), e_Champion_State_submitGemsStarted);
BC_VALUE(cambc::champion_state::recall_cancelled.bits(), e_Champion_State_recallCancelled);
BC_VALUE(cambc::champion_state::submit_gems_cancelled.bits(), e_Champion_State_submitGemsCancelled);
BC_VALUE(cambc::champion_state::can_cast.bits(), e_Champion_State_canCast);
BC_VALUE(cambc::champion_state::teleporting.bits(), e_Champion_State_isFlashing);
static_assert(sizeof(cambc::champion_state) == sizeof(std::uint32_t));
static_assert(sizeof(cambc::ability_target_mask) == sizeof(std::uint32_t));

BC_LAYOUT(cambc::detail::ability_props, Game_AbilityProps);
BC_FIELD(cambc::detail::ability_props, kind, Game_AbilityProps, m_type);
BC_FIELD(cambc::detail::ability_props, cooldown, Game_AbilityProps, m_cooldown);
BC_FIELD(cambc::detail::ability_props, max_charges, Game_AbilityProps, m_maxCharges);
BC_FIELD(cambc::detail::ability_props, targets, Game_AbilityProps, m_target);
BC_FIELD(cambc::detail::ability_props, attributes, Game_AbilityProps, m_attributes);
BC_FIELD(cambc::detail::ability_props, payload.enhanced_attack, Game_AbilityProps, m_attackReset.m_enhancedAuto);
BC_FIELD(cambc::detail::ability_props, payload.dash_range, Game_AbilityProps, m_dash.m_range);
BC_FIELD(cambc::detail::ability_props, payload.damage.range, Game_AbilityProps, m_damage.m_range);
BC_FIELD(cambc::detail::ability_props, payload.damage.modifier, Game_AbilityProps, m_damage.m_modifier);
BC_FIELD(cambc::detail::ability_props, payload.heal.range, Game_AbilityProps, m_heal.m_range);
BC_FIELD(cambc::detail::ability_props, payload.heal.modifier, Game_AbilityProps, m_heal.m_modifier);
BC_FIELD(cambc::detail::ability_props, payload.aoe_damage.range, Game_AbilityProps, m_aoeDamage.m_range);
BC_FIELD(cambc::detail::ability_props, payload.aoe_damage.radius, Game_AbilityProps, m_aoeDamage.m_radius);
BC_FIELD(cambc::detail::ability_props, payload.aoe_damage.modifier, Game_AbilityProps, m_aoeDamage.m_modifier);
BC_FIELD(cambc::detail::ability_props, payload.aoe_damage.duration, Game_AbilityProps, m_aoeDamage.m_stunDuration);
BC_FIELD(cambc::detail::ability_props, payload.cc.range, Game_AbilityProps, m_cc.m_range);
BC_FIELD(cambc::detail::ability_props, payload.cc.modifier, Game_AbilityProps, m_cc.m_modifier);
BC_FIELD(cambc::detail::ability_props, payload.cc.duration, Game_AbilityProps, m_cc.m_stunDuration);
BC_FIELD(cambc::detail::ability_props, payload.shield.range, Game_AbilityProps, m_shield.m_range);
BC_FIELD(cambc::detail::ability_props, payload.shield.modifier, Game_AbilityProps, m_shield.m_modifier);
BC_FIELD(cambc::detail::ability_props, payload.shield.duration, Game_AbilityProps, m_shield.m_duration);

BC_LAYOUT(cambc::detail::entity, Game_EntityRef);
BC_FIELD(cambc::detail::entity, kind, Game_EntityRef, m_kind);
BC_FIELD(cambc::detail::entity, handle, Game_EntityRef, m_index);
BC_LAYOUT(cambc::handle_champion_t, Handle_Champion);

// End markers also catch appended engine values missing from the SDK.
BC_VALUE(static_cast<int>(cambc::symmetry::rotational180) + 1, e_Symmetry_numSymmetryTypes);
BC_VALUE(static_cast<int>(cambc::camp_type::monolith) + 1, e_Camp_mapSize);
BC_VALUE(static_cast<int>(cambc::champion_draft::tavi) + 1, e_ChampionDraft_heart_of_ruin);
BC_VALUE(e_ChampionDraft_heart_of_ruin + 1, e_ChampionDraft_numChampions);
BC_VALUE(static_cast<int>(cambc::champion_type::tavi) + 1, e_Champion_heart_of_ruin);
BC_VALUE(e_Champion_heart_of_ruin + 1, e_Champion_numChampions);
BC_VALUE(std::variant_size_v<cambc::ability_info> + 1, e_Game_Ability_Type_numTypes);
BC_VALUE(std::variant_size_v<cambc::buff_t>, e_Buff_numTypes);
BC_ENUM(cambc::detail::buff_kind::shield, e_Buff_shield);
BC_ENUM(cambc::detail::buff_kind::stun, e_Buff_stun);
BC_ENUM(cambc::detail::buff_kind::enhanced_attack, e_Buff_enhancedAuto);
BC_ENUM(cambc::detail::buff_kind::count, e_Buff_numTypes);
BC_ENUM(cambc::buff_types::shield, e_Buff_shield);
BC_ENUM(cambc::buff_types::stun, e_Buff_stun);
BC_ENUM(cambc::buff_types::enhanced_attack, e_Buff_enhancedAuto);
BC_ENUM(cambc::buff_types::count, e_Buff_numTypes);

#undef BC_FIELD
#undef BC_LAYOUT
#undef BC_ENUM
#undef BC_VALUE
