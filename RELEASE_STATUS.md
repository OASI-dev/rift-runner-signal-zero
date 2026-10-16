# Release status — RIFT RUNNER: SIGNAL ZERO v0.21.0

## Current source layout
- `main.c` owns the application entry point, frame scheduling, and commented main gameplay process.
- Detailed implementation is split into named modules: player, enemies, bullets, pickups, background, colors, audio, leaderboard, effects, world rendering, UI and common helpers.
- The three local records remain Easy, Medium and Hard best scores.

## Local checks
- C source syntax was checked with GCC in C99 mode against a temporary test-only raylib API stub. This checks C syntax and cross-file declarations only; it is not a replacement for building against real raylib.
- The native raylib build, Emscripten build, GitHub Pages deploy, and actual gameplay still need verification after pushing this update.

## Browser URL after successful deployment
https://oasi-dev.github.io/rift-runner-signal-zero/
