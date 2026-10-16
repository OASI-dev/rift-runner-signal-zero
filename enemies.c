/* Enemy spawning, AI, contact damage, rewards and rendering. */

#include "game.h"
#include <math.h>
#include <stdlib.h>

void SpawnEnemy(void)
{
    int alive = 0;
    for (int i = 0; i < MAX_ENEMIES; ++i) if (g.enemies[i].active) ++alive;
    int limit = 8 + g.stage + (int)g.difficulty * 2;
    if (limit > 38) limit = 38;
    if (alive >= limit) return;

    /* Spawn visible red enemies at an edge, initially travelling inward toward
       a random point—not toward the player. After spawning they roam and bounce. */
    const float w = (float)SW(), h = (float)SH(), edge = 25.0f;
    Vector2 pos;
    switch (rand() % 4) {
        case 0: pos = V2(RandF(edge, w - edge), edge); break;
        case 1: pos = V2(w - edge, RandF(edge, h - edge)); break;
        case 2: pos = V2(RandF(edge, w - edge), h - edge); break;
        default: pos = V2(edge, RandF(edge, h - edge)); break;
    }
    Vector2 waypoint = V2(RandF(w * 0.18f, w * 0.82f), RandF(h * 0.18f, h * 0.82f));
    float heading = atan2f(waypoint.y - pos.y, waypoint.x - pos.x) + RandF(-0.55f, 0.55f);
    float stageSpeed = MinF((float)(g.stage - 1) * 3.0f, 78.0f);
    bool blue = (rand() % 100) < 42;
    float speed = GetDifficulty()->enemySpeed * (blue ? 0.78f : 0.70f) + stageSpeed;
    float hp = blue ? 1.0f + (float)((g.stage - 1) / 6) : 2.0f + (float)((g.stage - 1) / 4);
    if (hp > 5.0f) hp = 5.0f;

    for (int i = 0; i < MAX_ENEMIES; ++i) {
        if (!g.enemies[i].active) {
            g.enemies[i] = (Enemy){ true, pos, V2(0, 0), blue ? 15.5f : 21.0f, speed, hp, hp,
                                    heading, RandF(0.55f, 1.25f), RandF(-0.9f, 0.9f), 0.0f, blue };
            return;
        }
    }
}

void KillEnemy(Enemy *e)
{
    const int oldStage = g.stage;
    e->active = false;
    ++g.kills;
    g.combo = (g.comboClock > 0.0f) ? g.combo + 1 : 1;
    g.comboClock = 3.2f;
    int bonus = (g.combo > 1) ? ((g.combo > 12) ? 60 : g.combo * 5) : 0;
    int points = 200 + bonus;
    g.score += points;
    AddFloater(e->pos, points, (g.combo >= 5) ? C_GOLD : C_CYAN);
    Burst(e->pos, C_RED, 26, 215.0f, 3.2f);
    PlaySfx(SFX_ENEMY_DOWN);
    if ((rand() % 100) < 28) AddPickup(e->pos, PICKUP_DATA);
    else if ((rand() % 100) < 9) AddPickup(e->pos, PICKUP_REPAIR);
    g.stage = CurrentStageForScore(g.score);
    if (g.stage > oldStage) {
        g.stageNotice = 2.0f;
        g.flashAlpha = 0.15f;
        PlaySfx(SFX_SURGE);
        Burst(g.player.pos, C_GOLD, 24, 170.0f, 2.9f);
    }
}

void UpdateEnemies(float dt)
{
    const float w = (float)SW(), h = (float)SH();
    float damage = GetDifficulty()->contactDamage;
    for (int i = 0; i < MAX_ENEMIES; ++i) {
        Enemy *e = &g.enemies[i];
        if (!e->active) continue;

        e->flash = MaxF(0.0f, e->flash - dt);
        e->turnClock -= dt;
        if (e->turnClock <= 0.0f) {
            e->turnRate = RandF(-0.9f, 0.9f);
            e->turnClock = RandF(0.55f, 1.25f);
        }
        if (g.difficulty != DIFF_EASY) {
            Vector2 toPlayer = Normalize(Sub(g.player.pos, e->pos));
            float attraction = (g.difficulty == DIFF_MEDIUM) ? 0.34f : 0.68f;
            float desired = atan2f(toPlayer.y, toPlayer.x);
            float delta = atan2f(sinf(desired - e->heading), cosf(desired - e->heading));
            e->heading += ClampF(delta, -attraction * dt, attraction * dt);
        }
        e->heading += e->turnRate * dt;
        float stageFactor = 1.0f + MinF((float)(g.stage - 1) * 0.028f, 0.48f);
        e->vel = Scale(V2(cosf(e->heading), sinf(e->heading)), e->speed * stageFactor);
        e->pos = Add(e->pos, Scale(e->vel, dt));

        /* Bounce off the arena edges so the red enemies keep roaming on-screen. */
        if (e->pos.x < e->radius) { e->pos.x = e->radius; e->heading = PI_F - e->heading; e->turnRate = -e->turnRate; }
        if (e->pos.x > w - e->radius) { e->pos.x = w - e->radius; e->heading = PI_F - e->heading; e->turnRate = -e->turnRate; }
        if (e->pos.y < e->radius) { e->pos.y = e->radius; e->heading = -e->heading; e->turnRate = -e->turnRate; }
        if (e->pos.y > h - e->radius) { e->pos.y = h - e->radius; e->heading = -e->heading; e->turnRate = -e->turnRate; }

        float r = e->radius + 15.0f;
        if (DistSq(e->pos, g.player.pos) < r * r && g.player.invulnerable <= 0.0f && g.player.shieldTime <= 0.0f) {
            g.player.health -= e->blue ? damage * 0.55f : damage;
            g.player.invulnerable = 0.92f;
            g.screenShake = 0.20f;
            g.flashAlpha = 0.10f;
            Vector2 knock = Normalize(Sub(g.player.pos, e->pos));
            g.player.pos = Add(g.player.pos, Scale(knock, 30.0f));
            e->pos = Add(e->pos, Scale(knock, -22.0f));
            Burst(g.player.pos, C_PINK, 16, 150.0f, 2.3f);
            PlaySfx(SFX_IMPACT);
            if (g.player.health <= 0.0f) { g.player.health = 0.0f; EndRun(); return; }
        }
    }
}

void DrawEnemy(const Enemy *e)
{
    Color base = e->blue ? C_BLUE : C_RED;
    Color glow = e->blue ? C_CYAN : C_RED;
    Color body = e->flash > 0.0f ? C_GOLD : base;
    float r = e->radius;
    Vector2 forward = V2(cosf(e->heading), sinf(e->heading));
    float pulse = 1.0f + 0.07f * sinf(g.worldTime * (e->blue ? 5.5f : 3.2f) + e->pos.x * 0.02f);
    DrawCircleGradient(e->pos, r * (e->blue ? 2.9f : 2.5f), WithOpacity(glow, 0.35f), WithOpacity(glow, 0.0f));
    DrawCircleV(e->pos, r * pulse, body);
    DrawCircleLines((int)e->pos.x, (int)e->pos.y, r * pulse, e->blue ? (Color){ 180, 228, 255, 255 } : (Color){ 255, 172, 180, 255 });
    DrawLineEx(Add(e->pos, Scale(forward, -r * 0.48f)), Add(e->pos, Scale(forward, r * 0.50f)), e->blue ? 3.0f : 5.0f, e->blue ? (Color){ 12, 34, 88, 255 } : (Color){ 76, 12, 28, 255 });
    DrawCircleV(Add(e->pos, Scale(forward, r * 0.22f)), e->blue ? 2.4f : 3.2f, C_GOLD);
    if (e->hp < e->maxHp) {
        float barW = r * 2.2f;
        DrawRectangleRounded((Rectangle){ e->pos.x - barW * 0.5f, e->pos.y - r - 9.0f, barW, 4.0f }, 0.5f, 4, (Color){ 20, 25, 46, 230 });
        DrawRectangleRounded((Rectangle){ e->pos.x - barW * 0.5f, e->pos.y - r - 9.0f, barW * ClampF(e->hp / e->maxHp, 0, 1), 4.0f }, 0.5f, 4, C_GOLD);
    }
}
