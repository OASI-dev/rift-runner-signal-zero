#!/usr/bin/env sh
# Offline profile/gameplay tests under AddressSanitizer and UndefinedBehaviorSanitizer.
set -eu
ROOT="$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)"
BUILD_DIR="${1:-$ROOT/build-sanitized-tests}"

cmake -S "$ROOT" -B "$BUILD_DIR" \
  -DCMAKE_BUILD_TYPE=Debug \
  -DRR_TESTS_ONLY=ON \
  -DRR_BUILD_TESTS=ON \
  -DRR_ENABLE_SANITIZERS=ON
cmake --build "$BUILD_DIR" --parallel
ASAN_OPTIONS="${ASAN_OPTIONS:-detect_leaks=1}" UBSAN_OPTIONS="${UBSAN_OPTIONS:-print_stacktrace=1:halt_on_error=1}" \
  ctest --test-dir "$BUILD_DIR" --output-on-failure
python3 "$ROOT/tools/check_assets.py"
