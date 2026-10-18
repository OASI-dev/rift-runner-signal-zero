// ============================================================================
// RIFT RUNNER: SIGNAL ZERO  -  main_explained.c
//
// This is the SAME code as main.c, but EVERY LINE has a comment explaining it.
// Comments start with // and run to the end of the line. The computer ignores
// them, so this file behaves exactly like main.c.
//
// Quick C reminders:
//   ;        ends a statement (like a full stop)
//   { }      groups statements into one block
//   =        stores a value          ==  asks "are these equal?"
//   !        NOT      &&  AND      ||  OR
//   x += 1   means x = x + 1        x -= 1  means x = x - 1
//   a[i]     the i-th item of an array (counting starts at 0)
//   a.b      the field "b" inside the struct "a"
// ============================================================================

// ============================ PART 1: LIBRARIES AND SETTINGS ============================

#include <raylib.h>                // load raylib: window, drawing, keyboard, mouse, sound
#include <raymath.h>               // load vector math: Vector2Add, Vector2Scale, Clamp...
#include <stdio.h>                 // load file reading/writing and text formatting
#include <math.h>                  // load sinf, cosf, atan2f, fminf, fmaxf...
#include <stdlib.h>                // load rand() for random numbers
#include <time.h>                  // load time(), used to seed the random numbers
#if defined(__EMSCRIPTEN__)        // only when building for the web browser...
#include <emscripten.h>            // ...load the browser support library
#endif                             // end of the "only for web" part

#define TITLE "RIFT RUNNER: SIGNAL ZERO"   // the window title text
#define START_W 1280               // starting window width in pixels
#define START_H 720                // starting window height in pixels
#define MAX_BULLETS 240            // at most 240 bullets can exist at once
#define MAX_ENEMIES 90             // at most 90 enemies at once
#define MAX_PARTICLES 420          // at most 420 explosion dots at once
#define MAX_PICKUPS 4              // at most 4 bananas at once
#define MAX_STARS 120              // 120 background stars

enum { STATE_TITLE, STATE_PLAYING, STATE_PAUSED, STATE_GAME_OVER, STATE_SCORES };   // names for the screens: 0,1,2,3,4

enum { SFX_SHOT, SFX_IMPACT, SFX_PICKUP, SFX_ENEMY_DOWN, SFX_SURGE, SFX_COUNT };    // names for sounds: 0..4, and SFX_COUNT = 5

// ============================ PART 2: DATA (STRUCTS AND GLOBALS) ============================

typedef struct { bool active; Vector2 pos, vel; float radius, life; } Bullet;   // a bullet: exists?, position, velocity, size, seconds left
typedef struct { bool active; Vector2 pos, vel; float radius, life, fullLife; Color color; } Particle;   // an explosion dot (fullLife = starting life, for fading)
typedef struct { bool active; Vector2 pos; float phase, life; } Pickup;   // a banana: exists?, position, pulse timing, seconds left
typedef struct { float x, y, speed, size, phase; } Star;   // a background star: position, drift speed, size, twinkle timing
typedef struct {                   // start of the Enemy struct
    bool active;                   // does this enemy exist?
    bool blue;                     // true = small fast blue enemy, false = big red enemy
    Vector2 pos;                   // where it is (x and y)
    float radius, speed, hp, maxHp;   // size, speed, current health, maximum health
    float heading;                 // the angle (direction) it is moving toward
    float turnClock, turnRate;     // timer + speed for random wandering turns
    float flash;                   // seconds left to show the "I was hit" flash
} Enemy;                           // the new type is called Enemy
typedef struct {                   // start of the Player struct
    Vector2 pos, facing, velocity; // position, aim direction, current speed and direction
    float health, maxHealth;       // current and maximum health
    float fireClock;               // countdown until the next shot
    float shieldTime;              // seconds of shield left
    float invulnerable;            // seconds left of "cannot be hurt" (the blinking)
    float animation;               // a counter that moves the legs
} Player;                          // the new type is called Player

int state = STATE_TITLE;           // which screen is showing; starts on the title
int scoresReturnState = STATE_TITLE;   // which screen the "scores" screen returns to
int difficulty = 1;                // 0 = easy, 1 = medium, 2 = hard; starts on medium
int menuSelection = 1;             // the highlighted card on the title screen
Player player;                     // the one player
Bullet bullets[MAX_BULLETS];       // an array: 240 bullet boxes numbered 0..239
Enemy enemies[MAX_ENEMIES];        // an array: 90 enemy boxes
Particle particles[MAX_PARTICLES]; // an array: 420 particle boxes
Pickup pickups[MAX_PICKUPS];       // an array: 4 banana boxes
Star stars[MAX_STARS];             // an array: 120 star boxes
int bestScores[3] = { -1, -1, -1 };   // best score per difficulty; -1 means "none yet"
int score = 0, stage = 1, backgroundTheme = 0;   // score, level, and which background (0-4) is showing
bool newPersonalBest = false, audioReady = false, quitRequested = false, touchAiming = false;   // true/false flags (record broken, sound started, quit pressed, finger aiming)
float worldTime = 0;               // seconds since the program started (drives animations)
float spawnClock = 0, bananaClock = 0;   // countdowns until the next enemy / banana
float screenShake = 0, flashAlpha = 0, stageNotice = 0, backgroundTransition = 0;   // timers for effects: shake, white flash, level-up text, background change
Sound sounds[SFX_COUNT];           // the loaded sound effects
bool pointerPressed = false, touchWasDown = false;   // mouse/touch: pressed this frame? was touched last frame?
Vector2 pointerPos = { 0, 0 };     // where the mouse/touch is

// ============================ PART 3: DIFFICULTY NUMBERS ============================
// Each list has 3 numbers: [0] = EASY, [1] = MEDIUM, [2] = HARD.

float playerSpeed[3]    = { 292, 276, 266 };           // how fast the player moves
float fireInterval[3]   = { 0.245f, 0.225f, 0.205f }; // seconds between shots (smaller = faster fire)
float spawnInterval[3]  = { 1.02f, 0.78f, 0.58f };    // seconds between enemy spawns
float enemySpeed[3]     = { 72, 95, 120 };             // base enemy speed
float contactDamage[3]  = { 13, 25, 34 };              // damage when an enemy touches you
float startingHealth[3] = { 130, 100, 85 };            // health at the start of a run

#include "others.c"                // paste the drawing/sound/saving code here

// ============================ PART 4: GAME RULES ============================

void AddBullet(Vector2 pos, Vector2 dir)   // create a bullet at pos flying toward dir
{
    dir = Vector2Normalize(dir);   // make the direction length 1 (only direction matters)
    for (int i = 0; i < MAX_BULLETS; i++) {   // look at slots 0, 1, 2 ... 239
        if (!bullets[i].active) {   // if this slot is NOT in use...
            float speed = 620 + fminf((stage - 1) * 5.0f, 90);   // base 620, +5 per stage, but at most +90
            bullets[i].active = true;   // mark the slot as used
            bullets[i].pos = Vector2Add(pos, Vector2Scale(dir, 16));   // start 16 units ahead of the player
            bullets[i].vel = Vector2Scale(dir, speed);   // velocity = direction x speed
            bullets[i].radius = 4;   // bullet size
            bullets[i].life = 1.32f;   // it disappears after 1.32 seconds
            PlaySfx(SFX_SHOT);     // play the shot sound
            return;                // stop: only ONE bullet should be created
        }
    }
}

void EndRun(void)                  // the player died
{
    state = STATE_GAME_OVER;       // switch to the game-over screen
    newPersonalBest = (bestScores[difficulty] < 0 || score > bestScores[difficulty]);   // true if no score yet OR this one is higher
    if (newPersonalBest) {         // a new record?
        bestScores[difficulty] = score;   // remember it
        SaveBestScores();          // write it to the file
    }
    screenShake = 0.42f;           // shake the screen for 0.42 s
    flashAlpha = 0.32f;            // white flash strength
    Burst(player.pos, C_PINK, 55, 260, 4);   // pink explosion at the player
    PlaySfx(SFX_IMPACT);           // impact sound
}

void UpdatePlayer(float dt)        // move/aim/fire; dt = seconds since last frame
{
    Vector2 input = V2(0, 0);      // the direction the keys ask for; starts as "none"
    if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) input.x -= 1;   // A or left arrow held: go left (x smaller)
    if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) input.x += 1;  // D or right arrow held: go right
    if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)) input.y -= 1;     // W or up arrow: up (screen y grows downward, so up is minus)
    if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) input.y += 1;   // S or down arrow: down

    input = Vector2Add(input, ReadTouch());   // add phone-touch movement (function is in others.c)
    if (Vector2Length(input) > 1) input = Vector2Normalize(input);   // two keys = length 1.41; shrink to 1 so diagonals are not faster

    if (Vector2Length(input) > 0.14f) {   // are we actually moving?
        if (!touchAiming) player.facing = Vector2Normalize(input);   // face the movement direction (unless a finger is aiming)
        player.animation += dt * 14;   // advance the leg animation
    }

    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) || IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {   // a mouse button is held
        Vector2 aim = Vector2Normalize(Vector2Subtract(GetMousePosition(), player.pos));   // direction from player to mouse, length 1
        if (Vector2LengthSqr(aim) > 0.1f) player.facing = aim;   // if valid, aim there
    }

    if (Vector2Length(input) > 0.14f) player.velocity = Vector2Scale(input, playerSpeed[difficulty]);   // moving: velocity = direction x speed for this mode
    else player.velocity = Vector2Scale(player.velocity, fmaxf(0, 1 - 7.5f * dt));   // not moving: shrink the velocity each frame so we slide to a stop

    player.shieldTime = fmaxf(0, player.shieldTime - dt);   // shield timer counts down, not below 0
    player.invulnerable = fmaxf(0, player.invulnerable - dt);   // blinking timer counts down
    player.fireClock -= dt;        // shot timer counts down
    player.pos = Vector2Add(player.pos, Vector2Scale(player.velocity, dt));   // new position = old position + velocity x time
    player.pos.x = Clamp(player.pos.x, 28, SW() - 28);   // keep x inside the window (with a margin)
    player.pos.y = Clamp(player.pos.y, 34, SH() - 30);   // keep y inside the window

    if (player.fireClock <= 0) {   // shot timer finished?
        AddBullet(player.pos, player.facing);   // fire a bullet where we face
        player.fireClock = fireInterval[difficulty];   // restart the timer
    }
}

void KillEnemy(int i)              // enemy number i was shot dead
{
    int oldStage = stage;          // remember the stage before scoring
    enemies[i].active = false;     // remove the enemy
    score += 200;                  // +200 points
    Burst(enemies[i].pos, C_RED, 26, 215, 3.2f);   // red explosion where it died
    PlaySfx(SFX_ENEMY_DOWN);       // sound
    stage = 1 + score / 500;       // new stage every 500 points (whole-number division drops decimals)
    if (stage > oldStage) {        // did we just reach a higher stage?
        stageNotice = 2;           // show the "level increased" text for 2 seconds
        PlaySfx(SFX_SURGE);        // level-up sound
    }
}

void UpdateBullets(float dt)       // move every bullet and check hits
{
    for (int i = 0; i < MAX_BULLETS; i++) {   // visit every bullet slot
        if (!bullets[i].active) continue;   // empty slot: skip to the next one

        bullets[i].life -= dt;     // life counts down
        bullets[i].pos = Vector2Add(bullets[i].pos, Vector2Scale(bullets[i].vel, dt));   // move: position + velocity x time

        if (bullets[i].life <= 0 || bullets[i].pos.x < -30 || bullets[i].pos.x > SW() + 30   // too old, or left of / right of the window...
            || bullets[i].pos.y < -30 || bullets[i].pos.y > SH() + 30) {   // ...or above / below the window
            bullets[i].active = false;   // remove it
            continue;              // go to the next bullet
        }

        for (int j = 0; j < MAX_ENEMIES; j++) {   // test this bullet against EVERY enemy
            if (!enemies[j].active) continue;   // skip empty enemy slots
            float r = enemies[j].radius + bullets[i].radius;   // two circles touch if centers are closer than r1 + r2
            if (Vector2DistanceSqr(enemies[j].pos, bullets[i].pos) <= r * r) {   // squared distance compared with r squared (avoids a slow square root)
                bullets[i].active = false;   // the bullet is used up
                enemies[j].hp -= 1;        // enemy loses 1 health
                enemies[j].flash = 0.10f;  // flash for 0.1 seconds
                Burst(bullets[i].pos, C_CYAN, 4, 85, 1.7f);   // a few cyan sparks
                if (enemies[j].hp <= 0) KillEnemy(j);   // out of health: kill it
                break;             // leave the enemy loop: one bullet hits only one enemy
            }
        }
    }
}

void SpawnEnemy(void)              // create one enemy at a random screen edge
{
    int alive = 0;                 // counter for living enemies
    for (int i = 0; i < MAX_ENEMIES; i++) if (enemies[i].active) alive++;   // count the active ones (alive++ adds 1)
    int limit = 9 + stage + difficulty * 3;   // how many are allowed: grows with stage and difficulty
    int maxLimit = (difficulty == 2) ? 48 : 42;   // cap: 48 on hard, otherwise 42 (a ? b : c is a one-line if/else)
    if (limit > maxLimit) limit = maxLimit;   // never above the cap
    if (alive >= limit) return;    // already enough enemies: stop, create none

    float w = SW(), h = SH(), edge = 25;   // window width, height, and the distance from the edge
    Vector2 pos;                   // will hold the spawn position
    int side = rand() % 4;         // random 0, 1, 2 or 3 (% 4 = remainder after dividing by 4)
    if (side == 0) pos = V2(RandF(edge, w - edge), edge);          // 0: top edge, random x
    else if (side == 1) pos = V2(w - edge, RandF(edge, h - edge)); // 1: right edge, random y
    else if (side == 2) pos = V2(RandF(edge, w - edge), h - edge); // 2: bottom edge, random x
    else pos = V2(edge, RandF(edge, h - edge));                    // 3: left edge, random y

    Vector2 target = V2(RandF(w * 0.18f, w * 0.82f), RandF(h * 0.18f, h * 0.82f));   // a random point in the middle area
    float heading = atan2f(target.y - pos.y, target.x - pos.x) + RandF(-0.55f, 0.55f);   // angle toward that point, plus a little random wobble

    bool blue = (rand() % 100) < 42;   // random 0-99; below 42 means blue (42% chance)
    float speed = enemySpeed[difficulty] * (blue ? 0.78f : 0.70f) + fminf((stage - 1) * 3.0f, 78);   // mode speed x type factor, plus 3 per stage (max +78)
    float hp;                      // health, decided below
    if (blue) hp = 1 + (stage - 1) / 6;   // blue: 1 health, +1 every 6 stages (whole-number division)
    else hp = 2 + (stage - 1) / 4;        // red: 2 health, +1 every 4 stages
    if (hp > 5) hp = 5;            // never more than 5 health

    for (int i = 0; i < MAX_ENEMIES; i++) {   // find the first free enemy slot
        if (!enemies[i].active) {  // free slot found
            enemies[i].active = true;      // mark as used
            enemies[i].blue = blue;        // its type
            enemies[i].pos = pos;          // its position
            enemies[i].radius = blue ? 15.5f : 21;   // blue is smaller
            enemies[i].speed = speed;      // its speed
            enemies[i].hp = hp;            // current health
            enemies[i].maxHp = hp;         // maximum health (for the health bar)
            enemies[i].heading = heading;  // its starting direction
            enemies[i].turnClock = RandF(0.55f, 1.25f);   // time until its first random turn change
            enemies[i].turnRate = RandF(-0.9f, 0.9f);     // its random turning speed
            enemies[i].flash = 0;          // not flashing
            return;                // done: only ONE enemy
        }
    }
}

void UpdateEnemies(float dt)       // move, steer, bounce and hurt the player
{
    float w = SW(), h = SH();      // window size
    for (int i = 0; i < MAX_ENEMIES; i++) {   // visit every enemy slot
        if (!enemies[i].active) continue;   // skip empty slots

        enemies[i].flash = fmaxf(0, enemies[i].flash - dt);   // flash timer counts down to 0

        enemies[i].turnClock -= dt;   // wander timer counts down
        if (enemies[i].turnClock <= 0) {   // time for a new random turn
            enemies[i].turnRate = RandF(-0.9f, 0.9f);   // pick a new turning speed
            enemies[i].turnClock = RandF(0.55f, 1.25f); // wait 0.55 to 1.25 s before the next change
        }

        if (difficulty == 0) {     // EASY...
            enemies[i].heading += enemies[i].turnRate * dt * 1.35f;   // ...just wander randomly
        } else {                   // MEDIUM or HARD...
            Vector2 toPlayer = Vector2Subtract(player.pos, enemies[i].pos);   // the arrow from the enemy to the player
            float wanted = atan2f(toPlayer.y, toPlayer.x);   // the angle that arrow points at
            float diff = atan2f(sinf(wanted - enemies[i].heading), cosf(wanted - enemies[i].heading));   // angle difference, wrapped to the short way round
            float turnSpeed = (difficulty == 1) ? 0.52f : 0.68f;   // medium turns slower than hard
            enemies[i].heading += Clamp(diff, -turnSpeed * dt, turnSpeed * dt);   // turn toward the player, but only a small step per frame
            if (difficulty == 2) enemies[i].heading += enemies[i].turnRate * dt * 1.5f;   // HARD also wobbles
        }

        float speed = enemies[i].speed * (1 + fminf((stage - 1) * 0.028f, 0.48f));   // speeds up with stage, at most +48%
        enemies[i].pos.x += cosf(enemies[i].heading) * speed * dt;   // cosine turns the angle into the x step
        enemies[i].pos.y += sinf(enemies[i].heading) * speed * dt;   // sine turns the angle into the y step

        float r = enemies[i].radius;   // shortcut name for the size
        if (enemies[i].pos.x < r)     { enemies[i].pos.x = r;     enemies[i].heading = PI - enemies[i].heading; enemies[i].turnRate = -enemies[i].turnRate; }   // left wall: push back inside, mirror the direction, reverse the turning
        if (enemies[i].pos.x > w - r) { enemies[i].pos.x = w - r; enemies[i].heading = PI - enemies[i].heading; enemies[i].turnRate = -enemies[i].turnRate; }   // right wall: same idea
        if (enemies[i].pos.y < r)     { enemies[i].pos.y = r;     enemies[i].heading = -enemies[i].heading;     enemies[i].turnRate = -enemies[i].turnRate; }   // top wall: flip the up/down direction
        if (enemies[i].pos.y > h - r) { enemies[i].pos.y = h - r; enemies[i].heading = -enemies[i].heading;     enemies[i].turnRate = -enemies[i].turnRate; }   // bottom wall: same idea

        float touch = r + 15;      // touching distance: enemy radius + player radius (15)
        if (Vector2DistanceSqr(enemies[i].pos, player.pos) < touch * touch   // touching the player...
            && player.invulnerable <= 0 && player.shieldTime <= 0) {   // ...AND not blinking AND no shield
            float damage = contactDamage[difficulty];   // damage for this mode
            if (enemies[i].blue) damage = damage * 0.55f;   // blue enemies hurt less
            player.health -= damage;       // lose health
            player.invulnerable = 0.92f;   // cannot be hurt again for 0.92 s
            screenShake = 0.20f;           // shake
            flashAlpha = 0.10f;            // flash
            Vector2 knock = Vector2Normalize(Vector2Subtract(player.pos, enemies[i].pos));   // direction pointing away from the enemy
            player.pos = Vector2Add(player.pos, Vector2Scale(knock, 30));   // push the player 30 units away
            enemies[i].pos = Vector2Add(enemies[i].pos, Vector2Scale(knock, -22));   // push the enemy 22 units the other way
            Burst(player.pos, C_PINK, 16, 150, 2.3f);   // pink sparks
            PlaySfx(SFX_IMPACT);           // impact sound
            if (player.health <= 0) { player.health = 0; EndRun(); return; }   // dead: set to 0, end the run, leave the function
        }
    }
}

void AddPickup(Vector2 pos)        // put a banana at pos
{
    for (int i = 0; i < MAX_PICKUPS; i++) {   // look for a free slot
        if (!pickups[i].active) {  // free slot found
            pickups[i].active = true;       // mark as used
            pickups[i].pos = pos;           // position
            pickups[i].phase = RandF(0, 6.28f);   // random pulse start (6.28 is about 2 x pi)
            pickups[i].life = 8;            // lasts 8 seconds
            return;                // done: only ONE banana
        }
    }
}

void UpdatePickups(float dt)       // age bananas and collect them
{
    for (int i = 0; i < MAX_PICKUPS; i++) {   // visit every banana slot
        if (!pickups[i].active) continue;   // skip empty slots
        pickups[i].life -= dt;     // life counts down
        pickups[i].phase += dt * 3;   // advance the pulse animation
        if (pickups[i].life <= 0) { pickups[i].active = false; continue; }   // expired: remove and go on

        if (Vector2DistanceSqr(pickups[i].pos, player.pos) <= 18 * 18) {   // player within 18 units: picked up
            pickups[i].active = false;   // remove the banana
            player.health = fminf(player.maxHealth, player.health + 20);   // +20 health, but never above the maximum
            player.shieldTime = 8;       // an 8-second shield
            Burst(pickups[i].pos, C_GOLD, 14, 120, 2.1f);   // gold sparks
            PlaySfx(SFX_PICKUP);         // pickup sound
        }
    }
}

// ============================ PART 5: START A RUN AND UPDATE EACH FRAME ============================

void StartRun(void)                // reset everything for a new game
{
    StartAudioIfNeeded();          // start the sound system the first time
    for (int i = 0; i < MAX_BULLETS; i++) bullets[i].active = false;     // remove all bullets
    for (int i = 0; i < MAX_ENEMIES; i++) enemies[i].active = false;     // remove all enemies
    for (int i = 0; i < MAX_PARTICLES; i++) particles[i].active = false; // remove all particles
    for (int i = 0; i < MAX_PICKUPS; i++) pickups[i].active = false;     // remove all bananas

    score = 0;                     // score back to 0
    stage = 1;                     // level back to 1
    spawnClock = 1;                // first enemy after 1 second
    screenShake = 0;               // no shake
    flashAlpha = 0;                // no flash
    stageNotice = 0;               // no level-up text
    backgroundTransition = 0;      // no background-change effect
    backgroundTheme = 0;           // first background
    bananaClock = RandF(12, 19);   // first banana after 12-19 seconds
    newPersonalBest = false;       // no record yet

    player.pos = V2(SW() / 2, SH() / 2);   // start in the middle of the window
    player.facing = V2(1, 0);      // face right
    player.velocity = V2(0, 0);    // not moving
    player.health = startingHealth[difficulty];      // full health for this mode
    player.maxHealth = startingHealth[difficulty];   // maximum health
    player.fireClock = 0.12f;      // first shot very soon
    player.shieldTime = 0;         // no shield
    player.invulnerable = 0.65f;   // a short moment of safety
    player.animation = 0;          // reset leg animation

    state = STATE_PLAYING;         // switch to the game screen
    Burst(player.pos, C_CYAN, 34, 190, 3.3f);   // start effect
    PlaySfx(SFX_SURGE);            // start sound
}

void UpdatePlaying(float dt)       // everything in one frame of play; ORDER matters
{
    if (IsKeyPressed(KEY_P) || IsKeyPressed(KEY_ESCAPE)) { state = STATE_PAUSED; return; }   // P or Esc (pressed, not held): pause and leave

    UpdatePlayer(dt);              // 1. move, aim, fire
    UpdateBullets(dt);             // 2. move bullets, hit enemies
    UpdateEnemies(dt);             // 3. move enemies, hurt the player
    if (state != STATE_PLAYING) return;   // the player may have just died: stop here
    UpdatePickups(dt);             // 4. bananas
    UpdateEffects(dt);             // 5. particles and effect timers

    if (score / 1000 != backgroundTheme) {   // crossed a 1000-point line? (whole-number division)
        backgroundTheme = score / 1000;      // switch to the new background number
        backgroundTransition = 0.9f;         // start the background-change effect
    }

    bananaClock -= dt;             // banana timer counts down
    if (bananaClock <= 0) {        // time for a banana
        AddPickup(V2(RandF(55, SW() - 55), RandF(75, SH() - 75)));   // at a random spot, away from the edges
        bananaClock = RandF(22, 34);   // next one in 22-34 seconds
    }

    spawnClock -= dt;              // enemy timer counts down
    if (spawnClock <= 0) {         // time for an enemy
        SpawnEnemy();              // create one
        float interval = spawnInterval[difficulty] / (1 + (stage - 1) * 0.15f);   // gap shrinks as the stage rises
        spawnClock = fmaxf(0.27f, interval) * RandF(0.84f, 1.16f);   // never below 0.27 s, with +-16% randomness
        if (difficulty == 2) {     // HARD: extra enemies
            if (stage >= 3 && rand() % 100 < 34) SpawnEnemy();   // from stage 3: 34% chance of another
            if (stage >= 7 && rand() % 100 < 12) SpawnEnemy();   // from stage 7: 12% chance of one more
        } else if (stage >= 4 && rand() % 100 < 18) {   // EASY/MEDIUM: from stage 4...
            SpawnEnemy();          // ...an 18% chance of another enemy
        }
    }
}

// ============================ PART 6: MENU SCREENS (INPUT ONLY) ============================

void SetDifficulty(int index)      // choose a difficulty
{
    if (index < 0) index = 2;      // left of EASY wraps to HARD
    if (index > 2) index = 0;      // right of HARD wraps to EASY
    menuSelection = index;         // highlight that card
    difficulty = index;            // use that mode
}

void OpenScores(int returnState)   // show the personal-bests screen
{
    scoresReturnState = returnState;   // remember where to come back to
    state = STATE_SCORES;          // switch to the scores screen
}

void UpdateTitle(void)             // keys and clicks on the title screen
{
    if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A)) SetDifficulty(menuSelection - 1);   // left: previous card
    if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D)) SetDifficulty(menuSelection + 1);  // right: next card
    if (IsKeyPressed(KEY_ONE) || IsKeyPressed(KEY_KP_1)) SetDifficulty(0);     // key 1: easy
    if (IsKeyPressed(KEY_TWO) || IsKeyPressed(KEY_KP_2)) SetDifficulty(1);     // key 2: medium
    if (IsKeyPressed(KEY_THREE) || IsKeyPressed(KEY_KP_3)) SetDifficulty(2);   // key 3: hard
    if (IsKeyPressed(KEY_B)) { OpenScores(STATE_TITLE); return; }              // B: personal bests
    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) { StartRun(); return; }   // Enter/Space: start playing

    if (pointerPressed) {          // a mouse click or touch happened this frame
        for (int i = 0; i < 3; i++) {   // check each of the 3 difficulty cards
            if (HitRect(pointerPos, DifficultyCard(i))) SetDifficulty(i);   // clicked inside card i: select it
        }
        if (HitRect(pointerPos, TitleScoresLink())) { OpenScores(STATE_TITLE); return; }   // clicked the scores link
        if (HitRect(pointerPos, StartBounds())) { StartRun(); return; }   // clicked the start button
    }
#if !defined(__EMSCRIPTEN__)       // only in the desktop version...
    if (IsKeyPressed(KEY_ESCAPE)) quitRequested = true;   // ...Esc quits (a web page cannot close itself)
#endif                             // end of the "only desktop" part
}

void UpdatePaused(void)            // keys and clicks while paused
{
    if (IsKeyPressed(KEY_P) || IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_ESCAPE)) { state = STATE_PLAYING; return; }   // P/Enter/Esc: resume
    if (IsKeyPressed(KEY_B)) { OpenScores(STATE_PAUSED); return; }   // B: scores, coming back to pause
    if (pointerPressed) {          // a click or touch
        if (HitRect(pointerPos, PauseResume())) { state = STATE_PLAYING; return; }       // resume button
        if (HitRect(pointerPos, PauseTitle())) { state = STATE_TITLE; return; }          // return to the title
        if (HitRect(pointerPos, PauseScoresLink())) { OpenScores(STATE_PAUSED); return; }   // scores link
    }
}

void UpdateGameOver(void)          // keys and clicks on the game-over box
{
    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) { StartRun(); return; }   // Enter/Space: run again
    if (IsKeyPressed(KEY_B)) { OpenScores(STATE_GAME_OVER); return; }   // B: scores
    if (IsKeyPressed(KEY_ESCAPE)) { state = STATE_TITLE; return; }      // Esc: back to the title
    if (pointerPressed) {          // a click or touch
        if (HitRect(pointerPos, GameOverAgain())) { StartRun(); return; }               // run-again button
        if (HitRect(pointerPos, GameOverScores())) { OpenScores(STATE_GAME_OVER); return; }   // scores link
        if (HitRect(pointerPos, GameOverTitle())) { state = STATE_TITLE; return; }       // return link
    }
}

void UpdateScores(void)            // keys and clicks on the scores screen
{
    if (IsKeyPressed(KEY_B) || IsKeyPressed(KEY_ESCAPE)) { state = scoresReturnState; return; }   // B/Esc: go back where we came from
    if (pointerPressed && HitRect(pointerPos, ScoresBack())) state = scoresReturnState;   // back button clicked
}

// ============================ PART 7: THE MAIN LOOP ============================

void GameFrame(void)               // one frame: update, then draw (about 60 times per second)
{
    float dt = GetFrameTime();     // seconds since the previous frame
    if (!isfinite(dt) || dt < 0) dt = 0;   // safety: if the time is nonsense, use 0
    if (dt > 1.0f / 25) dt = 1.0f / 25;    // after a freeze, do not jump far: treat it as at most 0.04 s
    worldTime += dt;               // the animation clock
    PollPointer();                 // remember whether the mouse/touch was pressed this frame

    switch (state) {               // choose what to do by the current screen
    case STATE_TITLE:     UpdateTitle();    UpdateEffects(dt); break;   // title: handle input, keep effects moving
    case STATE_PLAYING:   UpdatePlaying(dt);                   break;   // playing: the game rules
    case STATE_PAUSED:    UpdatePaused();   UpdateEffects(dt); break;   // paused: handle input
    case STATE_GAME_OVER: UpdateGameOver(); UpdateEffects(dt); break;   // game over: handle input
    case STATE_SCORES:    UpdateScores();   UpdateEffects(dt); break;   // scores: handle input
    }                              // break; ends each case so the next one does not also run
    DrawFrame();                   // draw everything (function is in others.c)
}

int main(void)                     // the program starts here
{
#if defined(__EMSCRIPTEN__)        // web build...
    SetConfigFlags(FLAG_MSAA_4X_HINT);   // ...smoother edges only
#else                              // desktop build...
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT | FLAG_MSAA_4X_HINT);   // ...resizable window, vsync, smoother edges
#endif                             // end of the web/desktop choice
    InitWindow(START_W, START_H, TITLE);   // open the 1280x720 window with the title
    if (!IsWindowReady()) {        // did the window fail to open?
        fprintf(stderr, "RIFT RUNNER: could not initialize the raylib window.\n");   // print an error message
        return 1;                  // stop; 1 means "error"
    }
#if !defined(__EMSCRIPTEN__)       // desktop only...
    SetWindowMinSize(800, 480);    // ...the window cannot be smaller than this
#endif                             // end of desktop-only part
    SetTargetFPS(60);              // aim for 60 frames per second
    SetExitKey(KEY_NULL);          // Esc must not close the window by itself (it has its own jobs)

    srand((unsigned int)time(NULL));   // seed the random numbers with the current time so every run differs ((type) is a cast: it converts the number type)
    for (int i = 0; i < MAX_STARS; i++) {   // set up all 120 stars
        stars[i].x = RandF(0, START_W);     // random x position
        stars[i].y = RandF(0, START_H);     // random y position
        stars[i].speed = RandF(5, 25);      // random drift speed
        stars[i].size = RandF(0.6f, 1.8f);  // random size
        stars[i].phase = RandF(0, 6.28f);   // random twinkle start
    }
    LoadBestScores();              // read the saved best scores from the file

#if defined(__EMSCRIPTEN__)        // web build...
    emscripten_set_main_loop(GameFrame, 0, 1);   // ...the browser calls GameFrame over and over for us
#else                              // desktop build...
    while (!WindowShouldClose() && !quitRequested) GameFrame();   // ...repeat GameFrame until the window closes or quit is requested: THE GAME LOOP
    if (audioReady) {              // if sound was started...
        for (int i = 0; i < SFX_COUNT; i++) if (IsSoundValid(sounds[i])) UnloadSound(sounds[i]);   // ...free each loaded sound
        CloseAudioDevice();        // ...and shut the sound system down
    }
    CloseWindow();                 // close the window
#endif                             // end of the web/desktop choice
    return 0;                      // 0 means "finished normally"
}
