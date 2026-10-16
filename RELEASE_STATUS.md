# Release status — RIFT RUNNER: SIGNAL ZERO v0.23.0

## Included in this package

- `main.c` remains the short, commented game-process outline; system logic remains in separate modules.
- Red wraith and blue skitter enemies have distinct, animated, horror-leaning space-creature silhouettes.
- Five full procedural space environments rotate every 1,000 score with a quiet fade-and-scan transition, not a radial explosion.
- HUD now shows the named sector and progress to the next environment change.
- Pickups have distinct custom visuals: curved banana, cyan data chip, and green repair cross.
- Title, score, pause, and game-over screens have improved presentation and accessible mouse/touch navigation.
- Fixed touch aiming so right-side aiming remains independent while the player moves.
- Screen shake is deterministic, and background transition timing is updated outside of rendering.

## Verification status

- C99 syntax check across all game source files passed using a test-only raylib API stub.
- `tools/check_assets.py` passed: all six bundled WAV assets validated.
- A full CMake build could not finish because FetchContent stalled while trying to download the raylib dependency; this is not a passing native or browser build.
- A live gameplay session has not been verified in this environment.
- After pushing this version, confirm that both **Native build check** and **Build and publish browser game** pass in GitHub Actions, then test the deployed Pages game.
