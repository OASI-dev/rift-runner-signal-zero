# Changelog

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
