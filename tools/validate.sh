#!/usr/bin/env sh
# Reproducible offline validation for profile/game-state logic and WAV packaging.
set -eu
ROOT="$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)"
BUILD_DIR="${1:-$ROOT/build-offline-tests}"

cmake -S "$ROOT" -B "$BUILD_DIR" \
  -DCMAKE_BUILD_TYPE=Release \
  -DRR_TESTS_ONLY=ON \
  -DRR_BUILD_TESTS=ON
cmake --build "$BUILD_DIR" --parallel
ctest --test-dir "$BUILD_DIR" --output-on-failure
python3 "$ROOT/tools/check_assets.py"
