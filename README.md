# RIFT RUNNER: SIGNAL ZERO — teacher-friendly edition

**A neon survival shooter written in C with raylib.** The complete game logic is in one file: `main.c`. The game uses one permanent space background, enemy/player images, a banana pickup, and five sound effects.

## Play in your browser

After the browser workflow succeeds and GitHub Pages finishes deploying, open:

**https://oasi-dev.github.io/rift-runner-signal-zero/**

## Difficulty modes

- **Easy:** enemies move around the arena in random directions.
- **Medium:** enemies move toward the player.
- **Hard:** enemies chase the player with a little random wobble and more frequent spawns.

## Pictures needed

The six image files are stored in `assets/images/`:

| File | Purpose |
|---|---|
| `player.png` | Player character |
| `red_enemy.png` | Larger, stronger red enemy |
| `blue_enemy.png` | Smaller, faster blue enemy |
| `bullet.png` | Player projectile |
| `banana.png` | Health and shield pickup |
| `bg_cradle.png` | The single permanent space background |

The background is the custom space image supplied for this edition. The four unused background files have been removed from the project.

## Gameplay

- Each defeated enemy gives **200 points**.
- A new level begins every **500 points**. Enemies become faster and more numerous as the level rises.
- Collecting the banana restores up to **20 health** and grants an **8-second shield**.
- Personal best scores are saved separately for Easy, Medium, and Hard on the same device.
- The game has five one-shot WAV sound effects.
- The background stays the same for the entire run.

## Controls

| Action | Controls |
|---|---|
| Choose difficulty | `1` / `2` / `3`, left/right arrows, `A` / `D`, or click a card |
| Start | `Enter`, `Space`, or the Play button |
| Move | `WASD` or arrow keys; on touch screens, use the left side |
| Aim | Hold a mouse button; on touch screens, use the right side |
| Pause | `P` or `Escape` |
| Personal bests | `B` on the title screen |

## Understand the code

`main.c` is arranged into 13 parts: setup, game data, helper functions, resource loading and saved scores, particle effects, input, player and bullet rules, enemy and banana rules, screen controls, drawing helpers, game-world drawing, screen drawing, and the main loop. The source uses section headings and focused comments for the more difficult game rules. The current `main.c` is 873 lines, down from 974 lines in the supplied original.

Read [CODE-WALKTHROUGH.md](CODE-WALKTHROUGH.md) for a plain-English explanation and [PRESENTATION-SCRIPT.md](PRESENTATION-SCRIPT.md) for a short explanation to use with your teacher.

## Build

CMake downloads raylib 6.0 the first time it is configured, so an internet connection is needed for the first build. Follow [BUILDING.md](BUILDING.md).

## Verification note

The edited source passes a strict GNU C99 syntax check against a local raylib declaration stub, and the bundled WAV assets pass the asset checker. This does not replace a full build against raylib 6.0 or a real gameplay test. Follow `BUILDING.md` and run the game before submitting it.
