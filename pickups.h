#ifndef RR_PICKUPS_H
#define RR_PICKUPS_H
#include "raylib.h"
#include <stdbool.h>
/* Pickup types: data points, repairs, and the banana shield pickup. */
typedef enum PickupKind { PICKUP_DATA = 0, PICKUP_REPAIR = 1, PICKUP_BANANA = 2 } PickupKind;
typedef struct Pickup {
    bool active;
    Vector2 pos;
    float phase;
    float life;
    PickupKind kind;
} Pickup;
#endif
