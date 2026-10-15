#include "game.h"

#include <math.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
void TestSetPressed(int key);
void TestSetDown(int key);

static int fail(const char *message)
{
    fprintf(stderr, "FAIL: %s\n", message);
    return 1;
}

int main(void)
{
    /* Keep this test deterministic across repeated CTest runs. */
    (void)remove("rift_runner_profile.dat");
    (void)remove("rift_runner_profile.dat.tmp");

    Game game;
    Game_Init(&game);
    if (game.screen != GAME_SCREEN_TITLE) return fail("initial screen should be title");

    TestSetPressed(KEY_ENTER);
    Game_Update(&game, 1.0f / 60.0f);
    if (game.screen != GAME_SCREEN_ARENA) return fail("Enter should start the run");

    game.player.invulnerableTimer = 1000.0f; /* isolate hazard timing */
    int sawWarning = 0;
    int sawActive = 0;
    float largestRadius = 0.0f;
    for (int i = 0; i < 1800; ++i) {
        game.player.invulnerableTimer = 1000.0f; /* keep enemies from ending the test early */
        if (game.screen == GAME_SCREEN_UPGRADE) TestSetPressed(KEY_ONE);
        Game_Update(&game, 1.0f / 60.0f);
        if (game.coreSurgeWarning) sawWarning = 1;
        if (game.coreSurgeActive) {
            sawActive = 1;
            if (game.coreSurgeRadius > largestRadius) largestRadius = game.coreSurgeRadius;
        }
    }

    if (!sawWarning) return fail("Core Surge warning phase was never observed");
    if (!sawActive) return fail("Core Surge active phase was never observed");
    if (largestRadius <= 22.0f) return fail("Core Surge radius did not expand");

    /* Guard game timers against non-finite or implausibly large frame deltas. */
    Game frameTimeGame;
    Game_Init(&frameTimeGame);
    Game_Update(&frameTimeGame, NAN);
    if (!isfinite(frameTimeGame.titlePulse) || frameTimeGame.titlePulse != 0.0f)
        return fail("non-finite frame delta contaminated game timers");
    Game_Update(&frameTimeGame, 10.0f);
    if (!isfinite(frameTimeGame.titlePulse) || frameTimeGame.titlePulse > 0.051f)
        return fail("large frame delta was not capped");

    /* Controls edge cases: a sector reward cannot be skipped with Escape. */
    Game controls;
    Game_Init(&controls);
    TestSetPressed(KEY_ENTER);
    Game_Update(&controls, 1.0f / 60.0f);
    if (controls.screen != GAME_SCREEN_ARENA) return fail("controls test could not start a run");
    controls.screen = GAME_SCREEN_UPGRADE;
    TestSetPressed(KEY_ESCAPE);
    Game_Update(&controls, 1.0f / 60.0f);
    if (controls.screen != GAME_SCREEN_UPGRADE)
        return fail("Escape should not silently skip a sector upgrade");
    TestSetPressed(KEY_ONE);
    Game_Update(&controls, 1.0f / 60.0f);
    if (controls.screen != GAME_SCREEN_ARENA || controls.upgradeLevels[0] != 1)
        return fail("upgrade selection did not apply and resume gameplay");

    /* Dash input must activate temporary protection and remain inside the arena. */
    controls.player.position = (Vector2){-500.0f, -500.0f};
    TestSetDown(KEY_A);
    TestSetPressed(KEY_SPACE);
    Game_Update(&controls, 1.0f / 60.0f);
    if (controls.player.dashTimer <= 0.0f || controls.player.invulnerableTimer <= 0.0f)
        return fail("Phase Dash did not activate with temporary protection");
    if (controls.player.position.x < RR_ARENA_INSET_X + controls.player.radius - 0.01f ||
        controls.player.position.y < RR_ARENA_INSET_Y + controls.player.radius - 0.01f)
        return fail("player escaped the arena bounds after a dash");
    TestSetDown(KEY_NULL);

    /* Escape should pause during play; a second Escape returns to title and saves. */
    TestSetPressed(KEY_ESCAPE);
    Game_Update(&controls, 1.0f / 60.0f);
    if (controls.screen != GAME_SCREEN_PAUSED)
        return fail("Escape during play should open pause screen");
    TestSetPressed(KEY_ENTER);
    Game_Update(&controls, 1.0f / 60.0f);
    if (controls.screen != GAME_SCREEN_ARENA)
        return fail("Enter should resume from pause");
    TestSetPressed(KEY_ESCAPE);
    Game_Update(&controls, 1.0f / 60.0f);
    if (controls.screen != GAME_SCREEN_PAUSED)
        return fail("Escape should pause a resumed run");
    TestSetPressed(KEY_ESCAPE);
    Game_Update(&controls, 1.0f / 60.0f);
    if (controls.screen != GAME_SCREEN_TITLE || !controls.runFinalized)
        return fail("Escape from pause should finalize run and return to title");

    /* Screen-shake accessibility toggle is saved and navigable in settings. */
    Game settingsGame;
    Game_Init(&settingsGame);
    settingsGame.screen = GAME_SCREEN_SETTINGS;
    settingsGame.settingsSelection = 3;
    settingsGame.screenShakeEnabled = true;
    TestSetPressed(KEY_ENTER);
    Game_Update(&settingsGame, 1.0f / 60.0f);
    if (settingsGame.screenShakeEnabled)
        return fail("screen-shake setting did not toggle off");
    if (settingsGame.settingsSelection != 3)
        return fail("screen-shake setting changed selection unexpectedly");
    settingsGame.settingsSelection = 4;
    TestSetPressed(KEY_ENTER);
    Game_Update(&settingsGame, 1.0f / 60.0f);
    if (settingsGame.screen != GAME_SCREEN_TITLE)
        return fail("fifth settings row did not return to title");

    /* Extreme but valid saved counters must saturate instead of overflowing. */
    Game saturation;
    Game_Init(&saturation);
    saturation.runsPlayed = INT_MAX;
    TestSetPressed(KEY_ENTER);
    Game_Update(&saturation, 1.0f / 60.0f);
    if (saturation.screen != GAME_SCREEN_ARENA || saturation.runsPlayed != INT_MAX)
        return fail("run counter did not safely saturate at INT_MAX");
    saturation.score = INT_MAX - 5;
    saturation.combo = INT_MAX;
    saturation.comboTimer = 10.0f;
    saturation.lifetimeShards = INT_MAX - 1;
    saturation.shardSpawnTimer = 100.0f;
    saturation.shards[0].active = true;
    saturation.shards[0].position = saturation.player.position;
    TestSetPressed(KEY_NULL);
    Game_Update(&saturation, 1.0f / 60.0f);
    if (saturation.score != INT_MAX)
        return fail("score combo overflowed instead of saturating");
    Game_FinalizeOnExit(&saturation);
    if (saturation.lifetimeShards != INT_MAX)
        return fail("lifetime shard counter did not safely saturate");

    /* Closing the app during play must commit a partial run exactly once. */
    game.screen = GAME_SCREEN_ARENA;
    game.runFinalized = false;
    game.score = 4321;
    game.sector = 3;
    game.shardsCollected = 7;
    game.lifetimeShards = 11;
    game.runsPlayed = 1;
    Game_FinalizeOnExit(&game);
    if (!game.runFinalized) return fail("shutdown did not finalize active run");
    if (game.bestScore != 4321 || game.bestSector != 3)
        return fail("shutdown did not persist best score/sector");
    if (game.lifetimeShards != 18)
        return fail("shutdown did not add run shards exactly once");
    Game_FinalizeOnExit(&game);
    if (game.lifetimeShards != 18)
        return fail("repeated shutdown finalization double-counted shards");

    puts("game logic tests passed: Core Surge, frame-time guards, dash bounds, upgrades, pause/resume, screen-shake setting, saturated counters, and run finalization");
    (void)remove("rift_runner_profile.dat");
    (void)remove("rift_runner_profile.dat.tmp");
    return 0;
}
