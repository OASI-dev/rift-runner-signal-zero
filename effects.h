#ifndef RR_EFFECTS_H
#define RR_EFFECTS_H
#include "raylib.h"
#include <stdbool.h>
/* Small particles and floating score numbers. */
typedef struct Particle {
    bool active;
    Vector2 pos;
    Vector2 vel;
    float radius;
    float life;
    float fullLife;
    Color color;
} Particle;
typedef struct Floater {
    bool active;
    Vector2 pos;
    float life;
    int points;
    Color color;
} Floater;
#endif
