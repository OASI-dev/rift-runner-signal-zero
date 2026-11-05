# Building RIFT RUNNER: SIGNAL ZERO — picture edition

The whole game is **one file**: `main.c`. CMake builds it together with the raylib library. The 5 sound effects stay in `assets/`; the 10 pictures (player, 2 enemy types, bullet, banana, 5 backgrounds) go in a new folder, `assets/images/` -- see the exact file names in `README.md`. If a picture file is missing, that one thing just does not appear; the game still runs. The browser page and score-persistence helper remain in `web/`.

## Browser release

The `.github/workflows/deploy-web.yml` workflow builds with Emscripten and deploys to GitHub Pages. To publish, push the repository to `main`, set **Settings → Pages → Source** to **GitHub Actions**, then open **Actions → Build and publish browser game**. Wait for both the build and deploy jobs to pass and open the Pages URL from the summary.

The web workflow uses raylib 6.0 and Emscripten 3.1.64. The CMake configuration enables GNU C99 extensions for the web target because Emscripten's `EM_ASM` macros require GNU C mode.

## Windows desktop build (developers)

Install Visual Studio Community with **Desktop development with C++**, CMake tools and a Windows SDK. Run these commands in Developer PowerShell from the project root:

```powershell
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
.\build\bin\rift_runner.exe
```

The first configure downloads raylib 6.0, so it needs an internet connection. The build copies `assets/` next to the executable.

## Linux desktop build

Install CMake, a C compiler, pkg-config, OpenGL/Mesa, ALSA and X11 development libraries. Run:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
./build/bin/rift_runner
```

## macOS desktop build

Install Xcode command-line tools and CMake, then run:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
./build/bin/rift_runner
```

## Score storage

The three personal-best scores (Easy, Medium, Hard) are stored locally, one number per line. Desktop uses `rift_runner_scores.txt`; the web build uses Emscripten IDBFS at `/persist/rift_runner_scores.txt`, synced to browser storage. Scores are not submitted to a shared server. Records saved by older versions are not read.

## Verification

Run `tools/check_assets.py` to validate the five WAV files. A real build still requires raylib and the operating-system dependencies. After pushing a change, confirm both the native and browser GitHub Actions workflows pass and test the deployed game.
