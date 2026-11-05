# Contributing to RIFT RUNNER: SIGNAL ZERO

This repository uses a **single-file source layout**: the whole game is in `main.c`.

## Making changes

1. Read the relevant part of `main.c` before editing it. It is divided by labelled `PART` comments.
2. Keep gameplay behavior consistent with the version notes in `CHANGELOG.md`.
3. Keep the code simple: if/else, loops, arrays, functions, switch and small structs. Avoid pointer variables (the one exception, `FILE *f` for the score files, stays). Comment every line in plain English.
4. Do not create new project-owned `.c` or `.h` files unless you are intentionally changing this layout.
5. Keep the five one-shot sounds in `assets/`: `shot.wav`, `impact.wav`, `pickup.wav`, `enemy_down.wav` and `surge.wav`, and the six pictures in `assets/images/` (see `README.md` for the exact names). No music or ambient loops.
6. Run `python3 tools/check_assets.py` after changing audio assets.
7. Confirm both GitHub Actions workflows pass and test the deployed game before calling a release verified.

## Build

See `BUILDING.md`. CMake compiles `main.c` and links it with raylib 6.0 for desktop and web targets.
