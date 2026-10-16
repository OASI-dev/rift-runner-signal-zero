# Building RIFT RUNNER: SIGNAL ZERO

The game is organized into small C modules. Start with `main.c` for the commented frame loop and game-process outline. CMake compiles the modules together with raylib into a native executable or a browser/WebAssembly build.

## Browser release (recommended for sharing a link)

The repository's `.github/workflows/deploy-web.yml` builds with Emscripten on GitHub's runners and deploys to GitHub Pages. You do not need to install the Emscripten SDK, Visual Studio, or CMake locally just to publish the browser build:

1. Push the repository's files to the `main` branch.
2. In repository **Settings → Pages**, set the source to **GitHub Actions**.
3. Open **Actions → Build and publish browser game**.
4. Wait for the workflow's build and deploy jobs to both pass.
5. Open the Pages URL shown in the deployment summary.

The workflow uses raylib 6.0 and Emscripten 3.1.64. For raylib's Web backend, the CMake target enables GNU C99 extensions because Emscripten's `EM_ASM` macros are rejected in strict `-std=c99` mode.

## Windows desktop build (developers only)

Install Visual Studio Community with **Desktop development with C++**, including CMake tools and a Windows SDK. Then use a Developer PowerShell in the project root:

```powershell
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
.\build\bin\rift_runner.exe
```

The configure step fetches raylib 6.0 and therefore requires an internet connection the first time. The build copies `assets/` beside the executable.

## Linux desktop build

Install CMake, a C compiler, pkg-config, and the raylib desktop system dependencies (X11, OpenGL/Mesa, ALSA and related X11 extensions), then run:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
./build/bin/rift_runner
```

## macOS desktop build

With Xcode command-line tools and CMake installed:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
./build/bin/rift_runner
```

## Desktop release ZIPs

Push a version tag that begins with `v`, for example `v0.17.1`, only after the native build and browser deployment are passing. The `Tagged desktop release` workflow is configured to build Windows, Linux, and macOS ZIPs and publish them as a GitHub Release. Do not create a release tag until the current build has been verified.

## Browser score persistence

The browser pre-run script mounts `/persist` using Emscripten IDBFS, then syncs the filesystem to IndexedDB. The C game writes exactly three personal-best scores to `/persist/rift_runner_bests.txt` in the browser and `rift_runner_bests.txt` on desktop. Records remain local to the browser/device.


## CI reproducibility

The project workflows pin Linux jobs to `ubuntu-24.04` instead of relying on the moving `ubuntu-latest` label. CMake keeps C99 as the standard and enables GNU extensions for both targets to help raylib/GLFW compile consistently. A local source syntax check is not a substitute for the GitHub native build against the actual raylib dependency.
