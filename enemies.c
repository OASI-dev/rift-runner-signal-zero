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
        /* Stage changes are announced cleanly; no burst is emitted at the player. */
        g.stageNotice = 2.0f;
        PlaySfx(SFX_SURGE);
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

static Color EnemyFade(Color c, float opacity)
{
    c.a = (unsigned char)(ClampF(opacity, 0.0f, 1.0f) * (float)c.a);
    return c;
}

/* Convert an enemy-local point (forward, sideways) into arena coordinates. */
static Vector2 EnemyPoint(const Enemy *e, float forward, float sideways)
{
    float c = cosf(e->heading), s = sinf(e->heading);
    return V2(e->pos.x + forward * c - sideways * s,
              e->pos.y + forward * s + sideways * c);
}

/* Tendrils are drawn as bent, tapered-looking segments so the silhouette feels
   organic instead of like a glowing ball. The movement is purely cosmetic. */
static void DrawTendril(const Enemy *e, int index, float side, float length, Color shade, Color glint)
{
    float r = e->radius;
    float sway = sinf(g.worldTime * (e->blue ? 4.7f : 3.1f) + (float)index * 1.4f + e->pos.x * 0.01f);
    float rootX = -r * (0.18f + 0.16f * (float)(index % 3));
    float rootY = side * r * (0.24f + 0.11f * (float)(index % 2));
    float reachX = rootX - length * (0.55f + 0.08f * (float)(index % 2));
    float reachY = side * (r * (0.95f + 0.19f * (float)(index % 3)) + sway * r * 0.18f);
    Vector2 a = EnemyPoint(e, rootX, rootY);
    Vector2 b = EnemyPoint(e, rootX - length * 0.22f, rootY + side * length * 0.50f + sway * r * 0.10f);
    Vector2 c = EnemyPoint(e, reachX, reachY);
    DrawLineEx(a, b, e->blue ? 3.1f : 4.6f, shade);
    DrawLineEx(b, c, e->blue ? 1.8f : 2.7f, shade);
    DrawLineEx(b, c, 0.8f, EnemyFade(glint, 0.72f));
    DrawCircleV(c, e->blue ? 1.7f : 2.2f, glint);
}

static void DrawAlienEye(Vector2 p, float radius, Color glow, Color pupil)
{
    DrawCircleGradient(p, radius * 3.8f, EnemyFade(glow, 0.40f), EnemyFade(glow, 0.0f));
    DrawCircleV(p, radius, glow);
    DrawCircleV(V2(p.x + radius * 0.18f, p.y), radius * 0.48f, pupil);
    DrawCircleV(V2(p.x + radius * 0.25f, p.y - radius * 0.10f), radius * 0.16f, C_TEXT);
}

static void DrawRedWraith(const Enemy *e, float r, Color body, Color glow)
{
    /* Six animated hooked tendrils hang behind a many-pointed armored shell. */
    for (int i = 0; i < 3; ++i) {
        DrawTendril(e, i, -1.0f, r * (0.88f + 0.14f * (float)i), (Color){ 34, 11, 34, 255 }, glow);
        DrawTendril(e, i + 3, 1.0f, r * (0.88f + 0.14f * (float)i), (Color){ 34, 11, 34, 255 }, glow);
    }

    /* Bone-like dorsal spikes and side blades frame the silhouette. */
    for (int sideIndex = -1; sideIndex <= 1; sideIndex += 2) {
        DrawTriangle(EnemyPoint(e, -r * 0.23f, sideIndex * r * 0.48f),
                     EnemyPoint(e, r * 0.14f, sideIndex * r * 1.03f),
                     EnemyPoint(e, r * 0.33f, sideIndex * r * 0.43f),
                     (Color){ 67, 18, 43, 255 });
        DrawTriangle(EnemyPoint(e, -r * 0.58f, sideIndex * r * 0.34f),
                     EnemyPoint(e, -r * 0.90f, sideIndex * r * 0.78f),
                     EnemyPoint(e, -r * 0.16f, sideIndex * r * 0.58f),
                     (Color){ 42, 14, 35, 255 });
        DrawLineEx(EnemyPoint(e, -r * 0.58f, sideIndex * r * 0.34f),
                   EnemyPoint(e, -r * 0.90f, sideIndex * r * 0.78f), 1.2f,
                   EnemyFade(glow, 0.42f));
    }

    /* Split carapace: layered pointed plates, never a circular body. */
    Vector2 tip = EnemyPoint(e, r * 1.05f, 0.0f);
    Vector2 crown = EnemyPoint(e, r * 0.20f, -r * 0.58f);
    Vector2 shoulder = EnemyPoint(e, -r * 0.55f, -r * 0.43f);
    Vector2 tail = EnemyPoint(e, -r * 0.82f, 0.0f);
    Vector2 lower = EnemyPoint(e, -r * 0.55f, r * 0.43f);
    Vector2 jaw = EnemyPoint(e, r * 0.20f, r * 0.58f);
    Vector2 core = EnemyPoint(e, -r * 0.13f, 0.0f);
    DrawTriangle(tip, crown, core, body);
    DrawTriangle(tip, core, jaw, EnemyFade(body, 0.88f));
    DrawTriangle(crown, shoulder, core, (Color){ 60, 17, 42, 255 });
    DrawTriangle(jaw, core, lower, (Color){ 78, 18, 39, 255 });
    DrawTriangle(shoulder, tail, core, (Color){ 25, 12, 31, 255 });
    DrawTriangle(lower, core, tail, (Color){ 25, 12, 31, 255 });

    /* Sharp mandibles and a vertical, furnace-like central slit. */
    DrawTriangle(EnemyPoint(e, r * 0.66f, -r * 0.08f),
                 EnemyPoint(e, r * 1.00f, -r * 0.27f),
                 EnemyPoint(e, r * 0.79f, r * 0.02f), (Color){ 19, 8, 24, 255 });
    DrawTriangle(EnemyPoint(e, r * 0.66f, r * 0.08f),
                 EnemyPoint(e, r * 1.00f, r * 0.27f),
                 EnemyPoint(e, r * 0.79f, -r * 0.02f), (Color){ 19, 8, 24, 255 });
    DrawLineEx(EnemyPoint(e, r * 0.25f, 0.0f), EnemyPoint(e, r * 0.78f, 0.0f), r * 0.105f,
               EnemyFade(glow, 0.90f));
    DrawLineEx(EnemyPoint(e, r * 0.31f, 0.0f), EnemyPoint(e, r * 0.68f, 0.0f), 1.0f,
               (Color){ 255, 245, 222, 255 });

    DrawAlienEye(EnemyPoint(e, r * 0.23f, -r * 0.23f), r * 0.105f,
                 (Color){ 255, 83, 104, 255 }, (Color){ 30, 3, 15, 255 });
    DrawAlienEye(EnemyPoint(e, r * 0.23f, r * 0.23f), r * 0.105f,
                 (Color){ 255, 83, 104, 255 }, (Color){ 30, 3, 15, 255 });

    /* Etched seams catch light as the creature pulses. */
    DrawLineEx(crown, core, 1.0f, EnemyFade(glow, 0.56f));
    DrawLineEx(jaw, core, 1.0f, EnemyFade(glow, 0.38f));
    DrawLineEx(shoulder, tail, 1.1f, EnemyFade((Color){ 255, 159, 177, 255 }, 0.34f));
    DrawLineEx(lower, tail, 1.1f, EnemyFade((Color){ 255, 159, 177, 255 }, 0.34f));
}

static void DrawBlueSkitter(const Enemy *e, float r, Color body, Color glow)
{
    /* Smaller scout: four blade-like limbs, swept wings and a triple eye cluster. */
    for (int i = 0; i < 2; ++i) {
        DrawTendril(e, i, -1.0f, r * 0.92f, (Color){ 12, 26, 57, 255 }, glow);
        DrawTendril(e, i + 2, 1.0f, r * 0.92f, (Color){ 12, 26, 57, 255 }, glow);
    }
    for (int sideIndex = -1; sideIndex <= 1; sideIndex += 2) {
        DrawTriangle(EnemyPoint(e, r * 0.24f, sideIndex * r * 0.34f),
                     EnemyPoint(e, -r * 0.10f, sideIndex * r * 1.10f),
                     EnemyPoint(e, -r * 0.31f, sideIndex * r * 0.31f),
                     (Color){ 19, 47, 91, 255 });
        DrawLineEx(EnemyPoint(e, r * 0.18f, sideIndex * r * 0.40f),
                   EnemyPoint(e, -r * 0.11f, sideIndex * r * 1.03f), 1.2f, EnemyFade(glow, 0.76f));
    }

    Vector2 tip = EnemyPoint(e, r * 1.15f, 0.0f);
    Vector2 top = EnemyPoint(e, r * 0.08f, -r * 0.46f);
    Vector2 rear = EnemyPoint(e, -r * 0.85f, 0.0f);
    Vector2 bottom = EnemyPoint(e, r * 0.08f, r * 0.46f);
    Vector2 core = EnemyPoint(e, -r * 0.20f, 0.0f);
    DrawTriangle(tip, top, core, body);
    DrawTriangle(tip, core, bottom, EnemyFade(body, 0.88f));
    DrawTriangle(top, rear, core, (Color){ 11, 27, 59, 255 });
    DrawTriangle(bottom, core, rear, (Color){ 11, 27, 59, 255 });
    DrawTriangleLines(tip, top, bottom, EnemyFade(glow, 0.85f));
    DrawTriangleLines(top, rear, bottom, EnemyFade(glow, 0.48f));

    DrawLineEx(EnemyPoint(e, r * 0.29f, -r * 0.12f), EnemyPoint(e, r * 0.87f, 0.0f), r * 0.10f, glow);
    DrawAlienEye(EnemyPoint(e, r * 0.30f, 0.0f), r * 0.11f,
                 (Color){ 83, 236, 255, 255 }, (Color){ 2, 19, 42, 255 });
    DrawCircleV(EnemyPoint(e, r * 0.06f, -r * 0.20f), r * 0.055f, C_TEXT);
    DrawCircleV(EnemyPoint(e, r * 0.06f, r * 0.20f), r * 0.055f, C_TEXT);
}

void DrawEnemy(const Enemy *e)
{
    Color base = e->blue ? (Color){ 34, 113, 175, 255 } : (Color){ 123, 27, 48, 255 };
    Color glow = e->blue ? C_CYAN : (Color){ 255, 62, 103, 255 };
    float r = e->radius;
    float pulse = 1.0f + 0.045f * sinf(g.worldTime * (e->blue ? 5.6f : 3.2f) + e->pos.x * 0.02f);
    Color body = e->flash > 0.0f ? C_GOLD : base;

    /* Keep the aura soft and narrow; the body silhouette does the visual work. */
    DrawCircleGradient(e->pos, r * (e->blue ? 2.0f : 1.85f), EnemyFade(glow, 0.25f), EnemyFade(glow, 0.0f));
    if (e->blue) DrawBlueSkitter(e, r * pulse, body, glow);
    else DrawRedWraith(e, r * pulse, body, glow);

    /* Enemy health is shown only when damaged, keeping the arena uncluttered. */
    if (e->hp < e->maxHp) {
        float barW = r * 2.2f;
        DrawRectangleRounded((Rectangle){ e->pos.x - barW * 0.5f, e->pos.y - r - 9.0f, barW, 4.0f }, 0.5f, 4, (Color){ 20, 25, 46, 230 });
        DrawRectangleRounded((Rectangle){ e->pos.x - barW * 0.5f, e->pos.y - r - 9.0f, barW * ClampF(e->hp / e->maxHp, 0, 1), 4.0f }, 0.5f, 4, C_GOLD);
    }
}
