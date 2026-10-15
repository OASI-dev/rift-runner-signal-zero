# RIFT RUNNER: SIGNAL ZERO — Release Status (0.15.2)

## Current status

This archive is the consolidated source release candidate (all 15 installments merged into one repository). It includes the C99 game source, generated WAV assets, profile/game-state regression tests, build scripts, and project documentation.

**It is not a verified playable binary release yet.** The current build environment could not resolve `github.com`, so the pinned raylib 6.0 source archive could not be downloaded. The application target has previously been compiled and linked with strict warnings against a local test-only raylib stub. A final source audit also hardened oversized integer parsing in corrupt profile files and sanitized frame-time values in the renderer. That checks project/CMake wiring, but does not verify a real graphics context, real audio device, or live gameplay.

## Browser deployment setup (not yet live-verified)

- Added the Emscripten browser event loop, web CMake linker options, asset preloading, and a responsive GitHub Pages launcher page.
- Added `.github/workflows/deploy-web.yml` to compile to WebAssembly in GitHub Actions and publish the static site.
- The workflow has not run from this environment; a successful GitHub Actions run and in-browser playtest are still required before sharing the public play link as working.
- The current browser shell is intended for keyboard play; touch controls and browser-persistent save storage have not been implemented.

## Verified in this environment

- Offline Release configuration/build works without FetchContent.
- Both CTest test cases pass in Release mode.
- Both CTest test cases pass with AddressSanitizer and UndefinedBehaviorSanitizer.
- Application and test target compilation/linking has been checked using a test-only raylib stub with strict warnings.
- The six bundled WAV files pass the project's RIFF/WAVE + PCM validator.
- Oversized profile integer tokens are rejected safely, with regression coverage.
- Profile persistence includes invalid-input handling, backward-compatible settings defaults, range-checked integer parsing, and exactly-once run finalization.
- Final audit fixed frame-time sanitization in the renderer and standardized `build/bin/` output across CMake generator types.
- A GitHub tag-triggered workflow is included to attempt actual Linux, Windows, and macOS builds and package outputs; it has not run from this environment.
- ZIP archive integrity passes.

## Must be completed before calling it playable

1. Build in a clean directory against the real raylib 6.0 library (fetched, installed or supplied as a local source tree).
2. Run CTest against that native build.
3. Launch on a desktop and test title, settings, screen-shake toggle, movement, dash, shards, upgrades, Core Surge, pause, game over, retry, fullscreen, resize, and quit.
4. Listen to ambient music and each effect; ensure the `assets/` folder is beside the executable.
5. Confirm settings, high score, best sector and lifetime shards persist after restarting.
6. Confirm packaging/licensing and test at least one additional OS if claiming cross-platform support.

## Native build

Default (fetches raylib 6.0 during configure):

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
ctest --test-dir build --output-on-failure
```

Installed dependency:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DRR_USE_SYSTEM_RAYLIB=ON
cmake --build build --config Release
ctest --test-dir build --output-on-failure
```

Local raylib checkout:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DRR_RAYLIB_SOURCE_DIR=/path/to/raylib
cmake --build build --config Release
ctest --test-dir build --output-on-failure
```

A tag-triggered GitHub Actions workflow is included to build and package Linux, Windows, and macOS artifacts using real raylib; it has not run yet. See [BUILDING.md](BUILDING.md), [DIAGNOSTICS.md](DIAGNOSTICS.md), and [RELEASE_CHECKLIST.md](RELEASE_CHECKLIST.md).
