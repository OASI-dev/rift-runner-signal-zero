# Release checklist — RIFT RUNNER: SIGNAL ZERO v0.28.0 picture edition

## Local package checks

- [x] The gameplay source is a single file: `main.c`. No `others.c`.
- [x] CMake compiles `main.c` with raylib.
- [x] `main.c` compiles warning-free with the CMake warning flags against a raylib stand-in header that supports textures.
- [x] Headless logic tests pass for Easy, Medium and Hard, with no real picture files present (see `LOCAL-VERIFICATION.txt`).
- [x] Build, web, workflow, license and documentation files are preserved.

## Required after push

- [ ] Add the 10 picture files to `assets/images/` with the exact names in `README.md`.
- [ ] Native build check succeeds against raylib 6.0.
- [ ] Emscripten build succeeds, the pictures load in the browser, and GitHub Pages deployment completes.
- [ ] Test in the live browser game: title screen and the 3 mode cards, movement, firing, collisions, pause, banana, scores saving and loading, the 1,000-point background change -- and that every picture actually shows up correctly sized and facing the right way.
- [ ] Test Windows/Linux/macOS packages before claiming every desktop package is available.

**Verification boundary:** local checks do not prove a real raylib compile, deployment, sound, or how the pictures actually look once in place.
