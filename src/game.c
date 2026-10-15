#include "game.h"
#include "persistence.h"

#include <math.h>
#include <stddef.h>
#include <limits.h>

#define RR_DASH_SPEED 790.0f
#define RR_DASH_DURATION 0.22f
#define RR_DASH_COOLDOWN 1.28f
#define RR_DASH_TRAIL_INTERVAL 0.022f
#define RR_PLAYER_START_LIVES 3
#define RR_DAMAGE_INVULNERABILITY 1.25f

static float ClampFloat(float value, float low, float high)
{
    if (value < low) return low;
    if (value > high) return high;
    return value;
}

/* Persistent/run counters must not invoke signed-overflow undefined behavior. */
static int SaturatingAddNonNegative(int value, int amount)
{
    if (value < 0) value = 0;
    if (amount <= 0) return value;
    if (value > INT_MAX - amount) return INT_MAX;
    return value + amount;
}

static int ShardComboAward(int combo)
{
    if (combo <= 1) return 10;
    if (combo - 1 > (INT_MAX - 10) / 3) return INT_MAX;
    return 10 + (combo - 1) * 3;
}

static Vector2 Add(Vector2 a, Vector2 b)
{
    return (Vector2){a.x + b.x, a.y + b.y};
}

static Vector2 Subtract(Vector2 a, Vector2 b)
{
    return (Vector2){a.x - b.x, a.y - b.y};
}

static Vector2 Scale(Vector2 value, float amount)
{
    return (Vector2){value.x * amount, value.y * amount};
}

static float Length(Vector2 value)
{
    return sqrtf(value.x * value.x + value.y * value.y);
}

static Vector2 Normalize(Vector2 value)
{
    float length = Length(value);
    if (length < 0.0001f) return (Vector2){0.0f, 0.0f};
    return Scale(value, 1.0f / length);
}

static bool CirclesOverlap(Vector2 a, float radiusA, Vector2 b, float radiusB)
{
    float dx = a.x - b.x;
    float dy = a.y - b.y;
    float radius = radiusA + radiusB;
    return dx * dx + dy * dy <= radius * radius;
}

static void ResetPlayer(Player *player)
{
    if (player == NULL) return;

    player->position = (Vector2){GetScreenWidth() * 0.5f, GetScreenHeight() * 0.5f};
    player->velocity = (Vector2){0.0f, 0.0f};
    player->facing = (Vector2){1.0f, 0.0f};
    player->radius = 12.0f;
    player->moveSpeed = 285.0f;
    player->dashTimer = 0.0f;
    player->dashDuration = RR_DASH_DURATION;
    player->dashCooldown = 0.0f;
    player->dashCooldownDuration = RR_DASH_COOLDOWN;
    player->invulnerableTimer = 0.0f;
    player->trailSpawnTimer = 0.0f;
    player->trailCursor = 0;

    for (int i = 0; i < RR_PLAYER_TRAIL_COUNT; ++i) {
        player->trails[i].position = player->position;
        player->trails[i].life = 0.0f;
        player->trails[i].maxLife = 0.20f;
        player->trails[i].radius = 7.0f;
    }
}

static void ResetRun(Game *game)
{
    if (game == NULL) return;

    ResetPlayer(&game->player);
    for (int i = 0; i < RR_MAX_ENEMIES; ++i) {
        game->enemies[i].position = (Vector2){0.0f, 0.0f};
        game->enemies[i].velocity = (Vector2){0.0f, 0.0f};
        game->enemies[i].radius = 12.0f;
        game->enemies[i].speed = 0.0f;
        game->enemies[i].wobblePhase = 0.0f;
        game->enemies[i].hitFlashTimer = 0.0f;
        game->enemies[i].type = ENEMY_TYPE_DRONE;
        game->enemies[i].active = false;
    }

    game->elapsedSeconds = 0.0f;
    game->transitionTimer = 0.0f;
    game->enemySpawnTimer = 1.15f;
    game->shardSpawnTimer = 0.6f;
    game->comboTimer = 0.0f;
    game->coreSurgeTimer = 18.0f;
    game->coreSurgeWarningTimer = 0.0f;
    game->coreSurgeRadius = 0.0f;
    game->coreSurgeWarning = false;
    game->coreSurgeActive = false;
    game->coreSurgeHitPlayer = false;
    game->shardsCollected = 0;
    game->combo = 0;
    game->sector = 1;
    game->shardMagnet = false;
    for (int i = 0; i < 3; ++i) game->upgradeLevels[i] = 0;
    for (int i = 0; i < RR_MAX_SHARDS; ++i) {
        game->shards[i].position = (Vector2){0.0f, 0.0f};
        game->shards[i].phase = 0.0f;
        game->shards[i].active = false;
    }
    game->damageFlashTimer = 0.0f;
    game->screenShakeTimer = 0.0f;
    game->score = 0;
    game->lives = RR_PLAYER_START_LIVES;
    game->enemiesDefeated = 0;
}

static void AddDashTrail(Player *player)
{
    PlayerTrail *trail = &player->trails[player->trailCursor];
    player->trailCursor = (player->trailCursor + 1U) % RR_PLAYER_TRAIL_COUNT;
    trail->position = player->position;
    trail->life = 0.20f;
    trail->maxLife = 0.20f;
    trail->radius = player->radius * (0.60f + (float)(player->trailCursor % 4U) * 0.08f);
}

static Vector2 ReadMovementInput(void)
{
    Vector2 movement = {0.0f, 0.0f};
    if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) movement.x -= 1.0f;
    if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) movement.x += 1.0f;
    if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)) movement.y -= 1.0f;
    if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) movement.y += 1.0f;
    return Normalize(movement);
}

static void UpdateTrails(Player *player, float deltaTime)
{
    for (int i = 0; i < RR_PLAYER_TRAIL_COUNT; ++i) {
        if (player->trails[i].life > 0.0f) {
            player->trails[i].life -= deltaTime;
            if (player->trails[i].life < 0.0f) player->trails[i].life = 0.0f;
        }
    }
}

static void ClampPlayerToArena(Player *player)
{
    float minX = RR_ARENA_INSET_X + player->radius;
    float minY = RR_ARENA_INSET_Y + player->radius;
    float maxX = (float)GetScreenWidth() - RR_ARENA_INSET_X - player->radius;
    float maxY = (float)GetScreenHeight() - RR_ARENA_INSET_Y - player->radius;

    if (maxX < minX) maxX = minX;
    if (maxY < minY) maxY = minY;

    player->position.x = ClampFloat(player->position.x, minX, maxX);
    player->position.y = ClampFloat(player->position.y, minY, maxY);

    if (player->position.x <= minX && player->velocity.x < 0.0f) player->velocity.x = 0.0f;
    if (player->position.x >= maxX && player->velocity.x > 0.0f) player->velocity.x = 0.0f;
    if (player->position.y <= minY && player->velocity.y < 0.0f) player->velocity.y = 0.0f;
    if (player->position.y >= maxY && player->velocity.y > 0.0f) player->velocity.y = 0.0f;
}

static void BeginDash(Player *player, Vector2 movement)
{
    Vector2 direction = movement;
    if (Length(direction) < 0.001f) direction = player->facing;
    if (Length(direction) < 0.001f) direction = (Vector2){1.0f, 0.0f};

    direction = Normalize(direction);
    player->facing = direction;
    player->velocity = Scale(direction, RR_DASH_SPEED);
    player->dashTimer = player->dashDuration;
    player->dashCooldown = player->dashCooldownDuration;
    player->invulnerableTimer = player->dashDuration + 0.04f;
    player->trailSpawnTimer = 0.0f;
    AddDashTrail(player);
}

static void UpdatePlayer(Game *game, float deltaTime)
{
    Player *player = &game->player;
    Vector2 movement = ReadMovementInput();
    UpdateTrails(player, deltaTime);

    if (player->dashCooldown > 0.0f) {
        player->dashCooldown -= deltaTime;
        if (player->dashCooldown < 0.0f) player->dashCooldown = 0.0f;
    }
    if (player->dashTimer > 0.0f) {
        player->dashTimer -= deltaTime;
        if (player->dashTimer < 0.0f) player->dashTimer = 0.0f;
    }
    if (player->invulnerableTimer > 0.0f) {
        player->invulnerableTimer -= deltaTime;
        if (player->invulnerableTimer < 0.0f) player->invulnerableTimer = 0.0f;
    }

    if ((IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_LEFT_SHIFT)) &&
        player->dashCooldown <= 0.0f) {
        BeginDash(player, movement);
    }

    if (Length(movement) > 0.001f && player->dashTimer <= 0.0f) {
        player->facing = movement;
    }

    if (player->dashTimer > 0.0f) {
        player->trailSpawnTimer -= deltaTime;
        if (player->trailSpawnTimer <= 0.0f) {
            AddDashTrail(player);
            player->trailSpawnTimer = RR_DASH_TRAIL_INTERVAL;
        }
    } else {
        Vector2 targetVelocity = Scale(movement, player->moveSpeed);
        float blend = ClampFloat(13.5f * deltaTime, 0.0f, 1.0f);
        player->velocity.x += (targetVelocity.x - player->velocity.x) * blend;
        player->velocity.y += (targetVelocity.y - player->velocity.y) * blend;
        if (Length(movement) < 0.001f && Length(player->velocity) < 8.0f) {
            player->velocity = (Vector2){0.0f, 0.0f};
        }
    }

    player->position = Add(player->position, Scale(player->velocity, deltaTime));
    ClampPlayerToArena(player);
}

static int ActiveEnemyCount(const Game *game)
{
    int count = 0;
    for (int i = 0; i < RR_MAX_ENEMIES; ++i) {
        if (game->enemies[i].active) ++count;
    }
    return count;
}

static void SpawnEnemy(Game *game)
{
    int slot = -1;
    for (int i = 0; i < RR_MAX_ENEMIES; ++i) {
        if (!game->enemies[i].active) {
            slot = i;
            break;
        }
    }
    if (slot < 0) return;

    Enemy *enemy = &game->enemies[slot];
    float minX = RR_ARENA_INSET_X + 18.0f;
    float minY = RR_ARENA_INSET_Y + 18.0f;
    float maxX = (float)GetScreenWidth() - RR_ARENA_INSET_X - 18.0f;
    float maxY = (float)GetScreenHeight() - RR_ARENA_INSET_Y - 18.0f;
    int side = GetRandomValue(0, 3);

    if (side == 0) enemy->position = (Vector2){minX, (float)GetRandomValue((int)minY, (int)maxY)};
    else if (side == 1) enemy->position = (Vector2){maxX, (float)GetRandomValue((int)minY, (int)maxY)};
    else if (side == 2) enemy->position = (Vector2){(float)GetRandomValue((int)minX, (int)maxX), minY};
    else enemy->position = (Vector2){(float)GetRandomValue((int)minX, (int)maxX), maxY};

    /* Faster interceptors appear after the player has survived for a while. */
    bool canSpawnInterceptor = game->elapsedSeconds >= 15.0f;
    enemy->type = (canSpawnInterceptor && GetRandomValue(0, 99) < 24)
                    ? ENEMY_TYPE_INTERCEPTOR : ENEMY_TYPE_DRONE;
    enemy->radius = (enemy->type == ENEMY_TYPE_INTERCEPTOR) ? 9.0f : 12.0f;
    enemy->speed = 78.0f + fminf(game->elapsedSeconds * 1.25f, 105.0f);
    if (enemy->type == ENEMY_TYPE_INTERCEPTOR) enemy->speed *= 1.38f;
    enemy->wobblePhase = (float)GetRandomValue(0, 6283) / 1000.0f;
    enemy->hitFlashTimer = 0.0f;
    enemy->velocity = (Vector2){0.0f, 0.0f};
    enemy->active = true;
}

static void DamagePlayer(Game *game, Enemy *enemy)
{
    Player *player = &game->player;
    if (player->invulnerableTimer > 0.0f || player->dashTimer > 0.0f) return;

    game->lives--;
    player->invulnerableTimer = RR_DAMAGE_INVULNERABILITY;
    game->damageFlashTimer = 0.24f;
    game->screenShakeTimer = 0.22f;

    Vector2 away = Normalize(Subtract(player->position, enemy->position));
    if (Length(away) < 0.001f) away = (Vector2){1.0f, 0.0f};
    player->velocity = Scale(away, 340.0f);
    enemy->position = Add(enemy->position, Scale(away, -12.0f));

    if (game->lives <= 0) {
        game->lives = 0;
        game->screen = GAME_SCREEN_GAME_OVER;
    }
}

static void UpdateEnemies(Game *game, float deltaTime)
{
    Player *player = &game->player;

    for (int i = 0; i < RR_MAX_ENEMIES; ++i) {
        Enemy *enemy = &game->enemies[i];
        if (!enemy->active) continue;

        if (enemy->hitFlashTimer > 0.0f) {
            enemy->hitFlashTimer -= deltaTime;
            if (enemy->hitFlashTimer < 0.0f) enemy->hitFlashTimer = 0.0f;
        }

        Vector2 toPlayer = Normalize(Subtract(player->position, enemy->position));
        if (Length(toPlayer) < 0.001f) toPlayer = (Vector2){1.0f, 0.0f};
        Vector2 sideways = (Vector2){-toPlayer.y, toPlayer.x};
        float weave = sinf(game->elapsedSeconds * (enemy->type == ENEMY_TYPE_INTERCEPTOR ? 4.1f : 2.6f)
                           + enemy->wobblePhase) * (enemy->type == ENEMY_TYPE_INTERCEPTOR ? 0.20f : 0.34f);
        Vector2 steering = Normalize(Add(toPlayer, Scale(sideways, weave)));
        enemy->velocity = Scale(steering, enemy->speed);
        enemy->position = Add(enemy->position, Scale(enemy->velocity, deltaTime));

        /* Keep every enemy in the visible arena. */
        float minX = RR_ARENA_INSET_X + enemy->radius;
        float minY = RR_ARENA_INSET_Y + enemy->radius;
        float maxX = (float)GetScreenWidth() - RR_ARENA_INSET_X - enemy->radius;
        float maxY = (float)GetScreenHeight() - RR_ARENA_INSET_Y - enemy->radius;
        enemy->position.x = ClampFloat(enemy->position.x, minX, fmaxf(minX, maxX));
        enemy->position.y = ClampFloat(enemy->position.y, minY, fmaxf(minY, maxY));

        if (!CirclesOverlap(player->position, player->radius, enemy->position, enemy->radius)) continue;

        if (player->dashTimer > 0.0f) {
            enemy->active = false;
            game->score = SaturatingAddNonNegative(
                game->score, (enemy->type == ENEMY_TYPE_INTERCEPTOR) ? 45 : 25);
            game->enemiesDefeated = SaturatingAddNonNegative(game->enemiesDefeated, 1);
            game->screenShakeTimer = fmaxf(game->screenShakeTimer, 0.055f);
        } else if (player->invulnerableTimer <= 0.0f) {
            DamagePlayer(game, enemy);
            if (game->screen == GAME_SCREEN_GAME_OVER) return;
        } else {
            /* Separate overlapping actors while the player is protected. */
            Vector2 away = Normalize(Subtract(enemy->position, player->position));
            if (Length(away) < 0.001f) away = (Vector2){-1.0f, 0.0f};
            enemy->position = Add(enemy->position, Scale(away, 65.0f * deltaTime + 2.0f));
        }
    }
}

static void UpdateEnemySpawning(Game *game, float deltaTime)
{
    game->enemySpawnTimer -= deltaTime;
    if (game->enemySpawnTimer > 0.0f) return;

    if (ActiveEnemyCount(game) < RR_MAX_ENEMIES) SpawnEnemy(game);

    /* Pressure rises over time but never falls below a readable reaction window. */
    float interval = ClampFloat(1.35f - game->elapsedSeconds * 0.009f, 0.42f, 1.35f);
    game->enemySpawnTimer = interval;
}


static void SpawnShard(Game *game)
{
    for (int i = 0; i < RR_MAX_SHARDS; ++i) {
        EnergyShard *shard = &game->shards[i];
        if (shard->active) continue;
        float minX = RR_ARENA_INSET_X + 24.0f;
        float minY = RR_ARENA_INSET_Y + 24.0f;
        float maxX = fmaxf(minX, (float)GetScreenWidth() - RR_ARENA_INSET_X - 24.0f);
        float maxY = fmaxf(minY, (float)GetScreenHeight() - RR_ARENA_INSET_Y - 24.0f);
        shard->position = (Vector2){(float)GetRandomValue((int)minX, (int)maxX),
                                    (float)GetRandomValue((int)minY, (int)maxY)};
        shard->phase = (float)GetRandomValue(0, 6283) / 1000.0f;
        shard->active = true;
        return;
    }
}

static void UpdateShards(Game *game, float dt)
{
    game->shardSpawnTimer -= dt;
    if (game->shardSpawnTimer <= 0.0f) {
        SpawnShard(game);
        game->shardSpawnTimer = (float)GetRandomValue(90, 150) / 100.0f;
    }
    if (game->comboTimer > 0.0f) game->comboTimer -= dt;
    else game->combo = 0;

    for (int i = 0; i < RR_MAX_SHARDS; ++i) {
        EnergyShard *s = &game->shards[i];
        if (!s->active) continue;
        s->phase += dt * 2.5f;
        float dx = game->player.position.x - s->position.x;
        float dy = game->player.position.y - s->position.y;
        float dist = sqrtf(dx*dx + dy*dy);
        if (game->shardMagnet && dist < 170.0f && dist > 0.01f) {
            s->position.x += (dx / dist) * 180.0f * dt;
            s->position.y += (dy / dist) * 180.0f * dt;
        }
        float r = game->player.radius + 8.0f;
        dx = game->player.position.x - s->position.x;
        dy = game->player.position.y - s->position.y;
        if (dx*dx + dy*dy <= r*r) {
            s->active = false;
            game->shardsCollected = SaturatingAddNonNegative(game->shardsCollected, 1);
            game->combo = SaturatingAddNonNegative(game->combo, 1);
            game->comboTimer = 2.8f;
            game->score = SaturatingAddNonNegative(game->score, ShardComboAward(game->combo));
            if (game->shardsCollected % 8 == 0) {
                game->sector = SaturatingAddNonNegative(game->sector, 1);
                game->screen = GAME_SCREEN_UPGRADE;
                return;
            }
        }
    }
}

static void DamagePlayerFromCoreSurge(Game *game)
{
    Player *player = &game->player;
    if (player->invulnerableTimer > 0.0f || player->dashTimer > 0.0f) return;

    game->lives--;
    player->invulnerableTimer = RR_DAMAGE_INVULNERABILITY;
    game->damageFlashTimer = 0.30f;
    game->screenShakeTimer = 0.28f;

    Vector2 center = {(float)GetScreenWidth() * 0.5f,
                      (float)GetScreenHeight() * 0.5f};
    Vector2 away = Normalize(Subtract(player->position, center));
    if (Length(away) < 0.001f) away = (Vector2){0.0f, -1.0f};
    player->velocity = Scale(away, 265.0f);

    if (game->lives <= 0) {
        game->lives = 0;
        game->screen = GAME_SCREEN_GAME_OVER;
    }
}

static void UpdateCoreSurge(Game *game, float deltaTime)
{
    if (!game->coreSurgeActive && !game->coreSurgeWarning) {
        game->coreSurgeTimer -= deltaTime;
        if (game->coreSurgeTimer <= 0.0f) {
            game->coreSurgeWarning = true;
            game->coreSurgeWarningTimer = 2.8f;
        }
        return;
    }

    if (game->coreSurgeWarning) {
        game->coreSurgeWarningTimer -= deltaTime;
        if (game->coreSurgeWarningTimer <= 0.0f) {
            game->coreSurgeWarning = false;
            game->coreSurgeActive = true;
            game->coreSurgeHitPlayer = false;
            /* Start outside the tiny centre area so a centred spawn isn't auto-hit. */
            game->coreSurgeRadius = 22.0f;
        }
        return;
    }

    if (!game->coreSurgeActive) return;

    float previousRadius = game->coreSurgeRadius;
    game->coreSurgeRadius += 255.0f * deltaTime;
    Vector2 center = {(float)GetScreenWidth() * 0.5f,
                      (float)GetScreenHeight() * 0.5f};
    float playerDistance = Length(Subtract(game->player.position, center));
    float halfBand = game->player.radius + 7.0f;

    /* Detect crossing the player's radial distance even when a frame skips ahead. */
    if (!game->coreSurgeHitPlayer &&
        previousRadius <= playerDistance + halfBand &&
        game->coreSurgeRadius >= fmaxf(0.0f, playerDistance - halfBand)) {
        DamagePlayerFromCoreSurge(game);
        game->coreSurgeHitPlayer = true;
    }

    float maxRadius = sqrtf((float)GetScreenWidth() * (float)GetScreenWidth() +
                            (float)GetScreenHeight() * (float)GetScreenHeight());
    if (game->coreSurgeRadius > maxRadius) {
        game->coreSurgeActive = false;
        game->coreSurgeRadius = 0.0f;
        game->coreSurgeTimer = fmaxf(12.0f, 21.0f - (float)game->sector * 0.55f);
    }
}

static void ApplyUpgrade(Game *game, int choice)
{
    if (choice == 0) {
        game->upgradeLevels[0]++;
        game->player.moveSpeed += 22.0f;
        game->player.dashCooldownDuration = fmaxf(0.55f, game->player.dashCooldownDuration - 0.12f);
    } else if (choice == 1) {
        game->upgradeLevels[1]++;
        game->shardMagnet = true;
    } else {
        game->upgradeLevels[2]++;
        if (game->lives < 5) game->lives++;
        game->player.invulnerableTimer = 1.2f;
    }
    game->screen = GAME_SCREEN_ARENA;
}


static void FinalizeRun(Game *game)
{
    if (game == NULL || game->runFinalized) return;
    bool scoreRecord = game->score > game->bestScore;
    bool sectorRecord = game->sector > game->bestSector;
    game->newRecord = scoreRecord || sectorRecord;
    if (scoreRecord) game->bestScore = game->score;
    if (sectorRecord) game->bestSector = game->sector;
    game->lifetimeShards = SaturatingAddNonNegative(game->lifetimeShards, game->shardsCollected);
    game->runFinalized = true;
    (void)Profile_Save(game);
}

void Game_FinalizeOnExit(Game *game)
{
    if (game == NULL) return;

    /* Closing the window mid-run should record progress just like leaving via pause. */
    if (!game->runFinalized &&
        (game->screen == GAME_SCREEN_ARENA ||
         game->screen == GAME_SCREEN_UPGRADE ||
         game->screen == GAME_SCREEN_PAUSED)) {
        FinalizeRun(game);
    } else {
        (void)Profile_Save(game);
    }
}

void Game_Init(Game *game)
{
    if (game == NULL) return;

    game->screen = GAME_SCREEN_TITLE;
    game->titlePulse = 0.0f;
    game->shouldQuit = false;
    game->musicVolume = 0.32f;
    game->sfxVolume = 0.65f;
    game->audioMuted = false;
    game->screenShakeEnabled = true;
    game->settingsSelection = 0;
    game->bestScore = 0;
    game->bestSector = 1;
    game->runsPlayed = 0;
    game->lifetimeShards = 0;
    game->newRecord = false;
    game->runFinalized = true;
    ResetRun(game);
    Profile_Load(game);
}

void Game_Update(Game *game, float deltaTime)
{
    if (game == NULL) return;

    /* Bad frame-time values should never propagate NaNs into every timer/position. */
    if (!isfinite(deltaTime) || deltaTime < 0.0f) deltaTime = 0.0f;
    if (deltaTime > 0.05f) deltaTime = 0.05f;

    game->titlePulse += deltaTime;
    if (game->damageFlashTimer > 0.0f) {
        game->damageFlashTimer -= deltaTime;
        if (game->damageFlashTimer < 0.0f) game->damageFlashTimer = 0.0f;
    }
    if (game->screenShakeTimer > 0.0f) {
        game->screenShakeTimer -= deltaTime;
        if (game->screenShakeTimer < 0.0f) game->screenShakeTimer = 0.0f;
    }

    if (IsKeyPressed(KEY_F)) ToggleFullscreen();

    if (IsKeyPressed(KEY_ESCAPE)) {
        if (game->screen == GAME_SCREEN_SETTINGS) { game->screen = GAME_SCREEN_TITLE; return; }
        if (game->screen == GAME_SCREEN_ARENA) {
            game->screen = GAME_SCREEN_PAUSED;
            return;
        }
        /* Escape intentionally has no branch for GAME_SCREEN_UPGRADE: a reward
           must be chosen instead of being silently discarded. */
        if (game->screen == GAME_SCREEN_PAUSED || game->screen == GAME_SCREEN_GAME_OVER) {
            if (game->screen == GAME_SCREEN_PAUSED) FinalizeRun(game);
            game->screen = GAME_SCREEN_TITLE;
            return;
        }
        if (game->screen == GAME_SCREEN_TITLE) {
            game->shouldQuit = true;
            return;
        }
    }

    switch (game->screen) {
        case GAME_SCREEN_TITLE:
            if (IsKeyPressed(KEY_S)) {
                game->screen = GAME_SCREEN_SETTINGS;
            } else if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
                ResetRun(game);
                game->newRecord = false;
                game->runFinalized = false;
                game->runsPlayed = SaturatingAddNonNegative(game->runsPlayed, 1);
                (void)Profile_Save(game);
                game->screen = GAME_SCREEN_ARENA;
            }
            break;

        case GAME_SCREEN_SETTINGS:
            if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) {
                game->settingsSelection = (game->settingsSelection + 4) % 5;
            } else if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) {
                game->settingsSelection = (game->settingsSelection + 1) % 5;
            } else if (game->settingsSelection == 0 && (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_RIGHT))) {
                float step = IsKeyPressed(KEY_RIGHT) ? 0.05f : -0.05f;
                game->musicVolume = ClampFloat(game->musicVolume + step, 0.0f, 1.0f);
                (void)Profile_Save(game);
            } else if (game->settingsSelection == 1 && (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_RIGHT))) {
                float step = IsKeyPressed(KEY_RIGHT) ? 0.05f : -0.05f;
                game->sfxVolume = ClampFloat(game->sfxVolume + step, 0.0f, 1.0f);
                (void)Profile_Save(game);
            } else if (game->settingsSelection == 2 &&
                       (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_RIGHT))) {
                game->audioMuted = !game->audioMuted;
                (void)Profile_Save(game);
            } else if (game->settingsSelection == 3 &&
                       (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_RIGHT))) {
                game->screenShakeEnabled = !game->screenShakeEnabled;
                (void)Profile_Save(game);
            } else if (game->settingsSelection == 4 && (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE))) {
                game->screen = GAME_SCREEN_TITLE;
            }
            break;

        case GAME_SCREEN_ARENA:
            if (IsKeyPressed(KEY_P)) {
                game->screen = GAME_SCREEN_PAUSED;
                break;
            }
            game->elapsedSeconds += deltaTime;
            UpdatePlayer(game, deltaTime);
            UpdateEnemySpawning(game, deltaTime);
            UpdateEnemies(game, deltaTime);
            if (game->screen == GAME_SCREEN_ARENA) UpdateShards(game, deltaTime);
            if (game->screen == GAME_SCREEN_ARENA) UpdateCoreSurge(game, deltaTime);
            break;

        case GAME_SCREEN_UPGRADE:
            if (IsKeyPressed(KEY_ONE) || IsKeyPressed(KEY_KP_1)) ApplyUpgrade(game, 0);
            else if (IsKeyPressed(KEY_TWO) || IsKeyPressed(KEY_KP_2)) ApplyUpgrade(game, 1);
            else if (IsKeyPressed(KEY_THREE) || IsKeyPressed(KEY_KP_3)) ApplyUpgrade(game, 2);
            break;

        case GAME_SCREEN_PAUSED:
            if (IsKeyPressed(KEY_P) || IsKeyPressed(KEY_ENTER)) {
                game->screen = GAME_SCREEN_ARENA;
            }
            break;

        case GAME_SCREEN_GAME_OVER:
            if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
                ResetRun(game);
                game->newRecord = false;
                game->runFinalized = false;
                game->runsPlayed = SaturatingAddNonNegative(game->runsPlayed, 1);
                (void)Profile_Save(game);
                game->screen = GAME_SCREEN_ARENA;
            }
            break;

        default:
            game->screen = GAME_SCREEN_TITLE;
            break;
    }

    if (game->screen == GAME_SCREEN_GAME_OVER) FinalizeRun(game);

    if (game->transitionTimer < 1.0f) game->transitionTimer += deltaTime;
}
