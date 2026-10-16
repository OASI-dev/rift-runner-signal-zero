/* RIFT RUNNER: SIGNAL ZERO — application entry point. */
#include "game.h"
#include <stdio.h>
#include <math.h>
#if defined(__EMSCRIPTEN__)
#include <emscripten.h>
#endif

/* The shared live state is owned here; other systems access it through game.h. */
Game g;

/*
 * MAIN GAME PROCESS
 * This function is intentionally an outline: detailed logic lives in each system file.
 * The order matters: move and fire first, then resolve shots, enemies and pickups.
 */
void UpdatePlaying(float dt)
{
    /* 1. Pause input: do not update gameplay while the pause screen is open. */
    if (IsKeyPressed(KEY_P) || IsKeyPressed(KEY_ESCAPE)) {
        g.state = STATE_PAUSED;
        return;
    }

    /* 2. Player: read keyboard/touch/mouse input, move and aim. */
    UpdatePlayer(dt);

    /* 3. Blaster shots: move projectiles and check hits against enemies. */
    UpdateBullets(dt);
    if (g.state != STATE_PLAYING) return;

    /* 4. Enemies: update their movement/AI and check player contact damage. */
    UpdateEnemies(dt);
    if (g.state != STATE_PLAYING) return;

    /* 5. Pickups: the banana restores health and grants an 8-second shield. */
    UpdatePickups(dt);

    /* 6. Temporary effects: hit particles, screen shake and transition flashes. */
    UpdateEffects(dt);

    /* 7. Change to a new space environment every 1,000 score, with a rift sound. */
    if (g.score / 1000 != g.backgroundTheme) {
        g.backgroundTheme = g.score / 1000;
        g.backgroundTransition = 0.90f;
        PlaySfx(SFX_RIFT_SHIFT);
    }

    /* 8. Occasionally create a banana pickup somewhere inside the arena. */
    g.bananaClock -= dt;
    if (g.bananaClock <= 0.0f) {
        AddPickup(V2(RandF(55.0f, (float)SW() - 55.0f),
                     RandF(75.0f, (float)SH() - 75.0f)));
        g.bananaClock = RandF(22.0f, 34.0f);
    }

    /* 9. Spawn the next enemy wave based on difficulty and current stage. */
    g.spawnClock -= dt;
    if (g.spawnClock <= 0.0f) {
        SpawnEnemy();
        float scoreRate = 1.0f + (float)(g.stage - 1) * 0.15f;
        float interval = GetDifficulty()->spawnInterval / scoreRate;
        g.spawnClock = MaxF(0.27f, interval) * RandF(0.84f, 1.16f);
        if (g.stage >= 4 && (rand() % 100) < 18) SpawnEnemy();
    }
}

/* Run one update-and-draw cycle. This is called once per rendered frame. */
void GameFrame(void)
{
    /* A stable delta time keeps movement consistent across different machines. */
    float dt = GetFrameTime();
    if (!isfinite(dt) || dt < 0.0f) dt = 0.0f;
    if (dt > 1.0f / 25.0f) dt = 1.0f / 25.0f;
    g.worldTime += dt;

    /* Maintain non-musical space ambience and occasional distant sound events. */
    UpdateAmbientAudio(dt);

    /* Poll mouse/touch once per frame so every menu uses the same input edge. */
    PollUIPointer();

    /* Route input and time to the active screen/game state. */
    switch (g.state) {
        case STATE_TITLE:    UpdateTitle();    UpdateEffects(dt); break;
        case STATE_PLAYING:  UpdatePlaying(dt);                break;
        case STATE_PAUSED:   UpdatePaused();   UpdateEffects(dt); break;
        case STATE_GAME_OVER:UpdateGameOver(); UpdateEffects(dt); break;
        case STATE_SCORES:   UpdateScores();   UpdateEffects(dt); break;
    }

    /* DrawFrame calls the background, game world, HUD and active screen. */
    DrawFrame();
}

/* Program setup starts here; platform-specific loop selection is at the bottom. */
int main(void)
{
    /* 1. Configure a resizable desktop window (fixed browser canvas on Web). */
#if defined(__EMSCRIPTEN__)
    SetConfigFlags(FLAG_MSAA_4X_HINT);
#else
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT | FLAG_MSAA_4X_HINT);
#endif
    InitWindow(SCREEN_START_W, SCREEN_START_H, RR_TITLE);
    if (!IsWindowReady()) {
        fprintf(stderr, "RIFT RUNNER: could not initialize the raylib window.\n");
        return 1;
    }
#if !defined(__EMSCRIPTEN__)
    SetWindowMinSize(800, 480);
#endif
    SetTargetFPS(TARGET_FPS);
    SetExitKey(KEY_NULL);

    /* 2. Load starting state, background stars and the three stored best scores. */
    InitGame();

    /* 3. Hand control to the browser loop or run the desktop loop ourselves. */
#if defined(__EMSCRIPTEN__)
    emscripten_set_main_loop(GameFrame, 0, 1);
#else
    while (!WindowShouldClose() && !g.quitRequested) {
        GameFrame();
    }
    ShutdownAudio();
    CloseWindow();
#endif
    return 0;
}
