#!/usr/bin/env bash
# Usage: scripts/bench.sh BOT OPPONENT — plays every map in maps/ from both sides.
set -euo pipefail
cd "$(dirname "$0")/.."
bot="$1"
opp="$2"
cambc=".venv/bin/cambc"
out="$(mktemp -d)"
wins=0
games=0
for map in maps/*.map26a; do
    name="$(basename "$map" .map26a)"
    for side in A B; do
        if [ "$side" = A ]; then a="$bot"; b="$opp"; else a="$opp"; b="$bot"; fi
        log="$out/$name-$side.log"
        "$cambc" run "$a" "$b" --map "$map" -o "$out/replays" >"$log" 2>&1 || true
        result="$(rg -o 'Team [AB] \([^)]*\) wins: .*' "$log" | head -1 || true)"
        errors="$(rg -c 'bot error' "$log" || true)"
        games=$((games + 1))
        if echo "$result" | rg -q "Team $side "; then wins=$((wins + 1)); fi
        printf '%-10s %s  %s  errors=%s\n' "$name" "$side" "$result" "${errors:-0}"
        rg '^  [AB]  ' "$log" || true
    done
done
echo "$bot won $wins/$games vs $opp (logs: $out)"
