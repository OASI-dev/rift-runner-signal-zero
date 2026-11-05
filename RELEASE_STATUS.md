# Simplified teacher edition status

- `main.c` is 873 lines (the supplied original was 974 lines).
- One permanent background remains: `assets/images/bg_cradle.png`, matching the custom space image supplied for this edition.
- The four other background files and background-rotation logic are removed.
- Kept the three difficulty modes, player/enemy movement, automatic shooting, collision, particles, banana pickup, sound effects, saved best scores, menus and touch support.
- Simplified repeated mode text logic, sound loading and enemy edge selection; removed tiny one-use wrappers where the code is clearer inline.
- Added a plain-English code walkthrough and a presentation script based on the current source.

A full raylib build and hands-on play-test should still be performed on the presentation computer.
