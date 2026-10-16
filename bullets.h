#ifndef RR_BULLETS_H
#define RR_BULLETS_H
#include "raylib.h"
#include <stdbool.h>
/* A player blaster shot. */
typedef struct Bullet {
    bool active;
    Vector2 pos;
    Vector2 vel;
    float radius;
    float life;
    Color color;
} Bullet;
#endif
