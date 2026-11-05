# RIFT RUNNER: SIGNAL ZERO — picture edition

**A score-first neon survival shooter written in C and raylib.** The whole game is **one readable C file**: `main.c`. The player, the two enemy types, the bullet, the banana and the 5 backgrounds are real pictures, loaded from a folder called `assets/images/` (see **Pictures needed**, below).

## Play in your browser

After the `Build and publish browser game` workflow passes and GitHub Pages finishes deploying, open:

**https://oasi-dev.github.io/rift-runner-signal-zero/**

If the latest workflow run is red or deployment has not completed, the link may not yet contain this edition.

## Modes

- **Easy / Medium / Hard** — you against red Wraiths and blue Skitters. Easy enemies roam randomly, Medium enemies chase you, Hard enemies chase with a random wobble and extra spawns.

## Pictures needed

Create a folder `assets/images/` next to `main.c` and put these 10 files in it, using these exact names:

| File | What it's for |
|---|---|
| `player.png` | the player |
| `red_enemy.png` | the red "Wraith" enemy |
| `blue_enemy.png` | the blue "Skitter" enemy |
| `bullet.png` | the bullet |
| `banana.png` | the banana pickup |
| `bg_cradle.png` | background 1: The Cradle |
| `bg_graveyard.png` | background 2: Graveyard Belt |
| `bg_rift.png` | background 3: The Rift |
| `bg_bloodmoon.png` | background 4: Blood Moon |
| `bg_voidcathedral.png` | background 5: Void Cathedral |

If a file is missing, that one thing is simply invisible — the game still runs.

## Gameplay at a glance

- **Enemies:** red Wraiths are big and tough, blue Skitters are small and fast. Every kill is 200 points.
- **Levels:** a new level every 500 points makes enemies faster and more numerous.
- **Banana pickup:** restores up to 20 health and gives an 8-second shield.
- **Backgrounds:** five space scenes; a new one every 1,000 points.
- **Personal bests:** one local best per difficulty (Easy, Medium, Hard), kept on the same device/browser.
- **Audio:** five one-shot WAV effects. No music or ambience.

## Controls

| Action | Keys |
|---|---|
| Choose a mode | 1 / 2 / 3, left/right arrows, A / D, or click a card |
| Start | Enter, Space or the PLAY button |
| Move (single-player) | WASD or arrow keys; touch: drag on the left side |
| Aim (single-player) | Hold the mouse button; touch: right side. The gun fires by itself |
| Pause (single-player) | P or Escape |
| Personal bests | B on the title screen |

## Code map

`main.c` (one file): settings and colors; structs and variables, including the 10 picture variables; helper functions, including `draw_sprite` which draws every picture; loading pictures/sound/scores; sparks; input; player and bullet rules; enemy/banana/run rules; screen keys; drawing helpers and buttons; drawing the pictures and the arena; drawing the screens; the main loop.

See [CODE-WALKTHROUGH.md](CODE-WALKTHROUGH.md) for a plain-English tour.

## Build

CMake downloads raylib 6.0 on the first configure, so an internet connection is needed once. Follow [BUILDING.md](BUILDING.md). CMake compiles the one file, `main.c`.

## Verification boundary

The game logic was compiled and run headlessly against a stand-in for raylib. A real native/web build, the web deployment and hands-on play-testing still have to be confirmed on GitHub Actions and in the browser.

## License

The project source and bundled game assets are distributed under `LICENSE`. raylib is a separate third-party dependency with its own license.
