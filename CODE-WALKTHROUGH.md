# RIFT RUNNER: SIGNAL ZERO — explaining the code simply

This edition keeps **all game programming in one `main.c` file**. The `SECTION:` comments are dividers: they show where code used to live in separate source files before the project was combined. They are comments, not extra files required by the compiler.

## The whole game in everyday language

Imagine running a small arcade. The game needs to remember what is happening, read what the player does, move everything, decide whether anything hits, draw the scene, play sounds, and save the best score. `main.c` contains each of these jobs.

### 1. Shared types, settings and declarations — near the top

The first part includes raylib and standard C headers, then defines the data the game needs. A `Player` stores the runner's position, movement, health and shield. An `Enemy` stores an enemy's position, speed, health and type. `Bullet`, `Pickup`, `Particle` and `Star` store the information for their matching game objects. `Game` is the main container holding the current game state, player, object arrays, score, stage, difficulty, sound handles and personal-best values.

Function declarations tell the compiler which functions exist before their implementations appear later in the file. Constants such as the screen size and object limits keep repeated values consistent.

### 2. Math helpers and colors — around lines 227–298

These are small reusable tools. They clamp a value into a range, choose a random value, measure distances, add or scale 2D vectors, normalize a direction, adjust transparency, and draw text centered or right-aligned. Instead of repeating the same formula in many places, the game calls a helper. `RRNormalizeVector2` safely turns a direction into a unit-length direction and uses a unique name to avoid colliding with a raylib symbol.

### 3. Background environments — around lines 299–517

This section draws the star field and procedural space scenery. It uses raylib's drawing functions to create planets, rings, asteroids, rift shapes, glowing details and transitions. A score milestone changes the selected environment; the background is visual scenery and does not itself decide enemy behavior.

### 4. Player movement, aiming and appearance — around lines 518–653

`UpdatePlayer` reads keyboard and touch/mouse state through raylib, works out movement, keeps the runner inside the play area, updates facing direction and firing timing, and manages timers such as temporary protection. `DrawRunner` draws the player's ship/runner. Raylib reports input and draws shapes; the movement rules and the ship's design are game code written in C.

### 5. Enemies — around lines 654–930

`SpawnEnemy` chooses where a new enemy appears and sets its type and starting values. `UpdateEnemies` moves enemies and checks contact with the player. Easy uses random roaming, Medium follows the player, and Hard combines pursuit with randomized turning and extra spawn chances. `KillEnemy` handles defeat rewards and effects. The drawing functions make red Wraiths and blue Skitters look different. The damage values, movement behavior and health rules are project decisions, not rules supplied by raylib.

### 6. Bullets — around lines 931–977

`AddBullet` creates a shot at a position with a direction and velocity. `UpdateBullets` advances shots over time, checks whether a shot hits an enemy, applies the game's hit rules and removes expired or used bullets. Raylib provides timing and basic shape drawing; the projectile movement and collision rules are written for this game.

### 7. Banana pickup — around lines 978–1019

The banana is the only pickup in v0.26.0. Its update code animates the pickup and checks collection. When the runner collects it, health goes up by as much as 20 without exceeding maximum health, and the runner gets an 8-second shield. It doesn't add a health kit, repair cell or data-chip system.

### 8. Visual effects — around lines 1020–1064

Particles are tiny colored shapes that appear briefly when things happen, such as hits or transitions. The effect functions create them, move them, reduce their remaining lifetime and stop drawing them when their lifetime expires. These effects are feedback for the player rather than separate game rules.

### 9. World drawing — around lines 1065–1142

This section draws active objects in a deliberate order: the world/background, pickups, bullets, enemies, runner and other visible objects. Drawing order matters because objects drawn later can appear in front of objects drawn earlier. The renderer presents the current state; it should not normally be the place where game rules are updated.

### 10. Audio — around lines 1143–1187

The audio code initializes sound support, plays one of five one-shot effects, and releases audio resources when the game closes. The five files are `shot.wav`, `impact.wav`, `pickup.wav`, `enemy_down.wav` and `surge.wav`. There is no background music or ambient loop. Browser audio may wait for the first user gesture because browsers restrict sound before the player interacts.

### 11. Personal bests — around lines 1188–1234

The leaderboard loads and saves exactly three local records: Easy, Medium and Hard. The game updates a difficulty's record only when a new run beats that mode's best. Desktop uses a local text file; the browser build uses Emscripten's persistent filesystem bridge. These scores are local to that device/browser, not a public online leaderboard.

### 12. Start, reset and end a run — around lines 1235–1318

`ResetWorld` clears old bullets, enemies, pickups and effects and puts the game objects into a fresh state. `StartRun` starts a new attempt using the selected difficulty. `EndRun` finishes a run and checks the personal best. `SetDifficulty` changes the selected settings. `InitGame` sets up the initial game state and loads saved records.

### 13. Menus, HUD and touch controls — around lines 1319–1875

This long UI section reads menu input and draws the title screen, difficulty cards, score screen, heads-up display (HUD), pause screen and game-over screen. The HUD presents information such as health, shield time, score and stage. Touch-control code lets a browser player steer and aim without a keyboard. raylib is used to measure and draw text, draw panels and read input; the button layout, labels and menu rules are authored in the game.

### 14. Main game process — around lines 1876–2007

`UpdatePlaying` is the step-by-step update for a live round: handle pause, update the player, update bullets, update enemies, process pickups and effects, change backgrounds at score thresholds, spawn bananas and spawn enemies. The order is important: for example, shots are updated before enemies are advanced, so hit checks and state changes happen in a controlled order.

`GameFrame` is called once per rendered frame. It gets the frame time, limits unusually large time steps, polls shared mouse/touch input, updates whichever screen is active and then calls `DrawFrame`.

`main` is the C program's entry point. It configures and opens the raylib window, sets the frame rate, initializes the game, then starts the desktop loop or hands control to the browser's Emscripten loop. On desktop it also shuts down audio and closes the window at the end.

## How the other project files still work

Only **game source code** is combined into `main.c`; the project still needs supporting assets and build/deployment files:

| File or folder | What it does | Is it another source-code module? |
|---|---|---|
| `assets/` | Contains the five WAV sounds that the game loads. | No; sound data. |
| `web/index.html` | Provides the browser landing page around the web game. | No; web page. |
| `web/idbfs-pre.js` | Helps the browser build mount/sync persistent score storage. | No; browser support script. |
| `CMakeLists.txt` | Tells CMake to compile **only `main.c`** and link raylib. | No; build configuration. |
| `.github/workflows/` | Describes automated native builds, web builds and deployment. | No; CI configuration. |
| `tools/` | Contains asset validation and release/build helper scripts. | No; developer tools. |
| `README.md`, `BUILDING.md`, `LICENSE` | Explain the project, builds and license. | No; documentation/legal text. |

## raylib in a short, teacher-friendly summary

**raylib is a C library that gives the game ready-made building blocks for multimedia.** In this project it creates the window, reads keyboard/mouse/touch-related input, reports frame time, draws shapes and text, and plays the WAV sound effects. It saves us from having to write all the operating-system window, graphics and audio code ourselves.

**raylib does not design the game rules.** The movement rules, enemy AI, shooting, collision checks, health, shield, scoring, difficulty, menus and backgrounds are implemented in our own `main.c` code using raylib's functions.

A simple way to explain it to a teacher:

> “C is the programming language, raylib is the multimedia library, and `main.c` contains my game logic. raylib opens the game window, reads controls, draws the visuals and plays sounds. My code decides how the runner moves, how enemies behave, how shots collide, how score increases and when the game changes screens.”

## A few core raylib jobs used here

- `InitWindow(...)` opens the game window or canvas.
- `SetTargetFPS(...)` requests a target frame rate.
- `GetFrameTime()` gives the duration of the last frame, so movement can be time-based.
- `IsKeyDown(...)`, `IsKeyPressed(...)`, mouse/touch functions read player input.
- `BeginDrawing()` and `EndDrawing()` bracket a frame's drawing work.
- `DrawText(...)`, `DrawCircle(...)`, `DrawLine(...)` and rectangle functions draw the HUD, menus, ships and effects.
- Audio functions such as `LoadSound(...)`, `PlaySound(...)` and `UnloadSound(...)` load, play and release effects.
- `WindowShouldClose()` allows the desktop loop to detect a close request.

The exact functions called at any moment depend on the selected game screen and platform. CMake fetches raylib and links it to the one-file game during a build.
