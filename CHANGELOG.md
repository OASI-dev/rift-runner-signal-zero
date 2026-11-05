# v0.28.3 — simpler teacher edition

- Reduced `main.c` from 974 to 873 lines while retaining the main game features.
- Replaced repeated difficulty-name and description functions with lookup arrays and one small color function.
- Loaded the five sound effects with one short loop rather than five separate statements.
- Simplified the enemy edge selection with a `switch` statement.
- Moved very small one-use input, drawing and screen-update helpers next to the code that uses them.
- Kept the supplied space image as `bg_cradle.png`; the other four backgrounds and their rotation system remain removed.
- Updated the code walkthrough and presentation guide to match the current source.

---

# Changelog

## v0.28.2 — simplified: no more 2D arrays or 2D strings

- Removed every 2D array from `main.c`: `mode_name[3][10]`, `mode_note[3][24]`, `sector_name[5][16]`, `sound_file[5][30]`, and the background filename list in `load_pictures()`.
- Replaced them with plain `if`/`else` functions (`get_mode_name`, `get_mode_note`, `get_mode_color`, `get_sector_name`) and, for loading sounds and background pictures, explicit one-line-per-file calls instead of a loop over a filename list. Easier to read top to bottom, nothing indexed by two numbers.

## v0.28.1 — background fill bug fix

- Fixed `draw_background()`: the background picture was only covering the bottom-right quarter of the window (visible as a half-size image with black space around it) because of a position/center mix-up in the one-line call to `draw_sprite(...)`. It now fills the whole screen.

## v0.28.0 — picture edition, single file, duel removed

- Merged `others.c` into `main.c`. The whole game is now one file; there is no others.c anymore.
- The player, the two enemy types, the bullet, the banana and the 5 backgrounds are now real picture files, loaded from `assets/images/`, instead of drawn shapes. One small function, `draw_sprite(...)`, draws all of them (rotated and tinted where needed).
- Removed the 2-player duel mode. There are 3 modes now: Easy, Medium, Hard. The player is a single `Player player;` struct, not a 2-player array.
- Removed code that is no longer needed now that art is real pictures: the procedural background scenes, the twinkling star field, and the `glow()`, `triangle()` and `line()` drawing helpers.
- Easy, Medium and Hard keep the same speed, spawn, damage and health numbers as before. The banana (+20 health, 8-second shield), the 5 backgrounds changing every 1000 score, personal bests, pause, and touch controls are all unchanged.
- There are still no pointer variables anywhere in the file except `FILE *f` in the two score-file functions.

## v0.27.0 — simple rewrite and 2-player duel

- Rewrote `main.c` and `others.c` from scratch in a simpler style: snake_case names (`red_enemy` / `blue_enemy` types, `shoot`, `spawn_enemy`, `move_player`), only if/else, loops, arrays, functions, switch and a few structs. There are no pointer variables; the only `*` is `FILE *f` in the two score-file functions.
- Added **2 PLAYER** mode (same keyboard): Player 1 = WASD + Space, Player 2 = arrow keys + Enter, 100 health, 20 damage per hit, first to 5 points wins.
- The title screen now has four mode cards (Easy, Medium, Hard, 2 Player) and a PLAY button; keys 1-4, left/right and the mouse all choose a mode.
- Easy, Medium and Hard keep the same speed, spawn, damage and health numbers as v0.26.0. The banana (+20 health, 8-second shield), the five backgrounds (new one every 1000 score) and the five sound effects are unchanged.
- Simplified: enemies steer by easing their velocity toward a target direction, so they may feel a little different from v0.26.0 (the numbers are in tables at the top of `main.c`). Enemies are drawn as glowing circles with eyes, and the aim reticle, leg animation, rare third spawn on Hard and the personal-best link on the pause screen were removed.
- Personal bests now use the file `rift_runner_scores.txt` (desktop) or `/persist/rift_runner_scores.txt` (web). Records saved by older versions are not carried over.

## v0.26.0 — single-file edition (combined source)

- Combined every gameplay module into one labelled `main.c`, including shared structures and declarations formerly stored in custom headers.
- Updated CMake to compile only `main.c`; retained assets, web support, workflows, and build scripts.
- The gameplay rebalance, banana shield, five original one-shot sounds, no-music rule, and raylib naming fix are preserved.

## v0.26.0 — single-file edition and final difficulty rebalance

- Easy: slightly faster enemy arrivals than before, roaming behavior, 130 starting health, and 13 base red-enemy contact damage.
- Medium: faster arrivals, faster enemies, and 25 base red-enemy contact damage.
- Hard: 0.58-second base spawn interval, 120 base enemy speed, 34 base red-enemy contact damage, 85 starting health, a higher on-screen enemy cap (48), extra spawns from stage 3, and a rare third spawn from stage 7.
- Increased simultaneous enemy allowance by difficulty; stage scaling still increases pressure over time.
- Preserved +20 health / 8-second banana shield, original five sound effects, no music, and the raylib `RRNormalizeVector2` name-collision fix.

## v0.25.2 — fix native Normalize linker collision

- Renamed the project math helper `Normalize` to `RRNormalizeVector2` to avoid colliding with raylib’s internal `Normalize` symbol when linking the native build.
- Preserved the original five one-shot sound effects; no music or ambience loop.
- Kept Easy random roaming, Medium pursuit, Hard mixed pursuit/random movement, and banana +20 health / 8-second shield.

## v0.25.1 — restore original sound effects and stabilize CI

- Removed the added background ambience, whisper/rumble accents, and rift-transition sound. The game has no music and uses only its original five one-shot effects.
- Pinned Linux CI and browser workflow jobs to Ubuntu 24.04 rather than a moving `ubuntu-latest` image.
- Added the missing `<stdlib.h>` include for `rand()` in `main.c`; the compiler now treats undeclared function calls as errors. GNU C99 extensions are enabled consistently for native and web targets.


## v0.25.0 — difficulty behavior, banana tuning, and soundscape

- Easy enemies roam with randomized steering; Medium enemies aim for the player without random drift; Hard enemies combine pursuit and randomized steering.
- The banana restores up to 20 health and grants an 8-second shield.
- Removed the music stream entirely. Added a quiet, non-musical space ambience loop, rare deep-rumble/void-whisper accents, and a rift-shift effect on each 1,000-point environment transition.
- Updated the asset validation and documentation for the new WAV files and version metadata.

## v0.24.0 — distraction cleanup

- Removed combo tracking, combo score bonuses, the combo HUD, kill counters, and floating point popups.
- Removed repair/health-kit and data-chip pickups; the banana is now the only pickup.
- The banana restores up to 18 health and grants a 5-second shield.
- Enemy defeats award a fixed 200 score without extra counters or random loot.
- Simplified pickup and effects state to remove unused data and reduce visual clutter.

## v0.23.0 — final presentation and usability polish

- Added a named sector label and a progress meter showing how close the run is to the next 1,000-point environment shift.
- Refined the HUD and title screen with clearer visual hierarchy, better spacing, and more responsive layouts for short viewports.
- Added mouse/touch-accessible navigation for personal bests, pause/resume, and returning to the flight deck.
- Replaced the generic pickup diamonds with distinct data-chip, repair-cross, and curved banana silhouettes.
- Fixed right-side touch aiming so movement no longer overwrites the chosen aim direction.
- Replaced random-per-frame camera shake with a damped, deterministic motion so it no longer consumes gameplay random numbers.
- Moved the background transition timer into the update step so rendering no longer changes game state.
- Retained the five full procedural environments and the non-explosive fade-and-scan transition every 1,000 score.

## v0.22.0 — alien silhouettes and full space environments

- Rebuilt red enemies as angular red wraiths with segmented carapaces, glowing eyes, mandibles, dorsal blades, and animated tendrils.
- Rebuilt blue enemies as smaller skittering alien scouts with swept fins, multiple eye lights, and animated tendrils.
- Replaced the mostly color-swapping background with five distinct procedural scenes: The Cradle, Graveyard Belt, The Rift, Blood Moon, and Void Cathedral.
- The scene changes every 1,000 score with a soft dark fade-and-reveal; removed the center-out radial explosion transition.
- Kept backgrounds procedural and asset-free so desktop and browser builds use the same visuals.

## v0.21.0 — multi-file source organization

- Reorganized gameplay into focused, plainly named C modules and headers.
- Kept `main.c` as the commented game-flow outline and frame-loop entry point.
- Updated CMake to compile all modules together.

## v0.20.0 — gameplay changes

- Removed Phase Dash and its HUD/touch prompts.
- Added blue enemies and animated type-specific glows.
- Medium and Hard enemies steer toward the player; Easy remains random roaming.
- Added periodic banana pickups that restore health and grant five seconds of protection.
- Added score-driven sector changes as the visual system was further refined in v0.22.0.

## v0.19.0 — three-score leaderboard and simpler score code

- Replaced the unlimited run history with exactly three saved personal bests: Easy, Medium, and Hard.
- A record updates only when a run beats that mode's previous best.
- Removed dynamic score-history allocation, sorting, scrolling, and old history fields.
