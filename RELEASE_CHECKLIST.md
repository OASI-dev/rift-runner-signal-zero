# Release checklist

This project is still a release candidate, not a verified cross-platform binary release.

## Automated checks

- [x] Offline test-only CMake configuration succeeds without downloading raylib.
- [x] Both regression tests pass in Release mode.
- [x] Both regression tests pass with AddressSanitizer and UndefinedBehaviorSanitizer.
- [x] Full application target compiled and linked with strict warnings against the test-only raylib stub (integration check only, not a playable build).
- [x] Extreme counter regression verifies saturation instead of signed overflow.
- [x] Active-run shutdown finalization prevents duplicate lifetime-shard counting.
- [x] Profile save/load test includes malformed headers, non-finite volume values, and oversized integer tokens.
- [x] Headless game-state test observes Core Surge warning, activation and expansion.
- [x] All six bundled WAV assets pass RIFF/WAVE, PCM and non-empty data checks.
- [x] The release archive integrity check passes.
- [ ] Full native CMake build succeeds with the real raylib library from a clean build directory.
- [ ] Live game is played through title, settings, dash, pickups, upgrades, surge, pause, game-over, retry and save/load.
- [ ] Audio is listened to on at least one supported desktop platform.
- [ ] Window resizing, fullscreen and quit behavior are checked manually.
- [ ] Windows and macOS builds are confirmed by the tag-triggered GitHub workflow.

## Before publishing a binary

1. Configure a clean Release build using real raylib.
2. Build and run `ctest --test-dir build --output-on-failure`.
3. Play a complete run and confirm there are no missing assets or crashes.
4. Inspect the profile file and verify settings/high scores survive a restart.
5. Ship the executable with its `assets/` directory and include this project's `LICENSE`, README and the separate raylib license/notice where required.

## Part 14 validation additions

- [x] Sanitizer-enabled offline build completed, and both CTest cases passed under ASan/UBSan.
- [x] Score, combo, run count, sector, enemy count, and lifetime-shard counters are guarded against signed overflow.
- [ ] Native raylib build still requires network access or an installed/local real raylib dependency; current environment cannot resolve GitHub.

## Part 12 validation additions

- [x] Installed-raylib discovery supports CMake exported targets and pkg-config fallback.
- [x] Package-discovery/link path exercised against a local test-only stub package; this is not a real raylib build.
- [ ] Clean native configure/build with actual raylib remains blocked here because `github.com` DNS resolution fails.

## Part 11 validation additions

- [x] Offline regression test covers Escape on the upgrade screen.
- [x] Offline regression test covers Phase Dash protection and arena bounds.
- [ ] Verify upgrade-screen controls and HUD hints during live play with raylib.
- [ ] Perform a native raylib build and live window/audio test.

- [x] Invalid/NaN and oversized frame deltas are covered by the offline gameplay test.

- [ ] Verify Settings layout and the Screen Shake toggle at 800x480 and 1280x720 using real raylib.


## Final merged repository checkpoint (0.15.1)

- [x] All 15 installments are consolidated into one clean source tree.
- [x] Project version metadata updated to 0.15.1.
- [x] `RELEASE_STATUS.md` documents verified work and remaining requirements.
- [x] `tools/release-check.sh` runs Release offline tests, sanitizer tests, and audio validation.
- [x] Oversized profile integer tokens are rejected safely; regression test added.
- [x] CMake executable output path is standardized across single- and multi-config generators.
- [x] GitHub tag-triggered multi-platform build/package workflow added.
- [ ] CI workflow must run successfully after upload.
- [ ] Native build with real raylib and live desktop playtest remain outstanding.
