# Changelog

## v0.19.0 — Three-score leaderboard and simpler score code

- Replaced the unlimited run history with exactly three saved personal bests: Easy, Medium, and Hard.
- The leaderboard always shows one score per difficulty; a record updates only when a run beats that mode's previous best.
- Removed dynamic score-history allocation, sorting, scrolling, and old history fields.
- Removed unused player/enemy fields and labels that no longer matched the leaderboard.
- Preserved the player, firing, dash, red roaming enemies, pickups, effects, score milestones, audio, and neon presentation.
- `main.c` is now 1,149 physical lines, down from 1,273 in v0.18.0 (124 fewer lines, about 9.7%).

## v0.18.0 — Clearer, simpler enemy behavior

- Removed the pink and purple enemy variants; only large, bright red enemies remain.
- Red enemies wander in changing random directions and bounce off arena edges instead of steering toward the player.
- Made red enemies more visible and changed the player's blaster rounds to bright cyan.
- Removed the sound toggle and all mute UI; audio is always on after a player gesture starts the game, respecting browser autoplay rules.
- Simplified enemy spawning, update, and rendering by using one enemy type.

## v0.17.1 — Raylib API build fix

- Replace nonexistent `IsMusicReady` / `IsSoundReady` calls with raylib 6.0's `IsMusicValid` / `IsSoundValid` APIs.
- Pass a dummy variadic argument to the Emscripten `EM_ASM` persistence call for C99-compatible macro invocation.

## v0.17.0 — Single-file gameplay rebuild

- Consolidated the C gameplay implementation into root-level `main.c`.
- Added a visible armored runner whose facing follows movement, automatic blaster fire, and mouse aiming support.
- Added Easy, Medium, and Hard difficulty selection.
- Tied stage pressure to score milestones instead of elapsed time.
- Removed the survival-time counter and made score the primary objective.
- Added local score history.
- Added basic touch movement/aim support in the browser build.
