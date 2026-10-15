#!/usr/bin/env sh
# Final offline release check: Release tests, sanitizer tests, and bundled audio validation.
set -eu
ROOT="$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)"
WORK_ROOT="${1:-${TMPDIR:-/tmp}/rift-runner-release-check}"
RELEASE_BUILD="$WORK_ROOT/release"
SAN_BUILD="$WORK_ROOT/sanitized"

cmake -S "$ROOT" -B "$RELEASE_BUILD" \
  -DCMAKE_BUILD_TYPE=Release \
  -DRR_TESTS_ONLY=ON \
  -DRR_BUILD_TESTS=ON
cmake --build "$RELEASE_BUILD" --parallel
ctest --test-dir "$RELEASE_BUILD" --output-on-failure

cmake -S "$ROOT" -B "$SAN_BUILD" \
  -DCMAKE_BUILD_TYPE=Debug \
  -DRR_TESTS_ONLY=ON \
  -DRR_BUILD_TESTS=ON \
  -DRR_ENABLE_SANITIZERS=ON
cmake --build "$SAN_BUILD" --parallel
ASAN_OPTIONS="${ASAN_OPTIONS:-detect_leaks=1}" \
UBSAN_OPTIONS="${UBSAN_OPTIONS:-print_stacktrace=1:halt_on_error=1}" \
  ctest --test-dir "$SAN_BUILD" --output-on-failure
python3 "$ROOT/tools/check_assets.py"
printf '\nOffline release checks passed. This does not verify a real raylib game build.\n'
