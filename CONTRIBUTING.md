# Contributing

Thanks for helping with RIFT RUNNER: SIGNAL ZERO.

## Source layout

- Keep all gameplay, rendering, audio calls, and score handling in root-level `main.c` so the code stays approachable.
- Keep build/deployment instructions, browser hosting glue, workflows, and bundled WAV assets in their own files.
- Do not vendor raylib source into the repository; CMake fetches raylib 6.0 for builds.
- Avoid survival-time scoring UI. Rift Stage pressure must progress from score milestones.
- Describe the personal scoreboard as local-to-device; do not label it an online/global leaderboard.

## Local checks

```sh
./tools/validate.sh
```

This configures and builds the native target and checks the bundled WAV assets. The GitHub Actions workflows independently compile the native and WebAssembly targets. A successful source check is not a substitute for play-testing the actual browser build.
