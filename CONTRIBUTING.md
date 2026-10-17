# Contributing to RIFT RUNNER: SIGNAL ZERO

This repository uses the **single-file source edition**: all game code lives in `main.c`. The labelled `SECTION:` comments identify where code from the earlier modular files was merged.

## Making changes

1. Read the relevant section in `main.c` before editing it.
2. Keep gameplay behavior consistent with the version notes in `CHANGELOG.md`.
3. Do not add new project-owned `.c` or `.h` files unless you are intentionally changing the project away from the single-file edition. The standard library and raylib headers are still used.
4. Keep original one-shot sounds in `assets/`: `shot.wav`, `impact.wav`, `pickup.wav`, `enemy_down.wav` and `surge.wav`. No music or ambient loops are part of v0.26.0.
5. Run `python3 tools/check_assets.py` after changing audio assets.
6. Confirm both GitHub Actions workflows pass and test the deployed game before calling a release verified.

## Build

See `BUILDING.md`. CMake compiles only `main.c` and links it with raylib 6.0 for desktop and web targets.
