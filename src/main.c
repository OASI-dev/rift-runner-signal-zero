#include "game.h"
#include "render.h"
#include "audio.h"
#include "persistence.h"

#include "raylib.h"
#include <math.h>
#include <stdio.h>

#if defined(__EMSCRIPTEN__)
#include <emscripten/emscripten.h>
#endif

#define RR_WINDOW_WIDTH  1280
#define RR_WINDOW_HEIGHT 720
#define RR_TARGET_FPS    60
#define RR_MAX_DELTA     (1.0f / 20.0f)

static Game game;

static void UpdateAndDrawFrame(void)
{
    float dt = GetFrameTime();
    if (!isfinite(dt) || dt < 0.0f) dt = 0.0f;
    if (dt > RR_MAX_DELTA) dt = RR_MAX_DELTA;

    Game_Update(&game, dt);
#if defined(__EMSCRIPTEN__)
    /* A browser page cannot be closed by the game. Escape on the title screen
       therefore behaves as a harmless no-op rather than stopping the web loop. */
    if (game.shouldQuit) game.shouldQuit = false;
#endif
    Renderer_Update(dt);
    Audio_Update(&game, dt);

    BeginDrawing();
    Renderer_Draw(&game);
    EndDrawing();
}

int main(void)
{
#if defined(__EMSCRIPTEN__)
    /* Browser canvas sizing is controlled by the page; don't request a native
       resizable window or rely on desktop window-manager APIs. */
    SetConfigFlags(FLAG_MSAA_4X_HINT);
#else
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT | FLAG_MSAA_4X_HINT);
#endif
    InitWindow(RR_WINDOW_WIDTH, RR_WINDOW_HEIGHT, "RIFT RUNNER: SIGNAL ZERO");
    if (!IsWindowReady()) {
        fprintf(stderr, "RIFT RUNNER: unable to initialize the game window.\n");
        return 1;
    }

#if !defined(__EMSCRIPTEN__)
    SetWindowMinSize(800, 480);
#endif
    SetTargetFPS(RR_TARGET_FPS);
    SetExitKey(KEY_NULL);

    Game_Init(&game);
    Renderer_Init();
    Audio_Init();

#if defined(__EMSCRIPTEN__)
    /* Browsers must own the event loop; the classic blocking while-loop would
       freeze the page. A zero FPS value lets requestAnimationFrame pace frames. */
    emscripten_set_main_loop(UpdateAndDrawFrame, 0, 1);
    return 0; /* The simulated infinite loop normally never reaches here. */
#else
    while (!WindowShouldClose() && !game.shouldQuit) {
        UpdateAndDrawFrame();
    }

    Game_FinalizeOnExit(&game);
    Audio_Shutdown();
    Renderer_Shutdown();
    CloseWindow();
    return 0;
#endif
}
