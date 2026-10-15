#include "game.h"
#include "render.h"
#include "audio.h"
#include "persistence.h"

#include "raylib.h"
#include <math.h>
#include <stdio.h>

#define RR_WINDOW_WIDTH  1280
#define RR_WINDOW_HEIGHT 720
#define RR_TARGET_FPS    60
#define RR_MAX_DELTA     (1.0f / 20.0f)

int main(void)
{
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT | FLAG_MSAA_4X_HINT);
    InitWindow(RR_WINDOW_WIDTH, RR_WINDOW_HEIGHT, "RIFT RUNNER: SIGNAL ZERO");
    if (!IsWindowReady()) {
        fprintf(stderr, "RIFT RUNNER: unable to initialize the game window.\n");
        return 1;
    }

    SetWindowMinSize(800, 480);
    SetTargetFPS(RR_TARGET_FPS);
    SetExitKey(KEY_NULL);

    Game game;
    Game_Init(&game);
    Renderer_Init();
    Audio_Init();

    while (!WindowShouldClose() && !game.shouldQuit) {
        float dt = GetFrameTime();
        if (!isfinite(dt) || dt < 0.0f) dt = 0.0f;
        if (dt > RR_MAX_DELTA) dt = RR_MAX_DELTA;

        Game_Update(&game, dt);
        Renderer_Update(dt);
        Audio_Update(&game, dt);

        BeginDrawing();
        Renderer_Draw(&game);
        EndDrawing();
    }

    Game_FinalizeOnExit(&game);
    Audio_Shutdown();
    Renderer_Shutdown();
    CloseWindow();
    return 0;
}
