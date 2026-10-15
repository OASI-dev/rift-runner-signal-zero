# Changelog

## v0.17.0 — Single-file rebuild

- Rebuilt the gameplay implementation into a single `main.c`.
- Added a visible armored runner whose facing follows movement; blaster fire follows the facing direction, with mouse aiming support.
- Added Easy, Medium, and Hard difficulty selection.
- Difficulty pressure ramps with score milestones rather than an elapsed-time timer.
- Removed the survival-time display; score is the primary goal.
- Added local personal score history sorted from highest to lowest, including difficulty, stage, and kills.
- Preserved dash movement, particle effects, enemy variants, pickups, screen shake, audio cues, and ambient audio.
- Added basic touch movement/aim support in the browser build.
- Simplified CMake and CI/deploy workflows around one application C source file.
