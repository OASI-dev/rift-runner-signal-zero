# Release checklist — RIFT RUNNER: SIGNAL ZERO v0.26.0 single-file edition

## Local package checks

- [x] The complete gameplay source is combined in `main.c`.
- [x] CMake compiles only `main.c` with raylib.
- [x] Five original one-shot WAV assets are included and checked by `tools/check_assets.py`.
- [x] Build, web, workflow, license and documentation files are preserved.
- [x] The old modular .c/.h files are excluded from the single-file source package.

## Required after push

- [ ] Native build check succeeds against raylib 6.0.
- [ ] Emscripten build succeeds and GitHub Pages deployment completes.
- [ ] Test title/menu, movement, firing, collisions, pause, pickups, scores and 1,000-point environment transition in the live browser game.
- [ ] Test Windows/Linux/macOS packages before claiming every desktop package is available.

**Verification boundary:** ZIP integrity and source review are local checks. They do not prove a full compile, deployment or hands-on gameplay test.
