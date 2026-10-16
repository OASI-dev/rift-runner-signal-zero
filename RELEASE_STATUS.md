# Release status — RIFT RUNNER: SIGNAL ZERO v0.25.0

## Included in this package

- Removed combo counters, combo score bonuses, kill counters, and floating +points popups.
- The golden banana is the only pickup; it restores up to 20 health and grants an 8-second shield.
- Removed repair/health-kit and data-chip spawning and drawing.
- Enemy defeats award a fixed 200 score. The HUD keeps the main score, rift level, sector name/progress, health, and active shield timer.
- Easy enemies roam randomly; Medium enemies pursue steadily; Hard enemies combine pursuit and randomized steering.
- Music has been removed. Added a non-musical ambience bed, rare void whispers/deep rumbles, and a rift-transition sound.
- The five procedural environments, three personal-best records, and modular architecture remain.

## Verification status

- Desktop C99 and browser GNU C99 syntax checks passed using a test-only raylib API stub.
- `tools/check_assets.py` passed for all nine bundled PCM WAV files; the old music asset was removed.
- A passing GitHub Actions browser workflow and a live browser play test are required to verify the deployed release.
