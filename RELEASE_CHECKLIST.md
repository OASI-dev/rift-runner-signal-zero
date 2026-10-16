# Release checklist — RIFT RUNNER: SIGNAL ZERO 0.25.0

## Code/package checks

- [x] `main.c` is the commented entry point and main-process outline.
- [x] Game systems are split into plainly named C files and headers.
- [x] All game source files pass a local C99 syntax check against a test-only raylib API stub.
- [x] Nine bundled WAV files are present and validated by `tools/check_assets.py`.
- [ ] Native CI build passes against real raylib 6.0.
- [ ] Emscripten build and GitHub Pages deployment pass after pushing this version.

## Play-test after deploy

- [ ] Title screen difficulty cards respond to keyboard and mouse.
- [ ] Runner faces its movement/aim direction and auto-fire works.
- [ ] Red and blue enemy types appear; Easy roams randomly, Medium pursues steadily, and Hard combines pursuit with randomized steering.
- [ ] Easy, Medium, and Hard have visibly different initial pressure.
- [ ] Stage pressure increases at score milestones, not only due to elapsed time.
- [ ] No survival-time, kill, or combo counter is shown; no floating score popups appear.
- [ ] Leaderboard shows exactly three entries: best Easy, Medium, and Hard scores.
- [ ] No health kits, repair cells, or data chips spawn.
- [ ] A new score replaces only its own mode's personal best.
- [ ] Browser records persist after reload in the same browser/device.
- [ ] Pause, collision, death, and retry work; no music plays; space ambience and environmental sound effects play. The banana is the only pickup and restores up to 20 health plus an eight-second shield.
- [ ] Touch controls are checked on target browsers before claiming full mobile support.

## Release

- [ ] Native CI build succeeds on real raylib.
- [ ] Browser compile and deployment both succeed.
- [ ] Test the actual deployed browser game before sharing this version.
- [ ] Test Windows/Linux/macOS release packages before claiming all desktop builds are available.

Test-only stubs verify C API usage and syntax, not a playable game build.
