# RIFT RUNNER: SIGNAL ZERO

A neon-cosmic survival arcade game written in C99 using raylib 6.0. This repository merges the complete 15-part development series into one project, with source, original generated audio assets, regression tests, build scripts, GitHub CI, and release documentation. The source is consolidated; a real native build and hands-on playtest are still required before claiming a verified playable release.

## Features

- Cosmic rift backdrop, layered stars, neon arena, motion streaks, particle bursts and screen-shake feedback.
- WASD / arrow movement and Phase Dash on Space / Left Shift.
- Pursuing drones and faster interceptors, collision damage, invulnerability windows, and limited lives.
- Energy shards, combo scoring, eight-shard sector advancement, and three upgrade choices.
- Core Surge warning and expanding radial hazard.
- Original generated PCM WAV ambient music and effects, with music/SFX levels and a mute setting.
- Persistent profile storing personal-best score/sector, runs played, lifetime shards, audio settings, and the screen-shake preference; active runs are finalized on shutdown.
- Optional screen-shake toggle for reduced-motion comfort.
- Resizable window, fullscreen toggle, pause/retry/game-over flow.
- Headless profile/game-state regression tests, WAV-asset validation, and CI with independent offline-test and native-build jobs.


## Controls

- **WASD / arrow keys:** Move
- **Space / Left Shift:** Phase Dash
- **P:** Pause / resume
- **Esc:** Pause during a run; return to title from pause/game over; quit from title. Sector upgrade selection must be completed before continuing.
- **Enter / Space:** Start or retry
- **1 / 2 / 3:** Choose an upgrade
- **F:** Fullscreen
- **S (title screen):** Settings
- **Settings:** Up/Down select; Left/Right adjust volumes or toggle options; Enter toggles/confirms; Esc returns to title

## Build the game

Requirements: CMake 3.24+, a C99-capable compiler, desktop development libraries required by raylib, and internet access on first configure (unless using installed/local raylib).

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
ctest --test-dir build --output-on-failure
```

Use `-DRR_USE_SYSTEM_RAYLIB=ON` for an installed raylib package (via CMake config or pkg-config), or `-DRR_RAYLIB_SOURCE_DIR=/path/to/raylib` for an existing raylib source tree. Default builds fetch raylib 6.0. See [BUILDING.md](BUILDING.md) and [DIAGNOSTICS.md](DIAGNOSTICS.md).

## Offline validation without raylib

```sh
cmake -S . -B build-tests -DCMAKE_BUILD_TYPE=Release -DRR_TESTS_ONLY=ON -DRR_BUILD_TESTS=ON
cmake --build build-tests --config Release
ctest --test-dir build-tests --output-on-failure
python3 tools/check_assets.py
```

Or run `tools/validate.sh`. For the final offline Release + sanitizer + audio validation pass, run `tools/release-check.sh` (optionally pass a writable work directory as its first argument). This test-only mode does not build a playable executable and does not test graphics, real window creation or live audio.

For AddressSanitizer and UndefinedBehaviorSanitizer on GCC/Clang, run `tools/validate-sanitized.sh`. Or configure manually with `-DRR_ENABLE_SANITIZERS=ON`; sanitizers instrument project targets and make runtime memory/undefined-behavior issues easier to catch. This option is for supported GCC/Clang toolchains and is not needed for normal Release builds.

The game executable goes in `build/bin/` and CMake copies `assets/` beside it. Launch from that directory so relative asset paths behave predictably.

## Upload this repository to GitHub

1. Extract the source ZIP (or clone your empty GitHub repository). Keep the project files at the repository root—do not upload the 15 separate installment ZIPs.
2. Create an empty repository on GitHub, then run these commands from this project folder (replace the URL with your own repository URL):

```sh
git init
git add .
git commit -m "Import RIFT RUNNER: SIGNAL ZERO"
git branch -M main
git remote add origin https://github.com/YOUR-ACCOUNT/YOUR-REPOSITORY.git
git push -u origin main
```

3. Open the repository's **Actions** tab and wait for the offline tests and native build workflow to complete. The native job uses the real raylib dependency; passing stub-based tests alone is not proof that the game launches.
4. After native CI succeeds and a maintainer has completed a desktop playtest, push a version tag such as `v0.15.1` to trigger the multi-platform release workflow. It is configured to build archives for Linux, Windows, and macOS. Do not publish a release tag while the native workflow is failing.

The source repository includes everything needed to build the game; the real raylib library is fetched as a dependency and keeps its own license. The current downloadable source ZIP is not a prebuilt executable.

## Save data

`rift_runner_profile.dat` is written to the process working directory. Profile writes use a temporary file before replacement. Back up or remove the file to reset local progress.

## Licensing

This project code and original generated WAV assets are released under the MIT license in `LICENSE`. raylib is a separate dependency and retains its own license and notices. No paid asset packages are required.

## Validation disclosure

The offline path passes Release tests and AddressSanitizer/UndefinedBehaviorSanitizer tests, including settings, profile persistence, gameplay, and extreme-counter checks. Separately, the complete application and test targets compiled and linked with strict warnings against a local test-only raylib stub. That checks project integration but does not create a playable graphical binary. The WAV validator confirmed all six PCM WAV files. Native CMake configuration still cannot fetch the actual raylib 6.0 archive because this environment cannot resolve `github.com`; therefore a build using the real raylib library and a live graphics/audio run remain unverified. See [RELEASE_STATUS.md](RELEASE_STATUS.md) and [RELEASE_CHECKLIST.md](RELEASE_CHECKLIST.md) for the exact release state.

## History

- Final audit: range-checked profile integer parsing, robust frame-time handling, consistent executable output paths, and GitHub multi-platform release workflow.
- Part 14: sanitizer build option, repeatable ASan/UBSan test script, and overflow-safe counters.
- Part 13: persistent Screen Shake ON/OFF option; improved settings layout; backwards-compatible profile persistence and new regression checks.
- Part 12: system raylib discovery supports CMake package targets and pkg-config fallback; improves diagnostics for missing system raylib.
- Part 11: control review; prevent accidentally skipping sector upgrades; correct arena/pause hints; add regressions for upgrade flow, dash protection, and arena boundaries.
- Part 10: offline validation script, WAV checks, configurable raylib dependency path, and separate CI jobs.
- Part 09: finalize active runs on application exit and test exactly-once shutdown-save behavior.
- Part 08: release hardening, save validation, adaptive HUD, regression tests, build guide and CI workflow.
- Part 07: persistent profile and run summary.
- Part 06: generated soundtrack, sound effects and settings menu.
- Part 05: Core Surge and reactive visual effects.
- Part 04: shards, combos and sector upgrades.
- Part 03: hostile drones/interceptors and lives.
- Part 02: movement, Phase Dash and afterimages.
- Part 01: raylib/CMake foundation and cosmic title screen.
