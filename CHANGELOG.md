# Changelog

## Final repository audit (0.15.1)
- Merged the complete 15-part source into one GitHub-ready repository layout.
- Replaced unchecked `scanf("%d")` profile counter conversions with range-checked `strtol` parsing; added oversized-number regression coverage.
- Defensively sanitize frame-time values in the app loop and renderer.
- Keep executable output consistently in `build/bin/` for single- and multi-configuration CMake generators.
- Added GitHub tag-triggered Linux, Windows, and macOS build/package workflow.
- Re-ran offline Release and ASan/UBSan tests plus WAV validation; real raylib build/desktop playtest remain outstanding in this environment.

## Part 15
- Bumped project metadata to version 0.15.0.
- Added `RELEASE_STATUS.md` with the verified checks and explicit remaining native-playtest requirements.
- Added `tools/release-check.sh` to run offline Release tests, ASan/UBSan tests, and WAV asset validation from one command.
- Consolidated final build and release instructions; no new gameplay features were added in this release-preparation installment.

## Part 14
 — Sanitizer and counter hardening

- Added `RR_ENABLE_SANITIZERS=ON` for GCC/Clang AddressSanitizer and UndefinedBehaviorSanitizer builds.
- Added `tools/validate-sanitized.sh` to run offline tests and WAV validation under ASan/UBSan.
- Saturated score, combo, sector, enemy/run counts, and lifetime-shard accumulation to avoid signed integer overflow at extreme or corrupted profile values.
- Added regression checks for run/score/lifetime-shard counters near `INT_MAX`.
- Bumped project version to 0.14.0.


## Part 13 — Accessibility and interface polish

- Added a persistent Screen Shake setting; players can disable camera shake and the impact border effect.
- Expanded Settings from four to five keyboard-navigable rows and tightened spacing for smaller supported windows.
- Kept existing version-1 profile files compatible when the new optional `screen_shake` key is absent.
- Added regression coverage for the setting toggle, fifth settings row, persistence round-trip, and legacy profile compatibility.
- Bumped project version to 0.13.0.


## Part 12 — Native dependency discovery

- Expanded `RR_USE_SYSTEM_RAYLIB=ON` to discover installed raylib through either known CMake target names or pkg-config's `raylib.pc`.
- Improved the failure message to explain the supported system-package and local-source alternatives.
- Updated build instructions and disclosed the exact native-build limitation: this environment cannot resolve `github.com`, so the real raylib build remains unverified.

## Part 11 — Controls and edge cases

- Escape no longer silently skips a sector upgrade reward.
- Corrected the arena control hint: Escape pauses during a run; from pause, Escape returns to title.
- Added regression checks for mandatory upgrade selection, Phase Dash protection, and player arena bounds.
- Bumped project version to 0.11.0.

## Part 10
- Added a truly offline `RR_TESTS_ONLY` mode that skips FetchContent and the playable-game target.
- Added opt-in system raylib package support and a local raylib source-tree override.
- Added `tools/validate.sh` for repeatable regression testing and WAV validation.
- Added a Python standard-library WAV validator for six bundled PCM audio files.
- Split CI into an offline/headless test job and a real native build job.
- Documented the distinction between headless tests and a playable native build.

## Part 09
- Finalize an active run on application shutdown and avoid duplicate shard counting.
- Expand logic regression coverage and improve diagnostics/release documentation.

## Part 08
- Harden profile validation and saving, improve HUD layout, and add release checklist.

## Parts 01–07
- See README.md for the feature history from the raylib foundation through persistent progression, visual effects, audio, and settings.


## 0.15.2 — Browser deployment setup

- Added an Emscripten-compatible browser main loop for the C/raylib game.
- Added a GitHub Actions workflow to compile WebAssembly and deploy a browser page to GitHub Pages.
- Added a responsive launch page with keyboard controls.
- This is deployment scaffolding pending a successful hosted build and real browser playtest; mobile touch controls and browser-persistent saves are not included.
