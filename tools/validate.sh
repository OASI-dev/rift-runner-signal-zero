#!/usr/bin/env sh
# Configure/build the native game and check bundled WAV asset validity.
set -eu
ROOT="$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)"
BUILD_DIR="${1:-$ROOT/build-validate}"
cmake -S "$ROOT" -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE=Release
cmake --build "$BUILD_DIR" --config Release --parallel
python3 "$ROOT/tools/check_assets.py"
printf '\nNative compile and asset checks completed. Run the game manually for gameplay validation.\n'
