# Release checklist — RIFT RUNNER: SIGNAL ZERO v0.26.0

## Completed local checks

- [x] `main.c` is the commented 134-line entry point and main-process outline.
- [x] Gameplay systems are split into plainly named C source files and headers.
- [x] Difficulty settings and spawn-pressure changes are documented in `game.c`, `enemies.c`, and `main.c`.
- [x] Five original one-shot WAV files are present and validated by `tools/check_assets.py`.
- [x] Source and update ZIP archives pass ZIP integrity checks.
- [x] `START-HERE.txt`, README release notes, and this checklist identify v0.26.0 consistently.

## Required after pushing to GitHub

- [ ] Native CI build succeeds against real raylib 6.0.
- [ ] Emscripten build succeeds and GitHub Pages deployment completes.
- [ ] Open the deployed game URL and test the actual published browser build.
- [ ] Test Windows/Linux/macOS release packages before claiming all desktop builds are available.

## Manual gameplay smoke test

- [ ] Title screen difficulty cards respond to keyboard and mouse.
- [ ] Runner movement, aiming, automatic fire, collisions, pause, death and retry work.
- [ ] Red and blue enemy types appear; Easy roams, Medium pursues, and Hard combines pursuit with randomized steering.
- [ ] Easy, Medium, and Hard have visibly different starting pressure and later-stage difficulty.
- [ ] Stage pressure increases with score thresholds, not only elapsed time.
- [ ] No survival-time, kill, or combo counter is shown; no floating score popups appear.
- [ ] Leaderboard shows exactly three entries: best Easy, Medium, and Hard scores.
- [ ] No health kits, repair cells, or data pickups spawn.
- [ ] A new score replaces only the personal best for its own mode.
- [ ] Browser records persist after reload in the same browser/device.
- [ ] Banana is the only pickup and restores up to 20 health plus an eight-second shield.
- [ ] Only the original five one-shot sound effects play; no music or ambient loop plays.
- [ ] Touch controls are tested on target browsers before claiming mobile support.

**Verification boundary:** source review, asset validation, and ZIP integrity are local checks. Test-only syntax stubs do not replace a real raylib compile, a browser deployment, or hands-on play-testing.
