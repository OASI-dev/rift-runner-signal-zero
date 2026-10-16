/* Composition of pickups, bullets, enemies, effects and the player. */

#include "game.h"
#include <math.h>

void DrawWorld(void)
{
    for (int i = 0; i < MAX_PICKUPS; ++i) {
        Pickup *p = &g.pickups[i];
        if (!p->active) continue;
        Color c = p->kind == PICKUP_BANANA ? C_GOLD : (p->kind == PICKUP_DATA ? C_CYAN : C_GREEN);
        float r = (p->kind == PICKUP_BANANA ? 11.0f : 6.2f) + sinf(p->phase) * 1.6f;
        DrawCircleGradient(p->pos, 24.0f, WithOpacity(c, 0.27f), (Color){ c.r, c.g, c.b, 0 });
        DrawPoly(p->pos, 4, r + 2.0f, 45.0f, WithOpacity(c, 0.44f));
        DrawPoly(p->pos, 4, r, 45.0f, c);
        DrawCircleV(p->pos, 2.0f, C_TEXT);
    }
    for (int i = 0; i < MAX_BULLETS; ++i) {
        Bullet *b = &g.bullets[i];
        if (!b->active) continue;
        Vector2 tail = Sub(b->pos, Scale(Normalize(b->vel), 16.0f));
        DrawLineEx(tail, b->pos, 3.0f, WithOpacity(b->color, 0.45f));
        DrawCircleGradient(b->pos, 10.0f, WithOpacity(b->color, 0.56f), (Color){ b->color.r, b->color.g, b->color.b, 0 });
        DrawCircleV(b->pos, b->radius, C_CYAN);
    }
    for (int i = 0; i < MAX_ENEMIES; ++i) if (g.enemies[i].active) DrawEnemy(&g.enemies[i]);
    for (int i = 0; i < MAX_PARTICLES; ++i) {
        Particle *p = &g.particles[i];
        if (!p->active) continue;
        float a = ClampF(p->life / p->fullLife, 0.0f, 1.0f);
        DrawCircleV(p->pos, p->radius * (0.36f + 0.64f * a), WithOpacity(p->color, a));
    }
    if (g.state == STATE_PLAYING || g.state == STATE_PAUSED || g.state == STATE_GAME_OVER) DrawRunner();
}
