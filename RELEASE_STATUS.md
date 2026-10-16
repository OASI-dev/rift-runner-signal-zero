# Release status — RIFT RUNNER: SIGNAL ZERO v0.24.0

## Included in this package

- Removed combo counters, combo score bonuses, kill counters, and floating +points popups.
- The golden banana is the only pickup; it restores up to 18 health and grants a 5-second shield.
- Removed repair/health-kit and data-chip spawning and drawing.
- Enemy defeats award a fixed 200 score. The HUD keeps the main score, rift level, sector name/progress, health, and active shield timer.
- Existing modular architecture, alien designs, five procedural environments, and 1,000-score soft sector transitions are retained.
- The three personal-best records (Easy, Medium, Hard) remain unchanged.

## Verification status

- Source-level checks and bundled asset checks should be run after applying this update.
- A passing GitHub Actions browser workflow and a live browser play test are required to verify the deployed release.
