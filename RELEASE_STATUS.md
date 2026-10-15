# Release status — v0.17.0 single-file rebuild

This is a rebuilt candidate source package. It is prepared for GitHub Actions to compile a native Linux target and an Emscripten browser target. The local environment cannot fetch/build raylib 6.0 or Emscripten, so the actual game binary, browser gameplay, and the new deployment have **not** been independently verified here.

Before announcing this version as live:

1. Push the update to the GitHub repository.
2. Confirm **Native build check** passes.
3. Confirm **Build and publish browser game** passes both compile and deploy jobs.
4. Open the published page and verify title screen, mouse/keyboard controls, sound, difficulty, score progression, and score persistence after refresh.
5. Test touch input separately on a real mobile browser before promising complete mobile support.

The browser scoreboard is local to the browser profile. It is not a shared online leaderboard.
