# Build and runtime diagnostics

## Use sanitizers to investigate crashes or undefined behavior

With GCC or Clang, run `tools/validate-sanitized.sh` for the offline tests under AddressSanitizer and UndefinedBehaviorSanitizer. To instrument a native build, configure with `-DRR_ENABLE_SANITIZERS=ON` after arranging a real raylib dependency. Sanitizers can help identify invalid memory access and undefined operations; they do not validate graphics, windowing, or sound output.

## CMake cannot fetch raylib

The default first configure downloads the pinned raylib 6.0 source archive from GitHub. If it fails with DNS, proxy, TLS or timeout errors, check whether GitHub is reachable and retry later. As alternatives, install raylib with either a CMake package target or a pkg-config `raylib.pc` file and configure using `-DRR_USE_SYSTEM_RAYLIB=ON`, or use a local source checkout with `-DRR_RAYLIB_SOURCE_DIR=/path/to/raylib`. When using pkg-config, run `pkg-config --modversion raylib` to check discovery before configuring.

To test project logic without any raylib download, configure with `-DRR_TESTS_ONLY=ON`. This skips the game executable and is not a playable substitute for real raylib. The `tools/validate.sh` helper runs that offline test path and checks the WAV assets.

If CMake repeatedly reuses a partial download, remove only the incomplete raylib download directory under `build/_deps/` or use a fresh build directory. Do not delete source files or the profile save unless you intend to reset progress.

## CMake reports missing compiler or system libraries

Install a C compiler and CMake 3.24+. On Linux, use the packages listed in [BUILDING.md](BUILDING.md), especially X11, OpenGL and ALSA development headers. On Windows/macOS, use a supported CMake generator and the desktop SDK/toolchain required by raylib.

## Invalid profile values

Profile counters are parsed with range-checked integer conversion. Values outside the `int` range, negative counters, and non-finite volume values are ignored or safely bounded instead of being trusted. If a profile is malformed, close the game, back up the file, and remove `rift_runner_profile.dat` only if you are willing to reset local progress.

## The executable opens but audio is silent

Confirm that `assets/` is beside the executable and contains `ambient_loop.wav`, `dash.wav`, `pickup.wav`, `impact.wav`, `enemy_down.wav`, and `surge.wav`. Open Settings with `S` from the title screen, raise music/effects volume, and ensure mute is off. The game tolerates an unavailable audio device, so silent audio does not necessarily prevent graphics from running.

## The profile does not appear to save

The profile is written to the process working directory as `rift_runner_profile.dat`; it is not necessarily beside the executable unless you launch the process from that directory. Ensure the directory is writable. Closing during an active run finalizes it, but a failed disk write still cannot guarantee that progress reaches disk.

## Automated checks vs. live validation

`ctest --test-dir build --output-on-failure` runs headless logic and profile tests. These don't validate GPU rendering, real audio output, fullscreen/window-manager behavior, or soundtrack quality. Use the outstanding items in [RELEASE_CHECKLIST.md](RELEASE_CHECKLIST.md) for the manual release pass.

## Visual motion preference

If camera movement is distracting, open Settings (`S`) and disable **SCREEN SHAKE**. Gameplay and hit detection are unchanged; only the camera shake and impact-edge effect are hidden.
