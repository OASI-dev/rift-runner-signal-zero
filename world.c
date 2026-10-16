/* Composition of pickups, bullets, enemies, effects and the player. */

#include "game.h"
#include <math.h>

static Vector2 RotateLocal(Vector2 center, Vector2 local, float angle)
{
    float c = cosf(angle), s = sinf(angle);
    return V2(center.x + local.x * c - local.y * s,
              center.y + local.x * s + local.y * c);
}

/* Draw a recognizable golden crescent, not the old generic diamond icon. */
static void DrawBananaPickup(const Pickup *p, float r)
{
    const int segments = 12;
    float angle = -0.48f + 0.10f * sinf(p->phase * 0.5f);
    Vector2 outer[segments + 1], inner[segments + 1];
    for (int i = 0; i <= segments; ++i) {
        float t = (float)i / (float)segments;
        float x = (t - 0.5f) * 2.0f * r;
        float curve = sinf(PI_F * t);
        float yOuter = -0.10f * r - 0.52f * r * curve;
        float yInner = yOuter + 0.34f * r * curve;
        outer[i] = RotateLocal(p->pos, V2(x, yOuter), angle);
        inner[i] = RotateLocal(p->pos, V2(x, yInner), angle);
    }
    DrawCircleGradient(p->pos, r * 2.5f, WithOpacity(C_GOLD, 0.32f), WithOpacity(C_GOLD, 0.0f));
    for (int i = 0; i < segments; ++i) {
        DrawTriangle(outer[i], outer[i + 1], inner[i + 1], (Color){ 255, 191, 54, 255 });
        DrawTriangle(outer[i], inner[i + 1], inner[i], (Color){ 255, 226, 104, 255 });
        DrawLineEx(outer[i], outer[i + 1], 1.7f, (Color){ 255, 246, 173, 255 });
        DrawLineEx(inner[i], inner[i + 1], 1.0f, (Color){ 204, 130, 28, 255 });
    }
    DrawCircleV(outer[0], 2.0f, (Color){ 255, 238, 143, 255 });
    DrawCircleV(outer[segments], 2.0f, (Color){ 255, 238, 143, 255 });
}

static void DrawPickup(const Pickup *p)
{
    float pulse = 1.0f + sinf(p->phase * 1.6f) * 0.08f;
    DrawBananaPickup(p, 12.0f * pulse);
}

void DrawWorld(void)
{
    for (int i = 0; i < MAX_PICKUPS; ++i) {
        const Pickup *p = &g.pickups[i];
        if (p->active) DrawPickup(p);
    }

    for (int i = 0; i < MAX_BULLETS; ++i) {
        Bullet *b = &g.bullets[i];
        if (!b->active) continue;
        Vector2 tail = Sub(b->pos, Scale(Normalize(b->vel), 16.0f));
        DrawLineEx(tail, b->pos, 3.0f, WithOpacity(b->color, 0.45f));
        DrawCircleGradient(b->pos, 10.0f, WithOpacity(b->color, 0.56f),
                           (Color){ b->color.r, b->color.g, b->color.b, 0 });
        DrawCircleV(b->pos, b->radius, C_CYAN);
    }

    for (int i = 0; i < MAX_ENEMIES; ++i) {
        if (g.enemies[i].active) DrawEnemy(&g.enemies[i]);
    }

    for (int i = 0; i < MAX_PARTICLES; ++i) {
        Particle *p = &g.particles[i];
        if (!p->active) continue;
        float a = ClampF(p->life / p->fullLife, 0.0f, 1.0f);
        DrawCircleV(p->pos, p->radius * (0.36f + 0.64f * a), WithOpacity(p->color, a));
    }

    if (g.state == STATE_PLAYING || g.state == STATE_PAUSED || g.state == STATE_GAME_OVER) DrawRunner();
}
