(** Cambridge Battlecode OCaml SDK. Mirrors the C++ [cambc] namespaces. *)

module Vec2 : sig
  type t = { x : int; y : int }

  (** [make x y] is the vector [(x, y)].
      @param x Horizontal coordinate.
      @param y Vertical coordinate. *)
  val make : int -> int -> t
  (** [(0, 0)]. *)
  val zero : t
  (** [(0, 1)]; +y is north, matching the engine and [rotate_cw]/[rotate_ccw]. *)
  val north : t
  (** [(1, 1)]. *)
  val north_east : t
  (** [(1, 0)]. *)
  val east : t
  (** [(1, -1)]. *)
  val south_east : t
  (** [(0, -1)]. *)
  val south : t
  (** [(-1, -1)]. *)
  val south_west : t
  (** [(-1, 0)]. *)
  val west : t
  (** [(-1, 1)]. *)
  val north_west : t
  (** [equal a b] is true when both coordinates match.
      @param a First vector.
      @param b Second vector. *)
  val equal : t -> t -> bool
  (** Total order on vectors, by [x] then [y].
      @param a First vector.
      @param b Second vector. *)
  val compare : t -> t -> int
  (** Hash consistent with [equal].
      @param v Vector. *)
  val hash : t -> int
  (** [add a b] is the component-wise sum.
      @param a First vector.
      @param b Second vector. *)
  val add : t -> t -> t
  (** [sub a b] is the component-wise difference [a - b].
      @param a First vector.
      @param b Second vector. *)
  val sub : t -> t -> t
  (** [neg v] negates both components.
      @param v Vector. *)
  val neg : t -> t
  (** [scale k v] multiplies both components by [k].
      @param k Scale factor.
      @param v Vector. *)
  val scale : int -> t -> t
  (** [div v k] divides both components by [k], truncating; raises on zero.
      @param v Vector.
      @param k Divisor. *)
  val div : t -> int -> t
  (** Same as [add]. *)
  val ( + ) : t -> t -> t
  (** Same as [sub]. *)
  val ( - ) : t -> t -> t
  (** Same as [neg]. *)
  val ( ~- ) : t -> t
  (** Same as [scale]. *)
  val ( * ) : int -> t -> t
  (** [dot a b] is the dot product.
      @param a First vector.
      @param b Second vector. *)
  val dot : t -> t -> int
  (** [cross a b] is the signed 2D cross product.
      @param a First vector.
      @param b Second vector. *)
  val cross : t -> t -> int
  (** [length_squared v] is the squared Euclidean length.
      @param v Vector. *)
  val length_squared : t -> int
  (** [chebyshev_distance a b] is the number of king moves between the tiles.
      @param a First vector.
      @param b Second vector. *)
  val chebyshev_distance : t -> t -> int
  (** [rotate_cw v] rotates a direction 45 degrees clockwise.
      @param v Vector. *)
  val rotate_cw : t -> t
  (** [rotate_ccw v] rotates a direction 45 degrees counter-clockwise.
      @param v Vector. *)
  val rotate_ccw : t -> t
  (** [reflect_horizontal p] reflects a map position horizontally, reversing x.
      @param p Map position. *)
  val reflect_horizontal : t -> t
  (** [reflect_vertical p] reflects a map position vertically, reversing y.
      @param p Map position. *)
  val reflect_vertical : t -> t
  (** [rotate_180 p] rotates a map position 180 degrees within the map.
      @param p Map position. *)
  val rotate_180 : t -> t
  (** [to_string v] formats the vector as [(x, y)].
      @param v Vector. *)
  val to_string : t -> string
end

module Team : sig
  type t = Ally | Enemy

  (** [opponent t] is the other team.
      @param t Team. *)
  val opponent : t -> t
  (** [equal a b] is true for the same team.
      @param a First team.
      @param b Second team. *)
  val equal : t -> t -> bool
  (** Name of a team.
      @param t Team. *)
  val to_string : t -> string
end
module Symmetry : sig
  type t =
      | Horizontal
      | Vertical
      | Rotational180

  (** Every value, in declaration order. *)
  val all : t list
  (** [to_string x] is the name of [x].
      @param x Value to name. *)
  val to_string : t -> string
end

module Camp_type : sig
  type t =
      | Wolf
      | Elk
      | Gem_cluster
      | Gold_cluster
      | Monolith

  (** Every value, in declaration order. *)
  val all : t list
  (** [to_string x] is the name of [x].
      @param x Value to name. *)
  val to_string : t -> string
end

module Passive : sig
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

  (** Every value, in declaration order. *)
  val all : t list
  (** [to_string x] is the name of [x].
      @param x Value to name. *)
  val to_string : t -> string
end

module Champion_draft : sig
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

  (** Every value, in declaration order. *)
  val all : t list
  (** [to_string x] is the name of [x].
      @param x Value to name. *)
  val to_string : t -> string
end

module Champion_type : sig
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

  (** Every value, in declaration order. *)
  val all : t list
  (** [to_string x] is the name of [x].
      @param x Value to name. *)
  val to_string : t -> string
end

module Ability_kind : sig
  type t =
      | Attack_reset
      | Damage
      | Aoe_damage
      | Cc
      | Heal
      | Dash
      | Shield

  (** Every value, in declaration order. *)
  val all : t list
  (** [to_string x] is the name of [x].
      @param x Value to name. *)
  val to_string : t -> string
end

module Item : sig
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

  (** Every value, in declaration order. *)
  val all : t list
  (** [to_string x] is the name of [x].
      @param x Value to name. *)
  val to_string : t -> string
end

module Buff_kind : sig
  type t =
      | Shield
      | Stun
      | Enhanced_attack

  (** Every value, in declaration order. *)
  val all : t list
  (** [to_string x] is the name of [x].
      @param x Value to name. *)
  val to_string : t -> string
end

module Error_code : sig
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

  (** Every value, in declaration order. *)
  val all : t list
  (** [to_string x] is the name of [x].
      @param x Value to name. *)
  val to_string : t -> string
end

exception Cambc_error of Error_code.t * string
(** Raised by every failing call. *)

exception No_range
(** An ability without a targeting range (attack reset); Rust [Error::NoRange]. *)

exception Missing_identity
(** A champion design from [Champion.designs] used where a live champion is needed; Rust [Error::MissingIdentity]. *)

module type FLAGS = sig
  type flag
  type t

  (** The set with no flags. *)
  val empty : t
  (** Every named flag. *)
  val all : flag list
  (** [mem f s] is true when [s] contains [f].
      @param f Flag.
      @param s Flag set. *)
  val mem : flag -> t -> bool
  (** [add f s] is [s] with [f] set.
      @param f Flag.
      @param s Flag set. *)
  val add : flag -> t -> t
  (** [remove f s] is [s] with [f] cleared.
      @param f Flag.
      @param s Flag set. *)
  val remove : flag -> t -> t
  (** [union a b] has the flags of either set.
      @param a First set.
      @param b Second set. *)
  val union : t -> t -> t
  (** [inter a b] has the flags of both sets.
      @param a First set.
      @param b Second set. *)
  val inter : t -> t -> t
  (** [diff a b] has the flags of [a] not in [b].
      @param a First set.
      @param b Flags to remove. *)
  val diff : t -> t -> t
  (** [subset a b] is true when every flag of [a] is in [b].
      @param a Candidate subset.
      @param b Superset. *)
  val subset : t -> t -> bool
  (** [is_empty s] is true when no flag is set.
      @param s Flag set. *)
  val is_empty : t -> bool
  (** [equal a b] is true for identical sets.
      @param a First set.
      @param b Second set. *)
  val equal : t -> t -> bool
  (** Hash consistent with [equal].
      @param s Flag set. *)
  val hash : t -> int
  (** [of_list fs] is the set of the listed flags.
      @param fs Flags to set. *)
  val of_list : flag list -> t
  (** [to_list s] lists the set flags.
      @param s Flag set. *)
  val to_list : t -> flag list
end

module Ability_target : sig
  type flag = Camps | Enemies | Allies | Self | Ground

  include FLAGS with type flag := flag
end

module Buff_restrictions : sig
  type flag = Move | Attack | Cast | Shop | Consumable | Ward

  include FLAGS with type flag := flag
end

module Champion_state : sig
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

  include FLAGS with type flag := flag

  (** [is_dead s] is true when the alive flag is clear.
      @param s Champion state. *)
  val is_dead : t -> bool
end

module Constants : sig
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

  (** Game constants loaded at startup. *)
  val get : unit -> t

  module Passive_abilities : sig
    (** Focus Prism gains per carried gem. *)
    val prism_focus_per_gem : float
    (** Radius of Varg's splash damage, in tiles. *)
    val varg_splash_radius : float
    (** Extra gold Vesper Vale gains, in percent. *)
    val vesper_vale_bonus_gold_percent : int
    (** Strength Ryth gains per point of bonus health. *)
    val ryth_health_strength_ratio : float
    (** Fraction of focus Ren deals back as retaliation damage. *)
    val ren_retaliation_focus_ratio : float
  end
end

module Item_passive : sig
  type t = Monster | Durability | Treasure | Jungle | Home

  (** [to_string p] is the name of an item passive.
      @param p Item passive. *)
  val to_string : t -> string
end

module Item_info : sig
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

  (** [get item] is the item's static design data.
      @param item Item type. *)
  val get : Item.t -> t

  (** [cost item] is the gold cost to buy the item.
      @param item Item type. *)
  val cost : Item.t -> int
  (** [is_purchasable item] is true when the shop sells the item directly.
      @param item Item type. *)
  val is_purchasable : Item.t -> bool
  (** [combine_count item] is how many copies combine into the result item.
      @param item Item type. *)
  val combine_count : Item.t -> int
  (** [result item] is the item these copies combine into, if any.
      @param item Item type. *)
  val result : Item.t -> Item.t option
  (** Maximum health the item grants.
      @param item Item type. *)
  val health_points : Item.t -> int
  (** Strength the item grants.
      @param item Item type. *)
  val strength_points : Item.t -> int
  (** Focus the item grants.
      @param item Item type. *)
  val focus_points : Item.t -> int
  (** Passive effect the item grants, if any.
      @param item Item type. *)
  val passive : Item.t -> Item_passive.t option
  (** Strength of the item's passive effect.
      @param item Item type. *)
  val passive_magnitude : Item.t -> float
end

(** Item counts indexed by [Item.t]. *)
module Item_counts : sig
  type t

  (** [get c k] is the number of [k] in the inventory.
      @param c Inventory counts.
      @param k Item type. *)
  val get : t -> Item.t -> int
  (** [iter fn c] calls [fn] on each held type and its count.
      @param fn Callback.
      @param c Inventory counts. *)
  val iter : (Item.t -> int -> unit) -> t -> unit
  (** [fold fn c init] folds [fn] over each held type and its count.
      @param fn Folding function.
      @param c Inventory counts.
      @param init Initial accumulator. *)
  val fold : (Item.t -> int -> 'a -> 'a) -> t -> 'a -> 'a
  (** [to_list c] lists each held type with its count.
      @param c Inventory counts. *)
  val to_list : t -> (Item.t * int) list
end

module Consumable : sig
  type t = Tp_crystal | Altar_lock

  type info = { cost : int; carry_limit : int }

  (** Every value, in declaration order. *)
  val all : t list
  (** [to_string x] is the name of [x].
      @param x Value to name. *)
  val to_string : t -> string
  (** [info c] is the consumable's static design data.
      @param c Consumable type. *)
  val info : t -> info
  (** [cost c] is the gold cost of one.
      @param c Consumable type. *)
  val cost : t -> int
  (** [carry_limit c] is how many a champion can carry.
      @param c Consumable type. *)
  val carry_limit : t -> int
end

(** Consumable counts indexed by [Consumable.t]. *)
module Consumable_counts : sig
  type t

  (** [get c k] is the number of [k] in the inventory.
      @param c Inventory counts.
      @param k Consumable type. *)
  val get : t -> Consumable.t -> int
  (** [iter fn c] calls [fn] on each held type and its count.
      @param fn Callback.
      @param c Inventory counts. *)
  val iter : (Consumable.t -> int -> unit) -> t -> unit
  (** [fold fn c init] folds [fn] over each held type and its count.
      @param fn Folding function.
      @param c Inventory counts.
      @param init Initial accumulator. *)
  val fold : (Consumable.t -> int -> 'a -> 'a) -> t -> 'a -> 'a
  (** [to_list c] lists each held type with its count.
      @param c Inventory counts. *)
  val to_list : t -> (Consumable.t * int) list
end

module Bitboard : sig
  type t

  (** A board with no tiles set. *)
  val create : unit -> t
  (** [copy b] is an independent copy.
      @param b Bitboard. *)
  val copy : t -> t
  (** Map width in tiles.
      @param b Bitboard. *)
  val width : t -> int
  (** Map height in tiles.
      @param b Bitboard. *)
  val height : t -> int
  (** Size in bytes of the engine-chunked buffer.
      @param b Bitboard. *)
  val raw_buffer_size : t -> int
  (** Size in bytes of the row-major buffer.
      @param b Bitboard. *)
  val linear_buffer_size : t -> int
  (** [clear b] clears every tile.
      @param b Bitboard. *)
  val clear : t -> unit
  (** [fill b] sets every tile on the map.
      @param b Bitboard. *)
  val fill : t -> unit
  (** [is_set b p] is true when [p] is set; raises outside the map.
      @param b Bitboard.
      @param p Tile. *)
  val is_set : t -> Vec2.t -> bool
  (** [set b p value] sets or clears [p]; raises outside the map.
      @param b Bitboard.
      @param p Tile.
      @param value [true] to set, [false] to clear. *)
  val set : t -> Vec2.t -> bool -> unit
  (** [toggle b p] flips [p]; raises outside the map.
      @param b Bitboard.
      @param p Tile. *)
  val toggle : t -> Vec2.t -> unit
  (** [is_empty b] is true when no tile is set.
      @param b Bitboard. *)
  val is_empty : t -> bool
  (** [count b] is the number of set tiles.
      @param b Bitboard. *)
  val count : t -> int
  (** [positions b] lists the set tiles in row-major order.
      @param b Bitboard. *)
  val positions : t -> Vec2.t list
  (** [print b] prints the board to stdout for debugging.
      @param b Bitboard. *)
  val print : t -> unit
  (** [equal a b] is true when both boards set the same tiles.
      @param a First board.
      @param b Second board. *)
  val equal : t -> t -> bool
  (** [complement b] sets exactly the map tiles [b] leaves clear.
      @param b Bitboard. *)
  val complement : t -> t
  (** [union a b] is a new board with tiles set in either.
      @param a First board.
      @param b Second board; may be [a]. *)
  val union : t -> t -> t
  (** [intersect a b] is a new board with tiles set in both.
      @param a First board.
      @param b Second board; may be [a]. *)
  val intersect : t -> t -> t
  (** [xor a b] is a new board with tiles set in exactly one.
      @param a First board.
      @param b Second board; may be [a]. *)
  val xor : t -> t -> t
  (** [union_assign a b] adds the tiles of [b] to [a].
      @param a First board.
      @param b Second board; may be [a]. *)
  val union_assign : t -> t -> unit
  (** [intersect_assign a b] keeps only the tiles of [a] also in [b].
      @param a First board.
      @param b Second board; may be [a]. *)
  val intersect_assign : t -> t -> unit
  (** [xor_assign a b] flips the tiles of [a] set in [b].
      @param a First board.
      @param b Second board; may be [a]. *)
  val xor_assign : t -> t -> unit
  (** [shift_right b count] is a copy shifted horizontally; tiles shifted off the map are dropped.
      @param b Bitboard.
      @param count Columns to shift; negative values shift left. *)
  val shift_right : t -> int -> t
  (** [_into] functions overwrite the whole output. Unary bitboard operations may
      write in place.
      @param b Bitboard.
      @param count Columns to shift; negative values shift left.
      @param out Destination; its previous contents are overwritten. *)
  val shift_right_into : t -> int -> t -> unit
  (** [shift_up b count] is a copy shifted vertically; tiles shifted off the map are dropped.
      @param b Bitboard.
      @param count Rows to shift; negative values shift down. *)
  val shift_up : t -> int -> t
  (** Like [shift_up], but writes into [out].
      @param b Bitboard.
      @param count Rows to shift; negative values shift down.
      @param out Destination; its previous contents are overwritten. *)
  val shift_up_into : t -> int -> t -> unit
  (** [reflect b s] is a copy transformed by a map symmetry.
      @param b Bitboard.
      @param s Map symmetry. *)
  val reflect : t -> Symmetry.t -> t
  (** Like [reflect], but writes into [out].
      @param b Bitboard.
      @param s Map symmetry.
      @param out Destination; its previous contents are overwritten. *)
  val reflect_into : t -> Symmetry.t -> t -> unit
  (** [reflect_horizontal b] is a copy reflected horizontally, reversing x.
      @param b Bitboard. *)
  val reflect_horizontal : t -> t
  (** Like [reflect_horizontal], but writes into [out].
      @param b Bitboard.
      @param out Destination; its previous contents are overwritten. *)
  val reflect_horizontal_into : t -> t -> unit
  (** [reflect_vertical b] is a copy reflected vertically, reversing y.
      @param b Bitboard. *)
  val reflect_vertical : t -> t
  (** Like [reflect_vertical], but writes into [out].
      @param b Bitboard.
      @param out Destination; its previous contents are overwritten. *)
  val reflect_vertical_into : t -> t -> unit
  (** [rotate_180 b] is a copy rotated 180 degrees within the map.
      @param b Bitboard. *)
  val rotate_180 : t -> t
  (** Like [rotate_180], but writes into [out].
      @param b Bitboard.
      @param out Destination; its previous contents are overwritten. *)
  val rotate_180_into : t -> t -> unit
  (** [draw_circle b center radius] sets every tile within a Euclidean radius.
      @param b Bitboard.
      @param center Centre tile.
      @param radius Euclidean radius in tiles. *)
  val draw_circle : t -> Vec2.t -> float -> unit
  (** [draw_chebyshev b center radius] sets every tile within a Chebyshev radius.
      @param b Bitboard.
      @param center Centre tile.
      @param radius Chebyshev radius in tiles. *)
  val draw_chebyshev : t -> Vec2.t -> float -> unit

  (** [reachable_tiles ?scratch walls origin ~distance] is the tiles reachable from [origin] around [walls].
      @param scratch Optional pair of working boards; their contents are overwritten.
      @param walls Impassable tiles.
      @param origin Start tile.
      @param distance Maximum number of 8-directional steps. *)
  val reachable_tiles : ?scratch:t * t -> t -> Vec2.t -> distance:int -> t
  (** [reachable_tiles walls origin ~distance]; [?scratch] is reusable (frontier, scratch) storage.
      @param scratch Optional pair of working boards; their contents are overwritten.
      @param walls Impassable tiles.
      @param origin Start tile.
      @param distance Maximum number of 8-directional steps.
      @param out Destination; its previous contents are overwritten. *)
  val reachable_tiles_into : ?scratch:t * t -> t -> Vec2.t -> distance:int -> t -> unit

  (** [shadow_cast walls origin ~distance] is the tiles visible from [origin] past opaque [walls].
      @param walls Opaque tiles.
      @param origin Viewer tile.
      @param distance Maximum sight radius in tiles. *)
  val shadow_cast : t -> Vec2.t -> distance:int -> t
  (** Like [shadow_cast], but writes into [out].
      @param walls Opaque tiles.
      @param origin Viewer tile.
      @param distance Maximum sight radius in tiles.
      @param out Destination; its previous contents are overwritten. *)
  val shadow_cast_into : t -> Vec2.t -> distance:int -> t -> unit
  (** [import_raw_buffer b data] replaces the contents from an engine-chunked buffer.
      @param b Bitboard.
      @param data Engine-chunked bytes, exactly [raw_buffer_size] long. *)
  val import_raw_buffer : t -> Bytes.t -> unit
  (** [export_raw_buffer b] is the contents as an engine-chunked buffer.
      @param b Bitboard. *)
  val export_raw_buffer : t -> Bytes.t
  (** [import_linear_buffer b data] replaces the contents from a row-major buffer.
      @param b Bitboard.
      @param data One value per tile in row-major order. *)
  val import_linear_buffer : t -> Bytes.t -> unit
  (** [export_linear_buffer b] is the contents as a row-major buffer.
      @param b Bitboard. *)
  val export_linear_buffer : t -> Bytes.t
  (** Like [export_raw_buffer], but writes into [out].
      @param b Bitboard.
      @param out Destination, exactly [raw_buffer_size] long. *)
  val export_raw_buffer_into : t -> Bytes.t -> unit
  (** Like [export_linear_buffer], but writes into [out].
      @param b Bitboard.
      @param out Destination, exactly [linear_buffer_size] long. *)
  val export_linear_buffer_into : t -> Bytes.t -> unit
end

module Distance_field : sig
  type t

  (** Distance stored for tiles the search did not reach. *)
  val unreachable : int
  (** A field with every tile unreachable. *)
  val create : unit -> t
  (** [copy d] is an independent copy.
      @param d Distance field. *)
  val copy : t -> t
  (** Map width in tiles.
      @param d Distance field. *)
  val width : t -> int
  (** Map height in tiles.
      @param d Distance field. *)
  val height : t -> int
  (** Size in bytes of the engine-chunked buffer.
      @param d Distance field. *)
  val raw_buffer_size : t -> int
  (** Number of row-major values.
      @param d Distance field. *)
  val linear_buffer_size : t -> int
  (** [get d p] is the distance at [p]; raises outside the map.
      @param d Distance field.
      @param p Tile. *)
  val get : t -> Vec2.t -> int
  (** [set d p value] stores a distance at [p]; raises outside the map.
      @param d Distance field.
      @param p Tile.
      @param value Distance to store. *)
  val set : t -> Vec2.t -> int -> unit
  (** [print d] prints the field to stdout for debugging.
      @param d Distance field. *)
  val print : t -> unit
  (** [import_raw_buffer d data] replaces the contents from an engine-chunked buffer.
      @param d Distance field.
      @param data Engine-chunked bytes, exactly [raw_buffer_size] long. *)
  val import_raw_buffer : t -> Bytes.t -> unit
  (** [export_raw_buffer d] is the contents as an engine-chunked buffer.
      @param d Distance field. *)
  val export_raw_buffer : t -> Bytes.t
  (** [import_linear_buffer d values] replaces the contents from row-major distances.
      @param d Distance field.
      @param values One distance per tile in row-major order. *)
  val import_linear_buffer : t -> int array -> unit
  (** [export_linear_buffer d] is the row-major distances.
      @param d Distance field. *)
  val export_linear_buffer : t -> int array
  (** Like [export_raw_buffer], but writes into [out].
      @param d Distance field.
      @param out Destination, exactly [raw_buffer_size] long. *)
  val export_raw_buffer_into : t -> Bytes.t -> unit
  (** Like [export_linear_buffer], but writes into [out].
      @param d Distance field.
      @param out Destination with one entry per tile. *)
  val export_linear_buffer_into : t -> int array -> unit

  val compute :
    ?destinations:Bitboard.t ->
    t -> walls:Bitboard.t -> roots:Bitboard.t -> visited:Bitboard.t -> max_distance:int -> unit
  (** BFS from [roots]; with [?destinations] stops once all are reached. *)

  (** Like [compute], but returns the number of expansion steps.
      @param destinations Optional target tiles; stop once all are reached.
      @param walls Impassable tiles.
      @param roots Tiles at distance zero.
      @param visited Receives the tiles the search reached.
      @param max_distance Tiles farther than this are left unreached. *)
  val compute_with_expansion_count :
    ?destinations:Bitboard.t ->
    t -> walls:Bitboard.t -> roots:Bitboard.t -> visited:Bitboard.t -> max_distance:int -> int

  (** [next_move_from d p] is the first optimal step towards a root as a direction delta, or [None] at a root or unreached tile.
      @param d Distance field.
      @param p Tile to step from. *)
  val next_move_from : t -> Vec2.t -> Vec2.t option
  (** First optimal step from a tile towards a root, as a direction delta in compass order from
      north; [None] at a root or unreached tile. *)

  val next_moves_from : t -> Vec2.t -> Vec2.t list
  (** Every optimal step from a tile towards a root, as direction deltas in compass order. *)
end

module Buff : sig
  type t =
    | Shield of { duration : int; value : float }
    | Stun of { duration : int }
    | Enhanced_attack of { duration : int }

  (** [kind b] is the buff's kind.
      @param b Buff snapshot. *)
  val kind : t -> Buff_kind.t
  (** [duration b] is the remaining time in rounds.
      @param b Buff snapshot. *)
  val duration : t -> int
  (** [shield_value b] is the remaining shield, or [None] for other buffs.
      @param b Buff snapshot. *)
  val shield_value : t -> float option
  (** [get buffs k] is the buff of kind [k], if present.
      @param buffs Buff snapshots.
      @param k Buff kind. *)
  val get : t list -> Buff_kind.t -> t option
  (** [has buffs k] is true when a buff of kind [k] is present.
      @param buffs Buff snapshots.
      @param k Buff kind. *)
  val has : t list -> Buff_kind.t -> bool
  (** Check whether the collection has a shield buff.
      @param buffs Buff snapshots. *)
  val has_shield : t list -> bool
  (** Check whether the collection has a stun buff.
      @param buffs Buff snapshots. *)
  val has_stun : t list -> bool
  (** Check whether the collection has an enhanced-attack buff.
      @param buffs Buff snapshots. *)
  val has_enhanced_attack : t list -> bool
end

type champion
type camp

module Ability : sig
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

  (** [kind i] is the ability's kind.
      @param i Ability metadata. *)
  val kind : info -> Ability_kind.t
  (** [properties i] is the properties shared by every ability kind.
      @param i Ability metadata. *)
  val properties : info -> properties
  (** [range i] is the casting range, or [None] for range-less abilities.
      @param i Ability metadata. *)
  val range : info -> float option

  type t

  (** [slot a] is the slot index on its champion.
      @param a Ability slot. *)
  val slot : t -> int
  (** [info a] is the cached metadata, or [None] for an unused slot.
      @param a Ability slot. *)
  val info : t -> info option
  (** [is a k] is true when the slot holds an ability of kind [k].
      @param a Ability slot.
      @param k Ability kind. *)
  val is : t -> Ability_kind.t -> bool
  (** Check whether this is an attack-reset ability.
      @param a Ability slot. *)
  val is_attack_reset : t -> bool
  (** Check whether this is a damage ability.
      @param a Ability slot. *)
  val is_damage : t -> bool
  (** Check whether this is an area-damage ability.
      @param a Ability slot. *)
  val is_aoe_damage : t -> bool
  (** Check whether this is a crowd-control ability.
      @param a Ability slot. *)
  val is_cc : t -> bool
  (** Check whether this is a healing ability.
      @param a Ability slot. *)
  val is_heal : t -> bool
  (** Check whether this is a dash ability.
      @param a Ability slot. *)
  val is_dash : t -> bool
  (** Check whether this is a shield ability.
      @param a Ability slot. *)
  val is_shield : t -> bool
  (** [cooldown a] is the remaining cooldown; 0 means ready.
      @param a Ability slot. *)
  val cooldown : t -> int
  (** [is_on_cooldown a] is true while the cooldown is running.
      @param a Ability slot. *)
  val is_on_cooldown : t -> bool
  (** Check whether this ability is ready to cast.
      @param a Ability slot. *)
  val is_ready : t -> bool
  (** [valid_targets a] is the categories of target the ability accepts.
      @param a Ability slot. *)
  val valid_targets : t -> Ability_target.t
  (** [valid_tiles a] is the tiles within casting range; visibility and occupancy are not checked.
      @param a Ability slot. *)
  val valid_tiles : t -> Bitboard.t
  (** Like [valid_tiles], but writes into [out]. Raises [No_range] for attack resets.
      @param a Ability slot.
      @param out Destination; its previous contents are overwritten. *)
  val mask_into : t -> Bitboard.t -> unit

  (** [can_target_camps a] is true when the ability can target camps.
      @param a Ability slot. *)
  val can_target_camps : t -> bool
  (** [can_target_enemies a] is true when the ability can target enemy champions.
      @param a Ability slot. *)
  val can_target_enemies : t -> bool
  (** [can_target_allies a] is true when the ability can target allied champions.
      @param a Ability slot. *)
  val can_target_allies : t -> bool
  (** [can_target_self a] is true when the ability can target its caster.
      @param a Ability slot. *)
  val can_target_self : t -> bool
  (** [can_target_ground a] is true when the ability can target a tile.
      @param a Ability slot. *)
  val can_target_ground : t -> bool
  (** [damage_amount ?against a] is the damage the ability would deal, after mitigation and shields when [against] is given.
      @param against Optional visible champion the amount is computed against.
      @param a Ability slot. *)
  val damage_amount : ?against:champion -> t -> float
  (** [heal_amount ?against a] is the healing the ability would do, capped at the target's missing health when [against] is given.
      @param against Optional visible champion the amount is computed against.
      @param a Ability slot. *)
  val heal_amount : ?against:champion -> t -> float
  (** [shield_amount ?against a] is the shield the ability would grant.
      @param against Optional visible champion the amount is computed against.
      @param a Ability slot. *)
  val shield_amount : ?against:champion -> t -> float

  (** Pass at most one of [~target] and [~at]. Unit casts land on the target's tile; ground casts take a tile.
      Returns false for a newly revealed obstruction; known invalid destinations raise [Cambc_error].
      @param target Champion or camp to cast on.
      @param at Ground tile to cast at.
      @param a Ability slot of the bot's own champion. *)
  val cast : ?target:[ `Champion of champion | `Camp of camp ] -> ?at:Vec2.t -> t -> bool
  (** Altar locks are not valid ability targets. *)
end

module Ward : sig
  type t = { position : Vec2.t; team : Team.t; remaining_time : int option }

  (** Check whether this ward belongs to the observing team.
      @param w Ward snapshot. *)
  val is_ally : t -> bool
  (** Check whether this ward belongs to the opposing team.
      @param w Ward snapshot. *)
  val is_enemy : t -> bool
  (** [visible_tiles w] is the tiles the ward sees.
      @param w Ward snapshot. *)
  val visible_tiles : t -> Bitboard.t
  (** Like [visible_tiles], but writes into [out].
      @param w Ward snapshot.
      @param out Destination; its previous contents are overwritten. *)
  val visible_tiles_into : t -> Bitboard.t -> unit
  (** [visible_tiles_from p] is the vision an ordinary ward would have at [p].
      @param p Hypothetical ward tile. *)
  val visible_tiles_from : Vec2.t -> Bitboard.t
  (** Like [visible_tiles_from], but writes into [out].
      @param p Hypothetical ward tile.
      @param out Destination; its previous contents are overwritten. *)
  val visible_tiles_from_into : Vec2.t -> Bitboard.t -> unit
end

module Altar_lock : sig
  type t = { team : Team.t; position : Vec2.t; hits_remaining : int }

  (** Check whether this altar lock belongs to the observing team.
      @param l Altar lock snapshot. *)
  val is_ally : t -> bool
  (** Check whether this altar lock belongs to the opposing team.
      @param l Altar lock snapshot. *)
  val is_enemy : t -> bool
end

module Teleport : sig
  type t = {
    champion : champion;
    team : Team.t;
    position : Vec2.t;
    remaining_time : int;
  }

  (** Check whether this teleport belongs to the observing team.
      @param t Teleport snapshot. *)
  val is_ally : t -> bool
  (** Check whether this teleport belongs to the opposing team.
      @param t Teleport snapshot. *)
  val is_enemy : t -> bool
  (** [visible_tiles t] is the tiles visible from the reservation.
      @param t Teleport snapshot. *)
  val visible_tiles : t -> Bitboard.t
  (** Like [visible_tiles], but writes into [out].
      @param t Teleport snapshot.
      @param out Destination; its previous contents are overwritten. *)
  val visible_tiles_into : t -> Bitboard.t -> unit
end

module Champion : sig
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

  (** [equal a b] is true for the same champion.
      @param a First champion.
      @param b Second champion. *)
  val equal : t -> t -> bool
  (** Hash consistent with [equal].
      @param c Champion. *)
  val hash : t -> int
  (** Static designs of every champion type, as used during the draft. *)
  val designs : unit -> t array
  (** [champion_name c] is the champion's type.
      @param c Champion. *)
  val champion_name : t -> Champion_type.t
  (** [passive c] is the champion's passive.
      @param c Champion. *)
  val passive : t -> Passive.t
  (** [team c] is the champion's team.
      @param c Champion. *)
  val team : t -> Team.t
  (** Check whether this champion belongs to the observing team.
      @param c Champion. *)
  val is_ally : t -> bool
  (** Check whether this champion belongs to the opposing team.
      @param c Champion. *)
  val is_enemy : t -> bool
  (** [base_stats c] is the champion type's design stats.
      @param c Champion. *)
  val base_stats : t -> base_stats
  (** [is_alive c] is true while the champion is alive.
      @param c Champion. *)
  val is_alive : t -> bool
  (** [is_dead c] is [not (is_alive c)].
      @param c Champion. *)
  val is_dead : t -> bool
  (** [state c] is the champion's life, channel and action flags.
      @param c Champion. *)
  val state : t -> Champion_state.t
  (** Check whether this champion can move.
      @param c Champion. *)
  val can_move : t -> bool
  (** Check whether this champion can attack.
      @param c Champion. *)
  val can_attack : t -> bool
  (** Check whether this champion can cast.
      @param c Champion. *)
  val can_cast : t -> bool
  (** Check whether this champion is recalling.
      @param c Champion. *)
  val is_recalling : t -> bool
  (** Check whether this champion is submitting gems.
      @param c Champion. *)
  val is_submitting_gems : t -> bool
  (** Check whether this champion is teleporting.
      @param c Champion. *)
  val is_teleporting : t -> bool
  (** Check whether this champion has completed its turn.
      @param c Champion. *)
  val has_completed_turn : t -> bool
  (** [restrictions c] is the actions the champion's buffs currently prohibit.
      @param c Champion. *)
  val restrictions : t -> Buff_restrictions.t
  (** [buffs c] is the champion's current buffs; raises if [c] is not visible.
      @param c Champion. *)
  val buffs : t -> Buff.t list
  (** [health c] is current health; raises if [c] is not visible.
      @param c Champion. *)
  val health : t -> float
  (** [max_health c] is current maximum health; raises if [c] is not visible.
      @param c Champion. *)
  val max_health : t -> float
  (** [strength c] is current strength; raises if [c] is not visible.
      @param c Champion. *)
  val strength : t -> float
  (** [focus c] is current focus; raises if [c] is not visible.
      @param c Champion. *)
  val focus : t -> float
  (** [attack_range c] is the attack range in Chebyshev tiles; melee champions have range 1.
      @param c Champion. *)
  val attack_range : t -> int
  (** [attack_tiles c] is the tiles within attack range of [c]'s position.
      @param c Champion. *)
  val attack_tiles : t -> Bitboard.t
  (** Like [attack_tiles], but writes into [out].
      @param c Champion.
      @param out Destination; its previous contents are overwritten. *)
  val attack_tiles_into : t -> Bitboard.t -> unit
  (** [position c] is the current tile; raises if [c] is not visible.
      @param c Champion. *)
  val position : t -> Vec2.t
  (** [is_visible c] is true when the champion is currently visible.
      @param c Champion. *)
  val is_visible : t -> bool
  (** [visible_tiles c] is the tiles [c] currently sees.
      @param c Champion. *)
  val visible_tiles : t -> Bitboard.t
  (** Like [visible_tiles], but writes into [out].
      @param c Champion.
      @param out Destination; its previous contents are overwritten. *)
  val visible_tiles_into : t -> Bitboard.t -> unit
  (** [visible_tiles_from p c] is the vision [c] would have at [p].
      @param p Hypothetical tile to see from.
      @param c Champion. *)
  val visible_tiles_from : Vec2.t -> t -> Bitboard.t
  (** Like [visible_tiles_from], but writes into [out].
      @param p Hypothetical tile to see from.
      @param c Champion.
      @param out Destination; its previous contents are overwritten. *)
  val visible_tiles_from_into : Vec2.t -> t -> Bitboard.t -> unit
  (** [remaining_channel_time c] is the rounds left on the current recall, tribute or teleport channel.
      @param c Champion. *)
  val remaining_channel_time : t -> int
  (** [ward_positions c] is the tiles of [c]'s placed wards.
      @param c Champion. *)
  val ward_positions : t -> Vec2.t list
  (** [ward_cooldown c] is the rounds until [c] can place another ward.
      @param c Champion. *)
  val ward_cooldown : t -> int
  (** [ward_placement_tiles c] is the tiles where [c] could place a ward now.
      @param c Champion. *)
  val ward_placement_tiles : t -> Bitboard.t
  (** Like [ward_placement_tiles], but writes into [out].
      @param c Champion.
      @param out Destination; its previous contents are overwritten. *)
  val ward_placement_tiles_into : t -> Bitboard.t -> unit
  (** [can_buy ?count c item] is true when [c] can buy [item] in its current state.
      @param count Number of copies; defaults to 1.
      @param c Champion.
      @param item Item type. *)
  val can_buy : ?count:int -> t -> Item.t -> bool
  (** [can_buy_consumable c k] is true when [c] can buy a consumable now.
      @param c Champion.
      @param k Consumable type. *)
  val can_buy_consumable : t -> Consumable.t -> bool
  (** [can_sell c] is true when [c] can sell items in its current state.
      @param c Champion. *)
  val can_sell : t -> bool
  (** [abilities c] is the champion's ability slots.
      @param c Champion. *)
  val abilities : t -> Ability.t array
  (** [item_count c item] is how many of [item] [c] holds.
      @param c Champion.
      @param item Item type. *)
  val item_count : t -> Item.t -> int
  (** [item_counts c] is [c]'s item inventory.
      @param c Champion. *)
  val item_counts : t -> Item_counts.t
  (** [consumable_count c k] is how many of a consumable [c] carries.
      @param c Champion.
      @param k Consumable type. *)
  val consumable_count : t -> Consumable.t -> int
  (** [consumable_counts c] is [c]'s consumable inventory.
      @param c Champion. *)
  val consumable_counts : t -> Consumable_counts.t
  (** [gem_count c] is the gems [c] is carrying.
      @param c Champion. *)
  val gem_count : t -> int
  (** [gold_count c] is the gold [c] is carrying.
      @param c Champion. *)
  val gold_count : t -> int
  (** [prev_priority c] is the priority used for the current round.
      @param c Champion. *)
  val prev_priority : t -> int
  (** [priority c] is the priority that will apply from the next round.
      @param c Champion. *)
  val priority : t -> int

  (** [attack c target] basic-attacks a visible champion, camp or altar lock in range.
      @param c The bot's own champion.
      @param target Unit or altar lock to attack. *)
  val attack : t -> [ `Champion of champion | `Camp of camp | `Altar_lock of Altar_lock.t ] -> unit
  (** [buy ?count c item] buys items; the engine validates location, gold and recipe.
      @param count Number of copies; defaults to 1.
      @param c The bot's own champion.
      @param item Item type. *)
  val buy : ?count:int -> t -> Item.t -> unit
  (** [buy_consumable c k] buys one consumable; the engine validates gold and carry limit.
      @param c The bot's own champion.
      @param k Consumable type. *)
  val buy_consumable : t -> Consumable.t -> unit
  (** [sell ?count c item] sells items; the engine validates inventory and location.
      @param count Number of copies; defaults to 1.
      @param c The bot's own champion.
      @param item Item type. *)
  val sell : ?count:int -> t -> Item.t -> unit
  (** Returns false for a newly revealed obstruction; known invalid destinations raise [Cambc_error].
      @param at Tile to use the consumable at, for targeted consumables.
      @param c The bot's own champion.
      @param k Consumable to use. *)
  val use_consumable : ?at:Vec2.t -> t -> Consumable.t -> bool
  (** Returns false for a newly revealed obstruction; known invalid destinations raise [Cambc_error].
      @param c The bot's own champion.
      @param delta Displacement from the current position. *)
  val move : t -> Vec2.t -> bool
  (** Returns false for a newly revealed obstruction; known invalid destinations raise [Cambc_error].
      @param c The bot's own champion.
      @param destination Map tile to move to. *)
  val move_absolute : t -> Vec2.t -> bool
  (** Move to the reachable tile on a shortest path to [target], avoiding walls, holes, altars, the enemy spawn and
      visible units. Returns false without moving when already there, unable to move, or [target] is blocked or
      unreachable, and for a newly revealed obstruction.
      @param c The bot's own champion.
      @param target Map tile to approach. *)
  val move_towards : t -> Vec2.t -> bool
  (** [recall c] starts channelling a recall to the team spawn.
      @param c The bot's own champion. *)
  val recall : t -> unit
  (** [cancel_recall c] cancels the current recall.
      @param c The bot's own champion. *)
  val cancel_recall : t -> unit
  (** [submit_gems c altar] starts tributing carried gems at an altar.
      @param c The bot's own champion.
      @param altar Altar tile to tribute at. *)
  val submit_gems : t -> Vec2.t -> unit
  (** [cancel_submit_gems c] cancels the current tribute.
      @param c The bot's own champion. *)
  val cancel_submit_gems : t -> unit
  (** Returns false for a newly revealed obstruction; known invalid destinations raise [Cambc_error].
      @param c The bot's own champion.
      @param destination Tile to place the ward on. *)
  val place_ward : t -> Vec2.t -> bool
  (** [set_priority c priority] sets the scheduling priority.
      @param c The bot's own champion.
      @param priority New priority; takes effect from the next round and persists. *)
  val set_priority : t -> int -> unit
  (** [end_turn c] ends the turn now.
      @param c The bot's own champion. *)
  val end_turn : t -> unit
end

module Camp_group : sig
  type t
  type state = Alive | Reward_available | Respawning of int

  (** [equal a b] is true for the same group.
      @param a First group.
      @param b Second group. *)
  val equal : t -> t -> bool
  (** Hash consistent with [equal].
      @param g Camp group. *)
  val hash : t -> int
  (** [gold_value g] is the gold awarded for clearing the group.
      @param g Camp group. *)
  val gold_value : t -> int
  (** [gem_value g] is the gems awarded for clearing the group.
      @param g Camp group. *)
  val gem_value : t -> int
  (** [max_respawn_timer g] is the rounds the group takes to respawn after being cleared.
      @param g Camp group. *)
  val max_respawn_timer : t -> int
  (** [camps g] is the camps in the group.
      @param g Camp group. *)
  val camps : t -> camp list
  (** [spawn_tiles g] is the spawn tiles of the group's camps. The board is shared and cached: copy before mutating.
      @param g Camp group. *)
  val spawn_tiles : t -> Bitboard.t
  (** Like [spawn_tiles], but writes into [out].
      @param g Camp group.
      @param out Destination; its previous contents are overwritten. *)
  val spawn_tiles_into : t -> Bitboard.t -> unit

  (** [state g] is the group's current state.
      @param g Camp group. *)
  val state : t -> state
end

module Camp : sig
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

  (** [equal a b] is true for the same camp.
      @param a First camp.
      @param b Second camp. *)
  val equal : t -> t -> bool
  (** Hash consistent with [equal].
      @param c Camp. *)
  val hash : t -> int
  (** [kind c] is the camp's type.
      @param c Camp. *)
  val kind : t -> Camp_type.t
  (** Check whether this camp is a wolf.
      @param c Camp. *)
  val is_wolf : t -> bool
  (** Check whether this camp is an elk.
      @param c Camp. *)
  val is_elk : t -> bool
  (** Check whether this camp is a gem cluster.
      @param c Camp. *)
  val is_gem_cluster : t -> bool
  (** Check whether this camp is a gold cluster.
      @param c Camp. *)
  val is_gold_cluster : t -> bool
  (** Check whether this camp is a monolith.
      @param c Camp. *)
  val is_monolith : t -> bool
  (** [spawn_position c] is the camp's spawn tile, not its live position.
      @param c Camp. *)
  val spawn_position : t -> Vec2.t
  (** [camp_group c] is the group the camp belongs to.
      @param c Camp. *)
  val camp_group : t -> Camp_group.t
  (** [base_stats c] is the camp type's design stats.
      @param c Camp. *)
  val base_stats : t -> base_stats
  (** [buffs c] is the current buffs; raises if [c] is not visible.
      @param c Camp. *)
  val buffs : t -> Buff.t list
  (** [health c] is current health; raises if [c] is not visible.
      @param c Camp. *)
  val health : t -> float
  (** [max_health c] is current maximum health; raises if [c] is not visible.
      @param c Camp. *)
  val max_health : t -> float
  (** [strength c] is current strength; raises if [c] is not visible.
      @param c Camp. *)
  val strength : t -> float
  (** [focus c] is current focus; raises if [c] is not visible.
      @param c Camp. *)
  val focus : t -> float
  (** [attack_range c] is the attack range in Chebyshev tiles; melee units have range 1.
      @param c Camp. *)
  val attack_range : t -> int
  (** [attack_tiles c] is the tiles within attack range of [c]'s position.
      @param c Camp. *)
  val attack_tiles : t -> Bitboard.t
  (** Like [attack_tiles], but writes into [out].
      @param c Camp.
      @param out Destination; its previous contents are overwritten. *)
  val attack_tiles_into : t -> Bitboard.t -> unit
  (** [position c] is the current tile; raises if [c] is not visible.
      @param c Camp. *)
  val position : t -> Vec2.t
  (** [is_visible c] is true when [c] is currently visible.
      @param c Camp. *)
  val is_visible : t -> bool
end

module Game_unit : sig
  type t = [ `Champion of champion | `Camp of camp ]

  (** [buffs u] is the current buffs; raises if [u] is not visible.
      @param u Unit. *)
  val buffs : t -> Buff.t list
  (** [health u] is current health; raises if [u] is not visible.
      @param u Unit. *)
  val health : t -> float
  (** [max_health u] is current maximum health; raises if [u] is not visible.
      @param u Unit. *)
  val max_health : t -> float
  (** [strength u] is current strength; raises if [u] is not visible.
      @param u Unit. *)
  val strength : t -> float
  (** [focus u] is current focus; raises if [u] is not visible.
      @param u Unit. *)
  val focus : t -> float
  (** [attack_range u] is the attack range in Chebyshev tiles; melee units have range 1.
      @param u Unit. *)
  val attack_range : t -> int
  (** [attack_tiles u] is the tiles within attack range of [u]'s position.
      @param u Unit. *)
  val attack_tiles : t -> Bitboard.t
  (** Like [attack_tiles], but writes into [out].
      @param u Unit.
      @param out Destination; its previous contents are overwritten. *)
  val attack_tiles_into : t -> Bitboard.t -> unit
  (** [position u] is the current tile; raises if [u] is not visible.
      @param u Unit. *)
  val position : t -> Vec2.t
  (** [is_visible u] is true when [u] is currently visible.
      @param u Unit. *)
  val is_visible : t -> bool
end

module Game_map : sig
  (** Map width in tiles. *)
  val width : unit -> int
  (** Map height in tiles. *)
  val height : unit -> int
  (** [is_in_bounds p] is true when [p] lies on the map.
      @param p Position to check. *)
  val is_in_bounds : Vec2.t -> bool
  (** The symmetry the map was generated with. *)
  val symmetry : unit -> Symmetry.t
  (** [reflect_horizontal p] reflects [p] horizontally, reversing x.
      @param p Map position. *)
  val reflect_horizontal : Vec2.t -> Vec2.t
  (** [reflect_vertical p] reflects [p] vertically, reversing y.
      @param p Map position. *)
  val reflect_vertical : Vec2.t -> Vec2.t
  (** [rotate_180 p] rotates [p] 180 degrees within the map.
      @param p Map position. *)
  val rotate_180 : Vec2.t -> Vec2.t

  (** Wall tiles. Terrain and spawn boards are shared cached values: copy before mutating. *)
  val walls : unit -> Bitboard.t
  (** Bush tiles, which hide champions from outside vision. *)
  val bushes : unit -> Bitboard.t
  (** Hole tiles, which block movement but not vision. *)
  val holes : unit -> Bitboard.t
  (** Altar tiles, where gems are tributed. *)
  val altars : unit -> Bitboard.t
  (** [spawn_region team] is the team's spawn region.
      @param team Team whose spawn region to return. *)
  val spawn_region : Team.t -> Bitboard.t
  (** Every camp group on the map, also available in draft. *)
  val camp_groups : unit -> Camp_group.t list
  (** [camp_at p] is the camp that spawns at [p]; raises if none does.
      @param p Spawn tile. *)
  val camp_at : Vec2.t -> Camp.t
  (** Like [camp_at], but [None] if no camp spawns at [p].
      @param p Spawn tile. *)
  val camp_at_opt : Vec2.t -> Camp.t option
end

module Vision : sig
  (** Every tile visible to the bot's champion. *)
  val visible_tiles : unit -> Bitboard.t
  (** Like [visible_tiles], but writes into [out].
      @param out Destination; its previous contents are overwritten. *)
  val visible_tiles_into : Bitboard.t -> unit
  (** Tiles of visible camps. *)
  val camp_tiles : unit -> Bitboard.t
  (** Like [camp_tiles], but writes into [out].
      @param out Destination; its previous contents are overwritten. *)
  val camp_tiles_into : Bitboard.t -> unit
  (** Tiles of visible teleport reservations. *)
  val teleport_tiles : unit -> Bitboard.t
  (** Like [teleport_tiles], but writes into [out].
      @param out Destination; its previous contents are overwritten. *)
  val teleport_tiles_into : Bitboard.t -> unit
  (** [champion_tiles ?team ()] is the tiles of visible champions.
      @param team Team to include; both teams when omitted. *)
  val champion_tiles : ?team:Team.t -> unit -> Bitboard.t
  (** Like [champion_tiles], but writes into [out].
      @param team Team to include; both teams when omitted.
      @param out Destination; its previous contents are overwritten. *)
  val champion_tiles_into : ?team:Team.t -> Bitboard.t -> unit
  (** [ward_tiles ?team ()] is the tiles of visible wards.
      @param team Team to include; both teams when omitted. *)
  val ward_tiles : ?team:Team.t -> unit -> Bitboard.t
  (** Like [ward_tiles], but writes into [out].
      @param team Team to include; both teams when omitted.
      @param out Destination; its previous contents are overwritten. *)
  val ward_tiles_into : ?team:Team.t -> Bitboard.t -> unit
  (** [altar_lock_tiles ?team ()] is the tiles of visible altar locks.
      @param team Team to include; both teams when omitted. *)
  val altar_lock_tiles : ?team:Team.t -> unit -> Bitboard.t
  (** Like [altar_lock_tiles], but writes into [out].
      @param team Team to include; both teams when omitted.
      @param out Destination; its previous contents are overwritten. *)
  val altar_lock_tiles_into : ?team:Team.t -> Bitboard.t -> unit
  (** [champions ?team ()] is the visible champions.
      @param team Team to include; both teams when omitted. *)
  val champions : ?team:Team.t -> unit -> Champion.t list
  (** Visible camps. *)
  val camps : unit -> Camp.t list
  (** Camp groups with at least one visible camp. *)
  val camp_groups : unit -> Camp_group.t list
  (** [wards ?team ()] is snapshots of visible wards.
      @param team Team to include; both teams when omitted. *)
  val wards : ?team:Team.t -> unit -> Ward.t list
  (** Snapshots of visible teleport reservations. *)
  val teleports : unit -> Teleport.t list
  (** [altar_locks ?team ()] is snapshots of visible altar locks.
      @param team Team to include; both teams when omitted. *)
  val altar_locks : ?team:Team.t -> unit -> Altar_lock.t list
  (** [champion_at p] is the visible champion on [p]; raises if there is none or the tile is hidden.
      @param p Tile to query. *)
  val champion_at : Vec2.t -> Champion.t
  (** Like [champion_at], but [None] instead of raising.
      @param p Tile to query. *)
  val champion_at_opt : Vec2.t -> Champion.t option
  (** [camp_at p] is the visible camp currently on [p]; raises if there is none or the tile is hidden.
      @param p Tile to query. *)
  val camp_at : Vec2.t -> Camp.t
  (** Like [camp_at], but [None] instead of raising.
      @param p Tile to query. *)
  val camp_at_opt : Vec2.t -> Camp.t option
  (** [ward_at ?team p] is the visible ward on [p]; raises if there is none.
      @param team Team to restrict to; either team when omitted.
      @param p Tile to query. *)
  val ward_at : ?team:Team.t -> Vec2.t -> Ward.t
  (** Like [ward_at], but [None] instead of raising.
      @param team Team to restrict to; either team when omitted.
      @param p Tile to query. *)
  val ward_at_opt : ?team:Team.t -> Vec2.t -> Ward.t option
  (** [teleport_at p] is the visible teleport reservation on [p]; raises if there is none.
      @param p Tile to query. *)
  val teleport_at : Vec2.t -> Teleport.t
  (** Like [teleport_at], but [None] instead of raising.
      @param p Tile to query. *)
  val teleport_at_opt : Vec2.t -> Teleport.t option
  (** [altar_lock_at p] is the visible altar lock on [p], ally first; raises if there is none.
      @param p Tile to query. *)
  val altar_lock_at : Vec2.t -> Altar_lock.t
  (** Like [altar_lock_at], but [None] instead of raising.
      @param p Tile to query. *)
  val altar_lock_at_opt : Vec2.t -> Altar_lock.t option
  (** [has_champion p] is true when a visible champion is on [p].
      @param p Tile to query. *)
  val has_champion : Vec2.t -> bool
  (** [has_camp p] is true when a visible camp is on [p].
      @param p Tile to query. *)
  val has_camp : Vec2.t -> bool
  (** [has_ward ?team p] is true when a visible ward is on [p]; hidden wards give [false].
      @param team Team to restrict to; either team when omitted.
      @param p Tile to query. *)
  val has_ward : ?team:Team.t -> Vec2.t -> bool
  (** [has_teleport p] is true when a visible teleport reservation is on [p].
      @param p Tile to query. *)
  val has_teleport : Vec2.t -> bool
  (** [has_altar_lock p] is true when a visible altar lock of either team is on [p].
      @param p Tile to query. *)
  val has_altar_lock : Vec2.t -> bool
end

module Draft : sig
  (** Number of champions each team drafts. *)
  val team_size : unit -> int
  (** [current team] is the team's picks so far, in order.
      @param team Team whose picks to return. *)
  val current : Team.t -> Champion_draft.t list
  (** [pick champion] picks a champion for the current draft round; only valid during the draft.
      @param champion Champion to pick. *)
  val pick : Champion_draft.t -> unit
end

module Shared_memory : sig
  (** Shared memory size in bytes. *)
  val size : unit -> int
  (** [read ~offset len] reads [len] bytes; raises if the range does not fit.
      @param offset Byte offset into shared memory.
      @param len Number of bytes to read. *)
  val read : offset:int -> int -> Bytes.t
  (** [write ~offset data] writes bytes; raises if the range does not fit.
      @param offset Byte offset into shared memory.
      @param data Bytes to write. *)
  val write : offset:int -> Bytes.t -> unit
  (** [get offset] reads one byte.
      @param offset Byte offset into shared memory. *)
  val get : int -> int
  (** [set offset value] writes one byte.
      @param offset Byte offset into shared memory.
      @param value Byte to store, 0 to 255. *)
  val set : int -> int -> unit
  (** [read_into ~offset out] fills [out] from shared memory.
      @param offset Byte offset into shared memory.
      @param out Destination; its length sets how many bytes are read. *)
  val read_into : offset:int -> Bytes.t -> unit

  (** Element types for typed reads/writes (little-endian, like the Rust/C++ SDKs). *)
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

  (** [read_values ~offset kind n] reads [n] typed values in little-endian layout.
      @param offset Byte offset into shared memory.
      @param kind Element type.
      @param n Number of values to read. *)
  val read_values : offset:int -> 'a kind -> int -> 'a array
  (** [write_values ~offset kind values] writes typed values in little-endian layout.
      @param offset Byte offset into shared memory.
      @param kind Element type.
      @param values Values to write. *)
  val write_values : offset:int -> 'a kind -> 'a array -> unit
end

module Draft_memory : sig
  (** Copy of the draft memory snapshot. *)
  val bytes : unit -> Bytes.t
  (** Length of the draft memory snapshot in bytes. *)
  val length : unit -> int
  (** [get offset] reads one byte of the snapshot.
      @param offset Byte offset into the snapshot. *)
  val get : int -> int
  (** [submit data] sets the draft memory passed to this team's gameplay bots; only valid during the draft.
      @param data Bytes to submit. *)
  val submit : Bytes.t -> unit
end

module Debug : sig
  (** [draw_dot p rgb] draws a debug dot in the replay.
      @param p Tile to mark.
      @param rgb Packed [0xRRGGBB] colour. *)
  val draw_dot : Vec2.t -> int -> unit
  (** [draw_line a b rgb] draws a debug line in the replay.
      @param a Start tile.
      @param b End tile.
      @param rgb Packed [0xRRGGBB] colour. *)
  val draw_line : Vec2.t -> Vec2.t -> int -> unit
end

(** Load the SDK caches. This already runs at startup, before the bot's modules; calling it again does nothing. *)
val init : unit -> unit
(** Game constants loaded at startup. *)
val constants : unit -> Constants.t
(** True when this instance runs the draft rather than controlling a champion. *)
val is_draft : unit -> bool
(** The champion this bot controls; raises during the draft. *)
val self_champion : unit -> Champion.t
(** Like [self_champion], but [None] during the draft. *)
val self_champion_opt : unit -> Champion.t option
(** [champions ?team ()] is every roster champion, including hidden ones.
    @param team Team to include; both teams when omitted. *)
val champions : ?team:Team.t -> unit -> Champion.t list
(** The current round number. *)
val round_number : unit -> int
(** [global_submitted_gems team] is the total gems the team has tributed.
    @param team Team to count. *)
val global_submitted_gems : Team.t -> int
(** Fuel left in the current turn. *)
val fuel_remaining : unit -> int
(** End the current turn. *)
val end_turn : unit -> unit
(** Resign the game for the bot's team. *)
val resign : unit -> unit
