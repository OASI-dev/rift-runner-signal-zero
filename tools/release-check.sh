#!/usr/bin/env sh
# Native pre-release build plus bundled-asset validation.
set -eu
ROOT="$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)"
BUILD_DIR="${1:-${TMPDIR:-/tmp}/rift-runner-release-build}"
cmake -S "$ROOT" -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE=Release
cmake --build "$BUILD_DIR" --config Release --parallel
python3 "$ROOT/tools/check_assets.py"
[ -f "$ROOT/web/index.html" ]
[ -f "$ROOT/web/idbfs-pre.js" ]
printf '\nPre-release source checks completed. Browser deployment and live play-test still required.\n'
