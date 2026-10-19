(* Dispatch draft and gameplay; a failed turn is logged and still yields. *)
open Cambc

let () =
  (* WASI randomness is always zero in matches, so seed from our spawn and this champion's start. *)
  let spawn = List.hd (Bitboard.positions (Game_map.spawn_region Team.Ally)) in
  let start = Option.to_list (Option.map Champion.position (self_champion_opt ())) in
  Random.full_init (Array.of_list (List.concat_map (fun p -> [p.Vec2.x; p.Vec2.y]) (spawn :: start)));
  let bot = Bot.{ chosen_stat = None } in
  while true do
    (try
      if is_draft () then Starter_draft.play ()
      else Bot.play_turn bot
    with error -> Printf.printf "bot error: %s\n%!" (Printexc.to_string error));
    end_turn ()
  done
