#include "game.h"

#include "game.h"
#include <math.h>

void AddBullet(Vector2 pos, Vector2 dir)
{
    dir = Normalize(dir);
    if (LengthSq(dir) < 0.5f) dir = g.player.facing;
    for (int i = 0; i < MAX_BULLETS; ++i) {
        if (!g.bullets[i].active) {
            float stageBoost = MinF((float)(g.stage - 1) * 5.0f, 90.0f);
            float spd = 620.0f + stageBoost;
            g.bullets[i] = (Bullet){ true, Add(pos, Scale(dir, 16.0f)), Scale(dir, spd), 4.0f, 1.32f, (g.combo >= 5) ? C_GOLD : C_CYAN };
            PlaySfx(SFX_SHOT);
            return;
        }
    }
}

void UpdateBullets(float dt)
{
    for (int i = 0; i < MAX_BULLETS; ++i) {
        Bullet *b = &g.bullets[i];
        if (!b->active) continue;
        b->life -= dt;
        b->pos = Add(b->pos, Scale(b->vel, dt));
        if (b->life <= 0.0f || b->pos.x < -30 || b->pos.x > SW() + 30 || b->pos.y < -30 || b->pos.y > SH() + 30) {
            b->active = false;
            continue;
        }
        for (int j = 0; j < MAX_ENEMIES; ++j) {
            Enemy *e = &g.enemies[j];
            if (!e->active) continue;
            float r = e->radius + b->radius;
            if (DistSq(e->pos, b->pos) <= r * r) {
                b->active = false;
                e->hp -= 1.0f;
                e->flash = 0.10f;
                Burst(b->pos, b->color, 4, 85.0f, 1.7f);
                if (e->hp <= 0.0f) KillEnemy(e);
                break;
            }
        }
    }
}
