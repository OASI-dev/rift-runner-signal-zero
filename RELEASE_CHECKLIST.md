# Release checklist — RIFT RUNNER: SIGNAL ZERO 0.19.0

## Code/package checks

- [x] All game C code is in root-level `main.c`.
- [x] `main.c` passes strict C99 native-mode syntax checks against a test-only raylib stub.
- [x] `main.c` passes GNU C99 browser-conditional syntax checks against test-only raylib/Emscripten stubs.
- [x] Seven bundled WAV files are present and validated by `tools/check_assets.py`.
- [ ] Native CI build passes against real raylib 6.0.
- [ ] Emscripten build and GitHub Pages deployment pass after pushing this version.

## Play-test after deploy

- [ ] Title screen difficulty cards respond to keyboard and mouse.
- [ ] Runner faces its movement/aim direction and auto-fire works.
- [ ] Only bright red enemies remain; they roam and bounce rather than homing.
- [ ] Easy, Medium, and Hard have visibly different initial pressure.
- [ ] Stage pressure increases at score milestones, not only due to elapsed time.
- [ ] No survival-time counter is shown.
- [ ] Leaderboard shows exactly three entries: best Easy, Medium, and Hard scores.
- [ ] A new score replaces only its own mode's personal best.
- [ ] Browser records persist after reload in the same browser/device.
- [ ] Pause, dash, collision, death, and retry work; sound stays enabled without a mute toggle.
- [ ] Touch controls are checked on target browsers before claiming full mobile support.

## Release

- [ ] Native CI build succeeds on real raylib.
- [ ] Browser compile and deployment both succeed.
- [ ] Test the actual deployed browser game before sharing this version.
- [ ] Test Windows/Linux/macOS release packages before claiming all desktop builds are available.

Test-only stubs verify C API usage and syntax, not a playable game build.
