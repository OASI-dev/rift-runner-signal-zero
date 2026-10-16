# Release status — RIFT RUNNER: SIGNAL ZERO v0.25.1

## Included in this package

- Easy enemy behavior is random roaming; Medium pursues the player; Hard combines pursuit with randomized steering.
- The banana restores up to 20 health and grants an 8-second shield.
- There is no music and no added background ambience. The original five one-shot effects remain: shot, impact, pickup, enemy-down and stage-surge.
- No combo counter, kill counter, floating score popups, repair kits or data pickups.
- The five procedural backgrounds, three personal-best scores, modular source and 127-line main flow remain.
- Added the missing standard-library declaration for `rand()` that the native compiler could reject, and configured compiler diagnostics to fail on future undeclared function calls. Linux CI and web workflows are pinned to Ubuntu 24.04, and the build uses GNU C99 extensions for compatibility.

## Verification status

- Desktop GNU C99 and browser GNU C99 syntax checks passed with implicit-function declarations treated as errors, using a test-only raylib API stub; these are not substitutes for linking the real raylib library.
- The bundled WAV asset validator passed with exactly five PCM WAV sound effects.
- The actual GitHub Actions native build has not been confirmed from this environment because fetching raylib from upstream timed out during the local CMake configure attempt. Push this version and check **Native build check** plus **Build and publish browser game** before considering it verified.
