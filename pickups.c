/* Banana-only pickup behavior: a little health plus temporary protection. */

#include "game.h"

void AddPickup(Vector2 pos)
{
    for (int i = 0; i < MAX_PICKUPS; ++i) {
        if (!g.pickups[i].active) {
            g.pickups[i] = (Pickup){ true, pos, RandF(0.0f, 6.28f), 8.0f };
            return;
        }
    }
}

void UpdatePickups(float dt)
{
    for (int i = 0; i < MAX_PICKUPS; ++i) {
        Pickup *p = &g.pickups[i];
        if (!p->active) continue;

        p->life -= dt;
        p->phase += dt * 3.0f;
        if (p->life <= 0.0f) {
            p->active = false;
            continue;
        }

        const float collectRadius = 18.0f;
        if (DistSq(p->pos, g.player.pos) <= collectRadius * collectRadius) {
            p->active = false;
            g.player.health = MinF(g.player.maxHealth, g.player.health + 18.0f);
            g.player.shieldTime = 5.0f;
            Burst(p->pos, C_GOLD, 14, 120.0f, 2.1f);
            PlaySfx(SFX_PICKUP);
        }
    }
}
