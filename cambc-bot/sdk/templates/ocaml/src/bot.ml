(* Champion decisions. main.ml dispatches here after the draft. *)
open Cambc
module C = Champion
module B = Bitboard
module M = Game_map
module U = Game_unit

(* Per-champion state survives between turns. Pick one stat and keep buying it. *)
type bot = {
  mutable chosen_stat : int option;
}

let distance = Vec2.chebyshev_distance

let tiles_around position radius =
  let tiles = B.create () in
  B.draw_chebyshev tiles position (float_of_int radius);
  tiles

let blocked_tiles () =
  (* Combine terrain and visible occupants using native bitboard operations. *)
  let blocked = List.fold_left B.union (M.walls ())
    [M.holes (); M.altars (); M.spawn_region Team.Enemy;
     Vision.champion_tiles (); Vision.camp_tiles (); Vision.teleport_tiles ()] in
  B.set blocked (C.position (self_champion ())) false;
  blocked

let move_towards_closest goals =
  let me = self_champion () in
  let origin = C.position me in
  if not (C.can_move me) || B.is_set goals origin then false
  else
    let blocked = blocked_tiles () in
    let goals = B.intersect goals (B.union (B.complement blocked) (Vision.camp_tiles ())) in
    if B.is_empty goals then false
    else if B.count goals = 1 && not (B.is_set blocked (List.hd (B.positions goals)))
    then C.move_towards me (List.hd (B.positions goals))
    else
      let field = Distance_field.create () in
      Distance_field.compute field ~walls:(B.intersect blocked (B.complement goals)) ~roots:goals
        ~destinations:(tiles_around origin 0) ~visited:(B.create ())
        ~max_distance:Distance_field.unreachable;

      let reachable = B.reachable_tiles blocked origin ~distance:(C.base_stats me).base_move_speed in
      let step = List.fold_left (fun best tile ->
        if Distance_field.get field tile < Distance_field.get field best then tile else best)
        origin (B.positions reachable) in
      step <> origin && C.move_absolute me step

let buy_items bot me =
  (* Spend only on power shards of the randomly chosen stat. *)
  let item = [|Item.Power_shard_of_strength; Item.Power_shard_of_focus; Item.Power_shard_of_health|]
      .(Option.get bot.chosen_stat) in
  let count = C.gold_count me / Item_info.cost item in
  if count > 0 && C.can_buy ~count me item then C.buy ~count me item

let attack_if_in_range (target : U.t) =
  (* Make one basic attack if the target is already in attack range. *)
  let me = self_champion () in
  if C.can_attack me && U.is_visible target && U.health target > 0.
     && B.is_set (C.attack_tiles me) (U.position target) then begin
    C.attack me (target :> [ `Champion of champion | `Camp of camp | `Altar_lock of Altar_lock.t ]);
    true
  end else false

let farm_camps () =
  (* Attack a camp already in range, otherwise move toward the nearest camp tiles. *)
  if List.exists (fun camp -> attack_if_in_range (`Camp camp)) (Vision.camps ()) then ()
  else begin
    let visible = Vision.visible_tiles () in
    let goals = List.fold_left (fun goals group ->
      let respawning = not (B.is_empty (B.intersect (Camp_group.spawn_tiles group) visible))
        && (match Camp_group.state group with Camp_group.Respawning _ -> true | _ -> false) in
      if respawning then goals else B.union goals (Camp_group.spawn_tiles group))
      (B.create ()) (M.camp_groups ()) in

    if move_towards_closest goals then
      ignore (List.exists (fun camp -> attack_if_in_range (`Camp camp)) (Vision.camps ()))
  end

let bank_gems me =
  (* Union all altar approach tiles: the field chooses the nearest reachable altar. *)
  let altars = B.positions (M.altars ()) |> List.filter (fun p -> not (Vision.has_altar_lock p)) in
  let range = (constants ()).submit_gems_range in

  match List.find_opt (fun p -> distance (C.position me) p <= range) altars with
  | Some altar -> C.submit_gems me altar
  | None ->
      let goals = List.fold_left (fun goals p -> B.union goals (tiles_around p range)) (B.create ()) altars in
      ignore (move_towards_closest goals)

let play_turn bot =
  if bot.chosen_stat = None then begin
    bot.chosen_stat <- Some (Random.int 3);
    Printf.printf "draft: %s\n%!" (Bytes.sub_string (Draft_memory.bytes ()) 0 11)
  end;
  (* Read before writing: little-endian u32 at shared-memory byte offset zero. *)
  let previous = (Shared_memory.read_values ~offset:0 Shared_memory.U32 1).(0) in
  Printf.printf "shared round: %d\n%!" previous;
  Shared_memory.write_values ~offset:0 Shared_memory.U32 [|round_number ()|];

  (* Priorities: shop and recall, attack enemies in range, bank gems, then farm. *)
  let me = self_champion () in
  if not (C.is_alive me) || C.is_recalling me || C.is_submitting_gems me || C.is_teleporting me
     || (not (C.can_move me) && not (C.can_attack me)) then ()
  else begin
    let at_spawn = B.is_set (M.spawn_region Team.Ally) (C.position me) in
    if at_spawn then buy_items bot me;
    if not at_spawn && C.health me < C.max_health me *. 0.35 then C.recall me
    else if List.exists (fun enemy -> attack_if_in_range (`Champion enemy))
      (Vision.champions ~team:Team.Enemy ()) then ()
    else if C.gem_count me > 0 then bank_gems me
    else begin
      farm_camps ();
      C.set_priority me (1 + Random.int 100)
    end
  end

