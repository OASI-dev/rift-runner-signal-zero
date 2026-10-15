#include "render.h"

#include "raylib.h"
#include <math.h>
#include <stddef.h>

#define STAR_COUNT 190
#define ARC_SEGMENTS 120
#define EFFECT_PARTICLE_COUNT 360
#define TAU 6.28318530718f

static const Color COLOR_VOID       = { 4, 5, 18, 255 };
static const Color COLOR_CYAN       = { 67, 239, 255, 255 };
static const Color COLOR_BLUE       = { 56, 102, 255, 255 };
static const Color COLOR_VIOLET     = { 157, 71, 255, 255 };
static const Color COLOR_MAGENTA    = { 255, 53, 181, 255 };
static const Color COLOR_WHITE      = { 237, 250, 255, 255 };
static const Color COLOR_MUTED      = { 144, 162, 198, 255 };
static const Color COLOR_PANEL      = { 8, 12, 35, 225 };

static const Color C_CYAN = {67, 239, 255, 255};
static const Color C_PINK = {255, 53, 181, 255};

static const char *DASH_READY_LABEL = "PHASE DASH READY";

typedef struct Star {
    Vector2 position;
    float radius;
    float phase;
    float twinkleSpeed;
    float driftSpeed;
    unsigned char layer;
} Star;

typedef struct Spark {
    float angle;
    float orbit;
    float speed;
    float radius;
    float phase;
} Spark;

typedef struct EffectParticle {
    Vector2 position;
    Vector2 velocity;
    float life;
    float maxLife;
    float radius;
    Color color;
    bool active;
} EffectParticle;

static Star stars[STAR_COUNT];
static Spark sparks[40];
static EffectParticle effectParticles[EFFECT_PARTICLE_COUNT];
static unsigned int effectSeed = 0x5EEDC0DEu;
static float rendererTime = 0.0f;
static float previousDashTimer = 0.0f;
static int previousLives = 3;
static int previousShardCount = 0;
static int previousEnemiesDefeated = 0;
static bool previousSurgeActive = false;

static float ClampFloat(float value, float low, float high)
{
    if (value < low) return low;
    if (value > high) return high;
    return value;
}

static float Hash01(unsigned int *seed)
{
    *seed = (*seed * 1664525u) + 1013904223u;
    return (float)((*seed >> 8) & 0x00FFFFFFu) / 16777215.0f;
}

static Color WithAlpha(Color color, float alpha)
{
    color.a = (unsigned char)ClampFloat(alpha, 0.0f, 255.0f);
    return color;
}

static Vector2 RiftPoint(Vector2 center, float radiusX, float radiusY,
                         float angle, float wobble, float rotation)
{
    float a = angle + rotation;
    float r = 1.0f + wobble * sinf(angle * 5.0f + rendererTime * 0.8f)
                    + wobble * 0.45f * cosf(angle * 9.0f - rendererTime * 1.15f);
    return (Vector2){center.x + cosf(a) * radiusX * r,
                     center.y + sinf(a) * radiusY * r};
}

static void DrawRiftRing(Vector2 center, float radiusX, float radiusY,
                         float rotation, float wobble, Color color,
                         int segments, float thickness)
{
    for (int i = 0; i < segments; ++i) {
        float a0 = ((float)i / (float)segments) * TAU;
        float a1 = ((float)(i + 1) / (float)segments) * TAU;
        Vector2 p0 = RiftPoint(center, radiusX, radiusY, a0, wobble, rotation);
        Vector2 p1 = RiftPoint(center, radiusX, radiusY, a1, wobble, rotation);
        float pulse = 0.62f + 0.38f * sinf(a0 * 4.0f + rendererTime * 2.0f);
        DrawLineEx(p0, p1, thickness * pulse, color);
    }
}

static void DrawGradientBackdrop(void)
{
    int width = GetScreenWidth();
    int height = GetScreenHeight();
    DrawRectangleGradientV(0, 0, width, height, (Color){10, 10, 37, 255}, COLOR_VOID);

    for (int i = 0; i < 9; ++i) {
        float f = (float)i / 8.0f;
        float x = width * (0.16f + f * 0.75f) + sinf(rendererTime * 0.17f + f * 5.0f) * width * 0.035f;
        float y = height * (0.19f + 0.62f * (0.5f + 0.5f * sinf(f * 5.9f + rendererTime * 0.12f)));
        float radius = height * (0.17f + 0.12f * (0.5f + 0.5f * sinf(rendererTime * 0.23f + f * 4.0f)));
        Color tint = (i % 3 == 0) ? COLOR_BLUE : ((i % 3 == 1) ? COLOR_VIOLET : COLOR_MAGENTA);
        DrawCircleGradient((int)x, (int)y, radius, WithAlpha(tint, 14.0f), WithAlpha(tint, 0.0f));
    }
}

static void DrawStars(void)
{
    const int width = GetScreenWidth();
    const int height = GetScreenHeight();

    for (int i = 0; i < STAR_COUNT; ++i) {
        Star *star = &stars[i];
        float parallax = (float)star->layer * 0.25f;
        float x = star->position.x + sinf(rendererTime * 0.035f + star->phase) * (7.0f + 14.0f * parallax);
        float y = star->position.y + fmodf(rendererTime * star->driftSpeed * parallax, (float)height + 8.0f);
        if (y > (float)height + 4.0f) y -= (float)height + 8.0f;
        if (x < -4.0f) x += (float)width + 8.0f;
        if (x > (float)width + 4.0f) x -= (float)width + 8.0f;

        float twinkle = 0.35f + 0.65f * (0.5f + 0.5f * sinf(rendererTime * star->twinkleSpeed + star->phase));
        Color c = (star->layer == 2) ? COLOR_CYAN : COLOR_WHITE;
        c.a = (unsigned char)(25.0f + twinkle * (star->layer == 2 ? 145.0f : 100.0f));
        DrawCircleV((Vector2){x, y}, star->radius * (0.75f + 0.4f * twinkle), c);
        if (star->layer == 2 && twinkle > 0.93f) {
            DrawLineEx((Vector2){x - 4.0f, y}, (Vector2){x + 4.0f, y}, 1.0f, WithAlpha(C_CYAN, 75.0f));
            DrawLineEx((Vector2){x, y - 4.0f}, (Vector2){x, y + 4.0f}, 1.0f, WithAlpha(C_CYAN, 75.0f));
        }
    }
}

static void DrawEnergyLanes(void)
{
    const int width = GetScreenWidth();
    const int height = GetScreenHeight();
    Vector2 center = {(float)width * 0.5f, (float)height * 0.48f};
    for (int i = -7; i <= 7; ++i) {
        Vector2 edge = {(float)width * 0.5f + (float)i * width * 0.11f, (float)height + 25.0f};
        DrawLineEx(center, edge, 1.0f, (Color){55, 115, 210, 18});
    }
    for (int i = 0; i < 9; ++i) {
        float t = (float)i / 8.0f;
        float y = height * (0.28f + t * 0.83f);
        DrawLineEx((Vector2){0, y}, (Vector2){(float)width, y}, 1.0f, (Color){80, 104, 210, (unsigned char)(7 + i)});
    }
}

static void DrawCosmicRift(void)
{
    Vector2 center = {(float)GetScreenWidth() * 0.5f, (float)GetScreenHeight() * 0.46f};
    float minDimension = fminf((float)GetScreenWidth(), (float)GetScreenHeight());
    float breath = 1.0f + 0.025f * sinf(rendererTime * 0.9f);
    float rx = minDimension * 0.255f * breath;
    float ry = minDimension * 0.155f * (2.0f - breath);

    DrawCircleGradient((int)center.x, (int)center.y, minDimension * 0.26f,
                       (Color){49, 28, 130, 36}, (Color){12, 5, 35, 0});
    DrawRiftRing(center, rx * 1.38f, ry * 1.38f, -rendererTime * 0.055f, 0.055f,
                 (Color){73, 65, 255, 39}, ARC_SEGMENTS, 2.0f);
    DrawRiftRing(center, rx * 1.12f, ry * 1.12f, rendererTime * 0.11f, 0.075f,
                 (Color){255, 53, 181, 90}, ARC_SEGMENTS, 2.0f);
    DrawRiftRing(center, rx, ry, -rendererTime * 0.16f, 0.035f,
                 (Color){67, 239, 255, 155}, ARC_SEGMENTS, 2.4f);
    DrawRiftRing(center, rx * 0.91f, ry * 0.91f, rendererTime * 0.23f, 0.10f,
                 (Color){182, 113, 255, 170}, ARC_SEGMENTS, 1.5f);

    for (int arc = 0; arc < 7; ++arc) {
        float start = rendererTime * (0.15f + arc * 0.018f) + arc * (TAU / 7.0f);
        float span = 0.12f + 0.19f * (0.5f + 0.5f * sinf(rendererTime * 1.4f + arc));
        for (int j = 0; j < 17; ++j) {
            float f0 = (float)j / 17.0f;
            float f1 = (float)(j + 1) / 17.0f;
            Vector2 a = RiftPoint(center, rx * 1.05f, ry * 1.05f, start + span * f0, 0.025f, 0.0f);
            Vector2 b = RiftPoint(center, rx * 1.05f, ry * 1.05f, start + span * f1, 0.025f, 0.0f);
            DrawLineEx(a, b, 1.0f + (j % 4 == 0 ? 1.3f : 0.0f),
                       (arc % 2 == 0) ? WithAlpha(C_CYAN, 175.0f) : WithAlpha(C_PINK, 150.0f));
        }
    }

    for (size_t i = 0; i < sizeof(sparks) / sizeof(sparks[0]); ++i) {
        Spark *spark = &sparks[i];
        float angle = spark->angle + rendererTime * spark->speed;
        float wobble = 1.0f + 0.06f * sinf(rendererTime * 1.6f + spark->phase);
        Vector2 p = {center.x + cosf(angle) * spark->orbit * wobble,
                     center.y + sinf(angle) * spark->orbit * 0.58f * wobble};
        DrawCircleV(p, spark->radius + 2.5f, (Color){67, 239, 255, 20});
        DrawCircleV(p, spark->radius, (i % 3 == 0) ? COLOR_MAGENTA : COLOR_CYAN);
    }
}

static float Random01(void)
{
    return Hash01(&effectSeed);
}

static void SpawnEffectBurst(Vector2 position, Color color, int count,
                             float minSpeed, float maxSpeed, float lifeScale)
{
    for (int i = 0; i < count; ++i) {
        int slot = -1;
        for (int j = 0; j < EFFECT_PARTICLE_COUNT; ++j) {
            int candidate = (int)((effectSeed + (unsigned int)j) % EFFECT_PARTICLE_COUNT);
            if (!effectParticles[candidate].active) { slot = candidate; break; }
        }
        if (slot < 0) slot = (int)(effectSeed % EFFECT_PARTICLE_COUNT);

        EffectParticle *particle = &effectParticles[slot];
        float angle = Random01() * TAU;
        float speed = minSpeed + Random01() * (maxSpeed - minSpeed);
        particle->position = position;
        particle->velocity = (Vector2){cosf(angle) * speed, sinf(angle) * speed};
        particle->life = lifeScale * (0.55f + Random01() * 0.65f);
        particle->maxLife = particle->life;
        particle->radius = 1.2f + Random01() * 3.0f;
        particle->color = color;
        particle->active = true;
    }
}

static void UpdateEffectParticles(float deltaTime)
{
    for (int i = 0; i < EFFECT_PARTICLE_COUNT; ++i) {
        EffectParticle *particle = &effectParticles[i];
        if (!particle->active) continue;
        particle->life -= deltaTime;
        if (particle->life <= 0.0f) {
            particle->life = 0.0f;
            particle->active = false;
            continue;
        }
        particle->position.x += particle->velocity.x * deltaTime;
        particle->position.y += particle->velocity.y * deltaTime;
        particle->velocity.x *= 0.985f;
        particle->velocity.y *= 0.985f;
    }
}

static void DrawEffectParticles(void)
{
    for (int i = 0; i < EFFECT_PARTICLE_COUNT; ++i) {
        const EffectParticle *particle = &effectParticles[i];
        if (!particle->active || particle->maxLife <= 0.0f) continue;
        float fade = ClampFloat(particle->life / particle->maxLife, 0.0f, 1.0f);
        Color tint = WithAlpha(particle->color, (float)particle->color.a * fade);
        Vector2 tail = {particle->position.x - particle->velocity.x * 0.022f,
                        particle->position.y - particle->velocity.y * 0.022f};
        DrawLineEx(tail, particle->position, fmaxf(0.7f, particle->radius * fade),
                   WithAlpha(tint, (float)tint.a * 0.70f));
        DrawCircleV(particle->position, particle->radius * fade, tint);
    }
}

static void ObserveGameplayEvents(const Game *game)
{
    if (game->player.dashTimer > 0.0f && previousDashTimer <= 0.0f) {
        SpawnEffectBurst(game->player.position, COLOR_CYAN, 24, 40.0f, 255.0f, 0.34f);
    }
    if (game->shardsCollected > previousShardCount) {
        SpawnEffectBurst(game->player.position, (Color){255, 225, 95, 255}, 20, 25.0f, 165.0f, 0.42f);
    }
    if (game->enemiesDefeated > previousEnemiesDefeated) {
        SpawnEffectBurst(game->player.position, COLOR_VIOLET, 24, 30.0f, 205.0f, 0.45f);
    }
    if (game->lives < previousLives) {
        SpawnEffectBurst(game->player.position, COLOR_MAGENTA, 34, 45.0f, 245.0f, 0.48f);
    }
    if (game->coreSurgeActive && !previousSurgeActive) {
        Vector2 center = {(float)GetScreenWidth() * 0.5f, (float)GetScreenHeight() * 0.5f};
        SpawnEffectBurst(center, COLOR_CYAN, 66, 65.0f, 270.0f, 0.72f);
    }

    previousDashTimer = game->player.dashTimer;
    previousLives = game->lives;
    previousShardCount = game->shardsCollected;
    previousEnemiesDefeated = game->enemiesDefeated;
    previousSurgeActive = game->coreSurgeActive;
}

static void DrawCoreSurge(const Game *game)
{
    int w = GetScreenWidth();
    int h = GetScreenHeight();
    Vector2 center = {(float)w * 0.5f, (float)h * 0.5f};

    if (game->coreSurgeWarning) {
        float urgency = 0.5f + 0.5f * sinf(rendererTime * 18.0f);
        float alpha = 80.0f + 145.0f * urgency;
        DrawCircleLines((int)center.x, (int)center.y, 40.0f + urgency * 10.0f,
                        WithAlpha(COLOR_MAGENTA, alpha));
        DrawLineEx((Vector2){center.x - 65.0f, center.y},
                   (Vector2){center.x + 65.0f, center.y}, 2.0f,
                   WithAlpha(COLOR_MAGENTA, alpha * 0.65f));
        DrawLineEx((Vector2){center.x, center.y - 65.0f},
                   (Vector2){center.x, center.y + 65.0f}, 2.0f,
                   WithAlpha(COLOR_MAGENTA, alpha * 0.65f));
        const char *warning = "CORE SURGE INCOMING";
        DrawText(warning, (w - MeasureText(warning, 20)) / 2, 115, 20,
                 WithAlpha(COLOR_MAGENTA, alpha));
        DrawRectangle((w - 170) / 2, 143, 170, 4, (Color){38, 24, 66, 230});
        float fraction = ClampFloat(game->coreSurgeWarningTimer / 2.8f, 0.0f, 1.0f);
        DrawRectangle((w - 170) / 2, 143, (int)(170.0f * fraction), 4,
                      WithAlpha(COLOR_MAGENTA, alpha));
    }

    if (game->coreSurgeActive) {
        float r = game->coreSurgeRadius;
        DrawCircleLines((int)center.x, (int)center.y, r + 8.0f,
                        (Color){255, 53, 181, 48});
        DrawCircleLines((int)center.x, (int)center.y, r,
                        (Color){67, 239, 255, 225});
        DrawCircleLines((int)center.x, (int)center.y, fmaxf(0.0f, r - 5.0f),
                        (Color){157, 71, 255, 135});
        for (int arc = 0; arc < 12; ++arc) {
            float angle = rendererTime * 0.7f + (TAU * (float)arc / 12.0f);
            float a = angle;
            float b = angle + 0.075f + 0.025f * sinf(rendererTime * 5.0f + arc);
            Vector2 p0 = {center.x + cosf(a) * r, center.y + sinf(a) * r};
            Vector2 p1 = {center.x + cosf(b) * r, center.y + sinf(b) * r};
            DrawLineEx(p0, p1, 4.0f, (arc % 2 == 0) ? COLOR_WHITE : COLOR_MAGENTA);
        }
        DrawText("CORE SURGE  /  EVADE THE RING", 20, 119, 15, COLOR_MAGENTA);
    }
}

static void DrawCornerMarks(void)
{
    int w = GetScreenWidth();
    int h = GetScreenHeight();
    const float pad = 22.0f, len = 28.0f;
    Color c = {67, 239, 255, 110};
    DrawLineEx((Vector2){pad, pad}, (Vector2){pad+len, pad}, 2.0f, c);
    DrawLineEx((Vector2){pad, pad}, (Vector2){pad, pad+len}, 2.0f, c);
    DrawLineEx((Vector2){w-pad, pad}, (Vector2){w-pad-len, pad}, 2.0f, c);
    DrawLineEx((Vector2){w-pad, pad}, (Vector2){w-pad, pad+len}, 2.0f, c);
    DrawLineEx((Vector2){pad, h-pad}, (Vector2){pad+len, h-pad}, 2.0f, c);
    DrawLineEx((Vector2){pad, h-pad}, (Vector2){pad, h-pad-len}, 2.0f, c);
    DrawLineEx((Vector2){w-pad, h-pad}, (Vector2){w-pad-len, h-pad}, 2.0f, c);
    DrawLineEx((Vector2){w-pad, h-pad}, (Vector2){w-pad, h-pad-len}, 2.0f, c);
}

static void DrawTitleScreen(const Game *game)
{
    const int w = GetScreenWidth(), h = GetScreenHeight();
    float pulse = 0.78f + 0.22f * sinf(game->titlePulse * 2.1f);
    const char *title = "RIFT RUNNER";
    const char *subtitle = "SIGNAL ZERO";
    const char *tagline = "OUTRUN THE COLLAPSE. SURVIVE THE SIGNAL.";
    int titleSize = (w < 900) ? 46 : 66;
    int titleWidth = MeasureText(title, titleSize);

    DrawText(title, (w-titleWidth)/2, (int)(h*0.19f), titleSize, COLOR_WHITE);
    DrawRectangle((w-titleWidth)/2, (int)(h*0.19f)+titleSize+9, titleWidth, 2,
                  WithAlpha(C_CYAN, 85.0f + pulse*100.0f));
    DrawText(subtitle, (w-MeasureText(subtitle, 23))/2, (int)(h*0.19f)+titleSize+24, 23, COLOR_MAGENTA);
    DrawText(tagline, (w-MeasureText(tagline, 16))/2, (int)(h*0.19f)+titleSize+63, 16, COLOR_MUTED);
    const char *profileLine = TextFormat("BEST %06d   /   BEST SECTOR %02d   /   RUNS %d",
                                         game->bestScore, game->bestSector, game->runsPlayed);
    DrawText(profileLine, (w-MeasureText(profileLine, 14))/2, (int)(h*0.19f)+titleSize+91, 14, COLOR_CYAN);

    DrawRectangle(34, h-114, 260, 70, COLOR_PANEL);
    DrawRectangleLines(34, h-114, 260, 70, (Color){67,239,255,65});
    DrawText("RIFT SIGNAL", 50, h-99, 12, COLOR_CYAN);
    DrawText("UNSTABLE  /  DETECTED", 50, h-77, 14, COLOR_WHITE);
    DrawRectangle(w-294, h-114, 260, 70, COLOR_PANEL);
    DrawRectangleLines(w-294, h-114, 260, 70, (Color){255,53,181,65});
    DrawText("SYSTEM STATUS", w-278, h-99, 12, COLOR_MAGENTA);
    DrawText("ENGINE ONLINE", w-278, h-77, 14, COLOR_WHITE);

    const char *start = "[ ENTER ]  INITIALIZE RUN";
    int startWidth = MeasureText(start, 19);
    Rectangle btn = {(w-startWidth-42)/2.0f, h*0.77f, startWidth+42.0f, 48.0f};
    DrawRectangleRounded(btn, 0.22f, 10, (Color){7,21,45,215});
    DrawRectangleRoundedLinesEx(btn, 0.22f, 10, 1.4f, WithAlpha(C_CYAN, 110.0f + pulse*120.0f));
    DrawText(start, (w-startWidth)/2, (int)(h*0.77f)+14, 19, COLOR_WHITE);
    const char *hint = "S: SETTINGS  /  EVADE HOSTILE DRONES  /  SPACE OR SHIFT TO DASH";
    DrawText(hint, (w-MeasureText(hint, 12))/2, h-23, 12, (Color){110,127,167,205});
}


static void DrawEnemies(const Game *game)
{
    for (int i = 0; i < RR_MAX_ENEMIES; ++i) {
        const Enemy *enemy = &game->enemies[i];
        if (!enemy->active) continue;

        bool interceptor = (enemy->type == ENEMY_TYPE_INTERCEPTOR);
        Color primary = interceptor ? (Color){255, 151, 67, 255} : COLOR_MAGENTA;
        Color secondary = interceptor ? (Color){255, 224, 142, 255} : COLOR_CYAN;
        if (enemy->hitFlashTimer > 0.0f) primary = COLOR_WHITE;

        Vector2 velocityDir = enemy->velocity;
        float velocityLength = sqrtf(velocityDir.x * velocityDir.x + velocityDir.y * velocityDir.y);
        if (velocityLength > 0.01f) {
            velocityDir.x /= velocityLength;
            velocityDir.y /= velocityLength;
            Vector2 trailEnd = {enemy->position.x - velocityDir.x * (enemy->radius + 13.0f),
                                enemy->position.y - velocityDir.y * (enemy->radius + 13.0f)};
            DrawLineEx(trailEnd, enemy->position, interceptor ? 3.0f : 2.0f,
                       WithAlpha(primary, interceptor ? 100.0f : 70.0f));
        }

        DrawCircleV(enemy->position, enemy->radius + 9.0f, WithAlpha(primary, 20.0f));
        DrawCircleV(enemy->position, enemy->radius + 4.0f, WithAlpha(primary, 42.0f));
        if (interceptor) {
            DrawPoly(enemy->position, 3, enemy->radius + 2.0f,
                     rendererTime * 115.0f + (float)i * 17.0f, primary);
            DrawPoly(enemy->position, 3, enemy->radius * 0.62f,
                     -rendererTime * 140.0f, secondary);
        } else {
            DrawPoly(enemy->position, 6, enemy->radius + 1.5f,
                     -rendererTime * 62.0f + (float)i * 11.0f, primary);
            DrawPoly(enemy->position, 4, enemy->radius * 0.58f,
                     rendererTime * 70.0f, secondary);
        }
        DrawCircleV(enemy->position, enemy->radius * 0.22f, COLOR_WHITE);
    }
}


static void DrawShards(const Game *game)
{
    for (int i = 0; i < RR_MAX_SHARDS; ++i) {
        const EnergyShard *s = &game->shards[i];
        if (!s->active) continue;
        float pulse = 1.0f + 0.18f * sinf(s->phase * 3.0f);
        DrawCircleV(s->position, 12.0f * pulse, (Color){255, 218, 75, 28});
        DrawCircleV(s->position, 7.0f * pulse, (Color){255, 218, 75, 65});
        DrawPoly(s->position, 4, 6.0f * pulse, 45.0f + s->phase * 35.0f,
                 (Color){255, 225, 95, 255});
        DrawCircleV(s->position, 2.0f, COLOR_WHITE);
    }
}

static void DrawUpgradeOverlay(const Game *game)
{
    int w = GetScreenWidth(), h = GetScreenHeight();
    DrawRectangle(0, 0, w, h, (Color){2, 4, 17, 225});
    const char *title = "SECTOR CLEARED";
    DrawText(title, (w - MeasureText(title, 38))/2, h/2 - 145, 38, (Color){80,255,166,255});
    const char *subtitle = "CHOOSE ONE UPGRADE  /  PRESS 1, 2 OR 3";
    DrawText(subtitle, (w - MeasureText(subtitle, 15))/2, h/2 - 92, 15, COLOR_WHITE);
    int cardH = 125, gap = 12;
    int cardW = (w - 64 - gap * 2) / 3;
    if (cardW > 250) cardW = 250;
    int x = (w - (cardW*3 + gap*2))/2;
    int y = h/2 - 45;
    const char *names[3] = {"1  QUICKSTEP", "2  SHARD MAGNET", "3  GUARDIAN"};
    const char *desc[3] = {"Move faster; dash recharges sooner.",
                           "Nearby energy shards move to you.",
                           "Restore one life and gain protection."};
    Color colors[3] = {{67,239,255,255},{255,225,95,255},{80,255,166,255}};
    for (int i=0; i<3; ++i) {
        Rectangle card = {(float)(x+i*(cardW+gap)),(float)y,(float)cardW,(float)cardH};
        DrawRectangleRounded(card, 0.08f, 8, (Color){13,20,47,250});
        DrawRectangleRoundedLinesEx(card, 0.08f, 8, 2.0f, colors[i]);
        DrawText(names[i], (int)card.x+15, (int)card.y+20, 17, colors[i]);
        DrawText(desc[i], (int)card.x+15, (int)card.y+58, 13, COLOR_WHITE);
    }
    DrawText(TextFormat("SECTOR %02d  |  SHARDS %02d  |  SCORE %06d",
        game->sector, game->shardsCollected, game->score),
        (w-MeasureText(TextFormat("SECTOR %02d  |  SHARDS %02d  |  SCORE %06d",
        game->sector, game->shardsCollected, game->score), 15))/2, y+cardH+28, 15, COLOR_CYAN);
}


static void DrawGameHUD(const Game *game)
{
    int w = GetScreenWidth();
    int fontSize = (w < 1000) ? 11 : 13;
    int top = 67;
    DrawRectangle(62, 57, w - 124, 34, (Color){5, 9, 27, 205});
    DrawRectangleLines(62, 57, w - 124, 34, (Color){67, 239, 255, 55});

    DrawText(TextFormat("SCORE %06d", game->score), 72, top, fontSize, COLOR_WHITE);
    DrawText(TextFormat("DRONES %02d", game->enemiesDefeated), (int)(w * 0.20f), top, fontSize, COLOR_CYAN);
    DrawText(TextFormat("SHARDS %02d", game->shardsCollected), (int)(w * 0.31f), top, fontSize, (Color){255,225,95,255});
    DrawText(TextFormat("COMBO x%d", game->combo), (int)(w * 0.42f), top, fontSize,
             game->combo > 3 ? COLOR_MAGENTA : COLOR_WHITE);
    DrawText(TextFormat("SECTOR %02d", game->sector), (int)(w * 0.54f), top, fontSize, COLOR_CYAN);

    int livesX = (int)(w * 0.67f);
    DrawText(TextFormat("LIVES %02d", game->lives > 0 ? game->lives : 0), livesX, top, fontSize, COLOR_MUTED);
    const char *timeLabel = TextFormat("SURVIVED %02d:%02d",
        (int)game->elapsedSeconds / 60, (int)game->elapsedSeconds % 60);
    int timeWidth = MeasureText(timeLabel, fontSize);
    DrawText(timeLabel, w - 74 - timeWidth, top, fontSize, COLOR_WHITE);

    /* At wider sizes, add compact life markers; hide them on small windows to prevent overlap. */
    if (w >= 1000) {
        int markerX = livesX + MeasureText(TextFormat("LIVES %02d", game->lives > 0 ? game->lives : 0), fontSize) + 9;
        for (int i = 0; i < game->lives; ++i) {
            DrawPoly((Vector2){(float)(markerX + i * 12), 74.0f}, 4, 4.0f,
                     45.0f + rendererTime * 35.0f, COLOR_MAGENTA);
        }
    }
}

static void DrawSettingsScreen(const Game *game)
{
    const int w = GetScreenWidth(), h = GetScreenHeight();
    DrawRectangle(0, 0, w, h, (Color){2, 4, 18, 165});
    const char *title = "SYSTEM SETTINGS";
    DrawText(title, (w - MeasureText(title, 39)) / 2, h / 2 - 205, 39, COLOR_CYAN);
    DrawText("TUNE YOUR SIGNAL", (w - MeasureText("TUNE YOUR SIGNAL", 13)) / 2,
             h / 2 - 158, 13, COLOR_MUTED);

    const int rowX = w / 2 - 220;
    const int rowW = 440;
    const int rowH = 46;
    const int rowGap = 51;
    const int rowY = h / 2 - 132;
    for (int i = 0; i < 5; ++i) {
        int y = rowY + i * rowGap;
        bool selected = game->settingsSelection == i;
        Color outline = selected ? WithAlpha(C_CYAN, 175.0f) : (Color){100, 120, 170, 45};
        DrawRectangleRounded((Rectangle){(float)rowX, (float)y, (float)rowW, (float)rowH},
                             0.12f, 8, selected ? (Color){8, 26, 48, 245} : (Color){5, 9, 27, 210});
        DrawRectangleRoundedLinesEx((Rectangle){(float)rowX, (float)y, (float)rowW, (float)rowH},
                                    0.12f, 8, 1.2f, outline);
        const char *label = (i == 0) ? "MUSIC VOLUME" : (i == 1) ? "SFX VOLUME" :
                            (i == 2) ? "AUDIO OUTPUT" :
                            (i == 3) ? "SCREEN SHAKE" : "RETURN TO TITLE";
        DrawText(label, rowX + 18, y + 15, 14, selected ? COLOR_WHITE : COLOR_MUTED);
        if (i < 2) {
            float amount = (i == 0) ? game->musicVolume : game->sfxVolume;
            int bx = rowX + 238, by = y + 19, bw = 140;
            DrawRectangle(bx, by, bw, 9, (Color){28, 40, 70, 255});
            DrawRectangle(bx, by, (int)(bw * ClampFloat(amount, 0.0f, 1.0f)), 9,
                          i == 0 ? COLOR_CYAN : COLOR_MAGENTA);
            DrawText(TextFormat("%d%%", (int)(amount * 100.0f + 0.5f)), bx + bw - 43, y + 4, 12, COLOR_WHITE);
        } else if (i == 2 || i == 3) {
            bool enabled = (i == 2) ? !game->audioMuted : game->screenShakeEnabled;
            const char *state = (i == 2) ? (enabled ? "ON" : "MUTED") : (enabled ? "ON" : "OFF");
            DrawText(state, rowX + rowW - 76, y + 15, 14,
                     enabled ? COLOR_CYAN : COLOR_MAGENTA);
        }
    }

    const char *controls = "UP/DOWN SELECT    LEFT/RIGHT ADJUST    ENTER TOGGLE / CONFIRM";
    DrawText(controls, (w - MeasureText(controls, 11)) / 2,
             rowY + 5 * rowGap + 2, 11, COLOR_MUTED);
    DrawText("ESC  /  BACK TO TITLE", (w - MeasureText("ESC  /  BACK TO TITLE", 12)) / 2,
             h - 32, 12, COLOR_MUTED);
}

static void DrawGameOverOverlay(const Game *game)
{
    int w = GetScreenWidth();
    int h = GetScreenHeight();
    DrawRectangle(0, 0, w, h, (Color){2, 3, 15, 218});

    const char *title = "SIGNAL LOST";
    const char *subtitle = "YOUR SIGNAL HAS COLLAPSED";
    DrawText(title, (w - MeasureText(title, 48)) / 2, h / 2 - 130, 48, COLOR_MAGENTA);
    DrawText(subtitle, (w - MeasureText(subtitle, 16)) / 2, h / 2 - 68, 16, COLOR_MUTED);

    const char *scoreLine = TextFormat("FINAL SCORE   %06d", game->score);
    const char *defeatLine = TextFormat("HOSTILES DISABLED   %d", game->enemiesDefeated);
    const char *timeLine = TextFormat("SURVIVAL TIME   %02d:%02d",
        (int)game->elapsedSeconds / 60, (int)game->elapsedSeconds % 60);
    DrawText(scoreLine, (w - MeasureText(scoreLine, 23)) / 2, h / 2 - 12, 23, COLOR_CYAN);
    DrawText(defeatLine, (w - MeasureText(defeatLine, 17)) / 2, h / 2 + 32, 17, COLOR_WHITE);
    DrawText(timeLine, (w - MeasureText(timeLine, 17)) / 2, h / 2 + 62, 17, COLOR_WHITE);
    const char *bestLine = TextFormat("PERSONAL BEST %06d   /   BEST SECTOR %02d",
                                      game->bestScore, game->bestSector);
    DrawText(bestLine, (w - MeasureText(bestLine, 14)) / 2, h / 2 + 89, 14,
             game->newRecord ? (Color){80,255,166,255} : COLOR_MUTED);
    if (game->newRecord) {
        const char *record = "NEW RUN RECORD!";
        DrawText(record, (w - MeasureText(record, 16)) / 2, h / 2 + 106, 16,
                 (Color){80,255,166,255});
    }

    const char *retry = "ENTER  /  SPACE  -  RETRY RUN";
    const char *menu = "ESC  -  RETURN TO TITLE";
    DrawText(retry, (w - MeasureText(retry, 17)) / 2, h / 2 + 130, 17, COLOR_CYAN);
    DrawText(menu, (w - MeasureText(menu, 14)) / 2, h / 2 + 158, 14, COLOR_MUTED);
}

static void DrawArena(const Game *game)
{
    const int w = GetScreenWidth(), h = GetScreenHeight();
    Rectangle arena = {48.0f, 52.0f, (float)w-96.0f, (float)h-104.0f};
    DrawRectangleRounded(arena, 0.025f, 8, (Color){5,9,26,140});

    /* Neon arena boundary and low-contrast internal scan grid. */
    DrawRectangleRoundedLinesEx(arena, 0.025f, 8, 2.0f, (Color){67,239,255,120});
    Rectangle inner = {arena.x+8.0f, arena.y+8.0f, arena.width-16.0f, arena.height-16.0f};
    DrawRectangleRoundedLinesEx(inner, 0.025f, 8, 1.0f, (Color){157,71,255,45});
    for (int x = (int)arena.x+20; x < (int)(arena.x+arena.width); x += 48)
        DrawLine(x, (int)arena.y+12, x, (int)(arena.y+arena.height)-12, (Color){54,110,180,20});
    for (int y = (int)arena.y+12; y < (int)(arena.y+arena.height); y += 48)
        DrawLine((int)arena.x+12, y, (int)(arena.x+arena.width)-12, y, (Color){54,110,180,20});

    DrawText("WASD / ARROWS MOVE     SPACE / SHIFT PHASE DASH", 66, h-82, 13, COLOR_MUTED);
    DrawText("P / ESC PAUSE     F FULLSCREEN", 66, h-61, 12, COLOR_MUTED);

    DrawCoreSurge(game);
    DrawShards(game);
    DrawEnemies(game);
    const Player *p = &game->player;
    /* Draw afterimages from newest/oldest entries with smooth alpha fade. */
    for (int i = 0; i < RR_PLAYER_TRAIL_COUNT; ++i) {
        const PlayerTrail *trail = &p->trails[i];
        if (trail->life <= 0.0f || trail->maxLife <= 0.0f) continue;
        float alpha = ClampFloat(trail->life / trail->maxLife, 0.0f, 1.0f);
        DrawCircleV(trail->position, trail->radius + 5.0f, WithAlpha(C_CYAN, 15.0f*alpha));
        DrawCircleV(trail->position, trail->radius, WithAlpha(COLOR_VIOLET, 68.0f*alpha));
        DrawCircleV(trail->position, trail->radius*0.45f, WithAlpha(C_CYAN, 125.0f*alpha));
    }

    bool blink = p->invulnerableTimer > 0.0f && ((int)(game->elapsedSeconds * 28.0f) % 2 == 0);
    if (!blink) {
        float dashGlow = p->dashTimer > 0.0f ? 1.0f : 0.0f;
        DrawCircleV(p->position, p->radius+13.0f, (Color){67,239,255,(unsigned char)(20+dashGlow*55)});
        DrawCircleV(p->position, p->radius+7.0f, (Color){157,71,255,45});
        DrawCircleV(p->position, p->radius, C_CYAN);
        DrawCircleV(p->position, p->radius*0.54f, COLOR_WHITE);
        Vector2 tip = {p->position.x + p->facing.x*(p->radius+12.0f),
                       p->position.y + p->facing.y*(p->radius+12.0f)};
        DrawLineEx(p->position, tip, 3.0f, COLOR_WHITE);
        DrawPoly(p->position, 4, p->radius+3.0f, rendererTime*55.0f, WithAlpha(COLOR_MAGENTA, 170.0f));
        DrawCircleV(p->position, p->radius*0.40f, C_CYAN);
    }

    /* Minimal dash HUD: a readable cooldown bar changes when the dash is ready. */
    float barX = w-242.0f, barY = h-72.0f, barW = 170.0f;
    float readiness = 1.0f - ClampFloat(p->dashCooldown / p->dashCooldownDuration, 0.0f, 1.0f);
    DrawText("PHASE DASH", (int)barX, (int)barY-19, 12, COLOR_CYAN);
    DrawRectangle((int)barX, (int)barY, (int)barW, 8, (Color){30,42,78,230});
    DrawRectangle((int)barX, (int)barY, (int)(barW*readiness), 8,
                  readiness > 0.99f ? (Color){67,239,255,255} : (Color){157,71,255,230});
    DrawText(readiness > 0.99f ? DASH_READY_LABEL : "CHARGING...", (int)barX, (int)barY+14, 10, COLOR_MUTED);
    DrawEffectParticles();
    DrawGameHUD(game);
    DrawText(TextFormat("UPGRADES  Q:%d  M:%d  G:%d", game->upgradeLevels[0], game->upgradeLevels[1], game->upgradeLevels[2]), 66, 101, 12, COLOR_MUTED);
}

static void DrawPauseOverlay(void)
{
    int w = GetScreenWidth(), h = GetScreenHeight();
    DrawRectangle(0, 0, w, h, (Color){2,4,15,190});
    DrawText("PAUSED", (w-MeasureText("PAUSED", 42))/2, h/2-45, 42, COLOR_CYAN);
    DrawText("P OR ENTER  /  RESUME", (w-MeasureText("P OR ENTER  /  RESUME", 16))/2,
             h/2+15, 16, COLOR_WHITE);
    DrawText("ESC  /  RETURN TO TITLE", (w-MeasureText("ESC  /  RETURN TO TITLE", 13))/2,
             h/2+48, 13, COLOR_MUTED);
}

void Renderer_Init(void)
{
    unsigned int seed = 0x52494654u;
    for (int i = 0; i < STAR_COUNT; ++i) {
        stars[i].position = (Vector2){Hash01(&seed) * (float)GetScreenWidth(), Hash01(&seed) * (float)GetScreenHeight()};
        stars[i].radius = 0.45f + Hash01(&seed) * 1.45f;
        stars[i].phase = Hash01(&seed) * TAU;
        stars[i].twinkleSpeed = 0.4f + Hash01(&seed) * 2.4f;
        stars[i].driftSpeed = 5.0f + Hash01(&seed) * 22.0f;
        stars[i].layer = (unsigned char)(Hash01(&seed) * 3.0f);
    }
    for (size_t i = 0; i < sizeof(sparks) / sizeof(sparks[0]); ++i) {
        sparks[i].angle = Hash01(&seed) * TAU;
        sparks[i].orbit = 40.0f + Hash01(&seed) * fminf((float)GetScreenWidth(), (float)GetScreenHeight()) * 0.24f;
        sparks[i].speed = (0.12f + Hash01(&seed) * 0.45f) * ((i % 2 == 0) ? 1.0f : -1.0f);
        sparks[i].radius = 0.7f + Hash01(&seed) * 1.8f;
        sparks[i].phase = Hash01(&seed) * TAU;
    }
    rendererTime = 0.0f;
    effectSeed = 0x5EEDC0DEu;
    for (int i = 0; i < EFFECT_PARTICLE_COUNT; ++i) {
        effectParticles[i].active = false;
        effectParticles[i].life = 0.0f;
        effectParticles[i].maxLife = 0.0f;
    }
    previousDashTimer = 0.0f;
    previousLives = 3;
    previousShardCount = 0;
    previousEnemiesDefeated = 0;
    previousSurgeActive = false;
}

void Renderer_Update(float deltaTime)
{
    if (!isfinite(deltaTime) || deltaTime < 0.0f) deltaTime = 0.0f;
    if (deltaTime > 0.05f) deltaTime = 0.05f;
    rendererTime += deltaTime;
    UpdateEffectParticles(deltaTime);
}

void Renderer_Draw(const Game *game)
{
    if (game == NULL) return;

    ObserveGameplayEvents(game);
    ClearBackground(COLOR_VOID);
    DrawGradientBackdrop();
    DrawStars();
    DrawEnergyLanes();
    DrawCosmicRift();
    DrawCornerMarks();

    if (game->screen == GAME_SCREEN_TITLE) {
        DrawTitleScreen(game);
    } else if (game->screen == GAME_SCREEN_SETTINGS) {
        DrawSettingsScreen(game);
    } else {
        if (game->screen == GAME_SCREEN_ARENA && game->screenShakeEnabled && game->screenShakeTimer > 0.0f) {
            float strength = ClampFloat(game->screenShakeTimer / 0.28f, 0.0f, 1.0f);
            float shakeX = sinf(rendererTime * 91.0f) * 7.0f * strength;
            float shakeY = cosf(rendererTime * 113.0f) * 5.0f * strength;
            Camera2D shakeCamera = {0};
            shakeCamera.offset = (Vector2){(float)GetScreenWidth() * 0.5f,
                                           (float)GetScreenHeight() * 0.5f};
            shakeCamera.target = (Vector2){(float)GetScreenWidth() * 0.5f - shakeX,
                                           (float)GetScreenHeight() * 0.5f - shakeY};
            shakeCamera.rotation = 0.0f;
            shakeCamera.zoom = 1.0f;
            BeginMode2D(shakeCamera);
            DrawArena(game);
            EndMode2D();
        } else {
            DrawArena(game);
        }
        if (game->screen == GAME_SCREEN_PAUSED) DrawPauseOverlay();
        if (game->screen == GAME_SCREEN_UPGRADE) DrawUpgradeOverlay(game);
        if (game->screen == GAME_SCREEN_GAME_OVER) DrawGameOverOverlay(game);
        if (game->damageFlashTimer > 0.0f && game->screen == GAME_SCREEN_ARENA) {
            float strength = ClampFloat(game->damageFlashTimer / 0.24f, 0.0f, 1.0f);
            DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(),
                          (Color){255, 30, 68, (unsigned char)(strength * 42.0f)});
        }
        if (game->screen == GAME_SCREEN_ARENA && game->screenShakeEnabled && game->screenShakeTimer > 0.0f) {
            float strength = ClampFloat(game->screenShakeTimer / 0.22f, 0.0f, 1.0f);
            DrawRectangleLinesEx((Rectangle){3.0f, 3.0f, (float)GetScreenWidth()-6.0f,
                              (float)GetScreenHeight()-6.0f}, 3.0f,
                              WithAlpha(COLOR_MAGENTA, 100.0f * strength));
        }
    }
}

void Renderer_Shutdown(void)
{
    /* No GPU resources are allocated yet; later installments will release textures/audio here. */
}
