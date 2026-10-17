# RIFT RUNNER: SIGNAL ZERO — single-file edition

**A score-first neon survival shooter written in C and raylib.** In this edition, all game code has been combined into **one `main.c` file** so you can read the whole game in one place. There are no separate game-module `.c` or custom `.h` source files.

## Play in your browser

After the `Build and publish browser game` workflow passes and GitHub Pages finishes deploying, open:

**https://oasi-dev.github.io/rift-runner-signal-zero/**

The workflow publishes the WebAssembly build from this repository. If the latest workflow run is red or deployment has not completed, the link may not yet contain this edition.

## Gameplay at a glance

- **Movement:** WASD or arrow keys. Touch users can drag on the left side of the browser canvas to move.
- **Aiming and firing:** hold the left mouse button to aim/fire toward the cursor; touch users aim on the right side.
- **Difficulty:** Easy, Medium, or Hard. Easy enemies roam randomly; Medium enemies pursue; Hard enemies combine pursuit and randomized steering with extra pressure spawns.
- **Enemies:** red Wraiths and blue Skitters, with different size, durability and contact damage.
- **Banana pickup:** restores up to 20 health and grants an 8-second shield. Health never exceeds maximum.
- **Progression:** score raises stage pressure. The space environment changes every 1,000 points.
- **Personal bests:** one local best score for each of Easy, Medium and Hard. Records stay on the same device/browser.
- **Audio:** exactly five original one-shot WAV effects—shot, impact, pickup, enemy defeated and stage surge. No music or ambience.

## Controls

| Action | Keyboard / mouse |
|---|---|
| Move | WASD or arrow keys |
| Aim / fire | Hold left mouse button toward the cursor |
| Pause / resume | P or Escape |
| Select difficulty | 1 / 2 / 3 or left/right arrows on the title screen |
| Personal-best screen | B |
| Start / choose | Enter or Space; use mouse/touch on menu buttons |

## Single-file code map

Open `main.c`. The file is arranged in labelled sections to make a long file easier to navigate:

- **Shared declarations:** game structs, difficulty settings, and function declarations that previously lived in headers.
- **Math and color helpers:** reusable vector, distance, clamp, opacity and drawing helpers.
- **Backgrounds:** procedural space scenes and transitions.
- **Player:** movement, aiming, firing cadence, shield and runner drawing.
- **Enemies:** spawn rules, difficulty AI, contact damage, hit points, rewards and enemy drawing.
- **Bullets / pickups / effects:** projectiles, banana behavior, particles and screen feedback.
- **World rendering:** draws the action and objects in layers.
- **Audio / leaderboard:** five sound effects and local per-difficulty records.
- **Game lifecycle:** initialize, reset, start and end runs; choose difficulty.
- **UI:** title menu, HUD, pause, game over, leaderboard, touch controls.
- **Main process:** `UpdatePlaying`, `GameFrame`, and `main`—the program entry point.

Search for `SECTION:` in `main.c` to jump between the original modules now embedded in the same file.

## Build a desktop version

CMake downloads raylib 6.0 on the first configure, so an internet connection is needed the first time. Follow [BUILDING.md](BUILDING.md). The code itself is one `main.c`; CMake and raylib are still used to build the application.

## Build and publish the browser version

Push the repository to GitHub and check **Actions → Build and publish browser game**. The workflow uses Emscripten and deploys the resulting game to GitHub Pages. In repository **Settings → Pages**, select **GitHub Actions** as the source.

## Verification boundary

Source packaging and audio checks can be run locally; a successful ZIP check does not prove a full compile or gameplay test. Confirm the native and browser GitHub Actions workflows pass, then test the live game.

## License

The project source and bundled game assets are distributed under `LICENSE`. raylib is a separate third-party dependency with its own license.
