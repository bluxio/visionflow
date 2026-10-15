#!/usr/bin/env bash
# Usage: scripts/variant.sh NAME [--from SOURCE] 'name = value' ['name = value' ...]
# Copies bots/SOURCE (default blux) to bots/NAME and overrides tuning.h constants by name.
set -euo pipefail
cd "$(dirname "$0")/.."
name="$1"
shift
source_bot="blux"
if [ "${1:-}" = "--from" ]; then
    source_bot="$2"
    shift 2
fi
rm -rf "bots/$name"
mkdir -p "bots/$name"
cp -R "bots/$source_bot/src" "bots/$source_bot/.clangd" "bots/$source_bot/.gitignore" "bots/$name/"
for override in "$@"; do
    key="${override%% =*}"
    value="${override#*= }"
    grep -q " $key = " "bots/$name/src/tuning.h" || { echo "unknown knob: $key" >&2; exit 1; }
    sed -i '' -E "s/( $key = )[^;]*;/\1$value;/" "bots/$name/src/tuning.h"
done
