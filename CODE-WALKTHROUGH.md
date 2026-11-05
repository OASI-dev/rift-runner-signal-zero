# CODE WALKTHROUGH — RIFT RUNNER: SIGNAL ZERO

This guide explains the current `main.c` in simple language and follows the order of the source file.

## 1. What the game does

RIFT RUNNER is a one-player survival shooter made with C and raylib. The player moves around the screen and aims at enemies. The gun fires automatically. Enemies enter from the edges and try to reach the player. Defeating an enemy gives 200 points. Every 500 points, the game moves to the next level.

The player can collect a banana to restore up to 20 health and receive an 8-second shield. There are three difficulty modes: Easy, Medium and Hard. The game always uses the supplied `bg_cradle.png` image as its background.

## 2. The C ideas used in this project

- **Variables** store values such as health, score, player position and timers.
- **Arrays** store many similar values. For example, `bullet[]` stores bullets and `enemy[]` stores enemies.
- **Structures (`struct`)** group values that belong together. `Player`, `Bullet`, `Enemy` and `Particle` are the four main structures.
- **Enums** give readable names to numbers. For example, `EASY`, `MEDIUM` and `HARD` represent the difficulty choices.
- **Conditions** (`if`, `else`, `switch`) make decisions.
- **Loops** (`for`, `while`) repeat actions for every bullet, enemy or particle.
- **Functions** divide the game into jobs so each part can be understood separately.

## 3. How the code is arranged

1. **Libraries and settings:** imports raylib and standard C libraries, then sets limits such as the maximum number of bullets, enemies and particles.
2. **Game data:** defines colors, enums, structures, arrays and global variables. The `mode_names[]` and `mode_notes[]` arrays store the text for each difficulty. `mode_color()` returns its display color.
3. **Helper functions:** `random_float()` creates a random decimal number, `limit()` keeps a value inside a range, `touching()` checks whether two circular objects overlap, and `draw_sprite()` draws an image.
4. **Resources and saved scores:** `load_pictures()` loads the five game sprites and one background. `start_audio()` loads the sound effects. `save_best()` and `load_best()` write and read the best score for each difficulty.
5. **Particle effects:** `explosion()` creates small particles. `update_effects()` moves them and counts down the effect timers.
6. **Input:** `read_touch()` handles phone movement and aiming. `game_frame()` also reads the mouse and detects a new click or tap.
7. **Player and bullets:** `reset_player()` prepares a new player, `move_player()` handles movement and aiming, `shoot()` creates a bullet, and `update_bullets()` moves bullets and removes expired ones.
8. **Game rules:** `spawn_enemy()` creates an enemy, `update_enemies()` moves enemies and handles contact damage, `enemy_hits()` checks bullet collisions, `kill_enemy()` awards points, and `update_banana()` handles the health pickup. `update_run()` calls these actions in the correct order.
9. **Screen input and game states:** `start_game()` resets the game. `update_title()` handles the title screen. The pause and best-score keyboard checks are short enough to be handled directly inside `game_frame()`.
10. **Drawing helpers:** `draw_center()` and `draw_right()` align text. `panel()` draws a menu panel. `button()` draws a button and reports when it is clicked.
11. **Drawing the arena:** `draw_player()`, `draw_enemy()` and `draw_world()` draw the characters, bullets, banana and particles. The permanent background is drawn directly in `draw_frame()`.
12. **Drawing the screens:** `draw_hud()`, `draw_title()`, `draw_best()`, `draw_pause()` and `draw_over()` draw the interface. `draw_frame()` selects the correct screen to display.
13. **Main loop:** `game_frame()` updates the active game state and draws one frame. `main()` opens the window, loads resources, runs the loop and cleans up.

## 4. Important pieces of code

### Movement

```c
player.x += dx * player_speed[mode] * dt;
player.y += dy * player_speed[mode] * dt;
```

`dx` and `dy` describe the direction. `player_speed[mode]` selects the speed for the chosen difficulty. `dt` is the time since the last frame. Using `dt` keeps movement consistent on different computers.

### Collision detection

`touching()` measures the difference between two positions. It compares the squared distance with the squared sum of the two circle radii. If the circles overlap, it returns true. Squared values are used so the program does not need to calculate a square root.

### Enemy behavior

Easy enemies choose random directions and wander. Medium enemies steer toward the player. Hard enemies also chase the player, but include a little random movement. `update_enemies()` handles their movement, wall bouncing and damage when they touch the player.

### Level and score

Every defeated enemy adds 200 points. The level is calculated with `1 + score / 500`. In C, integer division removes the decimal part, so the level increases after each 500-point step.

### Main game loop

`game_frame()` is called repeatedly. It gets the frame time, reads mouse/touch input, updates whichever game state is active, and calls `draw_frame()` to display the result.

## 5. What raylib does

raylib is a C library. This project uses it to open the window, read keyboard/mouse/touch input, get frame time, load images, draw text and shapes, and play sounds. The project's own C code controls the game rules: movement, enemies, shooting, collision, score, health and difficulty.

## 6. Questions a teacher may ask

**What is `main()`?** It is where the C program starts. It opens the window and loads the resources before starting the game loop.

**Why use arrays?** Arrays let the game manage many bullets, enemies and particles with loops instead of writing separate variables for each object.

**Why use structures?** A structure keeps related information together. A `Player`, for example, has a position, angle, health and timers.

**What is `dt`?** It is the time since the previous frame. Movement and timers use it so the game is not tied to one computer's frame rate.

**How does a bullet hit an enemy?** `enemy_hits()` checks each active bullet against each active enemy. On a collision, the bullet disappears and the enemy loses health. If its health reaches zero, `kill_enemy()` awards the points.

**How does the level increase?** Each defeated enemy gives 200 points. The level rises every 500 points.

**What does the banana do?** It restores up to 20 health and gives an 8-second shield.

**Why is there only one background?** The project uses `bg_cradle.png` for the whole run. The other four background files and the code that changed between them were removed.

**What did you simplify in the code?** I replaced repeated difficulty-name and description functions with lookup arrays, used a loop to load the sound files, simplified enemy edge selection, and moved very small one-use helper routines next to the code that uses them. This keeps the game behavior while reducing repetition.
