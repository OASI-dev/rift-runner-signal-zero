# Release status — v0.19.0 candidate

The personal leaderboard now contains exactly three local records: one highest score for Easy, one for Medium, and one for Hard. The old unbounded run history, sorting, allocation, and scrolling code have been removed. All gameplay remains in root-level `main.c`.

Validation performed for this package:

- `main.c` passed strict C99 native-mode syntax validation against a test-only raylib API stub with warnings treated as errors.
- `main.c` passed GNU C99 Emscripten-conditional syntax validation against test-only raylib/Emscripten API stubs with warnings treated as errors.
- A focused save/load smoke test confirmed that Easy, Medium, and Hard each keep their own best score, lower runs do not replace a record, and the three values reload correctly.
- The source was checked to confirm the old dynamic-history fields/functions are removed.
- The seven bundled WAV assets remain present and are validated separately by `tools/check_assets.py`.

These are source-level checks, not a real raylib/Emscripten link or gameplay session. The native build, browser build/deployment, persistence across reload, and live gameplay still need to pass GitHub Actions and manual play-testing.
