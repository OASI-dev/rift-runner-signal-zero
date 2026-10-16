#ifndef RR_ENEMY_H
#define RR_ENEMY_H
#include "raylib.h"
#include <stdbool.h>
/* One enemy, including movement/turning state and red-or-blue type. */
typedef struct Enemy {
    bool active;
    Vector2 pos, vel;
    float radius, speed, hp, maxHp;
    float heading, turnClock, turnRate, flash;
    bool blue;
} Enemy;
#endif
