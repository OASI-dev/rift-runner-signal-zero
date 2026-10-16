# RIFT RUNNER: SIGNAL ZERO

**A score-first neon survival shooter written in C and raylib.** Start with [`main.c`](main.c) to follow the main game process. Player movement, enemies, backgrounds, colors, sound, score storage, and menus are implemented in separate, plainly named files so each part is easier to understand and change.

## Play in your browser

After the `Build and publish browser game` workflow passes and GitHub Pages finishes deploying, open:

**https://oasi-dev.github.io/rift-runner-signal-zero/**

The workflow publishes the WebAssembly build from this repository. If the most recent workflow run is red or deployment has not completed, the link may not yet contain the latest version.

## Gameplay

- **Character movement:** WASD or arrow keys. The armored runner faces the latest movement direction.
- **Firing:** automatic blaster fire toward the facing direction. Hold the left mouse button to aim with the cursor.
- **Choose difficulty:** Easy, Medium, or Hard from the title screen using 1/2/3 or left/right arrows.
- **Two alien silhouettes:** larger red wraiths have angular carapaces, glowing eyes, mandibles and moving tendrils; smaller blue skitters have swept fins and a triple-eye cluster. Blue skitters deal less contact damage. Easy enemies roam randomly, Medium enemies steadily pursue the player, and Hard enemies combine pursuit with randomized steering.
- **Score-driven stages and backgrounds:** Rift Stage pressure rises at 500-point milestones. The entire space environment changes every 1,000 score: ringed worlds, asteroid graveyards, dimensional rifts, a cratered blood moon, and a void cathedral. A named-sector HUD and progress bar show when the next environment shift is coming. Transitions use a soft fade-and-scan, not an exploding radial burst.
- **Score is the objective:** no kill counter, combo counter, floating point popups, or survival-time counter. Each alien grants a fixed score.
- **Three-score leaderboard:** press **B** to see exactly three personal records: the best Easy score, best Medium score, and best Hard score. A mode's record changes only when you beat that mode's previous score. Records are saved on this device/browser, not on a shared server.
- **Pause/resume:** P or Esc during a run. Resume or return to the flight deck with keyboard, mouse, or touch.
- **One pickup only:** the golden banana restores up to 20 health and grants an 8-second shield. No health kits, repair cells, or data chips spawn.
- **Sound:** no music and no background ambience. Only the original five one-shot effects play: shot, impact, pickup, enemy-down and stage-surge. Background changes are visual only.

### Touch screens

The browser build includes touch movement and aiming. Drag on the left side to move and touch on the right side to aim; the two controls work independently. Menus and pause navigation also support mouse/touch. Keyboard/mouse controls remain the primary desktop experience, and specific mobile browsers should still be tested.

## Release verification

A successful source package is not the same as a verified game build. After pushing updates, check both GitHub Actions workflows—**Native build check** and **Build and publish browser game**—then open the deployed Pages link and test movement, aiming, pause/resume, pickups, score saving, and the 1,000-point environment transition.

## Build a desktop version

Players should not need Visual Studio or CMake when you provide a prebuilt release. These are developer tools. Tagged GitHub releases are configured to produce ready-to-run ZIPs for Windows, Linux, and macOS when all release jobs pass.

For developers building from source, see [BUILDING.md](BUILDING.md).

## Project structure

```text
rift-runner-signal-zero/
├── main.c                 # Main game flow and frame loop (commented and readable)
├── game.c / game.h        # Game state, difficulty and shared declarations
├── player.c / player.h    # Player movement, aiming and character drawing
├── enemies.c / enemy.h    # Enemy spawning, movement, damage and rendering
├── bullets.c / bullets.h  # Blaster projectiles and hit detection
├── pickups.c / pickups.h  # Banana-only health and shield pickup
├── background.c / background.h # Distinct procedural space environments and fade transitions
├── colors.c / colors.h    # Shared palette
├── audio.c / audio.h      # Sound initialization and playback
├── leaderboard.c          # Three personal-best records
├── effects.c              # Lightweight hit particles and screen feedback
├── world.c                # Gameplay object draw order
├── ui.c                   # Menus, HUD and pause/game-over screens
├── helpers.c              # Shared math and drawing helpers
├── CMakeLists.txt         # raylib build and desktop/browser targets
├── assets/                # five bundled one-shot WAV sound effects
├── web/
│   ├── index.html         # public browser landing page
│   └── idbfs-pre.js       # browser-persistent local score storage
└── .github/workflows/     # native CI, browser deployment, desktop releases
```

## Score storage and privacy

The three personal bests are stored locally as three integers, one per difficulty. They are not sent to a server. Clearing browser data can clear browser records. A cross-player online leaderboard would require a backend service and abuse prevention, and is not claimed in this version.

## License

The project source and bundled original game assets are distributed under the repository's [LICENSE](LICENSE). raylib is a separate third-party dependency with its own license.


## v0.20.0 gameplay changes

- Removed Phase Dash completely.
- Easy enemies roam randomly; Medium enemies gently steer toward the runner; Hard enemies pursue more strongly.
- Added smaller blue enemies (lower health and contact damage) alongside red enemies.
- The full background environment changes every 1,000 score with a soft fade transition; there is no radial explosion effect.
- Bananas appear periodically; collecting one restores up to 20 health and grants an 8-second shield.

## Source code layout

The code is split by responsibility so the main process is easy to follow:

- `main.c` — application entry point, frame loop, and the commented order of gameplay systems.
- `game.c` — start/reset/end a run and difficulty settings.
- `game.h` — shared game state and declarations used between systems.
- `player.c` / `player.h` — player movement, aiming, shield display and character drawing.
- `enemies.c` / `enemy.h` — enemy spawning, movement/AI, damage, rewards and drawing.
- `bullets.c` / `bullets.h` — blaster shots and collision checks.
- `pickups.c` / `pickups.h` — the single banana health-and-shield pickup.
- `background.c` / `background.h` — five procedural space environments with fade transitions.
- `colors.c` / `colors.h` — shared color palette.
- `audio.c` / `audio.h` — audio initialization and sound playback.
- `leaderboard.c` — load/save the three personal-best scores.
- `effects.c` — hit particles and screen feedback.
- `world.c` — draws the active gameplay objects in layers.
- `ui.c` — menus, HUD, pause/game-over screens and overall drawing composition.
- `helpers.c` — shared math and drawing helpers.

Start with `main.c` to understand the game flow. It is **127 lines** in v0.25.2; open the named system file for the implementation of any one part.


## v0.25.2 — native linker collision fix and audio rollback

- Removed the added space ambience loop, distant whispers/rumble, and rift-change sound. There is still no music; the original five one-shot sound effects remain.
- Pinned Linux CI and browser workflow runners to Ubuntu 24.04 for a stable CMake/dependency environment.
- Enabled GNU C99 extensions consistently for raylib's native and browser targets while retaining C99 as the language standard.
- Renamed the game math helper `Normalize` to `RRNormalizeVector2` to prevent a duplicate-symbol error against raylib internals when linking the desktop build.
