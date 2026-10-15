#ifndef RIFT_RUNNER_GAME_H
#define RIFT_RUNNER_GAME_H

#include "raylib.h"
#include <stdbool.h>

#define RR_PLAYER_TRAIL_COUNT 18
#define RR_MAX_ENEMIES 28
#define RR_MAX_SHARDS 32
#define RR_ARENA_INSET_X 48.0f
#define RR_ARENA_INSET_Y 52.0f

typedef enum GameScreen {
    GAME_SCREEN_TITLE = 0,
    GAME_SCREEN_SETTINGS,
    GAME_SCREEN_ARENA,
    GAME_SCREEN_PAUSED,
    GAME_SCREEN_UPGRADE,
    GAME_SCREEN_GAME_OVER
} GameScreen;

typedef enum EnemyType {
    ENEMY_TYPE_DRONE = 0,
    ENEMY_TYPE_INTERCEPTOR
} EnemyType;

typedef struct PlayerTrail {
    Vector2 position;
    float life;
    float maxLife;
    float radius;
} PlayerTrail;

typedef struct Player {
    Vector2 position;
    Vector2 velocity;
    Vector2 facing;
    float radius;
    float moveSpeed;
    float dashTimer;
    float dashDuration;
    float dashCooldown;
    float dashCooldownDuration;
    float invulnerableTimer;
    float trailSpawnTimer;
    unsigned int trailCursor;
    PlayerTrail trails[RR_PLAYER_TRAIL_COUNT];
} Player;

typedef struct Enemy {
    Vector2 position;
    Vector2 velocity;
    float radius;
    float speed;
    float wobblePhase;
    float hitFlashTimer;
    EnemyType type;
    bool active;
} Enemy;

typedef struct EnergyShard {
    Vector2 position;
    float phase;
    bool active;
} EnergyShard;

typedef struct Game {
    GameScreen screen;
    Player player;
    Enemy enemies[RR_MAX_ENEMIES];
    EnergyShard shards[RR_MAX_SHARDS];
    float elapsedSeconds;
    float titlePulse;
    float transitionTimer;
    float enemySpawnTimer;
    float shardSpawnTimer;
    float comboTimer;
    float coreSurgeTimer;       /* Seconds until the next warning phase. */
    float coreSurgeWarningTimer;/* Countdown during the warning phase. */
    float coreSurgeRadius;      /* Current expanding hazard radius. */
    bool coreSurgeWarning;
    bool coreSurgeActive;
    bool coreSurgeHitPlayer;
    int shardsCollected;
    int combo;
    int sector;
    int upgradeLevels[3];
    bool shardMagnet;
    float musicVolume;
    float sfxVolume;
    bool audioMuted;
    bool screenShakeEnabled;
    int settingsSelection;
    float damageFlashTimer;
    float screenShakeTimer;
    int score;
    int lives;
    int enemiesDefeated;
    /* Persistent profile values loaded from rift_runner_profile.dat. */
    int bestScore;
    int bestSector;
    int runsPlayed;
    int lifetimeShards;
    bool newRecord;
    bool runFinalized;
    bool shouldQuit;
} Game;

void Game_Init(Game *game);
void Game_Update(Game *game, float deltaTime);
/* Persist the profile and finalize an active run when the process exits. */
void Game_FinalizeOnExit(Game *game);

#endif /* RIFT_RUNNER_GAME_H */
