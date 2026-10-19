#!/usr/bin/env bash
# Usage: scripts/sweep.sh "OPPONENT [OPPONENT...]" "name|knob = value|knob = value" ...
# Set SEEDS=1,2,3 to sample several games per map. Builds each variant from bots/blux and prints its win totals against the opponents.
set -euo pipefail
cd "$(dirname "$0")/.."
opponents="$1"
shift
for spec in "$@"; do
    IFS='|' read -r -a parts <<< "$spec"
    name="${parts[0]}"
    scripts/variant.sh "$name" "${parts[@]:1}"
    # shellcheck disable=SC2086
    python3 scripts/arena.py "$name" $opponents --quiet ${SEEDS:+--seeds "$SEEDS"} | grep '^==' | tr '\n' ' '
    echo
done
