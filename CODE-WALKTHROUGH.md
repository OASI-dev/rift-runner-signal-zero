# RIFT RUNNER: SIGNAL ZERO — explaining the code simply

The whole game is **one C file**: `main.c`. It uses only: variables, `if / else`, `for` loops, functions, `switch`, arrays and a few structs. There are **no pointer variables** -- the only `*` in the program is `FILE *f` in the two small score-file functions, which is simply how C names an opened file.

## Pictures instead of drawn shapes

Earlier versions drew the player, enemies and backgrounds out of circles and lines. This version loads real picture files instead, with raylib's `LoadTexture(...)`, and draws each one with `DrawTexturePro(...)`, which can also rotate the picture to face the direction something is moving. One small function, `draw_sprite(...)`, does this for the player, both enemy types, the bullet, the banana, AND the full-screen background -- the same function, called with different pictures and sizes, instead of five separate drawing blocks.

## The whole game in everyday language

Imagine a small arcade machine. It must remember what is happening, read what the player does, move everything, decide what hits what, draw the scene, make sounds and save the best score. Each of those jobs is a function with a plain name.

### The three modes

`enum { EASY, MEDIUM, HARD };` gives the numbers 0, 1, 2 readable names. The variable `mode` holds the chosen one. Small tables such as `enemy_speed[3]` hold one value per mode, so `enemy_speed[mode]` means "the enemy speed of this mode".

### main.c — part by part

1. **Libraries and settings.** `#include` lines bring in raylib and the C library. `#define` gives names to fixed numbers. `enum` lines name the screens, the modes, the enemy types and the sounds.
2. **Data.** A *struct* groups the facts about one thing. `Player` holds x, y, angle, health and timers. `Enemy`, `Bullet` and `Particle` do the same for their things. There is only **one** player now (`Player player;`, not an array), since there is no 2-player mode anymore. A field called `on` says whether a bullet/enemy/spark slot is in use (1) or free (0). The 10 `Texture2D` variables hold the loaded pictures.
3. **Small helpers.** `random_float(low, high)` picks a random decimal number. `limit(value, low, high)` keeps a number inside a range. `touching(...)` tells whether two circles touch. `draw_sprite(...)` draws one picture, centered, sized and rotated -- used for everything visual in the game.
4. **Loading pictures, sound and saved scores.** `load_pictures()` loads the 10 picture files from `assets/images/` once, when the game starts (the window must already be open). `start_audio()` / `play(n)` handle sound. `save_best()` / `load_best()` read and write the score file, one number per line.
5. **Sparks.** `explosion(...)` creates small colored circles flying outward (used for hits, deaths and the banana) -- these stay as simple drawn circles, not pictures.
6. **Mouse and touch input.** Reads where the player clicked or touched, for the buttons and for phone movement/aiming.
7. **Player and bullet rules.** `shoot()` creates a bullet. `move_player(dt)` reads the keys (and phone fingers, and the mouse) and moves the player: *position = position + speed × time*. `update_bullets(dt)` moves bullets and removes old ones.
8. **Game rules: enemies, banana, a run.** `spawn_enemy()` makes a red (big, tough, slow) or blue (small, weak, fast) enemy at a random edge. `update_enemies(dt)` steers each enemy: Easy wanders randomly, Medium chases the player, Hard chases with a random wobble. `enemy_hits()` checks bullets against enemies and `kill_enemy(i)` gives 200 points and a new level every 500 points. `update_banana(dt)` gives +20 health and an 8-second shield. `update_run(dt)` runs all of this, in order, once per frame; every 1000 score the `background` number goes up, so a new picture is shown.
9. **Screens (keyboard part).** `start_game()` resets everything and starts the chosen mode. `update_title()`, `update_pause()`, `update_over()` and `update_best()` read the keys of each screen.
10. **Small drawing helpers.** `draw_center`, `draw_right`, `panel` and `button(...)` -- `button` draws itself and returns 1 when clicked, used by every screen with buttons.
11. **Drawing the player, enemies, background and the arena.** `draw_background()` is now just one call to `draw_sprite(...)` with the current sector's picture, stretched to fill the screen. `draw_player()` and `draw_enemy(i)` draw the picture turned to face the right way, with a gold tint when shielded or just hit. `draw_world()` draws the banana, bullets, enemies, sparks and the player, in that order.
12. **Drawing the screens and one whole frame.** The HUD, title screen (now 3 mode cards), best-scores screen, pause screen and game-over screen. `draw_frame()` draws one whole frame: background, arena, the screen on top, and the white flash.
13. **Main loop.** `game_frame()` runs about 60 times a second: it measures the time since the last frame (`dt`), updates the active screen with a `switch`, then draws. `main()` opens the window, loads the pictures and the best scores, and starts the loop.

## Good questions your teacher may ask

- **Why no `player[2]` array anymore?** Because there is only one player now that the duel mode is gone -- a plain `Player player;` is simpler than an array of one.
- **Why `dt`?** It is the time of the last frame. Moving by `speed * dt` makes the game run at the same speed on slow and fast computers.
- **Why does `draw_sprite` take width AND height separately, not just one "size"?** So the same function can draw a square player sprite AND a wide rectangular background with the same code.
- **What happens if a picture file is missing?** `LoadTexture` quietly returns an empty picture (0 width, 0 height); the game keeps running, that one object is just invisible. Nothing crashes.

## raylib in a short, teacher-friendly summary

**raylib is a C library that gives the game ready-made building blocks for multimedia.** In this project it creates the window, reads keyboard/mouse/touch input, reports frame time, loads and draws pictures, draws shapes and text, and plays the WAV sound effects. It saves us from writing the operating-system window, graphics and audio code ourselves.

**raylib does not design the game rules.** Movement, enemy behavior, shooting, collisions, health, shield, scoring, difficulty, menus and backgrounds are written in our own `main.c` using raylib's functions.

A simple way to say it to a teacher:

> "C is the programming language, raylib is the multimedia library, and `main.c` contains my game logic. raylib opens the game window, reads controls, loads and draws the pictures and plays sounds. My code decides how the runner moves, how enemies behave, how shots collide, how score increases and when the game changes screens."

## A few core raylib jobs used here

- `InitWindow(...)` opens the game window or canvas.
- `LoadTexture(...)` loads a picture file into memory; `DrawTexturePro(...)` draws it, rotated and tinted; `UnloadTexture(...)` frees it when the game closes.
- `GetFrameTime()` gives the duration of the last frame, so movement can be time-based.
- `IsKeyDown(...)`, `IsKeyPressed(...)` and the mouse/touch functions read player input.
- `BeginDrawing()` and `EndDrawing()` bracket a frame's drawing work.
- `DrawText(...)`, `DrawCircle(...)` and the rectangle functions draw the HUD, menus and sparks.
- `LoadSound(...)`, `PlaySound(...)` and `UnloadSound(...)` load, play and release the effects.
- `WindowShouldClose()` lets the desktop loop detect a close request.
