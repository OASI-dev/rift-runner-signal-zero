#ifndef RR_EFFECTS_H
#define RR_EFFECTS_H
#include "raylib.h"
#include <stdbool.h>

/* Lightweight visual particles for hits, impacts, and transitions. */
typedef struct Particle {
    bool active;
    Vector2 pos;
    Vector2 vel;
    float radius;
    float life;
    float fullLife;
    Color color;
} Particle;
#endif
