/* Data, repair and banana-shield pickups. */

#include "game.h"
#include <math.h>
#include <stdlib.h>

void AddPickup(Vector2 pos, PickupKind kind)
{
    for (int i = 0; i < MAX_PICKUPS; ++i) {
        if (!g.pickups[i].active) {
            g.pickups[i] = (Pickup){ true, pos, RandF(0.0f, 6.28f), 8.0f, kind };
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
        if (p->life <= 0.0f) { p->active = false; continue; }
        float r = 18.0f;
        if (DistSq(p->pos, g.player.pos) <= r * r) {
            p->active = false;
            if (p->kind == PICKUP_BANANA) {
                g.player.health = MinF(g.player.maxHealth, g.player.health + 18.0f);
                g.player.shieldTime = 5.0f;
                AddFloater(p->pos, 18, C_GOLD);
                Burst(p->pos, C_GOLD, 30, 190.0f, 3.0f);
            } else if (p->kind == PICKUP_DATA) {
                g.score += 50;
                AddFloater(p->pos, 50, C_CYAN);
            } else {
                float before = g.player.health;
                g.player.health = MinF(g.player.maxHealth, g.player.health + 22.0f);
                int actual = (int)(g.player.health - before);
                if (actual > 0) AddFloater(p->pos, actual, C_GREEN);
                g.score += 25;
                AddFloater(Add(p->pos, V2(0, -17)), 25, C_GOLD);
            }
            int oldStage = g.stage;
            g.stage = CurrentStageForScore(g.score);
            if (g.stage > oldStage) { g.stageNotice = 2.0f; PlaySfx(SFX_SURGE); }
            Burst(p->pos, p->kind == PICKUP_BANANA ? C_GOLD : (p->kind == PICKUP_DATA ? C_CYAN : C_GREEN), 14, 120.0f, 2.1f);
            PlaySfx(SFX_PICKUP);
        }
    }
}
