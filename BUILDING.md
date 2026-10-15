# Building RIFT RUNNER: SIGNAL ZERO

This is the single merged repository for all 15 installments. The source ZIP does not contain a prebuilt game binary.

## Requirements

- CMake 3.24 or newer
- A C99-capable C compiler (GCC, Clang, or a current Visual Studio C toolchain)
- For the default game build: internet access on first configure to fetch raylib 6.0, unless using a local source tree or system package
- Native desktop development libraries needed by raylib on your operating system

The game code and generated WAV assets are released under the included MIT license. raylib is a separate dependency with its own license and notices; see the [raylib project](https://github.com/raysan5/raylib).

## Full game build (default)

Run from the project root:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
ctest --test-dir build --output-on-failure
```

CMake fetches raylib 6.0 on first configure. The executable is placed in `build/bin/` across single- and multi-configuration generators, with `assets/` copied alongside it. Launch from that directory so the relative asset paths behave predictably.

## Offline tests without raylib

You can run the profile and game-state logic tests without downloading raylib or opening a graphical window:

```sh
cmake -S . -B build-tests -DCMAKE_BUILD_TYPE=Release -DRR_TESTS_ONLY=ON -DRR_BUILD_TESTS=ON
cmake --build build-tests --config Release
ctest --test-dir build-tests --output-on-failure
python3 tools/check_assets.py
```

Or run `tools/validate.sh` (optional first argument: build directory). This validates regression tests and bundled WAV assets; it does **not** build a playable game or test real rendering/audio.

## Use an installed raylib package

If raylib is already installed, ask CMake to use it instead of downloading the source. CMake first looks for exported targets (`raylib`, `raylib::raylib`, `raylib_static`, or `raylib_shared`), then falls back to pkg-config (`raylib.pc`) when a CMake config is unavailable:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DRR_USE_SYSTEM_RAYLIB=ON
cmake --build build --config Release
```

Ensure the installed library and development headers are compatible with the raylib API used by the game; the default dependency remains pinned to 6.0. For pkg-config discovery, `pkg-config --modversion raylib` should print the installed version. The native build requires the actual raylib library; the test stub is only for headless logic checks.

## Use a local raylib source tree

If you have already downloaded or unpacked raylib source, point CMake at its root (the folder that contains raylib's `CMakeLists.txt`):

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DRR_RAYLIB_SOURCE_DIR=/path/to/raylib
cmake --build build --config Release
```

This avoids FetchContent downloading raylib; raylib itself may still need platform libraries during compilation.

## Run tests with runtime sanitizers (GCC/Clang)

This is optional and applies to debug/testing builds. Configure the offline tests with AddressSanitizer and UndefinedBehaviorSanitizer:

```sh
cmake -S . -B build-sanitized -DCMAKE_BUILD_TYPE=Debug \
  -DRR_TESTS_ONLY=ON -DRR_BUILD_TESTS=ON -DRR_ENABLE_SANITIZERS=ON
cmake --build build-sanitized --parallel
ctest --test-dir build-sanitized --output-on-failure
python3 tools/check_assets.py
```

Or run `tools/validate-sanitized.sh`. To instrument the game target too, omit `-DRR_TESTS_ONLY=ON` and configure with a real raylib dependency available. Sanitizer builds are supported with GCC/Clang/AppleClang; they are not required for normal Release builds.

## Linux notes

On Debian/Ubuntu-based desktops you may need compiler tools plus X11, OpenGL and ALSA development headers before configuring. Common package names include `build-essential`, `cmake`, `pkg-config`, `libasound2-dev`, `libx11-dev`, `libxrandr-dev`, `libxi-dev`, `libxcursor-dev`, `libxinerama-dev` and `libgl1-mesa-dev`. Packages differ by distribution.

## Windows and macOS notes

Use a supported CMake generator with a working C compiler and install the desktop SDK/toolchain required by raylib. Open a terminal in the project root and run the same CMake commands. On multi-configuration generators, `--config Release` selects the release configuration.

## Running and saving

Launch `rift_runner` from `build/bin/` so the executable and copied assets are together. The profile `rift_runner_profile.dat` is written to the process working directory; back it up before deleting it to reset progress.

If the window closes during an active run, the game finalizes that partial run and writes the profile before audio/window teardown. A failed disk write cannot be guaranteed to persist, so keep backups of important saves.

## What automated tests cover

The test targets use a small raylib API stub to exercise profile persistence and selected game-state/Core Surge progression. They do not test GPU rendering, real window creation, fullscreen/window-manager behavior, or audio output. See [DIAGNOSTICS.md](DIAGNOSTICS.md) and [RELEASE_CHECKLIST.md](RELEASE_CHECKLIST.md).

## GitHub releases

The repository includes `.github/workflows/release.yml`. After uploading the repository, the normal CI workflows should pass using the actual raylib dependency. Only after a successful native build and desktop playtest, push a version tag such as `v0.15.1` to build and publish platform ZIPs for Linux, Windows, and macOS. The workflow has not yet been executed in this environment, so check its status in GitHub Actions before sharing any artifacts.

## Reduced-motion option

Open **Settings** from the title screen with `S`, move to **SCREEN SHAKE**, and press Enter to toggle camera shake on or off. The preference is saved in `rift_runner_profile.dat`; older profile files keep the default enabled setting.


## Final offline release check

Run `tools/release-check.sh` to configure and test both an offline Release build and an ASan/UBSan build, then validate all bundled WAV assets. You may supply a build workspace path as its first argument. This intentionally does not fetch or validate the real raylib graphics/audio runtime; follow the native build instructions above for that final step.
