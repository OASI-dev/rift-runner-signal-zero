#ifndef RR_PLAYER_H
#define RR_PLAYER_H
#include "raylib.h"
#include <stdbool.h>
/* The player state: position, movement, health, shield and animation. */
typedef struct Player {
    Vector2 pos;
    Vector2 facing;
    Vector2 velocity;
    float health;
    float maxHealth;
    float fireClock;
    float shieldTime;
    float invulnerable;
    float animation;
} Player;
#endif
