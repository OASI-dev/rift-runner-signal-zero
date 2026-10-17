/*
 * RIFT RUNNER: SIGNAL ZERO  -  main.c
 *
 * This file holds the GAME LOGIC: the rules of the game. It uses only basic C:
 * variables, arrays, structs, if/else, switch, for/while loops and functions.
 *
 * The drawing, sound and file-saving code is in "others.c". That file is
 * pulled into this one by the line   #include "others.c"   below, so both
 * files must stay in the SAME folder, and you compile only main.c.
 *
 * Parts of this file:
 *   PART 1  Libraries and settings
 *   PART 2  Data: structs and global variables
 *   PART 3  Difficulty numbers
 *   PART 4  Game rules: player, bullets, enemies, bananas
 *   PART 5  Starting a run and the per-frame game update
 *   PART 6  Menu screens (input only)
 *   PART 7  The main loop
 */

/* ================= PART 1: LIBRARIES AND SETTINGS ================= */

#include <raylib.h>    /* the game library: window, drawing, keyboard, mouse, sound */
#include <raymath.h>   /* ready-made vector math: Vector2Add, Vector2Scale ... */
#include <stdio.h>     /* file reading/writing and text formatting */
#include <math.h>      /* sinf, cosf, atan2f, fminf, fmaxf ... */
#include <stdlib.h>    /* rand() for random numbers */
#include <time.h>      /* time() to seed the random numbers */
#if defined(__EMSCRIPTEN__)
#include <emscripten.h>   /* only used when building for the web browser */
#endif

#define TITLE "RIFT RUNNER: SIGNAL ZERO"
#define START_W 1280          /* starting window width */
#define START_H 720           /* starting window height */
#define MAX_BULLETS 240       /* the most things that can exist at once */
#define MAX_ENEMIES 90
#define MAX_PARTICLES 420
#define MAX_PICKUPS 4
#define MAX_STARS 120

/* Names for the screens. STATE_TITLE = 0, STATE_PLAYING = 1, and so on. */
enum { STATE_TITLE, STATE_PLAYING, STATE_PAUSED, STATE_GAME_OVER, STATE_SCORES };

/* Names for the sound effects (used as positions in the sounds array). */
enum { SFX_SHOT, SFX_IMPACT, SFX_PICKUP, SFX_ENEMY_DOWN, SFX_SURGE, SFX_COUNT };

/* ================= PART 2: DATA (STRUCTS AND GLOBALS) ================= */

/* A struct groups the information about one thing. */
typedef struct { bool active; Vector2 pos, vel; float radius, life; } Bullet;
typedef struct { bool active; Vector2 pos, vel; float radius, life, fullLife; Color color; } Particle;
typedef struct { bool active; Vector2 pos; float phase, life; } Pickup;   /* a banana */
typedef struct { float x, y, speed, size, phase; } Star;
typedef struct {
    bool active;
    bool blue;                 /* true = small fast blue enemy, false = big red enemy */
    Vector2 pos;
    float radius, speed, hp, maxHp;
    float heading;             /* the angle the enemy is moving toward */
    float turnClock, turnRate; /* used for random wandering */
    float flash;               /* time left to show the "hit" flash */
} Enemy;
typedef struct {
    Vector2 pos, facing, velocity;
    float health, maxHealth;
    float fireClock;           /* countdown until the next shot */
    float shieldTime;          /* seconds of shield left */
    float invulnerable;        /* seconds of "cannot be hurt" left */
    float animation;           /* makes the legs move */
} Player;

/* Global variables: the whole game state. Any function can use them. */
int state = STATE_TITLE;            /* which screen is showing */
int scoresReturnState = STATE_TITLE;/* where the scores screen goes back to */
int difficulty = 1;                 /* 0 = easy, 1 = medium, 2 = hard */
int menuSelection = 1;              /* highlighted card on the title screen */
Player player;
Bullet bullets[MAX_BULLETS];        /* arrays: a numbered row of the same thing */
Enemy enemies[MAX_ENEMIES];
Particle particles[MAX_PARTICLES];
Pickup pickups[MAX_PICKUPS];
Star stars[MAX_STARS];
int bestScores[3] = { -1, -1, -1 }; /* best score per difficulty, -1 = none yet */
int score = 0, stage = 1, backgroundTheme = 0;
bool newPersonalBest = false, audioReady = false, quitRequested = false, touchAiming = false;
float worldTime = 0;                /* seconds since the program started */
float spawnClock = 0, bananaClock = 0;                 /* countdowns */
float screenShake = 0, flashAlpha = 0, stageNotice = 0, backgroundTransition = 0;
Sound sounds[SFX_COUNT];
bool pointerPressed = false, touchWasDown = false;     /* mouse/touch click info */
Vector2 pointerPos = { 0, 0 };

/* ================= PART 3: DIFFICULTY NUMBERS ================= */
/* Each list has 3 numbers: [0] = EASY, [1] = MEDIUM, [2] = HARD.
   Example: playerSpeed[difficulty] gives the speed for the chosen mode. */
float playerSpeed[3]    = { 292, 276, 266 };
float fireInterval[3]   = { 0.245f, 0.225f, 0.205f };   /* seconds between shots */
float spawnInterval[3]  = { 1.02f, 0.78f, 0.58f };      /* seconds between enemies */
float enemySpeed[3]     = { 72, 95, 120 };
float contactDamage[3]  = { 13, 25, 34 };               /* damage when touched */
float startingHealth[3] = { 130, 100, 85 };

/* Drawing, sound and saving code lives in the other file. It is pasted in here. */
#include "others.c"

/* ================= PART 4: GAME RULES ================= */

/* Create one bullet at "pos" flying in direction "dir". */
void AddBullet(Vector2 pos, Vector2 dir)
{
    dir = Vector2Normalize(dir);                      /* make the direction length 1 */
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (!bullets[i].active) {                     /* use the first free slot */
            float speed = 620 + fminf((stage - 1) * 5.0f, 90);   /* faster at higher stages */
            bullets[i].active = true;
            bullets[i].pos = Vector2Add(pos, Vector2Scale(dir, 16));
            bullets[i].vel = Vector2Scale(dir, speed);
            bullets[i].radius = 4;
            bullets[i].life = 1.32f;                  /* seconds until it disappears */
            PlaySfx(SFX_SHOT);
            return;
        }
    }
}

/* The player died: show the game-over screen and save the score if it is a record. */
void EndRun(void)
{
    state = STATE_GAME_OVER;
    newPersonalBest = (bestScores[difficulty] < 0 || score > bestScores[difficulty]);
    if (newPersonalBest) {
        bestScores[difficulty] = score;
        SaveBestScores();
    }
    screenShake = 0.42f;
    flashAlpha = 0.32f;
    Burst(player.pos, C_PINK, 55, 260, 4);
    PlaySfx(SFX_IMPACT);
}

/* Move the player, aim, and fire automatically. "dt" = seconds since last frame. */
void UpdatePlayer(float dt)
{
    /* 1. Read the keyboard: W A S D or arrow keys. */
    Vector2 input = V2(0, 0);
    if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) input.x -= 1;
    if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) input.x += 1;
    if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)) input.y -= 1;
    if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) input.y += 1;

    /* 2. Add phone touch movement (ReadTouch is in others.c). */
    input = Vector2Add(input, ReadTouch());
    if (Vector2Length(input) > 1) input = Vector2Normalize(input);   /* no faster diagonals */

    /* 3. Face the direction we are moving. */
    if (Vector2Length(input) > 0.14f) {
        if (!touchAiming) player.facing = Vector2Normalize(input);
        player.animation += dt * 14;
    }

    /* 4. Holding the mouse button aims toward the mouse. */
    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) || IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
        Vector2 aim = Vector2Normalize(Vector2Subtract(GetMousePosition(), player.pos));
        if (Vector2LengthSqr(aim) > 0.1f) player.facing = aim;
    }

    /* 5. Speed: full speed while a key is held, otherwise slide to a stop. */
    if (Vector2Length(input) > 0.14f) player.velocity = Vector2Scale(input, playerSpeed[difficulty]);
    else player.velocity = Vector2Scale(player.velocity, fmaxf(0, 1 - 7.5f * dt));

    /* 6. Count timers down and move. Position = position + speed * time. */
    player.shieldTime = fmaxf(0, player.shieldTime - dt);
    player.invulnerable = fmaxf(0, player.invulnerable - dt);
    player.fireClock -= dt;
    player.pos = Vector2Add(player.pos, Vector2Scale(player.velocity, dt));
    player.pos.x = Clamp(player.pos.x, 28, SW() - 28);    /* keep inside the window */
    player.pos.y = Clamp(player.pos.y, 34, SH() - 30);

    /* 7. Fire a bullet whenever the fire timer reaches zero. */
    if (player.fireClock <= 0) {
        AddBullet(player.pos, player.facing);
        player.fireClock = fireInterval[difficulty];
    }
}

/* Enemy number i was shot dead: give points and maybe go up a stage. */
void KillEnemy(int i)
{
    int oldStage = stage;
    enemies[i].active = false;
    score += 200;
    Burst(enemies[i].pos, C_RED, 26, 215, 3.2f);
    PlaySfx(SFX_ENEMY_DOWN);
    stage = 1 + score / 500;                  /* a new stage every 500 points */
    if (stage > oldStage) {
        stageNotice = 2;                      /* show "level increased" text */
        PlaySfx(SFX_SURGE);
    }
}

/* Move every bullet and check if it hits an enemy. */
void UpdateBullets(float dt)
{
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (!bullets[i].active) continue;     /* skip unused slots */

        bullets[i].life -= dt;
        bullets[i].pos = Vector2Add(bullets[i].pos, Vector2Scale(bullets[i].vel, dt));

        /* Remove the bullet if it is too old or has left the window. */
        if (bullets[i].life <= 0 || bullets[i].pos.x < -30 || bullets[i].pos.x > SW() + 30
            || bullets[i].pos.y < -30 || bullets[i].pos.y > SH() + 30) {
            bullets[i].active = false;
            continue;
        }

        /* Check this bullet against every enemy (circle touching circle). */
        for (int j = 0; j < MAX_ENEMIES; j++) {
            if (!enemies[j].active) continue;
            float r = enemies[j].radius + bullets[i].radius;
            if (Vector2DistanceSqr(enemies[j].pos, bullets[i].pos) <= r * r) {
                bullets[i].active = false;            /* the bullet is used up */
                enemies[j].hp -= 1;
                enemies[j].flash = 0.10f;
                Burst(bullets[i].pos, C_CYAN, 4, 85, 1.7f);
                if (enemies[j].hp <= 0) KillEnemy(j);
                break;                                /* one bullet hits one enemy */
            }
        }
    }
}

/* Create one new enemy at a random edge of the window. */
void SpawnEnemy(void)
{
    /* Count the living enemies. Only spawn if we are under the limit. */
    int alive = 0;
    for (int i = 0; i < MAX_ENEMIES; i++) if (enemies[i].active) alive++;
    int limit = 9 + stage + difficulty * 3;
    int maxLimit = (difficulty == 2) ? 48 : 42;
    if (limit > maxLimit) limit = maxLimit;
    if (alive >= limit) return;

    /* Choose a random edge: 0 top, 1 right, 2 bottom, 3 left. */
    float w = SW(), h = SH(), edge = 25;
    Vector2 pos;
    int side = rand() % 4;
    if (side == 0) pos = V2(RandF(edge, w - edge), edge);
    else if (side == 1) pos = V2(w - edge, RandF(edge, h - edge));
    else if (side == 2) pos = V2(RandF(edge, w - edge), h - edge);
    else pos = V2(edge, RandF(edge, h - edge));

    /* Start by heading toward a random point inside the window. */
    Vector2 target = V2(RandF(w * 0.18f, w * 0.82f), RandF(h * 0.18f, h * 0.82f));
    float heading = atan2f(target.y - pos.y, target.x - pos.x) + RandF(-0.55f, 0.55f);

    /* 42% of enemies are blue (fast, weak), the rest red (slow, tough). */
    bool blue = (rand() % 100) < 42;
    float speed = enemySpeed[difficulty] * (blue ? 0.78f : 0.70f) + fminf((stage - 1) * 3.0f, 78);
    float hp;
    if (blue) hp = 1 + (stage - 1) / 6;       /* whole-number division on purpose */
    else hp = 2 + (stage - 1) / 4;
    if (hp > 5) hp = 5;

    /* Put the enemy in the first free slot. */
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (!enemies[i].active) {
            enemies[i].active = true;
            enemies[i].blue = blue;
            enemies[i].pos = pos;
            enemies[i].radius = blue ? 15.5f : 21;
            enemies[i].speed = speed;
            enemies[i].hp = hp;
            enemies[i].maxHp = hp;
            enemies[i].heading = heading;
            enemies[i].turnClock = RandF(0.55f, 1.25f);
            enemies[i].turnRate = RandF(-0.9f, 0.9f);
            enemies[i].flash = 0;
            return;
        }
    }
}

/* Move every enemy, make it steer, bounce off walls, and hurt the player on touch. */
void UpdateEnemies(float dt)
{
    float w = SW(), h = SH();
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (!enemies[i].active) continue;

        enemies[i].flash = fmaxf(0, enemies[i].flash - dt);

        /* Every second or so, pick a new random turning speed. */
        enemies[i].turnClock -= dt;
        if (enemies[i].turnClock <= 0) {
            enemies[i].turnRate = RandF(-0.9f, 0.9f);
            enemies[i].turnClock = RandF(0.55f, 1.25f);
        }

        /* Steering depends on the difficulty. */
        if (difficulty == 0) {
            /* EASY: just wander randomly. */
            enemies[i].heading += enemies[i].turnRate * dt * 1.35f;
        } else {
            /* MEDIUM and HARD: slowly turn toward the player. */
            Vector2 toPlayer = Vector2Subtract(player.pos, enemies[i].pos);
            float wanted = atan2f(toPlayer.y, toPlayer.x);                    /* angle to player */
            float diff = atan2f(sinf(wanted - enemies[i].heading), cosf(wanted - enemies[i].heading));
            float turnSpeed = (difficulty == 1) ? 0.52f : 0.68f;              /* HARD turns faster */
            enemies[i].heading += Clamp(diff, -turnSpeed * dt, turnSpeed * dt);
            if (difficulty == 2) enemies[i].heading += enemies[i].turnRate * dt * 1.5f;   /* HARD also wobbles */
        }

        /* Move forward in the direction the enemy is facing. */
        float speed = enemies[i].speed * (1 + fminf((stage - 1) * 0.028f, 0.48f));
        enemies[i].pos.x += cosf(enemies[i].heading) * speed * dt;
        enemies[i].pos.y += sinf(enemies[i].heading) * speed * dt;

        /* Bounce off the four walls by flipping the direction. */
        float r = enemies[i].radius;
        if (enemies[i].pos.x < r)     { enemies[i].pos.x = r;     enemies[i].heading = PI - enemies[i].heading; enemies[i].turnRate = -enemies[i].turnRate; }
        if (enemies[i].pos.x > w - r) { enemies[i].pos.x = w - r; enemies[i].heading = PI - enemies[i].heading; enemies[i].turnRate = -enemies[i].turnRate; }
        if (enemies[i].pos.y < r)     { enemies[i].pos.y = r;     enemies[i].heading = -enemies[i].heading;     enemies[i].turnRate = -enemies[i].turnRate; }
        if (enemies[i].pos.y > h - r) { enemies[i].pos.y = h - r; enemies[i].heading = -enemies[i].heading;     enemies[i].turnRate = -enemies[i].turnRate; }

        /* Touching the player hurts, unless the player is blinking or shielded. */
        float touch = r + 15;
        if (Vector2DistanceSqr(enemies[i].pos, player.pos) < touch * touch
            && player.invulnerable <= 0 && player.shieldTime <= 0) {
            float damage = contactDamage[difficulty];
            if (enemies[i].blue) damage = damage * 0.55f;       /* blue enemies hurt less */
            player.health -= damage;
            player.invulnerable = 0.92f;
            screenShake = 0.20f;
            flashAlpha = 0.10f;
            /* Push the player and the enemy apart. */
            Vector2 knock = Vector2Normalize(Vector2Subtract(player.pos, enemies[i].pos));
            player.pos = Vector2Add(player.pos, Vector2Scale(knock, 30));
            enemies[i].pos = Vector2Add(enemies[i].pos, Vector2Scale(knock, -22));
            Burst(player.pos, C_PINK, 16, 150, 2.3f);
            PlaySfx(SFX_IMPACT);
            if (player.health <= 0) { player.health = 0; EndRun(); return; }
        }
    }
}

/* Put a banana at "pos" (first free slot). It lasts 8 seconds. */
void AddPickup(Vector2 pos)
{
    for (int i = 0; i < MAX_PICKUPS; i++) {
        if (!pickups[i].active) {
            pickups[i].active = true;
            pickups[i].pos = pos;
            pickups[i].phase = RandF(0, 6.28f);
            pickups[i].life = 8;
            return;
        }
    }
}

/* Bananas expire after a while; touching one gives +20 health and an 8-second shield. */
void UpdatePickups(float dt)
{
    for (int i = 0; i < MAX_PICKUPS; i++) {
        if (!pickups[i].active) continue;
        pickups[i].life -= dt;
        pickups[i].phase += dt * 3;               /* makes the banana pulse */
        if (pickups[i].life <= 0) { pickups[i].active = false; continue; }

        if (Vector2DistanceSqr(pickups[i].pos, player.pos) <= 18 * 18) {
            pickups[i].active = false;
            player.health = fminf(player.maxHealth, player.health + 20);
            player.shieldTime = 8;
            Burst(pickups[i].pos, C_GOLD, 14, 120, 2.1f);
            PlaySfx(SFX_PICKUP);
        }
    }
}

/* ================= PART 5: START A RUN AND UPDATE EACH FRAME ================= */

/* Clear everything and begin a fresh run. */
void StartRun(void)
{
    StartAudioIfNeeded();
    for (int i = 0; i < MAX_BULLETS; i++) bullets[i].active = false;
    for (int i = 0; i < MAX_ENEMIES; i++) enemies[i].active = false;
    for (int i = 0; i < MAX_PARTICLES; i++) particles[i].active = false;
    for (int i = 0; i < MAX_PICKUPS; i++) pickups[i].active = false;

    score = 0;
    stage = 1;
    spawnClock = 1;
    screenShake = 0;
    flashAlpha = 0;
    stageNotice = 0;
    backgroundTransition = 0;
    backgroundTheme = 0;
    bananaClock = RandF(12, 19);
    newPersonalBest = false;

    player.pos = V2(SW() / 2, SH() / 2);              /* start in the middle */
    player.facing = V2(1, 0);
    player.velocity = V2(0, 0);
    player.health = startingHealth[difficulty];
    player.maxHealth = startingHealth[difficulty];
    player.fireClock = 0.12f;
    player.shieldTime = 0;
    player.invulnerable = 0.65f;
    player.animation = 0;

    state = STATE_PLAYING;
    Burst(player.pos, C_CYAN, 34, 190, 3.3f);
    PlaySfx(SFX_SURGE);
}

/* Everything that happens in one frame while playing. The ORDER matters. */
void UpdatePlaying(float dt)
{
    /* P or ESC pauses the game. */
    if (IsKeyPressed(KEY_P) || IsKeyPressed(KEY_ESCAPE)) { state = STATE_PAUSED; return; }

    UpdatePlayer(dt);        /* 1. move, aim, fire */
    UpdateBullets(dt);       /* 2. move bullets, hit enemies */
    UpdateEnemies(dt);       /* 3. move enemies, hurt player */
    if (state != STATE_PLAYING) return;   /* the player may have just died */
    UpdatePickups(dt);       /* 4. bananas */
    UpdateEffects(dt);       /* 5. particles and timers */

    /* A new background every 1000 score. */
    if (score / 1000 != backgroundTheme) {
        backgroundTheme = score / 1000;
        backgroundTransition = 0.9f;
    }

    /* Every 22-34 seconds a banana appears at a random place. */
    bananaClock -= dt;
    if (bananaClock <= 0) {
        AddPickup(V2(RandF(55, SW() - 55), RandF(75, SH() - 75)));
        bananaClock = RandF(22, 34);
    }

    /* Spawn enemies. Later stages spawn faster, and sometimes two at once. */
    spawnClock -= dt;
    if (spawnClock <= 0) {
        SpawnEnemy();
        float interval = spawnInterval[difficulty] / (1 + (stage - 1) * 0.15f);
        spawnClock = fmaxf(0.27f, interval) * RandF(0.84f, 1.16f);
        if (difficulty == 2) {
            if (stage >= 3 && rand() % 100 < 34) SpawnEnemy();
            if (stage >= 7 && rand() % 100 < 12) SpawnEnemy();
        } else if (stage >= 4 && rand() % 100 < 18) {
            SpawnEnemy();
        }
    }
}

/* ================= PART 6: MENU SCREENS (INPUT ONLY) ================= */
/* Each screen has one function that reacts to keys and clicks.
   The button positions (DifficultyCard, StartBounds ...) come from others.c. */

/* Select a difficulty; going past the end wraps around. */
void SetDifficulty(int index)
{
    if (index < 0) index = 2;
    if (index > 2) index = 0;
    menuSelection = index;
    difficulty = index;
}

/* Show the personal-bests screen and remember where to return to. */
void OpenScores(int returnState)
{
    scoresReturnState = returnState;
    state = STATE_SCORES;
}

void UpdateTitle(void)
{
    if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A)) SetDifficulty(menuSelection - 1);
    if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D)) SetDifficulty(menuSelection + 1);
    if (IsKeyPressed(KEY_ONE) || IsKeyPressed(KEY_KP_1)) SetDifficulty(0);
    if (IsKeyPressed(KEY_TWO) || IsKeyPressed(KEY_KP_2)) SetDifficulty(1);
    if (IsKeyPressed(KEY_THREE) || IsKeyPressed(KEY_KP_3)) SetDifficulty(2);
    if (IsKeyPressed(KEY_B)) { OpenScores(STATE_TITLE); return; }
    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) { StartRun(); return; }

    if (pointerPressed) {                              /* a mouse click or a touch */
        for (int i = 0; i < 3; i++) {
            if (HitRect(pointerPos, DifficultyCard(i))) SetDifficulty(i);
        }
        if (HitRect(pointerPos, TitleScoresLink())) { OpenScores(STATE_TITLE); return; }
        if (HitRect(pointerPos, StartBounds())) { StartRun(); return; }
    }
#if !defined(__EMSCRIPTEN__)
    if (IsKeyPressed(KEY_ESCAPE)) quitRequested = true;   /* ESC quits (not on the web) */
#endif
}

void UpdatePaused(void)
{
    if (IsKeyPressed(KEY_P) || IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_ESCAPE)) { state = STATE_PLAYING; return; }
    if (IsKeyPressed(KEY_B)) { OpenScores(STATE_PAUSED); return; }
    if (pointerPressed) {
        if (HitRect(pointerPos, PauseResume())) { state = STATE_PLAYING; return; }
        if (HitRect(pointerPos, PauseTitle())) { state = STATE_TITLE; return; }
        if (HitRect(pointerPos, PauseScoresLink())) { OpenScores(STATE_PAUSED); return; }
    }
}

void UpdateGameOver(void)
{
    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) { StartRun(); return; }
    if (IsKeyPressed(KEY_B)) { OpenScores(STATE_GAME_OVER); return; }
    if (IsKeyPressed(KEY_ESCAPE)) { state = STATE_TITLE; return; }
    if (pointerPressed) {
        if (HitRect(pointerPos, GameOverAgain())) { StartRun(); return; }
        if (HitRect(pointerPos, GameOverScores())) { OpenScores(STATE_GAME_OVER); return; }
        if (HitRect(pointerPos, GameOverTitle())) { state = STATE_TITLE; return; }
    }
}

void UpdateScores(void)
{
    if (IsKeyPressed(KEY_B) || IsKeyPressed(KEY_ESCAPE)) { state = scoresReturnState; return; }
    if (pointerPressed && HitRect(pointerPos, ScoresBack())) state = scoresReturnState;
}

/* ================= PART 7: THE MAIN LOOP ================= */

/* One frame: update the active screen, then draw. Runs about 60 times a second. */
void GameFrame(void)
{
    float dt = GetFrameTime();                 /* seconds since the last frame */
    if (!isfinite(dt) || dt < 0) dt = 0;
    if (dt > 1.0f / 25) dt = 1.0f / 25;        /* avoid a giant jump after a lag spike */
    worldTime += dt;
    PollPointer();                             /* remember mouse/touch presses */

    switch (state) {
    case STATE_TITLE:     UpdateTitle();    UpdateEffects(dt); break;
    case STATE_PLAYING:   UpdatePlaying(dt);                   break;
    case STATE_PAUSED:    UpdatePaused();   UpdateEffects(dt); break;
    case STATE_GAME_OVER: UpdateGameOver(); UpdateEffects(dt); break;
    case STATE_SCORES:    UpdateScores();   UpdateEffects(dt); break;
    }
    DrawFrame();                               /* draw everything (in others.c) */
}

/* The program starts here. */
int main(void)
{
    /* 1. Create the window. */
#if defined(__EMSCRIPTEN__)
    SetConfigFlags(FLAG_MSAA_4X_HINT);
#else
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT | FLAG_MSAA_4X_HINT);
#endif
    InitWindow(START_W, START_H, TITLE);
    if (!IsWindowReady()) {
        fprintf(stderr, "RIFT RUNNER: could not initialize the raylib window.\n");
        return 1;
    }
#if !defined(__EMSCRIPTEN__)
    SetWindowMinSize(800, 480);
#endif
    SetTargetFPS(60);
    SetExitKey(KEY_NULL);                      /* ESC must not close the window by itself */

    /* 2. Random numbers, random stars, saved scores. */
    srand((unsigned int)time(NULL));
    for (int i = 0; i < MAX_STARS; i++) {
        stars[i].x = RandF(0, START_W);
        stars[i].y = RandF(0, START_H);
        stars[i].speed = RandF(5, 25);
        stars[i].size = RandF(0.6f, 1.8f);
        stars[i].phase = RandF(0, 6.28f);
    }
    LoadBestScores();

    /* 3. Run the game loop until the player quits. */
#if defined(__EMSCRIPTEN__)
    emscripten_set_main_loop(GameFrame, 0, 1);      /* the browser calls GameFrame for us */
#else
    while (!WindowShouldClose() && !quitRequested) GameFrame();
    if (audioReady) {                               /* clean up sound before exiting */
        for (int i = 0; i < SFX_COUNT; i++) if (IsSoundValid(sounds[i])) UnloadSound(sounds[i]);
        CloseAudioDevice();
    }
    CloseWindow();
#endif
    return 0;
}
