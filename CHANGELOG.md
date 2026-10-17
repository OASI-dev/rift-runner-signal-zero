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

# Changelog

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
