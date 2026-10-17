# Release status — RIFT RUNNER: SIGNAL ZERO v0.26.0 single-file edition

- All game code is combined in `main.c`; the old module .c and custom .h files are removed.
- CMake builds only `main.c` together with raylib 6.0.
- Easy, Medium and Hard retain the v0.26.0 rebalance and extra spawn pressure logic.
- Banana restores up to 20 health and grants an 8-second shield.
- The five original one-shot effects remain; no music or ambient loops.
- ZIP/source checks do not replace native compilation, web compilation/deployment, or manual gameplay validation. Confirm both GitHub Actions workflows after pushing.
