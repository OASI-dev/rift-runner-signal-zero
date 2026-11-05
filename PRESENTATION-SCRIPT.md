# Short presentation script — RIFT RUNNER: SIGNAL ZERO

Good morning. My project is called **RIFT RUNNER: SIGNAL ZERO**. It is a one-player survival shooting game made in C using the raylib library.

The player moves around the screen, aims at enemies, and the gun fires automatically. Enemies enter from the edges of the screen and try to reach the player. Each defeated enemy gives 200 points. Every 500 points, the game level increases and the enemies become more challenging.

There are three difficulty modes: Easy, Medium and Hard. Easy enemies wander in random directions. Medium enemies chase the player. Hard enemies chase the player with a little random movement and appear more often.

The banana is a power-up. When the player collects it, health is restored by up to 20 points and the player receives an 8-second shield.

I use structures to store related information about the player, bullets, enemies and particles. Arrays store groups of these objects. Loops update them, and conditions handle decisions such as collisions and game states. I also use functions so that the code is divided into clear jobs.

The `game_frame()` function is the main loop. It reads input, updates the current game state and draws the next frame. The `main()` function opens the window, loads the images and sounds, then starts the loop.

raylib handles the window, input, graphics and audio. My C code handles the game rules, such as movement, enemy behavior, shooting, collisions, health and score.

The game saves a separate best score for each difficulty on the device. It uses one permanent space background called The Cradle. I also simplified repeated code, such as difficulty labels and sound loading, while keeping the main gameplay features.

Thank you.

## Quick facts to remember

- **Language:** C
- **Library:** raylib
- **Main file:** `main.c` (873 lines)
- **Main structures:** `Player`, `Bullet`, `Enemy`, `Particle`
- **Main loop:** `game_frame()`
- **Program entry point:** `main()`
- **Points per enemy:** 200
- **Points per level:** 500
- **Banana:** up to 20 health and an 8-second shield
- **Background:** `bg_cradle.png` only
