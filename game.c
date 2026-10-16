/* Game lifecycle: reset a run, start it, and finish it. */
#include "game.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

const DifficultyInfo DIFFICULTIES[3] = {
    { "EASY",   "Learn the arena",       { 98, 255, 178, 255 }, 292.0f, 0.245f, 1.02f, 72.0f, 13.0f, 130.0f },
    { "MEDIUM", "Stay moving",           { 76, 231, 255, 255 }, 276.0f, 0.225f, 0.78f, 95.0f, 25.0f, 100.0f },
    { "HARD",   "Overwhelming pursuit", { 255, 83, 170, 255 }, 266.0f, 0.205f, 0.58f, 120.0f, 34.0f, 85.0f }
};

void ResetWorld(void)
{
    memset(g.bullets, 0, sizeof(g.bullets));
    memset(g.enemies, 0, sizeof(g.enemies));
    memset(g.particles, 0, sizeof(g.particles));
    memset(g.pickups, 0, sizeof(g.pickups));
    memset(&g.player, 0, sizeof(g.player));
    g.score = 0;
    g.stage = 1;
    g.spawnClock = 1.0f;
    g.screenShake = 0.0f;
    g.flashAlpha = 0.0f;
    g.stageNotice = 0.0f;
    g.bananaClock = RandF(12.0f, 19.0f);
    g.backgroundTransition = 0.0f;
    g.backgroundTheme = 0;
    g.player.pos = V2((float)SW() * 0.5f, (float)SH() * 0.5f);
    g.player.facing = V2(1.0f, 0.0f);
    g.player.health = GetDifficulty()->startingHealth;
    g.player.maxHealth = GetDifficulty()->startingHealth;
    g.player.fireClock = 0.12f;
    g.player.invulnerable = 0.65f;
    g.player.animation = 0.0f;
    g.newPersonalBest = false;
}

void StartRun(void)
{
    FirstAudioGesture();
    ResetWorld();
    g.state = STATE_PLAYING;
    Burst(g.player.pos, C_CYAN, 34, 190.0f, 3.3f);
    PlaySfx(SFX_SURGE);
}

void EndRun(void)
{
    if (g.state != STATE_PLAYING) return;
    g.state = STATE_GAME_OVER;
    RecordPersonalBest();
    g.screenShake = 0.42f;
    g.flashAlpha = 0.32f;
    Burst(g.player.pos, C_PINK, 55, 260.0f, 4.0f);
    PlaySfx(SFX_IMPACT);
}

void SetDifficulty(int index)
{
    if (index < 0) index = 2;
    if (index > 2) index = 0;
    g.menuSelection = index;
    g.difficulty = (Difficulty)index;
}

void InitGame(void)
{
    memset(&g, 0, sizeof(g));
    g.state = STATE_TITLE;
    g.scoresReturnState = STATE_TITLE;
    g.difficulty = DIFF_MEDIUM;
    g.menuSelection = DIFF_MEDIUM;
    g.worldTime = 0.0f;
    for (int i = 0; i < 3; ++i) g.bestScores[i] = -1;
    srand((unsigned int)time(NULL));
    for (int i = 0; i < MAX_STARS; ++i) {
        g.stars[i] = (Star){ RandF(0.0f, (float)SCREEN_START_W), RandF(0.0f, (float)SCREEN_START_H), RandF(5.0f, 25.0f), RandF(0.6f, 1.8f), RandF(0.0f, 6.28f) };
    }
    LoadBestScores();

}
