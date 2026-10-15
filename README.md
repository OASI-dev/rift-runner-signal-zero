# RIFT RUNNER: SIGNAL ZERO

**A score-first neon survival shooter written in C and raylib.** The gameplay code lives in one file: [`main.c`](main.c). The CMake file, browser launcher, workflows, and audio assets stay separate because they build, package, and publish the game rather than implement its gameplay.

## Play in your browser

After the `Build and publish browser game` workflow passes and GitHub Pages finishes deploying, open:

**https://oasi-dev.github.io/rift-runner-signal-zero/**

The workflow publishes the WebAssembly build from this repository. If the most recent workflow run is red or deployment has not completed, the link may not yet contain the latest version.

## Gameplay

- **Character movement:** WASD or arrow keys. The armored runner faces the latest movement direction.
- **Firing:** automatic blaster fire toward the facing direction. Hold the left mouse button to aim with the cursor.
- **Phase dash:** Space or either Shift key; briefly grants invulnerability.
- **Choose difficulty:** Easy, Medium, or Hard from the title screen using 1/2/3 or left/right arrows.
- **Red enemies only:** the hard-to-see small/purple/pink enemy types are removed. Red enemies are larger and brighter; they roam in changing random directions and bounce around the arena instead of homing in on the player.
- **Score-driven stages:** the next Rift Stage starts at each 500-point milestone. Spawn pressure and enemy speed increase based on score stage, not simply because time elapsed.
- **Score is the objective:** no survival-time counter is displayed. Kills, combos, and pickups grant points.
- **Three-score leaderboard:** press **B** to see exactly three personal records: the best Easy score, best Medium score, and best Hard score. A mode's record changes only when you beat that mode's previous score. Records are saved on this device/browser, not on a shared server.
- **Pause/resume:** P or Esc during a run, then P/Enter/Esc to resume.
- **Sound:** all music and effects are always on after you start a run. There is no sound toggle.

### Touch screens

The browser build includes basic touch movement and aiming. Use the left side to move, the right side to aim, and the lower-right dash area to phase dash. Keyboard/mouse controls on desktop are the primary supported experience; test specific mobile browsers before advertising universal device support.

## Build a desktop version

Players should not need Visual Studio or CMake when you provide a prebuilt release. These are developer tools. Tagged GitHub releases are configured to produce ready-to-run ZIPs for Windows, Linux, and macOS when all release jobs pass.

For developers building from source, see [BUILDING.md](BUILDING.md).

## Project structure

```text
rift-runner-signal-zero/
├── main.c                 # Entire C game: gameplay, drawing, audio, scores, menus
├── CMakeLists.txt         # raylib build and desktop/browser targets
├── assets/                # bundled WAV sound effects and ambient loop
├── web/
│   ├── index.html         # public browser landing page
│   └── idbfs-pre.js       # browser-persistent local score storage
└── .github/workflows/     # native CI, browser deployment, desktop releases
```

## Score storage and privacy

The three personal bests are stored locally as three integers, one per difficulty. They are not sent to a server. Clearing browser data can clear browser records. A cross-player online leaderboard would require a backend service and abuse prevention, and is not claimed in this version.

## License

The project source and bundled original game assets are distributed under the repository's [LICENSE](LICENSE). raylib is a separate third-party dependency with its own license.
