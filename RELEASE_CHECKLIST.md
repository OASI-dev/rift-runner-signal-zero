# Release checklist — RIFT RUNNER: SIGNAL ZERO 0.17.0

## Code/package checks

- [x] All game C code is in the root-level `main.c`.
- [x] `main.c` passes a strict C99 syntax check against a test-only raylib API stub.
- [x] CMake configured, compiled, and linked `main.c` against a test-only raylib stub; the stub executable completed a short smoke run.
- [x] Seven bundled WAV files pass PCM validation.
- [x] Browser save-storage pre-run script remains included.
- [ ] Native CI build passes against the real raylib 6.0 source.
- [ ] Emscripten build and GitHub Pages deployment pass after pushing this version.

## Play-test after deploy

- [ ] Title screen difficulty cards respond to keyboard and mouse.
- [ ] Armored runner visibly faces movement and cursor aim.
- [ ] Blaster shots follow facing/cursor direction and sound effects play.
- [ ] Easy, Medium, and Hard have visibly different initial pressure.
- [ ] Rift Stage pressure increases at score milestones, not due only to elapsed time.
- [ ] HUD and game-over UI contain no survival-time counter.
- [ ] Personal score history is sorted high-to-low, browsable, and stores score/difficulty/stage/kills.
- [ ] Browser score history persists after reload in the same browser/device.
- [ ] Pause, dash, collision, death, retry, and sound toggle work.
- [ ] Touch controls are tested on target mobile browsers before claiming full mobile support.

## Release packaging

- [ ] Native CI build succeeds on real raylib.
- [ ] Browser compile and deploy jobs both succeed.
- [ ] Test the actual deployed browser game before sharing the updated build.
- [ ] Test Windows/Linux/macOS release packages before claiming all desktop releases are available.

The local mock-raylib smoke build only validates basic C compilation/linking against a stub API. It is not a playable game build and is not a substitute for the real raylib/Emscripten workflows.
