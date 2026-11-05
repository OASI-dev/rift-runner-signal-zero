# Release status — RIFT RUNNER: SIGNAL ZERO v0.28.0 picture edition

- The whole game is one file, `main.c`; no `others.c` anymore; CMake builds only `main.c` with raylib 6.0.
- Modes: Easy, Medium, Hard (same difficulty numbers as before). The 2-player duel has been removed.
- The player, both enemy types, the bullet, the banana and the 5 backgrounds are real pictures loaded from `assets/images/`, drawn with one shared function.
- Banana still restores up to 20 health and grants an 8-second shield; the five sound effects, personal bests, pause and touch controls are unchanged.
- Local logic tests passed, including with no real picture files present (the game stays safe, just shows nothing for a missing picture). They do not replace a real native/web build, deployment, or manual gameplay/appearance check. Confirm both GitHub Actions workflows after pushing, and add the picture files.
