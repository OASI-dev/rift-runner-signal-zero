/* Particle bursts and small screen feedback effects. */

#include "game.h"
#include <math.h>

void AddParticle(Vector2 pos, Vector2 vel, float radius, float life, Color color)
{
    for (int i = 0; i < MAX_PARTICLES; ++i) {
        if (!g.particles[i].active) {
            g.particles[i] = (Particle){ true, pos, vel, radius, life, life, color };
            return;
        }
    }
}

void Burst(Vector2 pos, Color color, int count, float speed, float radius)
{
    for (int i = 0; i < count; ++i) {
        float angle = RandF(0.0f, 2.0f * PI_F);
        float spd = RandF(speed * 0.35f, speed);
        AddParticle(pos, V2(cosf(angle) * spd, sinf(angle) * spd),
                    RandF(radius * 0.55f, radius), RandF(0.24f, 0.72f), color);
    }
}

void UpdateEffects(float dt)
{
    for (int i = 0; i < MAX_PARTICLES; ++i) {
        Particle *p = &g.particles[i];
        if (!p->active) continue;
        p->life -= dt;
        if (p->life <= 0.0f) { p->active = false; continue; }
        p->pos = Add(p->pos, Scale(p->vel, dt));
        p->vel = Scale(p->vel, 1.0f - MinF(2.8f * dt, 0.22f));
        p->radius *= (1.0f - MinF(0.5f * dt, 0.06f));
    }
    g.screenShake = MaxF(0.0f, g.screenShake - dt);
    g.flashAlpha = MaxF(0.0f, g.flashAlpha - 1.75f * dt);
    g.stageNotice = MaxF(0.0f, g.stageNotice - dt);
    g.backgroundTransition = MaxF(0.0f, g.backgroundTransition - dt);
}
