open Cambc

let play () =
  (* Choose randomly from the champions our team has not picked. *)
  let chosen = Draft.current Team.Ally in
  let available = List.filter (fun pick -> not (List.mem pick chosen)) Champion_draft.all in
  (* Draft memory starts at byte zero and is copied to each champion. *)
  Draft_memory.submit (Bytes.of_string "hello world");
  Draft.pick (List.nth available (Random.int (List.length available)))
