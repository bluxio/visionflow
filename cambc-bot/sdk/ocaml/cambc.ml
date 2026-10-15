exception Invalid_tag of string * int

module Sys = struct
  external ld8 : int -> int = "cambc_ld8"
  external ld16 : int -> int = "cambc_ld16"
  external ld32 : int -> int = "cambc_ld32"
  external st16 : int -> int -> int = "cambc_st16"
  external ensure : int -> int = "cambc_ensure"
  external base : int -> int = "cambc_base"
  external blit_in : Bytes.t -> int -> int -> int = "cambc_blit_in"
  external blit_out : Bytes.t -> int -> int -> int = "cambc_blit_out"
  external game_settings : int -> int = "cambc_game_settings"
  external item_info : int -> int -> int = "cambc_item_info"
  external consumable_info : int -> int -> int = "cambc_consumable_info"
  external champion_buff_data : int -> int -> int -> int -> int = "cambc_champion_buff_data"
  external champion_ability_damage : int -> int -> int -> int = "cambc_champion_ability_damage"
  external champion_ability_damage_against : int -> int -> int -> int -> int = "cambc_champion_ability_damage_against"
  external champion_ability_heal : int -> int -> int -> int = "cambc_champion_ability_heal"
  external champion_ability_heal_against : int -> int -> int -> int -> int = "cambc_champion_ability_heal_against"
  external champion_ability_shield : int -> int -> int -> int = "cambc_champion_ability_shield"
  external champion_ability_shield_against : int -> int -> int -> int -> int = "cambc_champion_ability_shield_against"
  external bc_get_last_error : int -> int -> int = "cambc_bc_get_last_error"
  external my_champion : int -> int = "cambc_my_champion"
  external champion_is_alive : int -> int -> int = "cambc_champion_is_alive"
  external num_allies : int -> int = "cambc_num_allies"
  external num_enemies : int -> int = "cambc_num_enemies"
  external allies : int -> int -> int -> int = "cambc_allies"
  external enemies : int -> int -> int -> int = "cambc_enemies"
  external board_ally_occupancy : int -> int = "cambc_board_ally_occupancy"
  external board_enemy_occupancy : int -> int = "cambc_board_enemy_occupancy"
  external camp_occupancy : int -> int = "cambc_camp_occupancy"
  external board_ally_ward_occupancy : int -> int = "cambc_board_ally_ward_occupancy"
  external board_enemy_ward_occupancy : int -> int = "cambc_board_enemy_ward_occupancy"
  external ward_at : int -> int -> int -> int = "cambc_ward_at"
  external ally_ward_at : int -> int -> int -> int = "cambc_ally_ward_at"
  external enemy_ward_at : int -> int -> int -> int = "cambc_enemy_ward_at"
  external ward : int -> int -> int = "cambc_ward"
  external board_teleport_occupancy : int -> int = "cambc_board_teleport_occupancy"
  external is_occupied_by_teleport : int -> int -> int -> int = "cambc_is_occupied_by_teleport"
  external teleports : int -> int -> int -> int = "cambc_teleports"
  external board_ward_vision_mask : int -> int -> int -> int = "cambc_board_ward_vision_mask"
  external board_ally_altar_lock_occupancy : int -> int = "cambc_board_ally_altar_lock_occupancy"
  external board_enemy_altar_lock_occupancy : int -> int = "cambc_board_enemy_altar_lock_occupancy"
  external altar_lock : int -> int -> int -> int -> int = "cambc_altar_lock"
  external altar_locks : int -> int -> int -> int = "cambc_altar_locks"
  external attack : int -> int -> int = "cambc_attack"
  external cast : int -> int -> int -> int -> int -> int -> int = "cambc_cast"
  external use_consumable : int -> int -> int -> int -> int = "cambc_use_consumable"
  external buy_consumable : int -> int = "cambc_buy_consumable"
  external buy_item : int -> int -> int = "cambc_buy_item"
  external sell : int -> int = "cambc_sell"
  external recall : int -> int = "cambc_recall"
  external cancel_channel : int -> int = "cambc_cancel_channel"
  external submit_gems : int -> int -> int = "cambc_submit_gems"
  external place_ward : int -> int -> int -> int = "cambc_place_ward"
  external move_to : int -> int -> int -> int = "cambc_move_to"
  external set_priority : int -> int = "cambc_set_priority"
  external bc_champion_priority : int -> int -> int -> int = "cambc_bc_champion_priority"
  external bc_submitted_gems : int -> int -> int = "cambc_bc_submitted_gems"
  external bc_round_number : int -> int = "cambc_bc_round_number"
  external resign : int -> int = "cambc_resign"
  external shared_memory_size : int -> int = "cambc_shared_memory_size"
  external read_shared_memory : int -> int -> int -> int = "cambc_read_shared_memory"
  external write_shared_memory : int -> int -> int -> int = "cambc_write_shared_memory"
  external champion : int -> int -> int = "cambc_champion"
  external champion_design : int -> int -> int = "cambc_champion_design"
  external champion_restrictions : int -> int -> int = "cambc_champion_restrictions"
  external champion_ability_props : int -> int -> int -> int = "cambc_champion_ability_props"
  external champion_cooldown : int -> int -> int -> int = "cambc_champion_cooldown"
  external champion_items : int -> int -> int = "cambc_champion_items"
  external champion_consumables : int -> int -> int = "cambc_champion_consumables"
  external champion_gems : int -> int -> int = "cambc_champion_gems"
  external champion_gold : int -> int -> int = "cambc_champion_gold"
  external bc_champion_vision : int -> int -> int = "cambc_bc_champion_vision"
  external board_vision_mask : int -> int -> int -> int -> int = "cambc_board_vision_mask"
  external champion_wards : int -> int -> int = "cambc_champion_wards"
  external bc_champion_ward_positions : int -> int -> int -> int -> int = "cambc_bc_champion_ward_positions"
  external bc_champion_channel_time : int -> int -> int = "cambc_bc_champion_channel_time"
  external champion_can_shop : int -> int -> int -> int = "cambc_champion_can_shop"
  external champion_can_sell : int -> int -> int = "cambc_champion_can_sell"
  external wards : int -> int -> int -> int = "cambc_wards"
  external bc_metadata : int -> int = "cambc_bc_metadata"
  external my_team : int -> int = "cambc_my_team"
  external bc_map_mask : int -> int -> int = "cambc_bc_map_mask"
  external bc_map_symmetry : int -> int = "cambc_bc_map_symmetry"
  external map_num_camp_groups : int -> int = "cambc_map_num_camp_groups"
  external map_camp_groups : int -> int -> int -> int = "cambc_map_camp_groups"
  external map_camp_group_num_camps : int -> int -> int = "cambc_map_camp_group_num_camps"
  external map_camp_group_camps : int -> int -> int -> int -> int = "cambc_map_camp_group_camps"
  external map_camp_group : int -> int -> int = "cambc_map_camp_group"
  external map_camp_info : int -> int -> int = "cambc_map_camp_info"
  external map_camp_layout : int -> int -> int -> int = "cambc_map_camp_layout"
  external map_camp_group_of : int -> int -> int = "cambc_map_camp_group_of"
  external map_camp_at : int -> int -> int -> int = "cambc_map_camp_at"
  external camp_group_cooldown : int -> int -> int = "cambc_camp_group_cooldown"
  external camp_group_state : int -> int -> int = "cambc_camp_group_state"
  external camp_buffs : int -> int -> int -> int -> int = "cambc_camp_buffs"
  external bc_camp_group_max_respawn : int -> int -> int = "cambc_bc_camp_group_max_respawn"
  external camp : int -> int -> int = "cambc_camp"
  external camp_design : int -> int -> int = "cambc_camp_design"
  external camp_at : int -> int -> int -> int = "cambc_camp_at"
  external champion_at : int -> int -> int -> int = "cambc_champion_at"
  external has_camp : int -> int -> int -> int = "cambc_has_camp"
  external has_champion : int -> int -> int -> int = "cambc_has_champion"
  external board_vision : int -> int = "cambc_board_vision"
  external spawn_region : int -> int -> int = "cambc_spawn_region"
  external draft_num_rounds : int -> int = "cambc_draft_num_rounds"
  external draft_get : int -> int -> int -> int = "cambc_draft_get"
  external draft_champion : int -> int = "cambc_draft_champion"
  external bc_submit_draft_memory : int -> int -> int = "cambc_bc_submit_draft_memory"
  external draft_memory_size : int -> int = "cambc_draft_memory_size"
  external read_draft_memory : int -> int -> int -> int = "cambc_read_draft_memory"
  external debug_dot : int -> int -> int -> int = "cambc_debug_dot"
  external debug_line : int -> int -> int -> int -> int -> int = "cambc_debug_line"
  external yield : int -> int = "cambc_yield"
  external fuel_remaining : int -> int = "cambc_fuel_remaining"
  external bc_bitboard_one : Bytes.t -> int = "cambc_bc_Bitboard_One_gc"
  external bc_bitboard_importrawbuffer : Bytes.t -> Bytes.t -> int -> int = "cambc_bc_Bitboard_ImportRawBuffer_gc"
  external bc_bitboard_importlinearbuffer : Bytes.t -> Bytes.t -> int -> int = "cambc_bc_Bitboard_ImportLinearBuffer_gc"
  external bc_bitboard_exportrawbuffer : Bytes.t -> Bytes.t -> int -> int = "cambc_bc_Bitboard_ExportRawBuffer_gc"
  external bc_bitboard_exportlinearbuffer : Bytes.t -> Bytes.t -> int -> int = "cambc_bc_Bitboard_ExportLinearBuffer_gc"
  external bc_bitboard_print : Bytes.t -> int = "cambc_bc_Bitboard_Print_gc"
  external bc_bitboard_test : Bytes.t -> int -> int -> int -> int = "cambc_bc_Bitboard_Test_gc"
  external bc_bitboard_set : Bytes.t -> int -> int -> int -> int = "cambc_bc_Bitboard_Set_gc"
  external bc_bitboard_toggle : Bytes.t -> int -> int -> int = "cambc_bc_Bitboard_Toggle_gc"
  external bc_bitboard_iszero : Bytes.t -> int -> int = "cambc_bc_Bitboard_IsZero_gc"
  external bc_bitboard_numsetbits : Bytes.t -> int -> int = "cambc_bc_Bitboard_NumSetBits_gc"
  external bc_bitboard_getsetbits : Bytes.t -> int -> int -> int -> int = "cambc_bc_Bitboard_GetSetBits_gc"
  external bc_bitboard_not : Bytes.t -> Bytes.t -> int = "cambc_bc_Bitboard_Not_gc"
  external bc_bitboard_union : Bytes.t -> Bytes.t -> Bytes.t -> int = "cambc_bc_Bitboard_Union_gc"
  external bc_bitboard_intersect : Bytes.t -> Bytes.t -> Bytes.t -> int = "cambc_bc_Bitboard_Intersect_gc"
  external bc_bitboard_xor : Bytes.t -> Bytes.t -> Bytes.t -> int = "cambc_bc_Bitboard_Xor_gc"
  external bc_bitboard_isequal : Bytes.t -> Bytes.t -> int -> int = "cambc_bc_Bitboard_IsEqual_gc"
  external bc_bitboard_shiftright : Bytes.t -> Bytes.t -> int -> int = "cambc_bc_Bitboard_ShiftRight_gc"
  external bc_bitboard_shiftup : Bytes.t -> Bytes.t -> int -> int = "cambc_bc_Bitboard_ShiftUp_gc"
  external bc_bitboard_reflect : Bytes.t -> Bytes.t -> int -> int = "cambc_bc_Bitboard_Reflect_gc"
  external bc_bitboard_drawcircle : Bytes.t -> int -> int -> int -> int = "cambc_bc_Bitboard_DrawCircle_gc"
  external bc_bitboard_drawchebyshev : Bytes.t -> int -> int -> int -> int = "cambc_bc_Bitboard_DrawChebyshev_gc"
  external bc_bitboard_reflecthorizontalposition : int -> int -> int -> int -> int = "cambc_bc_Bitboard_ReflectHorizontalPosition"
  external bc_bitboard_reflectverticalposition : int -> int -> int -> int -> int = "cambc_bc_Bitboard_ReflectVerticalPosition"
  external bc_bitboard_rotate180position : int -> int -> int -> int -> int = "cambc_bc_Bitboard_Rotate180Position"
  external bc_bitboard_reachablemask : Bytes.t -> int -> int -> Bytes.t -> int -> int = "cambc_bc_Bitboard_ReachableMask_gc"
  external bc_bitboard_shadowcast : Bytes.t -> int -> int -> Bytes.t -> int -> int = "cambc_bc_Bitboard_ShadowCast_gc"
  external bc_bitboard_reachablemaskwithscratch : Bytes.t array -> int -> int -> int -> int = "cambc_bc_Bitboard_ReachableMaskWithScratch_gc"
  external bc_distancefield_importrawbuffer : Bytes.t -> Bytes.t -> int -> int = "cambc_bc_DistanceField_ImportRawBuffer_gc"
  external bc_distancefield_importlinearbuffer : Bytes.t -> Bytes.t -> int -> int = "cambc_bc_DistanceField_ImportLinearBuffer_gc"
  external bc_distancefield_exportrawbuffer : Bytes.t -> Bytes.t -> int -> int = "cambc_bc_DistanceField_ExportRawBuffer_gc"
  external bc_distancefield_exportlinearbuffer : Bytes.t -> Bytes.t -> int -> int = "cambc_bc_DistanceField_ExportLinearBuffer_gc"
  external bc_distancefield_print : Bytes.t -> int = "cambc_bc_DistanceField_Print_gc"
  external bc_distancefield_get : Bytes.t -> int -> int -> int -> int = "cambc_bc_DistanceField_Get_gc"
  external bc_distancefield_set : Bytes.t -> int -> int -> int -> int = "cambc_bc_DistanceField_Set_gc"
  external bc_distancefield_computewithexpansioncount : Bytes.t array -> int -> int -> int = "cambc_bc_DistanceField_ComputeWithExpansionCount_gc"
  external bc_distancefield_computetowithexpansioncount : Bytes.t array -> int -> int -> int = "cambc_bc_DistanceField_ComputeToWithExpansionCount_gc"
end

(* Scratch layout in the bridge's linear memory. *)
let mem_base = Sys.base 0
let out = mem_base + 16
let args = mem_base + 64
let rec_ = mem_base + 128
let designs_area = mem_base + 1024
let slots = mem_base + 2688

let i32 o = Sys.ld32 o
let u32 o = Sys.ld32 o
let f32 o = Int32.float_of_bits (Int32.logor (Int32.of_int (Sys.ld16 o)) (Int32.shift_left (Int32.of_int (Sys.ld16 (o + 2))) 16))
let f64 o =
  let w k = Int64.shift_left (Int64.of_int (Sys.ld16 (o + (2 * k)))) (16 * k) in
  Int64.float_of_bits (Int64.logor (Int64.logor (w 0) (w 1)) (Int64.logor (w 2) (w 3)))
let set_f32 o v =
  let b = Int32.bits_of_float v in
  ignore (Sys.st16 o (Int32.to_int (Int32.logand b 0xffffl)));
  ignore (Sys.st16 (o + 2) (Int32.to_int (Int32.shift_right_logical b 16)))

module Vec2_base = struct
  type t = { x : int; y : int }

  let make x y = { x; y }
  let zero = { x = 0; y = 0 }
  let north = { x = 0; y = 1 }
  let north_east = { x = 1; y = 1 }
  let east = { x = 1; y = 0 }
  let south_east = { x = 1; y = Stdlib.( ~- ) 1 }
  let south = { x = 0; y = Stdlib.( ~- ) 1 }
  let south_west = { x = Stdlib.( ~- ) 1; y = Stdlib.( ~- ) 1 }
  let west = { x = Stdlib.( ~- ) 1; y = 0 }
  let north_west = { x = Stdlib.( ~- ) 1; y = 1 }
  let equal a b = a.x = b.x && a.y = b.y
  let compare (a : t) b = Stdlib.compare a b
  let hash (a : t) = Hashtbl.hash (a.x, a.y)
  let add a b = { x = a.x + b.x; y = a.y + b.y }
  let sub a b = { x = a.x - b.x; y = a.y - b.y }
  let neg a = { x = -a.x; y = -a.y }
  let scale k a = { x = k * a.x; y = k * a.y }
  let ( + ) = add
  let ( - ) = sub
  let ( ~- ) = neg
  let ( * ) = scale
  let dot a b = Stdlib.( + ) (Stdlib.( * ) a.x b.x) (Stdlib.( * ) a.y b.y)
  let cross a b = Stdlib.( - ) (Stdlib.( * ) a.x b.y) (Stdlib.( * ) a.y b.x)
  let length_squared a = dot a a
  let chebyshev_distance a b = max (abs (Stdlib.( - ) a.x b.x)) (abs (Stdlib.( - ) a.y b.y))
  let rotate_cw a = { x = a.y; y = Stdlib.( ~- ) a.x }
  let rotate_ccw a = { x = Stdlib.( ~- ) a.y; y = a.x }
  let reflect_horizontal a = { a with x = Stdlib.( ~- ) a.x }
  let reflect_vertical a = { a with y = Stdlib.( ~- ) a.y }
  let rotate_180 = neg
  let to_string a = Printf.sprintf "(%d, %d)" a.x a.y
end

module Team = struct
  type t = Ally | Enemy

  let opponent = function Ally -> Enemy | Enemy -> Ally
  let equal (a : t) b = a = b
  let to_string = function Ally -> "Ally" | Enemy -> "Enemy"
end
module Symmetry = struct
  type t =
    | Horizontal
    | Vertical
    | Rotational180

  let all = [ Horizontal; Vertical; Rotational180 ]
  let to_int = function
    | Horizontal -> 0
    | Vertical -> 1
    | Rotational180 -> 2
  let of_int = function
    | 0 -> Horizontal
    | 1 -> Vertical
    | 2 -> Rotational180
    | n -> raise (Invalid_tag ("Symmetry", n))
  let to_string = function
    | Horizontal -> "Horizontal"
    | Vertical -> "Vertical"
    | Rotational180 -> "Rotational180"
end

module Camp_type = struct
  type t =
    | Wolf
    | Elk
    | Gem_cluster
    | Gold_cluster
    | Monolith

  let all = [ Wolf; Elk; Gem_cluster; Gold_cluster; Monolith ]
  let to_int = function
    | Wolf -> 0
    | Elk -> 1
    | Gem_cluster -> 2
    | Gold_cluster -> 3
    | Monolith -> 4
  let of_int = function
    | 0 -> Wolf
    | 1 -> Elk
    | 2 -> Gem_cluster
    | 3 -> Gold_cluster
    | 4 -> Monolith
    | n -> raise (Invalid_tag ("Camp_type", n))
  let to_string = function
    | Wolf -> "Wolf"
    | Elk -> "Elk"
    | Gem_cluster -> "GemCluster"
    | Gold_cluster -> "GoldCluster"
    | Monolith -> "Monolith"
end

module Passive = struct
  type t =
    | None
    | Bonus_gold
    | Remote_shop
    | Gem_focus
    | Splash_attack
    | Health_strength
    | Reset_cooldowns_on_kill
    | Retaliation
    | Stun_immunity

  let all = [ None; Bonus_gold; Remote_shop; Gem_focus; Splash_attack; Health_strength; Reset_cooldowns_on_kill; Retaliation; Stun_immunity ]
  let of_int = function
    | 0 -> None
    | 1 -> Bonus_gold
    | 2 -> Remote_shop
    | 3 -> Gem_focus
    | 4 -> Splash_attack
    | 5 -> Health_strength
    | 6 -> Reset_cooldowns_on_kill
    | 7 -> Retaliation
    | 8 -> Stun_immunity
    | n -> raise (Invalid_tag ("Passive", n))
  let to_string = function
    | None -> "None"
    | Bonus_gold -> "Bonus_gold"
    | Remote_shop -> "Remote_shop"
    | Gem_focus -> "Gem_focus"
    | Splash_attack -> "Splash_attack"
    | Health_strength -> "Health_strength"
    | Reset_cooldowns_on_kill -> "Reset_cooldowns_on_kill"
    | Retaliation -> "Retaliation"
    | Stun_immunity -> "Stun_immunity"
end

module Champion_draft = struct
  type t =
    | Ryth
    | Vesper_vale
    | Vendetta
    | Cassi_and_issac
    | Stellar
    | Prism
    | Marcella
    | Willow
    | Ren
    | Varg
    | Tavi

  let all = [ Ryth; Vesper_vale; Vendetta; Cassi_and_issac; Stellar; Prism; Marcella; Willow; Ren; Varg; Tavi ]
  let to_int = function
    | Ryth -> 0
    | Vesper_vale -> 1
    | Vendetta -> 2
    | Cassi_and_issac -> 3
    | Stellar -> 4
    | Prism -> 5
    | Marcella -> 6
    | Willow -> 7
    | Ren -> 8
    | Varg -> 9
    | Tavi -> 10
  let of_int = function
    | 0 -> Ryth
    | 1 -> Vesper_vale
    | 2 -> Vendetta
    | 3 -> Cassi_and_issac
    | 4 -> Stellar
    | 5 -> Prism
    | 6 -> Marcella
    | 7 -> Willow
    | 8 -> Ren
    | 9 -> Varg
    | 10 -> Tavi
    | n -> raise (Invalid_tag ("Champion_draft", n))
  let to_string = function
    | Ryth -> "Ryth"
    | Vesper_vale -> "Vesper_vale"
    | Vendetta -> "Vendetta"
    | Cassi_and_issac -> "Cassi_and_issac"
    | Stellar -> "Stellar"
    | Prism -> "Prism"
    | Marcella -> "Marcella"
    | Willow -> "Willow"
    | Ren -> "Ren"
    | Varg -> "Varg"
    | Tavi -> "Tavi"
end

module Champion_type = struct
  type t =
    | Ryth
    | Vesper_vale
    | Vendetta
    | Cassi
    | Issac
    | Stellar
    | Prism
    | Marcella
    | Willow
    | Ren
    | Varg
    | Tavi

  let all = [ Ryth; Vesper_vale; Vendetta; Cassi; Issac; Stellar; Prism; Marcella; Willow; Ren; Varg; Tavi ]
  let to_int = function
    | Ryth -> 0
    | Vesper_vale -> 1
    | Vendetta -> 2
    | Cassi -> 3
    | Issac -> 4
    | Stellar -> 5
    | Prism -> 6
    | Marcella -> 7
    | Willow -> 8
    | Ren -> 9
    | Varg -> 10
    | Tavi -> 11
  let of_int = function
    | 0 -> Ryth
    | 1 -> Vesper_vale
    | 2 -> Vendetta
    | 3 -> Cassi
    | 4 -> Issac
    | 5 -> Stellar
    | 6 -> Prism
    | 7 -> Marcella
    | 8 -> Willow
    | 9 -> Ren
    | 10 -> Varg
    | 11 -> Tavi
    | n -> raise (Invalid_tag ("Champion_type", n))
  let to_string = function
    | Ryth -> "Ryth"
    | Vesper_vale -> "Vesper_vale"
    | Vendetta -> "Vendetta"
    | Cassi -> "Cassi"
    | Issac -> "Issac"
    | Stellar -> "Stellar"
    | Prism -> "Prism"
    | Marcella -> "Marcella"
    | Willow -> "Willow"
    | Ren -> "Ren"
    | Varg -> "Varg"
    | Tavi -> "Tavi"
end

module Ability_kind = struct
  type t =
    | Attack_reset
    | Damage
    | Aoe_damage
    | Cc
    | Heal
    | Dash
    | Shield

  let all = [ Attack_reset; Damage; Aoe_damage; Cc; Heal; Dash; Shield ]
  let of_int = function
    | 1 -> Attack_reset
    | 2 -> Damage
    | 3 -> Aoe_damage
    | 4 -> Cc
    | 5 -> Heal
    | 6 -> Dash
    | 7 -> Shield
    | n -> raise (Invalid_tag ("Ability_kind", n))
  let to_string = function
    | Attack_reset -> "Attack_reset"
    | Damage -> "Damage"
    | Aoe_damage -> "Aoe_damage"
    | Cc -> "Cc"
    | Heal -> "Heal"
    | Dash -> "Dash"
    | Shield -> "Shield"
end

module Item = struct
  type t =
    | Monster_shard_of_strength
    | Monster_emblem_of_strength
    | Monster_shard_of_focus
    | Monster_emblem_of_focus
    | Monster_shard_of_health
    | Monster_emblem_of_health
    | Power_shard_of_strength
    | Power_emblem_of_strength
    | Power_shard_of_focus
    | Power_emblem_of_focus
    | Power_shard_of_health
    | Power_emblem_of_health
    | Durability_shard_of_strength
    | Durability_emblem_of_strength
    | Durability_shard_of_focus
    | Durability_emblem_of_focus
    | Durability_shard_of_health
    | Durability_emblem_of_health
    | Treasure_shard_of_strength
    | Treasure_emblem_of_strength
    | Treasure_shard_of_focus
    | Treasure_emblem_of_focus
    | Treasure_shard_of_health
    | Treasure_emblem_of_health
    | Jungle_shard_of_strength
    | Jungle_emblem_of_strength
    | Jungle_shard_of_focus
    | Jungle_emblem_of_focus
    | Jungle_shard_of_health
    | Jungle_emblem_of_health
    | Home_shard_of_strength
    | Home_emblem_of_strength
    | Home_shard_of_focus
    | Home_emblem_of_focus
    | Home_shard_of_health
    | Home_emblem_of_health
    | Bruiser_shard_of_strength
    | Bruiser_emblem_of_strength
    | Bruiser_shard_of_focus
    | Bruiser_emblem_of_focus
    | Bruiser_shard_of_health
    | Bruiser_emblem_of_health

  let all = [ Monster_shard_of_strength; Monster_emblem_of_strength; Monster_shard_of_focus; Monster_emblem_of_focus; Monster_shard_of_health; Monster_emblem_of_health; Power_shard_of_strength; Power_emblem_of_strength; Power_shard_of_focus; Power_emblem_of_focus; Power_shard_of_health; Power_emblem_of_health; Durability_shard_of_strength; Durability_emblem_of_strength; Durability_shard_of_focus; Durability_emblem_of_focus; Durability_shard_of_health; Durability_emblem_of_health; Treasure_shard_of_strength; Treasure_emblem_of_strength; Treasure_shard_of_focus; Treasure_emblem_of_focus; Treasure_shard_of_health; Treasure_emblem_of_health; Jungle_shard_of_strength; Jungle_emblem_of_strength; Jungle_shard_of_focus; Jungle_emblem_of_focus; Jungle_shard_of_health; Jungle_emblem_of_health; Home_shard_of_strength; Home_emblem_of_strength; Home_shard_of_focus; Home_emblem_of_focus; Home_shard_of_health; Home_emblem_of_health; Bruiser_shard_of_strength; Bruiser_emblem_of_strength; Bruiser_shard_of_focus; Bruiser_emblem_of_focus; Bruiser_shard_of_health; Bruiser_emblem_of_health ]
  let to_int = function
    | Monster_shard_of_strength -> 0
    | Monster_emblem_of_strength -> 1
    | Monster_shard_of_focus -> 2
    | Monster_emblem_of_focus -> 3
    | Monster_shard_of_health -> 4
    | Monster_emblem_of_health -> 5
    | Power_shard_of_strength -> 6
    | Power_emblem_of_strength -> 7
    | Power_shard_of_focus -> 8
    | Power_emblem_of_focus -> 9
    | Power_shard_of_health -> 10
    | Power_emblem_of_health -> 11
    | Durability_shard_of_strength -> 12
    | Durability_emblem_of_strength -> 13
    | Durability_shard_of_focus -> 14
    | Durability_emblem_of_focus -> 15
    | Durability_shard_of_health -> 16
    | Durability_emblem_of_health -> 17
    | Treasure_shard_of_strength -> 18
    | Treasure_emblem_of_strength -> 19
    | Treasure_shard_of_focus -> 20
    | Treasure_emblem_of_focus -> 21
    | Treasure_shard_of_health -> 22
    | Treasure_emblem_of_health -> 23
    | Jungle_shard_of_strength -> 24
    | Jungle_emblem_of_strength -> 25
    | Jungle_shard_of_focus -> 26
    | Jungle_emblem_of_focus -> 27
    | Jungle_shard_of_health -> 28
    | Jungle_emblem_of_health -> 29
    | Home_shard_of_strength -> 30
    | Home_emblem_of_strength -> 31
    | Home_shard_of_focus -> 32
    | Home_emblem_of_focus -> 33
    | Home_shard_of_health -> 34
    | Home_emblem_of_health -> 35
    | Bruiser_shard_of_strength -> 36
    | Bruiser_emblem_of_strength -> 37
    | Bruiser_shard_of_focus -> 38
    | Bruiser_emblem_of_focus -> 39
    | Bruiser_shard_of_health -> 40
    | Bruiser_emblem_of_health -> 41
  let of_int = function
    | 0 -> Monster_shard_of_strength
    | 1 -> Monster_emblem_of_strength
    | 2 -> Monster_shard_of_focus
    | 3 -> Monster_emblem_of_focus
    | 4 -> Monster_shard_of_health
    | 5 -> Monster_emblem_of_health
    | 6 -> Power_shard_of_strength
    | 7 -> Power_emblem_of_strength
    | 8 -> Power_shard_of_focus
    | 9 -> Power_emblem_of_focus
    | 10 -> Power_shard_of_health
    | 11 -> Power_emblem_of_health
    | 12 -> Durability_shard_of_strength
    | 13 -> Durability_emblem_of_strength
    | 14 -> Durability_shard_of_focus
    | 15 -> Durability_emblem_of_focus
    | 16 -> Durability_shard_of_health
    | 17 -> Durability_emblem_of_health
    | 18 -> Treasure_shard_of_strength
    | 19 -> Treasure_emblem_of_strength
    | 20 -> Treasure_shard_of_focus
    | 21 -> Treasure_emblem_of_focus
    | 22 -> Treasure_shard_of_health
    | 23 -> Treasure_emblem_of_health
    | 24 -> Jungle_shard_of_strength
    | 25 -> Jungle_emblem_of_strength
    | 26 -> Jungle_shard_of_focus
    | 27 -> Jungle_emblem_of_focus
    | 28 -> Jungle_shard_of_health
    | 29 -> Jungle_emblem_of_health
    | 30 -> Home_shard_of_strength
    | 31 -> Home_emblem_of_strength
    | 32 -> Home_shard_of_focus
    | 33 -> Home_emblem_of_focus
    | 34 -> Home_shard_of_health
    | 35 -> Home_emblem_of_health
    | 36 -> Bruiser_shard_of_strength
    | 37 -> Bruiser_emblem_of_strength
    | 38 -> Bruiser_shard_of_focus
    | 39 -> Bruiser_emblem_of_focus
    | 40 -> Bruiser_shard_of_health
    | 41 -> Bruiser_emblem_of_health
    | n -> raise (Invalid_tag ("Item", n))
  let to_string = function
    | Monster_shard_of_strength -> "Monster_shard_of_strength"
    | Monster_emblem_of_strength -> "Monster_emblem_of_strength"
    | Monster_shard_of_focus -> "Monster_shard_of_focus"
    | Monster_emblem_of_focus -> "Monster_emblem_of_focus"
    | Monster_shard_of_health -> "Monster_shard_of_health"
    | Monster_emblem_of_health -> "Monster_emblem_of_health"
    | Power_shard_of_strength -> "Power_shard_of_strength"
    | Power_emblem_of_strength -> "Power_emblem_of_strength"
    | Power_shard_of_focus -> "Power_shard_of_focus"
    | Power_emblem_of_focus -> "Power_emblem_of_focus"
    | Power_shard_of_health -> "Power_shard_of_health"
    | Power_emblem_of_health -> "Power_emblem_of_health"
    | Durability_shard_of_strength -> "Durability_shard_of_strength"
    | Durability_emblem_of_strength -> "Durability_emblem_of_strength"
    | Durability_shard_of_focus -> "Durability_shard_of_focus"
    | Durability_emblem_of_focus -> "Durability_emblem_of_focus"
    | Durability_shard_of_health -> "Durability_shard_of_health"
    | Durability_emblem_of_health -> "Durability_emblem_of_health"
    | Treasure_shard_of_strength -> "Treasure_shard_of_strength"
    | Treasure_emblem_of_strength -> "Treasure_emblem_of_strength"
    | Treasure_shard_of_focus -> "Treasure_shard_of_focus"
    | Treasure_emblem_of_focus -> "Treasure_emblem_of_focus"
    | Treasure_shard_of_health -> "Treasure_shard_of_health"
    | Treasure_emblem_of_health -> "Treasure_emblem_of_health"
    | Jungle_shard_of_strength -> "Jungle_shard_of_strength"
    | Jungle_emblem_of_strength -> "Jungle_emblem_of_strength"
    | Jungle_shard_of_focus -> "Jungle_shard_of_focus"
    | Jungle_emblem_of_focus -> "Jungle_emblem_of_focus"
    | Jungle_shard_of_health -> "Jungle_shard_of_health"
    | Jungle_emblem_of_health -> "Jungle_emblem_of_health"
    | Home_shard_of_strength -> "Home_shard_of_strength"
    | Home_emblem_of_strength -> "Home_emblem_of_strength"
    | Home_shard_of_focus -> "Home_shard_of_focus"
    | Home_emblem_of_focus -> "Home_emblem_of_focus"
    | Home_shard_of_health -> "Home_shard_of_health"
    | Home_emblem_of_health -> "Home_emblem_of_health"
    | Bruiser_shard_of_strength -> "Bruiser_shard_of_strength"
    | Bruiser_emblem_of_strength -> "Bruiser_emblem_of_strength"
    | Bruiser_shard_of_focus -> "Bruiser_shard_of_focus"
    | Bruiser_emblem_of_focus -> "Bruiser_emblem_of_focus"
    | Bruiser_shard_of_health -> "Bruiser_shard_of_health"
    | Bruiser_emblem_of_health -> "Bruiser_emblem_of_health"
end

module Buff_kind = struct
  type t =
    | Shield
    | Stun
    | Enhanced_attack

  let all = [ Shield; Stun; Enhanced_attack ]
  let of_int = function
    | 0 -> Shield
    | 1 -> Stun
    | 2 -> Enhanced_attack
    | n -> raise (Invalid_tag ("Buff_kind", n))
  let to_string = function
    | Shield -> "Shield"
    | Stun -> "Stun"
    | Enhanced_attack -> "Enhanced_attack"
end

module Error_code = struct
  type t =
    | Success
    | Invalid_handle
    | Out_of_memory
    | File_not_found
    | Null_pointer
    | Invalid_argument
    | Map_parse_error
    | Invalid_map_file
    | Invalid_map_version
    | Replay_parse_error
    | Invalid_replay_file
    | Invalid_replay_version
    | Invalid_draft
    | Draft_not_drafting
    | Draft_duplicate_champion
    | Draft_invalid_round
    | Draft_unavailable
    | Already_initialized
    | Invalid_action
    | Invalid_action_memory_bad_range
    | Invalid_action_memory_draft_frozen
    | Invalid_action_move_cant_move
    | Invalid_action_move_bad_destination
    | Invalid_action_spell_cast_invalid_source
    | Invalid_action_spell_cast_invalid_slot
    | Invalid_action_spell_cast_no_ability
    | Invalid_action_spell_cast_cant_cast
    | Invalid_action_spell_cast_already_cast
    | Invalid_action_spell_cast_on_cooldown
    | Invalid_action_spell_cast_no_charges
    | Invalid_action_spell_cast_bad_target
    | Invalid_action_spell_cast_out_of_range
    | Invalid_action_spell_cast_bad_destination
    | Invalid_action_cant_act
    | Invalid_action_is_recalling
    | Invalid_action_is_submitting_gems
    | Invalid_action_move_already_moved
    | Invalid_action_move_out_of_range
    | Invalid_action_move_unreachable
    | Invalid_action_attack_invalid_source
    | Invalid_action_attack_cant_attack
    | Invalid_action_attack_already_attacked
    | Invalid_action_attack_bad_target
    | Invalid_action_attack_out_of_range
    | Invalid_action_recall_cant_channel
    | Invalid_action_submit_gems_cant_channel
    | Invalid_action_submit_gems_no_gems
    | Invalid_action_submit_gems_bad_altar
    | Invalid_action_submit_gems_out_of_range
    | Invalid_action_buy_item_not_on_spawn
    | Invalid_action_buy_item_invalid_item
    | Invalid_action_buy_item_invalid_count
    | Invalid_action_buy_item_not_enough_gold
    | Invalid_action_buy_item_inventory_full
    | Invalid_action_buy_item_stats_overflow
    | Invalid_action_buy_consumable_not_on_spawn
    | Invalid_action_buy_consumable_invalid_type
    | Invalid_action_buy_consumable_invalid_count
    | Invalid_action_buy_consumable_inventory_full
    | Invalid_action_buy_consumable_not_enough_gold
    | Invalid_action_use_consumable_invalid_type
    | Invalid_action_use_consumable_not_owned
    | Invalid_action_place_ward_no_charges
    | Invalid_action_place_ward_cant_place
    | Invalid_action_place_ward_bad_destination
    | Invalid_action_place_ward_out_of_range
    | Map_set_team_aspawn_area_bad_position
    | Map_add_bush_bad_position
    | Map_remove_bush_bad_position
    | Map_add_wall_bad_position
    | Map_remove_wall_bad_position
    | Map_add_hole_bad_position
    | Map_remove_hole_bad_position
    | Map_add_altar_bad_position
    | Map_remove_altar_bad_position
    | Map_get_bush_bad_position
    | Map_get_wall_bad_position
    | Map_get_hole_bad_position
    | Map_get_altar_bad_position
    | Map_get_camp_handle_at_bad_position
    | Map_add_camp_bad_position
    | Map_set_camp_bad_position
    | Invalid_action_champion_teleport_bad_destination
    | Invalid_action_respawn_group_blocked_spawn
    | Invalid_action_camp_teleport_bad_destination
    | Timeline_bad_seek_position
    | Timeline_bad_seek_substep
    | Api_no_map
    | Managed_resources_bad_position
    | Managed_resources_bad_radius
    | Managed_resources_aliasing
    | Managed_resources_metadata_mismatch
    | Managed_resources_opaque_origin
    | Api_invalid_team
    | Api_invalid_perspective
    | Api_not_initialized
    | Api_bad_position
    | Api_not_in_vision
    | Api_not_found
    | Api_invalid_camp_group
    | Api_invalid_camp_info
    | Api_invalid_camp
    | Api_invalid_champion
    | Api_invalid_ward
    | Api_invalid_ability
    | Api_not_teammate
    | Api_camp_group_not_waiting_to_respawn
    | Api_champion_alive
    | Api_invalid_champion_type
    | Api_invalid_camp_type
    | Api_value_out_of_range
    | Api_buffer_too_small
    | Api_not_owner
    | Invalid_action_recall_already_used
    | Invalid_action_submit_gems_already_used
    | Invalid_action_cancel_channel_not_channeling
    | Invalid_action_cancel_channel_already_used
    | Invalid_action_place_ward_ally_ward_present
    | Invalid_action_altar_lock_bad_altar
    | Invalid_action_altar_lock_out_of_range
    | Invalid_action_altar_lock_already_locked
    | Invalid_action_is_teleporting
    | Invalid_action_cancel_channel_not_cancellable
    | Invalid_action_submit_gems_altar_locked
    | Invalid_action_spell_cast_already_moved
    | Invalid_action_move_exclusive_spell_cast
    | Invalid_action_sell_not_tavi
    | Invalid_action_sell_invalid_item
    | Invalid_action_sell_not_owned
    | Invalid_action_sell_already_sold
    | Invalid_action_sell_gold_overflow
    | Invalid_action_sell_not_on_spawn
    | Unknown of int

  let all = [ Success; Invalid_handle; Out_of_memory; File_not_found; Null_pointer; Invalid_argument; Map_parse_error; Invalid_map_file; Invalid_map_version; Replay_parse_error; Invalid_replay_file; Invalid_replay_version; Invalid_draft; Draft_not_drafting; Draft_duplicate_champion; Draft_invalid_round; Draft_unavailable; Already_initialized; Invalid_action; Invalid_action_memory_bad_range; Invalid_action_memory_draft_frozen; Invalid_action_move_cant_move; Invalid_action_move_bad_destination; Invalid_action_spell_cast_invalid_source; Invalid_action_spell_cast_invalid_slot; Invalid_action_spell_cast_no_ability; Invalid_action_spell_cast_cant_cast; Invalid_action_spell_cast_already_cast; Invalid_action_spell_cast_on_cooldown; Invalid_action_spell_cast_no_charges; Invalid_action_spell_cast_bad_target; Invalid_action_spell_cast_out_of_range; Invalid_action_spell_cast_bad_destination; Invalid_action_cant_act; Invalid_action_is_recalling; Invalid_action_is_submitting_gems; Invalid_action_move_already_moved; Invalid_action_move_out_of_range; Invalid_action_move_unreachable; Invalid_action_attack_invalid_source; Invalid_action_attack_cant_attack; Invalid_action_attack_already_attacked; Invalid_action_attack_bad_target; Invalid_action_attack_out_of_range; Invalid_action_recall_cant_channel; Invalid_action_submit_gems_cant_channel; Invalid_action_submit_gems_no_gems; Invalid_action_submit_gems_bad_altar; Invalid_action_submit_gems_out_of_range; Invalid_action_buy_item_not_on_spawn; Invalid_action_buy_item_invalid_item; Invalid_action_buy_item_invalid_count; Invalid_action_buy_item_not_enough_gold; Invalid_action_buy_item_inventory_full; Invalid_action_buy_item_stats_overflow; Invalid_action_buy_consumable_not_on_spawn; Invalid_action_buy_consumable_invalid_type; Invalid_action_buy_consumable_invalid_count; Invalid_action_buy_consumable_inventory_full; Invalid_action_buy_consumable_not_enough_gold; Invalid_action_use_consumable_invalid_type; Invalid_action_use_consumable_not_owned; Invalid_action_place_ward_no_charges; Invalid_action_place_ward_cant_place; Invalid_action_place_ward_bad_destination; Invalid_action_place_ward_out_of_range; Map_set_team_aspawn_area_bad_position; Map_add_bush_bad_position; Map_remove_bush_bad_position; Map_add_wall_bad_position; Map_remove_wall_bad_position; Map_add_hole_bad_position; Map_remove_hole_bad_position; Map_add_altar_bad_position; Map_remove_altar_bad_position; Map_get_bush_bad_position; Map_get_wall_bad_position; Map_get_hole_bad_position; Map_get_altar_bad_position; Map_get_camp_handle_at_bad_position; Map_add_camp_bad_position; Map_set_camp_bad_position; Invalid_action_champion_teleport_bad_destination; Invalid_action_respawn_group_blocked_spawn; Invalid_action_camp_teleport_bad_destination; Timeline_bad_seek_position; Timeline_bad_seek_substep; Api_no_map; Managed_resources_bad_position; Managed_resources_bad_radius; Managed_resources_aliasing; Managed_resources_metadata_mismatch; Managed_resources_opaque_origin; Api_invalid_team; Api_invalid_perspective; Api_not_initialized; Api_bad_position; Api_not_in_vision; Api_not_found; Api_invalid_camp_group; Api_invalid_camp_info; Api_invalid_camp; Api_invalid_champion; Api_invalid_ward; Api_invalid_ability; Api_not_teammate; Api_camp_group_not_waiting_to_respawn; Api_champion_alive; Api_invalid_champion_type; Api_invalid_camp_type; Api_value_out_of_range; Api_buffer_too_small; Api_not_owner; Invalid_action_recall_already_used; Invalid_action_submit_gems_already_used; Invalid_action_cancel_channel_not_channeling; Invalid_action_cancel_channel_already_used; Invalid_action_place_ward_ally_ward_present; Invalid_action_altar_lock_bad_altar; Invalid_action_altar_lock_out_of_range; Invalid_action_altar_lock_already_locked; Invalid_action_is_teleporting; Invalid_action_cancel_channel_not_cancellable; Invalid_action_submit_gems_altar_locked; Invalid_action_spell_cast_already_moved; Invalid_action_move_exclusive_spell_cast; Invalid_action_sell_not_tavi; Invalid_action_sell_invalid_item; Invalid_action_sell_not_owned; Invalid_action_sell_already_sold; Invalid_action_sell_gold_overflow; Invalid_action_sell_not_on_spawn ]
  let of_int = function
    | 0 -> Success
    | 1 -> Invalid_handle
    | 2 -> Out_of_memory
    | 3 -> File_not_found
    | 4 -> Null_pointer
    | 5 -> Invalid_argument
    | 6 -> Map_parse_error
    | 7 -> Invalid_map_file
    | 8 -> Invalid_map_version
    | 9 -> Replay_parse_error
    | 10 -> Invalid_replay_file
    | 11 -> Invalid_replay_version
    | 12 -> Invalid_draft
    | 13 -> Draft_not_drafting
    | 14 -> Draft_duplicate_champion
    | 15 -> Draft_invalid_round
    | 16 -> Draft_unavailable
    | 17 -> Already_initialized
    | 18 -> Invalid_action
    | 19 -> Invalid_action_memory_bad_range
    | 20 -> Invalid_action_memory_draft_frozen
    | 21 -> Invalid_action_move_cant_move
    | 22 -> Invalid_action_move_bad_destination
    | 23 -> Invalid_action_spell_cast_invalid_source
    | 24 -> Invalid_action_spell_cast_invalid_slot
    | 25 -> Invalid_action_spell_cast_no_ability
    | 26 -> Invalid_action_spell_cast_cant_cast
    | 27 -> Invalid_action_spell_cast_already_cast
    | 28 -> Invalid_action_spell_cast_on_cooldown
    | 29 -> Invalid_action_spell_cast_no_charges
    | 30 -> Invalid_action_spell_cast_bad_target
    | 31 -> Invalid_action_spell_cast_out_of_range
    | 32 -> Invalid_action_spell_cast_bad_destination
    | 33 -> Invalid_action_cant_act
    | 34 -> Invalid_action_is_recalling
    | 35 -> Invalid_action_is_submitting_gems
    | 36 -> Invalid_action_move_already_moved
    | 37 -> Invalid_action_move_out_of_range
    | 38 -> Invalid_action_move_unreachable
    | 39 -> Invalid_action_attack_invalid_source
    | 40 -> Invalid_action_attack_cant_attack
    | 41 -> Invalid_action_attack_already_attacked
    | 42 -> Invalid_action_attack_bad_target
    | 43 -> Invalid_action_attack_out_of_range
    | 44 -> Invalid_action_recall_cant_channel
    | 45 -> Invalid_action_submit_gems_cant_channel
    | 46 -> Invalid_action_submit_gems_no_gems
    | 47 -> Invalid_action_submit_gems_bad_altar
    | 48 -> Invalid_action_submit_gems_out_of_range
    | 49 -> Invalid_action_buy_item_not_on_spawn
    | 50 -> Invalid_action_buy_item_invalid_item
    | 51 -> Invalid_action_buy_item_invalid_count
    | 52 -> Invalid_action_buy_item_not_enough_gold
    | 53 -> Invalid_action_buy_item_inventory_full
    | 54 -> Invalid_action_buy_item_stats_overflow
    | 55 -> Invalid_action_buy_consumable_not_on_spawn
    | 56 -> Invalid_action_buy_consumable_invalid_type
    | 57 -> Invalid_action_buy_consumable_invalid_count
    | 58 -> Invalid_action_buy_consumable_inventory_full
    | 59 -> Invalid_action_buy_consumable_not_enough_gold
    | 60 -> Invalid_action_use_consumable_invalid_type
    | 61 -> Invalid_action_use_consumable_not_owned
    | 62 -> Invalid_action_place_ward_no_charges
    | 63 -> Invalid_action_place_ward_cant_place
    | 64 -> Invalid_action_place_ward_bad_destination
    | 65 -> Invalid_action_place_ward_out_of_range
    | 66 -> Map_set_team_aspawn_area_bad_position
    | 67 -> Map_add_bush_bad_position
    | 68 -> Map_remove_bush_bad_position
    | 69 -> Map_add_wall_bad_position
    | 70 -> Map_remove_wall_bad_position
    | 71 -> Map_add_hole_bad_position
    | 72 -> Map_remove_hole_bad_position
    | 73 -> Map_add_altar_bad_position
    | 74 -> Map_remove_altar_bad_position
    | 75 -> Map_get_bush_bad_position
    | 76 -> Map_get_wall_bad_position
    | 77 -> Map_get_hole_bad_position
    | 78 -> Map_get_altar_bad_position
    | 79 -> Map_get_camp_handle_at_bad_position
    | 80 -> Map_add_camp_bad_position
    | 81 -> Map_set_camp_bad_position
    | 82 -> Invalid_action_champion_teleport_bad_destination
    | 83 -> Invalid_action_respawn_group_blocked_spawn
    | 84 -> Invalid_action_camp_teleport_bad_destination
    | 85 -> Timeline_bad_seek_position
    | 86 -> Timeline_bad_seek_substep
    | 87 -> Api_no_map
    | 88 -> Managed_resources_bad_position
    | 89 -> Managed_resources_bad_radius
    | 90 -> Managed_resources_aliasing
    | 91 -> Managed_resources_metadata_mismatch
    | 92 -> Managed_resources_opaque_origin
    | 93 -> Api_invalid_team
    | 94 -> Api_invalid_perspective
    | 95 -> Api_not_initialized
    | 96 -> Api_bad_position
    | 97 -> Api_not_in_vision
    | 98 -> Api_not_found
    | 99 -> Api_invalid_camp_group
    | 100 -> Api_invalid_camp_info
    | 101 -> Api_invalid_camp
    | 102 -> Api_invalid_champion
    | 103 -> Api_invalid_ward
    | 104 -> Api_invalid_ability
    | 105 -> Api_not_teammate
    | 106 -> Api_camp_group_not_waiting_to_respawn
    | 107 -> Api_champion_alive
    | 108 -> Api_invalid_champion_type
    | 109 -> Api_invalid_camp_type
    | 110 -> Api_value_out_of_range
    | 111 -> Api_buffer_too_small
    | 112 -> Api_not_owner
    | 113 -> Invalid_action_recall_already_used
    | 114 -> Invalid_action_submit_gems_already_used
    | 115 -> Invalid_action_cancel_channel_not_channeling
    | 116 -> Invalid_action_cancel_channel_already_used
    | 117 -> Invalid_action_place_ward_ally_ward_present
    | 118 -> Invalid_action_altar_lock_bad_altar
    | 119 -> Invalid_action_altar_lock_out_of_range
    | 120 -> Invalid_action_altar_lock_already_locked
    | 123 -> Invalid_action_is_teleporting
    | 124 -> Invalid_action_cancel_channel_not_cancellable
    | 125 -> Invalid_action_submit_gems_altar_locked
    | 126 -> Invalid_action_spell_cast_already_moved
    | 127 -> Invalid_action_move_exclusive_spell_cast
    | 128 -> Invalid_action_sell_not_tavi
    | 129 -> Invalid_action_sell_invalid_item
    | 130 -> Invalid_action_sell_not_owned
    | 131 -> Invalid_action_sell_already_sold
    | 132 -> Invalid_action_sell_gold_overflow
    | 133 -> Invalid_action_sell_not_on_spawn
    | n -> Unknown n
  let to_string = function
    | Success -> "Success"
    | Invalid_handle -> "Invalid_handle"
    | Out_of_memory -> "Out_of_memory"
    | File_not_found -> "File_not_found"
    | Null_pointer -> "Null_pointer"
    | Invalid_argument -> "Invalid_argument"
    | Map_parse_error -> "Map_parse_error"
    | Invalid_map_file -> "Invalid_map_file"
    | Invalid_map_version -> "Invalid_map_version"
    | Replay_parse_error -> "Replay_parse_error"
    | Invalid_replay_file -> "Invalid_replay_file"
    | Invalid_replay_version -> "Invalid_replay_version"
    | Invalid_draft -> "Invalid_draft"
    | Draft_not_drafting -> "Draft_not_drafting"
    | Draft_duplicate_champion -> "Draft_duplicate_champion"
    | Draft_invalid_round -> "Draft_invalid_round"
    | Draft_unavailable -> "Draft_unavailable"
    | Already_initialized -> "Already_initialized"
    | Invalid_action -> "Invalid_action"
    | Invalid_action_memory_bad_range -> "Invalid_action_memory_bad_range"
    | Invalid_action_memory_draft_frozen -> "Invalid_action_memory_draft_frozen"
    | Invalid_action_move_cant_move -> "Invalid_action_move_cant_move"
    | Invalid_action_move_bad_destination -> "Invalid_action_move_bad_destination"
    | Invalid_action_spell_cast_invalid_source -> "Invalid_action_spell_cast_invalid_source"
    | Invalid_action_spell_cast_invalid_slot -> "Invalid_action_spell_cast_invalid_slot"
    | Invalid_action_spell_cast_no_ability -> "Invalid_action_spell_cast_no_ability"
    | Invalid_action_spell_cast_cant_cast -> "Invalid_action_spell_cast_cant_cast"
    | Invalid_action_spell_cast_already_cast -> "Invalid_action_spell_cast_already_cast"
    | Invalid_action_spell_cast_on_cooldown -> "Invalid_action_spell_cast_on_cooldown"
    | Invalid_action_spell_cast_no_charges -> "Invalid_action_spell_cast_no_charges"
    | Invalid_action_spell_cast_bad_target -> "Invalid_action_spell_cast_bad_target"
    | Invalid_action_spell_cast_out_of_range -> "Invalid_action_spell_cast_out_of_range"
    | Invalid_action_spell_cast_bad_destination -> "Invalid_action_spell_cast_bad_destination"
    | Invalid_action_cant_act -> "Invalid_action_cant_act"
    | Invalid_action_is_recalling -> "Invalid_action_is_recalling"
    | Invalid_action_is_submitting_gems -> "Invalid_action_is_submitting_gems"
    | Invalid_action_move_already_moved -> "Invalid_action_move_already_moved"
    | Invalid_action_move_out_of_range -> "Invalid_action_move_out_of_range"
    | Invalid_action_move_unreachable -> "Invalid_action_move_unreachable"
    | Invalid_action_attack_invalid_source -> "Invalid_action_attack_invalid_source"
    | Invalid_action_attack_cant_attack -> "Invalid_action_attack_cant_attack"
    | Invalid_action_attack_already_attacked -> "Invalid_action_attack_already_attacked"
    | Invalid_action_attack_bad_target -> "Invalid_action_attack_bad_target"
    | Invalid_action_attack_out_of_range -> "Invalid_action_attack_out_of_range"
    | Invalid_action_recall_cant_channel -> "Invalid_action_recall_cant_channel"
    | Invalid_action_submit_gems_cant_channel -> "Invalid_action_submit_gems_cant_channel"
    | Invalid_action_submit_gems_no_gems -> "Invalid_action_submit_gems_no_gems"
    | Invalid_action_submit_gems_bad_altar -> "Invalid_action_submit_gems_bad_altar"
    | Invalid_action_submit_gems_out_of_range -> "Invalid_action_submit_gems_out_of_range"
    | Invalid_action_buy_item_not_on_spawn -> "Invalid_action_buy_item_not_on_spawn"
    | Invalid_action_buy_item_invalid_item -> "Invalid_action_buy_item_invalid_item"
    | Invalid_action_buy_item_invalid_count -> "Invalid_action_buy_item_invalid_count"
    | Invalid_action_buy_item_not_enough_gold -> "Invalid_action_buy_item_not_enough_gold"
    | Invalid_action_buy_item_inventory_full -> "Invalid_action_buy_item_inventory_full"
    | Invalid_action_buy_item_stats_overflow -> "Invalid_action_buy_item_stats_overflow"
    | Invalid_action_buy_consumable_not_on_spawn -> "Invalid_action_buy_consumable_not_on_spawn"
    | Invalid_action_buy_consumable_invalid_type -> "Invalid_action_buy_consumable_invalid_type"
    | Invalid_action_buy_consumable_invalid_count -> "Invalid_action_buy_consumable_invalid_count"
    | Invalid_action_buy_consumable_inventory_full -> "Invalid_action_buy_consumable_inventory_full"
    | Invalid_action_buy_consumable_not_enough_gold -> "Invalid_action_buy_consumable_not_enough_gold"
    | Invalid_action_use_consumable_invalid_type -> "Invalid_action_use_consumable_invalid_type"
    | Invalid_action_use_consumable_not_owned -> "Invalid_action_use_consumable_not_owned"
    | Invalid_action_place_ward_no_charges -> "Invalid_action_place_ward_no_charges"
    | Invalid_action_place_ward_cant_place -> "Invalid_action_place_ward_cant_place"
    | Invalid_action_place_ward_bad_destination -> "Invalid_action_place_ward_bad_destination"
    | Invalid_action_place_ward_out_of_range -> "Invalid_action_place_ward_out_of_range"
    | Map_set_team_aspawn_area_bad_position -> "Map_set_team_aspawn_area_bad_position"
    | Map_add_bush_bad_position -> "Map_add_bush_bad_position"
    | Map_remove_bush_bad_position -> "Map_remove_bush_bad_position"
    | Map_add_wall_bad_position -> "Map_add_wall_bad_position"
    | Map_remove_wall_bad_position -> "Map_remove_wall_bad_position"
    | Map_add_hole_bad_position -> "Map_add_hole_bad_position"
    | Map_remove_hole_bad_position -> "Map_remove_hole_bad_position"
    | Map_add_altar_bad_position -> "Map_add_altar_bad_position"
    | Map_remove_altar_bad_position -> "Map_remove_altar_bad_position"
    | Map_get_bush_bad_position -> "Map_get_bush_bad_position"
    | Map_get_wall_bad_position -> "Map_get_wall_bad_position"
    | Map_get_hole_bad_position -> "Map_get_hole_bad_position"
    | Map_get_altar_bad_position -> "Map_get_altar_bad_position"
    | Map_get_camp_handle_at_bad_position -> "Map_get_camp_handle_at_bad_position"
    | Map_add_camp_bad_position -> "Map_add_camp_bad_position"
    | Map_set_camp_bad_position -> "Map_set_camp_bad_position"
    | Invalid_action_champion_teleport_bad_destination -> "Invalid_action_champion_teleport_bad_destination"
    | Invalid_action_respawn_group_blocked_spawn -> "Invalid_action_respawn_group_blocked_spawn"
    | Invalid_action_camp_teleport_bad_destination -> "Invalid_action_camp_teleport_bad_destination"
    | Timeline_bad_seek_position -> "Timeline_bad_seek_position"
    | Timeline_bad_seek_substep -> "Timeline_bad_seek_substep"
    | Api_no_map -> "Api_no_map"
    | Managed_resources_bad_position -> "Managed_resources_bad_position"
    | Managed_resources_bad_radius -> "Managed_resources_bad_radius"
    | Managed_resources_aliasing -> "Managed_resources_aliasing"
    | Managed_resources_metadata_mismatch -> "Managed_resources_metadata_mismatch"
    | Managed_resources_opaque_origin -> "Managed_resources_opaque_origin"
    | Api_invalid_team -> "Api_invalid_team"
    | Api_invalid_perspective -> "Api_invalid_perspective"
    | Api_not_initialized -> "Api_not_initialized"
    | Api_bad_position -> "Api_bad_position"
    | Api_not_in_vision -> "Api_not_in_vision"
    | Api_not_found -> "Api_not_found"
    | Api_invalid_camp_group -> "Api_invalid_camp_group"
    | Api_invalid_camp_info -> "Api_invalid_camp_info"
    | Api_invalid_camp -> "Api_invalid_camp"
    | Api_invalid_champion -> "Api_invalid_champion"
    | Api_invalid_ward -> "Api_invalid_ward"
    | Api_invalid_ability -> "Api_invalid_ability"
    | Api_not_teammate -> "Api_not_teammate"
    | Api_camp_group_not_waiting_to_respawn -> "Api_camp_group_not_waiting_to_respawn"
    | Api_champion_alive -> "Api_champion_alive"
    | Api_invalid_champion_type -> "Api_invalid_champion_type"
    | Api_invalid_camp_type -> "Api_invalid_camp_type"
    | Api_value_out_of_range -> "Api_value_out_of_range"
    | Api_buffer_too_small -> "Api_buffer_too_small"
    | Api_not_owner -> "Api_not_owner"
    | Invalid_action_recall_already_used -> "Invalid_action_recall_already_used"
    | Invalid_action_submit_gems_already_used -> "Invalid_action_submit_gems_already_used"
    | Invalid_action_cancel_channel_not_channeling -> "Invalid_action_cancel_channel_not_channeling"
    | Invalid_action_cancel_channel_already_used -> "Invalid_action_cancel_channel_already_used"
    | Invalid_action_place_ward_ally_ward_present -> "Invalid_action_place_ward_ally_ward_present"
    | Invalid_action_altar_lock_bad_altar -> "Invalid_action_altar_lock_bad_altar"
    | Invalid_action_altar_lock_out_of_range -> "Invalid_action_altar_lock_out_of_range"
    | Invalid_action_altar_lock_already_locked -> "Invalid_action_altar_lock_already_locked"
    | Invalid_action_is_teleporting -> "Invalid_action_is_teleporting"
    | Invalid_action_cancel_channel_not_cancellable -> "Invalid_action_cancel_channel_not_cancellable"
    | Invalid_action_submit_gems_altar_locked -> "Invalid_action_submit_gems_altar_locked"
    | Invalid_action_spell_cast_already_moved -> "Invalid_action_spell_cast_already_moved"
    | Invalid_action_move_exclusive_spell_cast -> "Invalid_action_move_exclusive_spell_cast"
    | Invalid_action_sell_not_tavi -> "Invalid_action_sell_not_tavi"
    | Invalid_action_sell_invalid_item -> "Invalid_action_sell_invalid_item"
    | Invalid_action_sell_not_owned -> "Invalid_action_sell_not_owned"
    | Invalid_action_sell_already_sold -> "Invalid_action_sell_already_sold"
    | Invalid_action_sell_gold_overflow -> "Invalid_action_sell_gold_overflow"
    | Invalid_action_sell_not_on_spawn -> "Invalid_action_sell_not_on_spawn"
    | Unknown n -> "Unknown " ^ string_of_int n
end

exception Cambc_error of Error_code.t * string
exception No_range
exception Missing_identity

let fail code msg = raise (Cambc_error (code, msg))
let ensure n = if Sys.ensure n <> 0 then fail Error_code.Api_buffer_too_small "SDK scratch memory exhausted"

let () =
  Printexc.register_printer (function
    | Cambc_error (c, m) ->
        Some (Printf.sprintf "Cambc.Cambc_error(%s, %S)" (Error_code.to_string c) m)
    | _ -> None)

let last_error () =
  let n = Sys.bc_get_last_error 0 0 in
  if n <= 0 then ""
  else begin
    (ensure (slots + n));
    ignore (Sys.bc_get_last_error slots n);
    let b = Bytes.create n in
    ignore (Sys.blit_out b slots n);
    let s = Bytes.to_string b in
    match String.index_opt s '\000' with Some i -> String.sub s 0 i | None -> s
  end

let check code = if code <> 0 then fail (Error_code.of_int code) (last_error ())
let query_i32 f = check (f out); i32 out
let query_u32 f = check (f out); u32 out
let query_f32 f = check (f out); f32 out
let query_bool f = query_i32 f <> 0
let missing = function
  | Cambc_error ((Error_code.Api_not_found | Error_code.Api_not_in_vision), _) -> true
  | _ -> false
let optional f x = try Some (f x) with e when missing e -> None

let vec2_div (a : Vec2_base.t) n =
  if n = 0 then fail Error_code.Invalid_argument "invalid integer division";
  Vec2_base.make (a.x / n) (a.y / n)

module Vec2 = struct
  include Vec2_base

  let div = vec2_div
end

let my_team () = Sys.my_team 0
let team_abs = function
  | Team.Ally -> my_team ()
  | Team.Enemy -> 1 - my_team ()
let team_of_abs v =
  if v < 0 || v > 1 then fail Error_code.Api_value_out_of_range "invalid team value";
  if v = my_team () then Team.Ally else Team.Enemy

module type FLAGS = sig
  type flag
  type t

  val empty : t
  val all : flag list
  val mem : flag -> t -> bool
  val add : flag -> t -> t
  val remove : flag -> t -> t
  val union : t -> t -> t
  val inter : t -> t -> t
  val diff : t -> t -> t
  val subset : t -> t -> bool
  val is_empty : t -> bool
  val equal : t -> t -> bool
  val hash : t -> int
  val of_list : flag list -> t
  val to_list : t -> flag list
end

module Flags (F : sig
  type flag

  val all : flag list
end) =
struct
  type t = int

  let all = F.all
  let bit f =
    let rec go i = function [] -> 0 | x :: r -> if x = f then 1 lsl i else go (i + 1) r in
    go 0 all
  let of_bits b = b
  let empty = 0
  let mem f t = t land bit f <> 0
  let add f t = t lor bit f
  let remove f t = t land lnot (bit f)
  let union = ( lor )
  let inter = ( land )
  let diff a b = a land lnot b
  let subset a b = a land lnot b = 0
  let is_empty t = t = 0
  let equal = Int.equal
  let hash (t : t) = Hashtbl.hash t
  let of_list = List.fold_left (fun t f -> add f t) 0
  let to_list t = List.filter (fun f -> mem f t) all
end

module Ability_target = struct
  type flag = Camps | Enemies | Allies | Self | Ground

  include Flags (struct
    type nonrec flag = flag

    let all = [ Camps; Enemies; Allies; Self; Ground ]
  end)
end

module Buff_restrictions = struct
  type flag = Move | Attack | Cast | Shop | Consumable | Ward

  include Flags (struct
    type nonrec flag = flag

    let all = [ Move; Attack; Cast; Shop; Consumable; Ward ]
  end)
end

module Champion_state = struct
  type flag =
    | Alive
    | Recalling
    | Submitting_gems
    | Can_move
    | Can_attack
    | Completed_turn
    | Recall_started
    | Submit_gems_started
    | Recall_cancelled
    | Submit_gems_cancelled
    | Can_cast
    | Teleporting

  type t = int
  let all =
    [ Alive; Recalling; Submitting_gems; Can_move; Can_attack; Completed_turn; Recall_started;
      Submit_gems_started; Recall_cancelled; Submit_gems_cancelled; Can_cast; Teleporting ]
  let bit = function
    | Alive -> 1 lsl 0 | Recalling -> 1 lsl 1 | Submitting_gems -> 1 lsl 2
    | Can_move -> 1 lsl 3 | Can_attack -> 1 lsl 4 | Completed_turn -> 1 lsl 5
    | Recall_started -> 1 lsl 6 | Submit_gems_started -> 1 lsl 7
    | Recall_cancelled -> 1 lsl 8 | Submit_gems_cancelled -> 1 lsl 9
    | Can_cast -> 1 lsl 11 | Teleporting -> 1 lsl 12
  let of_bits bits = bits
  let empty = 0
  let mem flag value = value land bit flag <> 0
  let add flag value = value lor bit flag
  let remove flag value = value land lnot (bit flag)
  let union = ( lor )
  let inter = ( land )
  let diff a b = a land lnot b
  let subset a b = a land lnot b = 0
  let is_empty value = value = 0
  let equal = Int.equal
  let hash (value : t) = Hashtbl.hash value
  let of_list = List.fold_left (fun value flag -> add flag value) empty
  let to_list value = List.filter (fun flag -> mem flag value) all
  let is_dead t = not (mem Alive t)
end

module Constants = struct
  type t = {
    capture_time : int;
    camp_sleep_time : int;
    respawn_time : int;
    recall_time : int;
    submit_gem_duration_multiplier : float;
    submit_gem_base_duration : int;
    submit_gem_minimum_duration : int;
    submit_gems_range : int;
    max_rounds : int;
    winning_gems : int;
    kill_gold : int;
    assist_gold : int;
    passive_gold : int;
    spawn_healing : float;
    active_wards : int;
    shared_memory_bytes : int;
    draft_memory_bytes : int;
    tile_reveal_rounds : int;
    ward_cooldown : int;
    ward_lifetime : int;
    ward_vision_radius : float;
    ward_placement_radius : float;
    altar_lock_hits : int;
    tp_crystal_time : int;
  }

  let load () =
    let o = rec_ in
    check (Sys.game_settings o);
    {
      capture_time = i32 o;
      camp_sleep_time = i32 (o + 4);
      respawn_time = i32 (o + 8);
      recall_time = i32 (o + 12);
      submit_gem_duration_multiplier = f64 (o + 16);
      submit_gem_base_duration = i32 (o + 24);
      submit_gem_minimum_duration = i32 (o + 28);
      submit_gems_range = i32 (o + 32);
      max_rounds = u32 (o + 36);
      winning_gems = u32 (o + 40);
      kill_gold = u32 (o + 44);
      assist_gold = u32 (o + 48);
      passive_gold = u32 (o + 52);
      spawn_healing = f32 (o + 56);
      active_wards = u32 (o + 60);
      shared_memory_bytes = u32 (o + 68);
      draft_memory_bytes = u32 (o + 72);
      tile_reveal_rounds = u32 (o + 76);
      ward_cooldown = i32 (o + 80);
      ward_lifetime = i32 (o + 84);
      ward_vision_radius = f32 (o + 88);
      ward_placement_radius = f32 (o + 92);
      altar_lock_hits = u32 (o + 104);
      tp_crystal_time = i32 (o + 112);
    }

  let cached = lazy (load ())
  let get () = Lazy.force cached

  module Passive_abilities = struct
    let prism_focus_per_gem = 0.2
    let varg_splash_radius = 1.0
    let vesper_vale_bonus_gold_percent = 20
    let ryth_health_strength_ratio = 0.01
    let ren_retaliation_focus_ratio = 0.2
  end
end

module Item_passive = struct
  type t = Monster | Durability | Treasure | Jungle | Home

  let to_string = function
    | Monster -> "Monster"
    | Durability -> "Durability"
    | Treasure -> "Treasure"
    | Jungle -> "Jungle"
    | Home -> "Home"
end

module Item_info = struct
  type t = {
    cost : int;
    combine_count : int;
    result : Item.t option;
    health_points : int;
    strength_points : int;
    focus_points : int;
    passive : Item_passive.t option;
    passive_magnitude : float;
    purchasable : bool;
  }

  let passive_of_int = function
    | 0 -> None
    | 1 -> Some Item_passive.Monster
    | 2 -> Some Item_passive.Durability
    | 3 -> Some Item_passive.Treasure
    | 4 -> Some Item_passive.Jungle
    | 5 -> Some Item_passive.Home
    | n -> fail Error_code.Api_value_out_of_range (Printf.sprintf "invalid item passive value: %d" n)

  let get kind =
    let o = rec_ in
    check (Sys.item_info (Item.to_int kind) o);
    {
      cost = u32 o;
      purchasable = u32 (o + 4) <> 0;
      health_points = i32 (o + 8);
      strength_points = i32 (o + 12);
      focus_points = i32 (o + 16);
      passive = passive_of_int (u32 (o + 20));
      passive_magnitude = f32 (o + 24);
      combine_count = u32 (o + 28);
      result = (match u32 (o + 32) with 0xff -> None | r -> Some (Item.of_int r));
    }

  let cost k = (get k).cost
  let is_purchasable k = (get k).purchasable
  let combine_count k = (get k).combine_count
  let result k = (get k).result
  let health_points k = (get k).health_points
  let strength_points k = (get k).strength_points
  let focus_points k = (get k).focus_points
  let passive k = (get k).passive
  let passive_magnitude k = (get k).passive_magnitude
end

module Consumable = struct
  type t = Tp_crystal | Altar_lock
  type info = { cost : int; carry_limit : int }

  let all = [ Tp_crystal; Altar_lock ]
  let to_int = function Tp_crystal -> 0 | Altar_lock -> 1
  let to_string = function Tp_crystal -> "Tp_crystal" | Altar_lock -> "Altar_lock"

  let info k =
    check (Sys.consumable_info (to_int k) rec_);
    { cost = u32 rec_; carry_limit = u32 (rec_ + 4) }

  let cost k = (info k).cost
  let carry_limit k = (info k).carry_limit
end

module Item_counts = struct
  type t = int array

  let get t k = t.(Item.to_int k)
  let iter f t = List.iter (fun k -> f k (get t k)) Item.all
  let fold f t init = List.fold_left (fun acc k -> f k (get t k) acc) init Item.all
  let to_list t = List.map (fun k -> (k, get t k)) Item.all

  let read o =
    let a = Array.make (List.length Item.all) 0 in
    List.iter (fun k -> let i = Item.to_int k in a.(i) <- u32 (o + (4 * i))) Item.all;
    a
end

module Consumable_counts = struct
  type t = int array

  let get t k = t.(Consumable.to_int k)
  let iter f t = List.iter (fun k -> f k (get t k)) Consumable.all
  let fold f t init = List.fold_left (fun acc k -> f k (get t k) acc) init Consumable.all
  let to_list t = List.map (fun k -> (k, get t k)) Consumable.all

  let read o =
    let a = Array.make (List.length Consumable.all) 0 in
    List.iter (fun k -> let i = Consumable.to_int k in a.(i) <- u32 (o + (4 * i))) Consumable.all;
    a
end

(* Board layout: width, height, board bytes, field bytes, slot size. *)
type layout = { w : int; h : int; board_bytes : int; field_bytes : int; slot : int }

let layout =
  lazy
    (check (Sys.bc_metadata rec_);
     let w = i32 rec_ and h = i32 (rec_ + 4) in
     let board_bytes = u32 (rec_ + 20) and field_bytes = u32 (rec_ + 24) in
     if w <= 0 || h <= 0 then fail Error_code.Invalid_argument "invalid host dimensions";
     let slot = (max board_bytes field_bytes + 15) land lnot 15 in
     (ensure (slots + (8 * slot)));
     { w; h; board_bytes; field_bytes; slot })

let slot k = slots + (k * (Lazy.force layout).slot)
let list_area () = slot 8

(* Fetch a host-filled list of [size]-byte records: count query, then fill. *)
let host_list size f =
  let count = query_u32 (fun o -> f 0 0 o) in
  let base = list_area () in
  (ensure (base + (count * size) + 16));
  let written = query_u32 (fun o -> f base count o) in
  if written > count then fail Error_code.Invalid_argument "host returned oversized list";
  List.init written (fun i -> base + (i * size))

let counted_list count size f =
  let base = list_area () in
  (ensure (base + (count * size) + 16));
  let written = query_u32 (fun o -> f base count o) in
  if written > count then fail Error_code.Invalid_argument "host returned oversized list";
  List.init written (fun i -> base + (i * size))

module Bitboard = struct
  type t = Bytes.t

  let size () = (Lazy.force layout).board_bytes
  let create () = Bytes.make (size ()) '\000'
  let copy = Bytes.copy
  let width _ = (Lazy.force layout).w
  let height _ = (Lazy.force layout).h
  let raw_buffer_size t = Bytes.length t
  let linear_buffer_size _ = let l = Lazy.force layout in ((l.w * l.h) + 7) / 8
  let load k t = ignore (Sys.blit_in t (slot k) (Bytes.length t))
  let store k t = ignore (Sys.blit_out t (slot k) (Bytes.length t))
  let clear t = Bytes.fill t 0 (Bytes.length t) '\000'

  (* Run a host query writing a board to linear slot 7 and return it as a new board. *)
  let query_into f out =
    clear out;
    load 7 out;
    check (f (slot 7));
    store 7 out

  let query f = let out = create () in query_into f out; out

  let make f = let b = create () in check (f b); b
  let fill t = check (Sys.bc_bitboard_one t)
  let is_set t (p : Vec2.t) = query_bool (Sys.bc_bitboard_test t p.x p.y)
  let set t (p : Vec2.t) v = check (Sys.bc_bitboard_set t p.x p.y (Bool.to_int v))
  let toggle t (p : Vec2.t) = check (Sys.bc_bitboard_toggle t p.x p.y)
  let is_empty t = query_bool (Sys.bc_bitboard_iszero t)
  let count t = query_u32 (Sys.bc_bitboard_numsetbits t)

  let positions t =
    let n = count t in
    let base = list_area () in
    (ensure (base + (n * 8) + 16));
    let written = query_u32 (Sys.bc_bitboard_getsetbits t base n) in
    List.init (min n written) (fun i -> Vec2.make (i32 (base + (8 * i))) (i32 (base + (8 * i) + 4)))

  let print t = check (Sys.bc_bitboard_print t)
  let equal a b = query_bool (Sys.bc_bitboard_isequal a b)
  let binary f a b = make (f a b)
  let unary f a = make (f a)
  let complement = unary Sys.bc_bitboard_not
  let union = binary Sys.bc_bitboard_union
  let intersect = binary Sys.bc_bitboard_intersect
  let xor = binary Sys.bc_bitboard_xor
  let assign f a b = check (f a b a)
  let union_assign = assign Sys.bc_bitboard_union
  let intersect_assign = assign Sys.bc_bitboard_intersect
  let xor_assign = assign Sys.bc_bitboard_xor
  let shift_right_into t n out = check (Sys.bc_bitboard_shiftright t out n)
  let shift_right t n = let out = create () in shift_right_into t n out; out
  let shift_up_into t n out = check (Sys.bc_bitboard_shiftup t out n)
  let shift_up t n = let out = create () in shift_up_into t n out; out
  let reflect_into t s out = check (Sys.bc_bitboard_reflect t out (Symmetry.to_int s))
  let reflect t s = let out = create () in reflect_into t s out; out
  let reflect_horizontal_into t out = reflect_into t Symmetry.Horizontal out
  let reflect_horizontal t = let out = create () in reflect_horizontal_into t out; out
  let reflect_vertical_into t out = reflect_into t Symmetry.Vertical out
  let reflect_vertical t = let out = create () in reflect_vertical_into t out; out
  let rotate_180_into t out = reflect_into t Symmetry.Rotational180 out
  let rotate_180 t = let out = create () in rotate_180_into t out; out

  let draw f t (c : Vec2.t) r =
    set_f32 args r;
    check (f t c.x c.y args)

  let draw_circle = draw Sys.bc_bitboard_drawcircle
  let draw_chebyshev = draw Sys.bc_bitboard_drawchebyshev

  let reachable_tiles_into ?scratch t (p : Vec2.t) ~distance out =
    match scratch with
    | None -> check (Sys.bc_bitboard_reachablemask t p.x p.y out distance)
    | Some (frontier, scr) ->
        check (Sys.bc_bitboard_reachablemaskwithscratch [| t; out; frontier; scr |] p.x p.y distance)

  let reachable_tiles ?scratch t p ~distance =
    let out = create () in
    reachable_tiles_into ?scratch t p ~distance out;
    out

  let shadow_cast_into t (p : Vec2.t) ~distance out =
    check (Sys.bc_bitboard_shadowcast t p.x p.y out distance)

  let shadow_cast t p ~distance =
    let out = create () in
    shadow_cast_into t p ~distance out;
    out
  let import f t (b : Bytes.t) = check (f t b (Bytes.length b))
  let export f t n = let b = Bytes.create n in check (f t b n); b
  let import_raw_buffer = import Sys.bc_bitboard_importrawbuffer
  let export_raw_buffer t = export Sys.bc_bitboard_exportrawbuffer t (raw_buffer_size t)
  let import_linear_buffer = import Sys.bc_bitboard_importlinearbuffer
  let export_linear_buffer t = export Sys.bc_bitboard_exportlinearbuffer t (linear_buffer_size t)
  let export_raw_buffer_into t b = import Sys.bc_bitboard_exportrawbuffer t b
  let export_linear_buffer_into t b = import Sys.bc_bitboard_exportlinearbuffer t b
end

module Distance_field = struct
  type t = Bytes.t

  let unreachable = 0xffff
  let create () = Bytes.make (Lazy.force layout).field_bytes '\000'
  let copy = Bytes.copy
  let width _ = (Lazy.force layout).w
  let height _ = (Lazy.force layout).h
  let raw_buffer_size t = Bytes.length t
  let linear_buffer_size _ = let l = Lazy.force layout in l.w * l.h * 2
  let get t (p : Vec2.t) = check (Sys.bc_distancefield_get t p.x p.y out); Sys.ld16 out
  let set t (p : Vec2.t) v = check (Sys.bc_distancefield_set t p.x p.y v)
  let print t = check (Sys.bc_distancefield_print t)
  let import_raw_buffer = Bitboard.import Sys.bc_distancefield_importrawbuffer
  let export_raw_buffer t = Bitboard.export Sys.bc_distancefield_exportrawbuffer t (raw_buffer_size t)

  let import_linear_buffer t a =
    let b = Bytes.create (2 * Array.length a) in
    Array.iteri (fun i v -> Bytes.set_uint16_le b (2 * i) v) a;
    Bitboard.import Sys.bc_distancefield_importlinearbuffer t b

  let export_linear_buffer t =
    let b = Bitboard.export Sys.bc_distancefield_exportlinearbuffer t (linear_buffer_size t) in
    Array.init (Bytes.length b / 2) (fun i -> Bytes.get_uint16_le b (2 * i))

  let export_raw_buffer_into t b = Bitboard.import Sys.bc_distancefield_exportrawbuffer t b

  let export_linear_buffer_into t a =
    let b = Bitboard.export Sys.bc_distancefield_exportlinearbuffer t (2 * Array.length a) in
    Array.iteri (fun i _ -> a.(i) <- Bytes.get_uint16_le b (2 * i)) a

  let compute_with_expansion_count ?destinations t ~walls ~roots ~visited ~max_distance =
    check
      (match destinations with
       | None -> Sys.bc_distancefield_computewithexpansioncount [| walls; roots; visited; t |] max_distance out
       | Some d -> Sys.bc_distancefield_computetowithexpansioncount [| walls; roots; d; visited; t |] max_distance out);
    Sys.ld16 out

  let compute ?destinations t ~walls ~roots ~visited ~max_distance =
    ignore (compute_with_expansion_count ?destinations t ~walls ~roots ~visited ~max_distance)

  let directions =
    Vec2.[ north; north_east; east; south_east; south; south_west; west; north_west ]

  let next_moves_from t (p : Vec2.t) =
    let here = get t p in
    let l = Lazy.force layout in
    let distance (d : Vec2.t) =
      let x = p.x + d.x and y = p.y + d.y in
      if x < 0 || y < 0 || x >= l.w || y >= l.h then unreachable else get t (Vec2.make x y)
    in
    let scored = List.map (fun d -> (d, distance d)) directions in
    let best = List.fold_left (fun b (_, v) -> min b v) unreachable scored in
    if here = unreachable || best >= here then []
    else List.filter_map (fun (d, v) -> if v = best then Some d else None) scored

  let next_move_from t p = match next_moves_from t p with [] -> None | d :: _ -> Some d
end

module Buff = struct
  type t =
    | Shield of { duration : int; value : float }
    | Stun of { duration : int }
    | Enhanced_attack of { duration : int }

  let kind = function Shield _ -> Buff_kind.Shield | Stun _ -> Buff_kind.Stun | Enhanced_attack _ -> Buff_kind.Enhanced_attack
  let duration = function Shield { duration; _ } | Stun { duration } | Enhanced_attack { duration } -> duration
  let shield_value = function Shield { value; _ } -> Some value | _ -> None
  let get l k = List.find_opt (fun b -> kind b = k) l
  let has l k = get l k <> None
  let has_shield l = has l Buff_kind.Shield
  let has_stun l = has l Buff_kind.Stun
  let has_enhanced_attack l = has l Buff_kind.Enhanced_attack

  let decode kind duration value =
    match Buff_kind.of_int kind with
    | Buff_kind.Shield -> Shield { duration; value }
    | Buff_kind.Stun -> Stun { duration }
    | Buff_kind.Enhanced_attack -> Enhanced_attack { duration }
end

module Ability_info = struct
  type properties = {
    max_cooldown : int;
    valid_targets : Ability_target.t;
    exclusive_with_movement : bool;
  }

  type info =
    | Attack_reset of { properties : properties; enhanced_attack : bool }
    | Damage of { properties : properties; range : float; modifier : float }
    | Aoe_damage of {
        properties : properties;
        range : float;
        modifier : float;
        radius : float;
        duration : int;
      }
    | Cc of { properties : properties; range : float; modifier : float; duration : int }
    | Heal of { properties : properties; range : float; modifier : float }
    | Dash of { properties : properties; range : float }
    | Shield of { properties : properties; range : float; modifier : float; duration : int }

  let decode o =
    match u32 o with
    | 0 -> None
    | kind ->
        let properties =
          { max_cooldown = i32 (o + 4); valid_targets = Ability_target.of_bits (i32 (o + 12));
            exclusive_with_movement = i32 (o + 16) land 1 <> 0 }
        in
        let p k = o + 20 + (4 * k) in
        let range = f32 (p 0) and modifier = f32 (p 1) in
        Some
          (match Ability_kind.of_int kind with
           | Ability_kind.Attack_reset -> Attack_reset { properties; enhanced_attack = i32 (p 0) <> 0 }
           | Ability_kind.Damage -> Damage { properties; range; modifier }
           | Ability_kind.Aoe_damage ->
               Aoe_damage { properties; range; radius = f32 (p 1); modifier = f32 (p 2); duration = i32 (p 3) }
           | Ability_kind.Cc -> Cc { properties; range; modifier; duration = i32 (p 2) }
           | Ability_kind.Heal -> Heal { properties; range; modifier }
           | Ability_kind.Dash -> Dash { properties; range }
           | Ability_kind.Shield -> Shield { properties; range; modifier; duration = i32 (p 2) })
end

type ability = { owner : int option; slot : int; ab_info : (Ability_info.info option, exn) result }
type champion = { handle : int option; design : Champion_type.t option; known : Champion_type.t option; abilities : ability array }

type camp = { camp_handle : int; group : int; camp_kind : Camp_type.t; spawn : Vec2.t }

let design_error () = raise Missing_identity
let owner_of = function Some h -> h | None -> design_error ()

let champion_raw h = check (Sys.champion h rec_); rec_

let live_champion_state h = Champion_state.of_bits (i32 (champion_raw h + 40))

let live_ability h slot =
  let ab_info =
    match check (Sys.champion_ability_props h slot rec_) with
    | () -> Ok (Ability_info.decode rec_)
    | exception (Cambc_error (Error_code.Api_not_in_vision, _) as e) -> Error e
  in
  { owner = Some h; slot; ab_info }

let make_champion h = { handle = Some h; design = None; known = None; abilities = [| live_ability h 0; live_ability h 1 |] }

let draw_cheb_into (p : Vec2.t) r out =
  Bitboard.clear out;
  Bitboard.draw_chebyshev out p r

let draw_cheb p r =
  let out = Bitboard.create () in
  draw_cheb_into p r out;
  out

let champion_name c =
  match c with
  | { design = Some k; _ } | { known = Some k; _ } -> k
  | { handle = Some h; _ } -> Champion_type.of_int (i32 (champion_raw h + 4))
  | _ -> design_error ()

let hypothetical_vision_into kind (p : Vec2.t) out =
  Bitboard.query_into (fun o -> Sys.board_vision_mask (Champion_type.to_int kind) p.x p.y o) out

let hypothetical_vision kind p =
  let out = Bitboard.create () in
  hypothetical_vision_into kind p out;
  out

module Ability = struct
  include Ability_info

  type t = ability

  let kind = function
    | Attack_reset _ -> Ability_kind.Attack_reset
    | Damage _ -> Ability_kind.Damage
    | Aoe_damage _ -> Ability_kind.Aoe_damage
    | Cc _ -> Ability_kind.Cc
    | Heal _ -> Ability_kind.Heal
    | Dash _ -> Ability_kind.Dash
    | Shield _ -> Ability_kind.Shield

  let properties = function
    | Attack_reset { properties; _ } | Damage { properties; _ } | Aoe_damage { properties; _ }
    | Cc { properties; _ } | Heal { properties; _ } | Dash { properties; _ } | Shield { properties; _ } ->
        properties

  let range = function
    | Attack_reset _ -> None
    | Damage { range; _ } | Aoe_damage { range; _ } | Cc { range; _ } | Heal { range; _ }
    | Dash { range; _ } | Shield { range; _ } ->
        Some range

  let slot t = t.slot
  let info t = match t.ab_info with Ok i -> i | Error e -> raise e
  let is t k = match t.ab_info with Ok (Some i) -> kind i = k | _ -> false
  let is_attack_reset t = is t Ability_kind.Attack_reset
  let is_damage t = is t Ability_kind.Damage
  let is_aoe_damage t = is t Ability_kind.Aoe_damage
  let is_cc t = is t Ability_kind.Cc
  let is_heal t = is t Ability_kind.Heal
  let is_dash t = is t Ability_kind.Dash
  let is_shield t = is t Ability_kind.Shield
  let cooldown t = query_i32 (Sys.champion_cooldown (owner_of t.owner) t.slot)
  let is_on_cooldown t = cooldown t > 0
  let is_ready t =
    match info t with
    | None -> false
    | Some _ ->
        not (is_on_cooldown t)
        && Champion_state.mem Champion_state.Can_cast (live_champion_state (owner_of t.owner))

  let valid_targets t =
    match t.owner with
    | Some h -> check (Sys.champion_ability_props h t.slot rec_); Ability_target.of_bits (i32 (rec_ + 12))
    | None -> ( match info t with Some i -> (properties i).valid_targets | None -> Ability_target.empty)

  let mask_into t out =
    match info t with
    | None -> fail Error_code.Invalid_action_spell_cast_no_ability "slot has no ability"
    | Some i -> (
        match range i with
        | None -> raise No_range
        | Some r ->
            let o = champion_raw (owner_of t.owner) in
            draw_cheb_into (Vec2.make (i32 (o + 8)) (i32 (o + 12))) r out)

  let valid_tiles t =
    let out = Bitboard.create () in
    mask_into t out;
    out

  let can t f = Ability_target.mem f (valid_targets t)
  let can_target_camps t = can t Ability_target.Camps
  let can_target_enemies t = can t Ability_target.Enemies
  let can_target_allies t = can t Ability_target.Allies
  let can_target_self t = can t Ability_target.Self
  let can_target_ground t = can t Ability_target.Ground

  let amount plain against ?against:target t =
    let h = owner_of t.owner in
    match target with
    | None -> query_f32 (plain h t.slot)
    | Some c -> query_f32 (against h t.slot (owner_of c.handle))

  let damage_amount = amount Sys.champion_ability_damage Sys.champion_ability_damage_against
  let heal_amount = amount Sys.champion_ability_heal Sys.champion_ability_heal_against
  let shield_amount = amount Sys.champion_ability_shield Sys.champion_ability_shield_against

  let cast ?target ?at t =
    (match target, at with
    | Some _, Some _ -> invalid_arg "Ability.cast: pass ~target or ~at, not both"
    | _ -> ());
    ignore (owner_of t.owner);
    let kind, h =
      match target with
      | None -> (0, 0)
      | Some (`Champion c) -> (1, owner_of c.handle)
      | Some (`Camp c) -> (2, c.camp_handle)
    in
    let (p : Vec2.t) = match at with Some p -> p | None -> Vec2.make (-1) (-1) in
    query_bool (Sys.cast t.slot kind h p.x p.y)
end

module Ward = struct
  type t = { position : Vec2.t; team : Team.t; remaining_time : int option }
  let is_ally w = w.team = Team.Ally
  let is_enemy w = w.team = Team.Enemy

  let decode o =
    let timer = i32 (o + 12) in
    { position = Vec2.make (i32 (o + 4)) (i32 (o + 8)); team = team_of_abs (i32 o);
      remaining_time = (if timer < 0 then None else Some timer) }

  let visible_tiles_from_into (p : Vec2.t) out =
    Bitboard.query_into (Sys.board_ward_vision_mask p.x p.y) out

  let visible_tiles_from p =
    let out = Bitboard.create () in
    visible_tiles_from_into p out;
    out

  let visible_tiles_into w out = visible_tiles_from_into w.position out
  let visible_tiles w = let out = Bitboard.create () in visible_tiles_into w out; out
end

module Altar_lock = struct
  type t = { team : Team.t; position : Vec2.t; hits_remaining : int }

  let decode o =
    if u32 o <> 6 then fail Error_code.Api_value_out_of_range "invalid altar lock kind";
    { team = team_of_abs (i32 (o + 8)); position = Vec2.make (i32 (o + 12)) (i32 (o + 16));
      hits_remaining = i32 (o + 20) }

  let is_ally t = t.team = Team.Ally
  let is_enemy t = t.team = Team.Enemy
  let raw_at (p : Vec2.t) team = check (Sys.altar_lock p.x p.y (team_abs team) rec_); rec_
end

module Teleport = struct
  type t = {
    champion : champion;
    team : Team.t;
    position : Vec2.t;
    remaining_time : int;
  }

  let decode o =
    let h = u32 o and name = Champion_type.of_int (i32 (o + 4)) and team = team_of_abs (i32 (o + 8)) in
    let position = Vec2.make (i32 (o + 12)) (i32 (o + 16)) in
    let remaining_time = i32 (o + 20) in
    { champion = { (make_champion h) with known = Some name }; team; position; remaining_time }

  let is_ally t = t.team = Team.Ally
  let is_enemy t = t.team = Team.Enemy
  let visible_tiles_into t out = hypothetical_vision_into (champion_name t.champion) t.position out
  let visible_tiles t = let out = Bitboard.create () in visible_tiles_into t out; out
end

module Unit_ops = struct
  let camp_raw c = check (Sys.camp c.camp_handle rec_); rec_
  let camp_design_raw c = check (Sys.camp_design (Camp_type.to_int c.camp_kind) rec_); rec_

  let buffs = function
    | `Champion { handle = None; _ } -> []
    | `Champion { handle = Some h; _ } ->
        List.map (fun o -> Buff.decode (u32 o) (i32 (o + 4)) (f32 (o + 8)))
          (host_list 12 (Sys.champion_buff_data h))
    | `Camp c ->
        List.map (fun o -> Buff.decode (u32 o) (i32 (o + 8)) (f32 (o + 4)))
          (counted_list 3 12 (fun p n o -> Sys.camp_buffs c.camp_handle p n o))

  let field k = function
    | `Champion c -> f32 (champion_raw (owner_of c.handle) + 16 + (4 * k))
    | `Camp c -> f32 (camp_raw c + 8 + (4 * k))

  let health u = field 0 u
  let max_health u = field 1 u
  let strength u = field 2 u
  let focus u = field 3 u

  let attack_range = function
    | `Champion c -> i32 (champion_raw (owner_of c.handle) + 36)
    | `Camp c -> i32 (camp_design_raw c + 88 + 8)

  let position = function
    | `Champion c -> let o = champion_raw (owner_of c.handle) in Vec2.make (i32 (o + 8)) (i32 (o + 12))
    | `Camp c -> let o = camp_raw c in Vec2.make (i32 o) (i32 (o + 4))

  let attack_tiles_into u out =
    draw_cheb_into (position u) (float_of_int (attack_range u)) out

  let attack_tiles u =
    let out = Bitboard.create () in
    attack_tiles_into u out;
    out

  let is_visible = function
    | `Champion { handle = None; _ } -> false
    | `Champion { handle = Some h; _ } -> (
        match champion_raw h with
        | o -> i32 (o + 8) >= 0 && i32 (o + 12) >= 0
        | exception Cambc_error (Error_code.Api_not_in_vision, _) -> false)
    | `Camp c -> ( try ignore (camp_raw c); true with e when missing e -> false)
end

module Game_unit = struct
  type t = [ `Champion of champion | `Camp of camp ]

  include Unit_ops
end

type camp_group = { g_handle : int; gold : int; gems : int; respawn : int; members : camp list; g_mask : Bitboard.t }

type cache = {
  self_champion : champion option;
  symmetry : Symmetry.t;
  terrain : Bitboard.t array;
  spawns : Bitboard.t array;
}

let read_mem f n =
  let base = list_area () in
  (ensure (base + n + 16));
  check (f base n);
  let b = Bytes.create n in
  ignore (Sys.blit_out b base n);
  b

let load_spawn_region team =
  let k = team_abs team in
  check (Sys.spawn_region k rec_);
  let x0 = i32 rec_ and y0 = i32 (rec_ + 4) and x1 = i32 (rec_ + 8) and y1 = i32 (rec_ + 12) in
  let l = Lazy.force layout in
  let b = Bytes.make (((l.w * l.h) + 7) / 8) '\000' in
  for y = y0 to y1 - 1 do
    for x = x0 to x1 - 1 do
      let i = y * l.w + x in
      let byte = i lsr 3 in
      Bytes.set_uint8 b byte (Bytes.get_uint8 b byte lor (1 lsl (i land 7)))
    done
  done;
  let region = Bitboard.create () in
  Bitboard.import_linear_buffer region b;
  region

let load_camp_group g =
      check (Sys.map_camp_group g rec_);
      let gems = u32 rec_ and gold = u32 (rec_ + 4) in
      let respawn = query_i32 (Sys.bc_camp_group_max_respawn g) in
      let mask = Bitboard.create () in
      let k = query_u32 (Sys.map_camp_group_num_camps g) in
      let ids = List.map u32 (counted_list k 4 (Sys.map_camp_group_camps g)) in
      let members =
        List.map
          (fun m ->
            check (Sys.map_camp_info m rec_);
            let spawn = Vec2.make (i32 rec_) (i32 (rec_ + 4)) in
            Bitboard.set mask spawn true;
            { camp_handle = m; group = g; camp_kind = Camp_type.of_int (i32 (rec_ + 8)); spawn })
          ids
      in
      { g_handle = g; gold; gems; respawn; members; g_mask = mask }

let load_camp_groups () =
  let n = query_u32 (Sys.map_camp_layout 0 0) in
  let base = list_area () in
  ensure (base + (n * 4) + 16);
  let written = query_u32 (Sys.map_camp_layout base n) in
  if written > n then fail Error_code.Invalid_argument "host returned oversized list";
  let w i = u32 (base + (4 * i)) in
  let rec camps g i k acc =
    if k = 0 then acc
    else
      let o = i + (4 * (k - 1)) in
      camps g i (k - 1) ({ camp_handle = w o; group = g; camp_kind = Camp_type.of_int (w (o + 1)); spawn = Vec2.make (w (o + 2)) (w (o + 3)) } :: acc)
  in
  let rec groups i acc =
    if i + 5 > written then List.rev acc
    else
      let g = w i and k = w (i + 4) in
      if i + 5 + (4 * k) > written then fail Error_code.Invalid_argument "malformed camp layout";
      let members = camps g (i + 5) k [] in
      let mask = Bitboard.create () in
      List.iter (fun c -> Bitboard.set mask c.spawn true) members;
      groups (i + 5 + (4 * k)) ({ g_handle = g; gold = w (i + 1); gems = w (i + 2); respawn = w (i + 3); members; g_mask = mask } :: acc)
  in
  groups 0 []

let load_camp h =
  check (Sys.map_camp_info h rec_);
  let spawn = Vec2.make (i32 rec_) (i32 (rec_ + 4)) and camp_kind = Camp_type.of_int (i32 (rec_ + 8)) in
  { camp_handle = h; group = query_u32 (Sys.map_camp_group_of h); camp_kind; spawn }

let draft_memory =
  lazy
    (let n = query_u32 Sys.draft_memory_size in
     read_mem (Sys.read_draft_memory 0) n)

let champion_design_size =
  let size = 88 + 40 in
  assert (size = 128);
  size

let load_terrain_mask kind =
  let b = Bytes.create (Bitboard.size ()) in
  let slot = slot 7 in
  check (Sys.bc_map_mask kind slot);
  ignore (Sys.blit_out b slot (Bytes.length b));
  b

let load_cache () =
  ignore (Lazy.force layout);
  ignore (Constants.get ());
  let h = Sys.my_champion 0 in
  let self_champion = if h < 0 then None else Some (make_champion h) in
  List.iter
    (fun kind ->
      let k = Champion_type.to_int kind in
      check (Sys.champion_design k (designs_area + k * champion_design_size)))
    Champion_type.all;
  let symmetry = Symmetry.of_int (query_i32 Sys.bc_map_symmetry) in
  let terrain = Array.init 4 load_terrain_mask in
  let spawns = [| load_spawn_region Team.Ally; load_spawn_region Team.Enemy |] in
  { self_champion; symmetry; terrain; spawns }

let cache_v = lazy (load_cache ())
let cache () = Lazy.force cache_v

let champion_design_raw kind =
  ignore (cache ());
  designs_area + Champion_type.to_int kind * champion_design_size

let all_camp_groups () =
  ignore (cache ());
  load_camp_groups ()

module Champion = struct
  type t = champion

  type base_stats = {
    vision_radius : int;
    base_move_speed : int;
    base_attack_range : int;
    base_health : float;
    health_growth : float;
    base_strength : float;
    strength_growth : float;
    base_focus : float;
    focus_growth : float;
    camp_damage_multiplier : float;
  }

  let equal a b = a.handle = b.handle && a.design = b.design
  let hash a = Hashtbl.hash a.handle

  let designs () =
    Array.of_list
      (List.map
         (fun kind ->
           let o = champion_design_raw kind in
           let ab slot =
             { owner = None; slot; ab_info = Ok (Ability_info.decode (o + 12 + (36 * slot))) }
           in
           { handle = None; design = Some kind; known = None; abilities = [| ab 0; ab 1 |] })
         Champion_type.all)

  let h c = owner_of c.handle
  let champion_name = champion_name
  let passive c = Passive.of_int (Sys.ld8 (champion_design_raw (champion_name c) + 8))
  let team c = team_of_abs (i32 (champion_raw (h c)))
  let is_ally c = team c = Team.Ally
  let is_enemy c = team c = Team.Enemy

  let base_stats c =
    let o = champion_design_raw (champion_name c) + 88 in
    { vision_radius = i32 o; base_move_speed = i32 (o + 4); base_attack_range = i32 (o + 8);
      base_health = f32 (o + 12); health_growth = f32 (o + 16); base_strength = f32 (o + 20);
      strength_growth = f32 (o + 24); base_focus = f32 (o + 28); focus_growth = f32 (o + 32);
      camp_damage_multiplier = f32 (o + 36) }

  let is_alive c = query_bool (Sys.champion_is_alive (h c))
  let is_dead c = not (is_alive c)
  let state c = live_champion_state (h c)
  let can_move c = Champion_state.mem Champion_state.Can_move (state c)
  let can_attack c = Champion_state.mem Champion_state.Can_attack (state c)
  let can_cast c = Champion_state.mem Champion_state.Can_cast (state c)
  let is_recalling c = Champion_state.mem Champion_state.Recalling (state c)
  let is_submitting_gems c = Champion_state.mem Champion_state.Submitting_gems (state c)
  let is_teleporting c = Champion_state.mem Champion_state.Teleporting (state c)
  let has_completed_turn c = Champion_state.mem Champion_state.Completed_turn (state c)
  let restrictions c = check (Sys.champion_restrictions (h c) out); Buff_restrictions.of_bits (Sys.ld8 out)
  let buffs c = Game_unit.buffs (`Champion c)
  let health c = Game_unit.health (`Champion c)
  let max_health c = Game_unit.max_health (`Champion c)
  let strength c = Game_unit.strength (`Champion c)
  let focus c = Game_unit.focus (`Champion c)
  let attack_range c = Game_unit.attack_range (`Champion c)
  let attack_tiles_into c out = Game_unit.attack_tiles_into (`Champion c) out
  let attack_tiles c = let out = Bitboard.create () in attack_tiles_into c out; out
  let position c = Game_unit.position (`Champion c)
  let is_visible c = Game_unit.is_visible (`Champion c)

  let visible_tiles_into c out =
    Bitboard.query_into (Sys.bc_champion_vision (h c)) out

  let visible_tiles c =
    let out = Bitboard.create () in
    visible_tiles_into c out;
    out

  let visible_tiles_from_into p c out =
    hypothetical_vision_into (champion_name c) p out

  let visible_tiles_from p c =
    let out = Bitboard.create () in
    visible_tiles_from_into p c out;
    out

  let remaining_channel_time c = query_i32 (Sys.bc_champion_channel_time (h c))

  let ward_positions c =
    let n = (Constants.get ()).active_wards in
    List.map (fun o -> Vec2.make (i32 o) (i32 (o + 4))) (counted_list n 8 (Sys.bc_champion_ward_positions (h c)))

  let ward_cooldown c = check (Sys.champion_wards (h c) rec_); i32 (rec_ + 4)

  let ward_placement_tiles_into c out =
    let me = match (cache ()).self_champion with Some m -> m | None -> design_error () in
    if not (equal me c) then fail Error_code.Api_invalid_perspective "champion is not controlled by this bot";
    draw_cheb_into (position c) (Constants.get ()).ward_placement_radius out

  let ward_placement_tiles c =
    let out = Bitboard.create () in
    ward_placement_tiles_into c out;
    out

  let shop_allowed c consumable = query_bool (Sys.champion_can_shop (h c) (Bool.to_int consumable))
  let can_sell c = query_bool (Sys.champion_can_sell (h c))
  let abilities c = c.abilities

  let item_counts c = check (Sys.champion_items (h c) rec_); Item_counts.read rec_
  let item_count c k = Item_counts.get (item_counts c) k
  let consumable_counts c = check (Sys.champion_consumables (h c) rec_); Consumable_counts.read rec_
  let consumable_count c k = Consumable_counts.get (consumable_counts c) k
  let gem_count c = query_u32 (Sys.champion_gems (h c))
  let gold_count c = query_u32 (Sys.champion_gold (h c))
  let can_buy ?(count = 1) c item =
    if not (shop_allowed c false) || count <= 0 then false
    else
      let info = Item_info.get item in
      info.purchasable
      && Int64.compare (Int64.of_int (gold_count c))
           (Int64.mul (Int64.of_int info.cost) (Int64.of_int count)) >= 0
  let can_buy_consumable c kind =
    if not (shop_allowed c true) then false
    else
      let info = Consumable.info kind in
      gold_count c >= info.cost && consumable_count c kind < info.carry_limit
  let prev_priority c = query_i32 (Sys.bc_champion_priority (h c) 0)
  let priority c = query_i32 (Sys.bc_champion_priority (h c) 1)

  let attack c target =
    ignore (h c);
    let kind, th =
      match target with
      | `Champion t -> (1, owner_of t.handle)
      | `Camp t -> (2, t.camp_handle)
      | `Altar_lock (l : Altar_lock.t) -> (6, u32 (Altar_lock.raw_at l.position l.team + 4))
    in
    check (Sys.attack kind th)

  let buy ?(count = 1) c k = ignore (h c); check (Sys.buy_item (Item.to_int k) count)
  let buy_consumable c k = ignore (h c); check (Sys.buy_consumable (Consumable.to_int k))
  let sell ?(count = 1) c k = ignore (h c); for _ = 1 to count do check (Sys.sell (Item.to_int k)) done

  let use_consumable ?at c k =
    ignore (h c);
    let (p : Vec2.t) = match at with Some p -> p | None -> Vec2.make (-1) (-1) in
    query_bool (Sys.use_consumable (Consumable.to_int k) p.x p.y)

  let move_absolute c (p : Vec2.t) = ignore (h c); query_bool (Sys.move_to p.x p.y)
  let move c d = move_absolute c (Vec2.add (position c) d)

  let move_towards c (target : Vec2.t) =
    ignore (h c);
    let origin = position c in
    if Vec2.equal origin target || not (Champion_state.mem Champion_state.Can_move (state c)) then false
    else begin
      let cached = cache () in
      let blocked = Bitboard.copy cached.terrain.(0) in
      List.iter (Bitboard.union_assign blocked) [ cached.terrain.(2); cached.terrain.(3); cached.spawns.(1) ];
      let temp = Bitboard.create () in
      List.iter
        (fun query -> Bitboard.query_into query temp; Bitboard.union_assign blocked temp)
        [ Sys.board_ally_occupancy; Sys.board_enemy_occupancy; Sys.camp_occupancy; Sys.board_teleport_occupancy ];
      Bitboard.set blocked origin false;
      if Bitboard.is_set blocked target then false
      else begin
        let roots = Bitboard.create () in
        Bitboard.set roots target true;
        let destinations = Bitboard.create () in
        Bitboard.set destinations origin true;
        let distances = Distance_field.create () in
        Distance_field.compute distances ~walls:blocked ~roots ~destinations ~visited:(Bitboard.create ())
          ~max_distance:Distance_field.unreachable;
        let closer best tile =
          if Distance_field.get distances tile < Distance_field.get distances best then tile else best in
        let reachable = Bitboard.reachable_tiles blocked origin ~distance:(base_stats c).base_move_speed in
        let best = List.fold_left closer origin (Bitboard.positions reachable) in
        (not (Vec2.equal best origin)) && move_absolute c best
      end
    end
  let recall c = ignore (h c); check (Sys.recall 0)

  let cancel_if flag msg c =
    if not (Champion_state.mem flag (state c)) then fail Error_code.Invalid_argument msg;
    check (Sys.cancel_channel 0)

  let cancel_recall = cancel_if Champion_state.Recalling "champion is not recalling"
  let cancel_submit_gems = cancel_if Champion_state.Submitting_gems "champion is not submitting gems"
  let submit_gems c (p : Vec2.t) = ignore (h c); check (Sys.submit_gems p.x p.y)
  let place_ward c (p : Vec2.t) = ignore (h c); query_bool (Sys.place_ward p.x p.y)
  let set_priority c p = ignore (h c); check (Sys.set_priority p)
  let end_turn c = ignore (h c); flush_all (); ignore (Sys.yield 0)
end

module Camp_group = struct
  type t = camp_group
  type state = Alive | Reward_available | Respawning of int

  let equal a b = a.g_handle = b.g_handle
  let hash a = Hashtbl.hash a.g_handle
  let gold_value g = g.gold
  let gem_value g = g.gems
  let max_respawn_timer g = g.respawn
  let camps g = g.members
  let spawn_tiles g = g.g_mask
  let spawn_tiles_into g out = Bytes.blit g.g_mask 0 out 0 (Bytes.length out)

  let state g =
    if Bitboard.is_empty (Bitboard.intersect (Bitboard.query Sys.board_vision) g.g_mask) then
      fail Error_code.Api_not_in_vision "camp group is not visible";
    match query_u32 (Sys.camp_group_state g.g_handle) with
    | 0 -> fail Error_code.Api_not_in_vision "camp group state is unknown"
    | 1 -> Alive
    | 2 -> Reward_available
    | 3 -> Respawning (query_i32 (Sys.camp_group_cooldown g.g_handle))
    | v -> fail Error_code.Api_value_out_of_range (Printf.sprintf "invalid camp group state: %d" v)
end

module Camp = struct
  type t = camp

  type base_stats = {
    vision_radius : float;
    base_move_speed : int;
    base_attack_range : int;
    base_health : float;
    health_growth : float;
    base_strength : float;
    strength_growth : float;
    base_focus : float;
    focus_growth : float;
  }

  let equal a b = a.camp_handle = b.camp_handle
  let hash a = Hashtbl.hash a.camp_handle
  let kind c = c.camp_kind
  let is_wolf c = kind c = Camp_type.Wolf
  let is_elk c = kind c = Camp_type.Elk
  let is_gem_cluster c = kind c = Camp_type.Gem_cluster
  let is_gold_cluster c = kind c = Camp_type.Gold_cluster
  let is_monolith c = kind c = Camp_type.Monolith
  let spawn_position c = c.spawn

  let camp_group c =
    ignore (cache ());
    load_camp_group c.group

  let base_stats c =
    let o = Unit_ops.camp_design_raw c + 88 in
    { vision_radius = f32 o; base_move_speed = i32 (o + 4); base_attack_range = i32 (o + 8);
      base_health = f32 (o + 12); health_growth = f32 (o + 16); base_strength = f32 (o + 20);
      strength_growth = f32 (o + 24); base_focus = f32 (o + 28); focus_growth = f32 (o + 32) }

  let buffs c = Game_unit.buffs (`Camp c)
  let health c = Game_unit.health (`Camp c)
  let max_health c = Game_unit.max_health (`Camp c)
  let strength c = Game_unit.strength (`Camp c)
  let focus c = Game_unit.focus (`Camp c)
  let attack_range c = Game_unit.attack_range (`Camp c)
  let attack_tiles_into c out = Game_unit.attack_tiles_into (`Camp c) out
  let attack_tiles c = let out = Bitboard.create () in attack_tiles_into c out; out
  let position c = Game_unit.position (`Camp c)
  let is_visible c = Game_unit.is_visible (`Camp c)
end

module Game_map = struct
  let width () = (Lazy.force layout).w
  let height () = (Lazy.force layout).h
  let is_in_bounds (p : Vec2.t) = p.x >= 0 && p.y >= 0 && p.x < width () && p.y < height ()
  let symmetry () = (cache ()).symmetry

  let position f (p : Vec2.t) = check (f p.x p.y out (out + 4)); Vec2.make (i32 out) (i32 (out + 4))
  let reflect_horizontal = position Sys.bc_bitboard_reflecthorizontalposition
  let reflect_vertical = position Sys.bc_bitboard_reflectverticalposition
  let rotate_180 = position Sys.bc_bitboard_rotate180position
  let terrain k () = Bitboard.copy (cache ()).terrain.(k)
  let walls = terrain 0
  let bushes = terrain 1
  let holes = terrain 2
  let altars = terrain 3

  let spawn_region team =
    Bitboard.copy (cache ()).spawns.(match team with Team.Ally -> 0 | Team.Enemy -> 1)

  let camp_groups () = all_camp_groups ()

  let camp_at p =
    ignore (cache ());
    load_camp (query_u32 (Sys.map_camp_at p.Vec2.x p.Vec2.y))

  let camp_at_opt p = optional camp_at p
end

module Vision = struct
  let visible_tiles_into out = Bitboard.query_into Sys.board_vision out
  let visible_tiles () = let out = Bitboard.create () in visible_tiles_into out; out

  let camp_tiles_into out = Bitboard.query_into Sys.camp_occupancy out
  let camp_tiles () = let out = Bitboard.create () in camp_tiles_into out; out

  let teleport_tiles_into out = Bitboard.query_into Sys.board_teleport_occupancy out
  let teleport_tiles () = let out = Bitboard.create () in teleport_tiles_into out; out

  let by_team_into ally enemy ?team out =
    match team with
    | Some Team.Ally -> Bitboard.query_into ally out
    | Some Team.Enemy -> Bitboard.query_into enemy out
    | None ->
        Bitboard.query_into ally out;
        let temp = Bitboard.create () in
        Bitboard.query_into enemy temp;
        Bitboard.union_assign out temp

  let by_team ally enemy ?team () =
    let out = Bitboard.create () in
    by_team_into ally enemy ?team out;
    out

  let champion_tiles_into ?team out =
    by_team_into Sys.board_ally_occupancy Sys.board_enemy_occupancy ?team out

  let champion_tiles ?team () =
    let out = Bitboard.create () in
    champion_tiles_into ?team out;
    out

  let ward_tiles_into ?team out =
    by_team_into Sys.board_ally_ward_occupancy Sys.board_enemy_ward_occupancy ?team out

  let ward_tiles ?team () =
    let out = Bitboard.create () in
    ward_tiles_into ?team out;
    out

  let altar_lock_tiles_into ?team out =
    by_team_into Sys.board_ally_altar_lock_occupancy Sys.board_enemy_altar_lock_occupancy ?team out

  let altar_lock_tiles ?team () =
    let out = Bitboard.create () in
    altar_lock_tiles_into ?team out;
    out

  let champion_at (p : Vec2.t) = make_champion (query_u32 (Sys.champion_at p.x p.y))
  let champion_at_opt p = optional champion_at p

  let camp_at (p : Vec2.t) =
    ignore (cache ());
    load_camp (query_u32 (Sys.camp_at p.x p.y))

  let camp_at_opt p = optional camp_at p
  let champions ?team () = List.map champion_at (Bitboard.positions (champion_tiles ?team ()))
  let camps () = List.map camp_at (Bitboard.positions (camp_tiles ()))

  let camp_groups () =
    let v = visible_tiles () in
    List.filter (fun g -> not (Bitboard.is_empty (Bitboard.intersect g.g_mask v))) (all_camp_groups ())

  let wards ?team () =
    List.filter
      (fun (w : Ward.t) -> match team with None -> true | Some t -> w.team = t)
      (List.map Ward.decode (host_list 20 Sys.wards))

  let ward_handle ?team (p : Vec2.t) =
    let f = match team with None -> Sys.ward_at | Some Team.Ally -> Sys.ally_ward_at | Some Team.Enemy -> Sys.enemy_ward_at in
    query_u32 (f p.x p.y)

  let ward_at ?team p =
    let h = ward_handle ?team p in
    check (Sys.ward h rec_);
    Ward.decode rec_

  let ward_at_opt ?team p = optional (ward_at ?team) p
  let has_ward ?team p = optional (ward_handle ?team) p <> None
  let has_teleport (p : Vec2.t) = query_bool (Sys.is_occupied_by_teleport p.x p.y)
  let teleports () = List.map Teleport.decode (host_list 28 Sys.teleports)

  let teleport_at p =
    let found =
      if has_teleport p then List.find_opt (fun (t : Teleport.t) -> Vec2.equal t.position p) (teleports ())
      else None
    in
    match found with Some t -> t | None -> fail Error_code.Api_not_found "no teleport at position"

  let teleport_at_opt p = optional teleport_at p

  let altar_locks ?team () =
    List.filter
      (fun (l : Altar_lock.t) -> match team with None -> true | Some t -> l.team = t)
      (List.map Altar_lock.decode (host_list 24 Sys.altar_locks))

  let altar_lock_at p =
    let o =
      try Altar_lock.raw_at p Team.Ally
      with Cambc_error (Error_code.Api_not_found, _) -> Altar_lock.raw_at p Team.Enemy
    in
    Altar_lock.decode o

  let altar_lock_at_opt p = optional altar_lock_at p
  let has_champion (p : Vec2.t) = query_bool (Sys.has_champion p.x p.y)
  let has_camp (p : Vec2.t) = query_bool (Sys.has_camp p.x p.y)
  let has_altar_lock p = altar_lock_at_opt p <> None
end

module Draft = struct
  let team_size () = check (Sys.draft_num_rounds out); Sys.ld8 out

  let current team =
    let t = team_abs team in
    List.filter_map
      (fun r ->
        match query_i32 (Sys.draft_get t r) with
        | v -> Some (Champion_draft.of_int v)
        | exception Cambc_error (Error_code.Draft_unavailable, _) -> None)
      (List.init (team_size ()) Fun.id)

  let pick k = check (Sys.draft_champion (Champion_draft.to_int k))
end

let write_mem f (b : Bytes.t) =
  let base = list_area () in
  let n = Bytes.length b in
  (ensure (base + n + 16));
  ignore (Sys.blit_in b base n);
  check (f base n)

module Shared_memory = struct
  let size () = query_u32 Sys.shared_memory_size
  let read ~offset n = read_mem (Sys.read_shared_memory offset) n
  let write ~offset b = write_mem (Sys.write_shared_memory offset) b
  let get i = Bytes.get_uint8 (read ~offset:i 1) 0
  let set i v = write ~offset:i (Bytes.make 1 (Char.chr (v land 0xff)))
  let read_into ~offset b = Bytes.blit (read ~offset (Bytes.length b)) 0 b 0 (Bytes.length b)

  type _ kind =
    | U8 : int kind
    | I8 : int kind
    | U16 : int kind
    | I16 : int kind
    | U32 : int kind
    | I32 : int kind
    | I64 : int64 kind
    | F32 : float kind
    | F64 : float kind

  let width : type a. a kind -> int = function
    | U8 | I8 -> 1
    | U16 | I16 -> 2
    | U32 | I32 | F32 -> 4
    | I64 | F64 -> 8

  let get_value : type a. a kind -> Bytes.t -> int -> a =
   fun k b o ->
    match k with
    | U8 -> Bytes.get_uint8 b o
    | I8 -> Bytes.get_int8 b o
    | U16 -> Bytes.get_uint16_le b o
    | I16 -> Bytes.get_int16_le b o
    | U32 -> Int32.to_int (Bytes.get_int32_le b o) land 0xffff_ffff
    | I32 -> Int32.to_int (Bytes.get_int32_le b o)
    | I64 -> Bytes.get_int64_le b o
    | F32 -> Int32.float_of_bits (Bytes.get_int32_le b o)
    | F64 -> Int64.float_of_bits (Bytes.get_int64_le b o)

  let set_value : type a. a kind -> Bytes.t -> int -> a -> unit =
   fun k b o v ->
    match k with
    | U8 -> Bytes.set_uint8 b o v
    | I8 -> Bytes.set_int8 b o v
    | U16 -> Bytes.set_uint16_le b o v
    | I16 -> Bytes.set_int16_le b o v
    | U32 -> Bytes.set_int32_le b o (Int32.of_int v)
    | I32 -> Bytes.set_int32_le b o (Int32.of_int v)
    | I64 -> Bytes.set_int64_le b o v
    | F32 -> Bytes.set_int32_le b o (Int32.bits_of_float v)
    | F64 -> Bytes.set_int64_le b o (Int64.bits_of_float v)

  let read_values ~offset k n =
    let w = width k in
    let b = read ~offset (n * w) in
    Array.init n (fun i -> get_value k b (i * w))

  let write_values ~offset k a =
    let w = width k in
    let b = Bytes.create (Array.length a * w) in
    Array.iteri (fun i v -> set_value k b (i * w) v) a;
    write ~offset b
end

module Draft_memory = struct
  let bytes () = Bytes.copy (Lazy.force draft_memory)
  let length () = Bytes.length (Lazy.force draft_memory)
  let get i = Bytes.get_uint8 (Lazy.force draft_memory) i

  let submit b =
    if (cache ()).self_champion <> None then
      fail Error_code.Invalid_action "draft memory can only be submitted during the draft";
    write_mem Sys.bc_submit_draft_memory b
end

module Debug = struct
  let draw_dot (p : Vec2.t) color = check (Sys.debug_dot p.x p.y color)
  let draw_line (a : Vec2.t) (b : Vec2.t) color = check (Sys.debug_line a.x a.y b.x b.y color)
end

let init () =
  ignore (cache ());
  ignore (Lazy.force draft_memory)
(* Load the caches on the first turn's fuel, before the bot's own modules run. *)
let () = init ()
let constants = Constants.get
let is_draft () = Sys.my_champion 0 < 0
let self_champion_opt () =
  match (cache ()).self_champion with
  | None -> None
  | Some c -> Some { c with abilities = Array.copy c.abilities }

let self_champion () =
  match self_champion_opt () with
  | Some c -> c
  | None -> fail Error_code.Api_invalid_perspective "no self champion during the draft"

let champions ?team () =
  List.concat_map
    (fun t ->
      if match team with Some s -> s <> t | None -> false then []
      else
        let num, list = match t with Team.Ally -> (Sys.num_allies, Sys.allies) | Team.Enemy -> (Sys.num_enemies, Sys.enemies) in
        List.map (fun o -> make_champion (u32 o)) (counted_list (query_u32 num) 4 list))
    [ Team.Ally; Team.Enemy ]

let round_number () = query_u32 Sys.bc_round_number
let global_submitted_gems team = query_u32 (Sys.bc_submitted_gems (team_abs team))

let fuel_remaining () =
  ignore (Sys.fuel_remaining out);
  let w k = Sys.ld16 (out + (2 * k)) lsl (16 * k) in
  w 0 lor w 1 lor w 2 lor w 3

let end_turn () = flush_all (); ignore (Sys.yield 0)
let resign () =
  (* A successful resign never returns, so flush buffered output first. *)
  flush_all ();
  check (Sys.resign 0)
