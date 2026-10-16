#ifndef RR_PICKUPS_H
#define RR_PICKUPS_H
#include "raylib.h"
#include <stdbool.h>

/* The banana is the only pickup: it restores up to 20 health and grants an 8-second shield. */
typedef struct Pickup {
    bool active;
    Vector2 pos;
    float phase;
    float life;
} Pickup;
#endif
