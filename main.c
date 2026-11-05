/*
 * RIFT RUNNER: SIGNAL ZERO  -  main.c   (single file, picture edition)
 *
 * The WHOLE game is in this ONE file. It uses only: variables, arrays,
 * structs, strings, if/else, for loops, functions and switch. There are NO
 * pointer variables anywhere (the only "*" is FILE *f, which is simply how
 * C names an opened file).
 *
 * Modes: EASY, MEDIUM, HARD. You run and shoot alien enemies, grab the
 * banana for health and a shield, and the scene changes every 1000 score.
 *
 * PICTURES: the player, the two enemy types, the bullet, the banana and
 * the 5 backgrounds are real image files, not drawn shapes. They must sit
 * in a folder called "assets/images/" next to this program, using these
 * exact names:
 *   player.png, red_enemy.png, blue_enemy.png, bullet.png, banana.png,
 *   bg_cradle.png, bg_graveyard.png, bg_rift.png, bg_bloodmoon.png,
 *   bg_voidcathedral.png
 * If a picture is missing, the game still runs - that one thing is just
 * invisible, nothing crashes.
 *
 * Parts of this file:
 *   PART 1   Libraries and settings
 *   PART 2   Data: colors, names, structs, arrays and variables
 *   PART 3   Small helper functions
 *   PART 4   Loading pictures, sound and saved best scores
 *   PART 5   Sparks (particle effects)
 *   PART 6   Mouse and touch input
 *   PART 7   Player and bullet rules
 *   PART 8   Game rules: enemies, banana, a run
 *   PART 9   Screens (keyboard part): title, pause, game over, best scores
 *   PART 10  Small drawing helpers (text, panel, button)
 *   PART 11  Drawing the player, enemies, background and the arena
 *   PART 12  Drawing the screens and one whole frame
 *   PART 13  The main loop
 */

// ======================= PART 1: LIBRARIES AND SETTINGS =======================

#include <raylib.h>     // the game library: window, drawing, pictures, keyboard, mouse, sound
#include <stdio.h>      // files (fopen, fprintf, fgets) for saving the best scores
#include <math.h>       // sinf, cosf, atan2f, sqrtf, fminf, fmaxf
#include <stdlib.h>     // rand() for random numbers, atoi() to turn text into a number
#include <time.h>       // time() so the random numbers are different every run
#if defined(__EMSCRIPTEN__)
#include <emscripten.h> // only used when the game is built for the web browser
#endif

#define GAME_NAME "RIFT RUNNER: SIGNAL ZERO"   // the window title
#define WIDTH GetScreenWidth()                 // current window width  (WIDTH is easier to read)
#define HEIGHT GetScreenHeight()               // current window height
#define MAX_BULLETS 200        // most bullets alive at the same time
#define MAX_ENEMIES 60         // most enemies alive at the same time
#define MAX_PARTICLES 300      // most sparks alive at the same time
#define PLAYER_SIZE 16          // size of the player for hit tests
#define BULLET_SIZE 4           // size of a bullet for hit tests

// Names for numbers. EASY = 0, MEDIUM = 1, HARD = 2.
enum { TITLE, PLAYING, PAUSED, GAME_OVER, BEST };                   // the 5 screens
enum { EASY, MEDIUM, HARD };                                        // the 3 modes
enum { RED_ENEMY, BLUE_ENEMY };                                     // the 2 enemy types
enum { SND_SHOT, SND_HIT, SND_BANANA, SND_BOOM, SND_LEVEL };        // the 5 sounds

// ======================= PART 2: DATA (COLORS, STRUCTS, VARIABLES) =======================

#define C_PANEL  (Color){ 15, 21, 45, 238 }      // dark blue box
#define C_CYAN   (Color){ 76, 231, 255, 255 }    // light blue
#define C_PURPLE (Color){ 179, 91, 255, 255 }    // purple
#define C_PINK   (Color){ 255, 83, 170, 255 }    // pink
#define C_GREEN  (Color){ 98, 255, 178, 255 }    // green
#define C_GOLD   (Color){ 255, 207, 100, 255 }   // gold
#define C_TEXT   (Color){ 236, 243, 255, 255 }   // almost white (text)
#define C_GRAY   (Color){ 143, 158, 196, 255 }   // soft gray (small text)
#define C_RED    (Color){ 255, 83, 99, 255 }     // red
#define C_DARK   (Color){ 38, 47, 74, 255 }      // dark bar background

// One entry for each mode: EASY, MEDIUM, HARD.
char mode_name[3][10]  = { "EASY", "MEDIUM", "HARD" };                                   // names
char mode_note[3][24]  = { "Learn the arena", "Stay moving", "Overwhelming pursuit" };    // small descriptions
Color mode_color[3]    = { { 98, 255, 178, 255 }, { 76, 231, 255, 255 }, { 255, 83, 170, 255 } };   // colors
char sector_name[5][16] = { "THE CRADLE", "GRAVEYARD BELT", "THE RIFT", "BLOOD MOON", "VOID CATHEDRAL" };   // the 5 backgrounds

// A struct groups the information about ONE thing.
typedef struct { float x, y, angle, health, shoot_timer, shield, safe; } Player;   // angle = where it looks
typedef struct { int on; float x, y, vx, vy, life; } Bullet;                       // on = 1 means "in use"
typedef struct { int on, type; float x, y, vx, vy, size, speed, health, max_health, angle, timer, flash; } Enemy;
typedef struct { int on; float x, y, vx, vy, size, life, max_life; Color color; } Particle;

Player player;                         // the one player
Bullet bullet[MAX_BULLETS];            // all bullets
Enemy enemy[MAX_ENEMIES];              // all enemies
Particle particle[MAX_PARTICLES];      // all sparks

// Variables any function can use.
int state = TITLE;                     // which screen is showing
int mode = MEDIUM;                     // which mode is chosen
int best[3] = { 0, 0, 0 };             // best score of EASY, MEDIUM, HARD (0 = no record yet)
int score = 0, level = 1, background = 0, new_best = 0;   // score, level, background number, record flag
float max_health = 100;                // health the player starts with in this mode
float spawn_timer = 0, banana_timer = 0, over_timer = 0;  // countdowns in seconds
int banana_on = 0;                     // 1 = a banana is on the screen
float banana_x = 0, banana_y = 0, banana_life = 0;        // where the banana is and how long it stays
float shake = 0, screen_flash = 0, level_msg = 0, bg_fade = 0, world_time = 0;   // effect timers
int audio_ready = 0, quit_game = 0;    // sound started? / does the player want to quit?
int clicked = 0, finger_was_down = 0;  // mouse click or touch this frame
float pointer_x = 0, pointer_y = 0;    // where the click or touch happened
float touch_x = 0, touch_y = 0;        // phone: movement asked by the left finger
int touch_aiming = 0;                  // phone: 1 while the right finger aims
Sound sounds[5];                       // the loaded sound effects

// Numbers for each mode: [0] = EASY, [1] = MEDIUM, [2] = HARD.
float player_speed[3]  = { 292, 276, 266 };               // how fast the player walks
float shoot_delay[3]   = { 0.245f, 0.225f, 0.205f };      // seconds between shots
float spawn_delay[3]   = { 1.02f, 0.78f, 0.58f };         // seconds between new enemies
float enemy_speed[3]   = { 72, 95, 120 };                 // how fast enemies move
float enemy_damage[3]  = { 13, 25, 34 };                  // health lost when an enemy touches you
float start_health[3]  = { 130, 100, 85 };                // health at the start of a run
float enemy_turn[3]    = { 1.2f, 0.8f, 1.1f };            // how quickly an enemy changes direction

// The 10 pictures used in the game.
Texture2D tex_player;                  // the player
Texture2D tex_red;                     // red enemy
Texture2D tex_blue;                    // blue enemy
Texture2D tex_bullet;                  // the bullet
Texture2D tex_banana;                  // the banana
Texture2D tex_bg[5];                   // the 5 backgrounds

// ======================= PART 3: SMALL HELPER FUNCTIONS =======================

// A random decimal number between low and high.
float random_float(float low, float high)
{
    return low + ((float)rand() / (float)RAND_MAX) * (high - low);   // rand() gives 0..RAND_MAX
}

// Keep a number between low and high.
float limit(float value, float low, float high)
{
    if (value < low) return low;       // too small: use the lowest allowed
    if (value > high) return high;     // too big: use the highest allowed
    return value;                      // fine as it is
}

// Do two circles touch? Returns 1 for yes and 0 for no.
int touching(float x1, float y1, float size1, float x2, float y2, float size2)
{
    float dx = x1 - x2, dy = y1 - y2;                  // distance sideways and up/down
    float together = size1 + size2;                    // the circles touch when the distance is smaller than this
    return dx * dx + dy * dy <= together * together;   // compare squares, so no square root is needed
}

// Draw one picture "tex", centered at (x, y), sized w by h, turned by "angle_deg" degrees.
// This ONE function draws the player, an enemy, a bullet, the banana AND a full-screen background.
void draw_sprite(Texture2D tex, float x, float y, float w, float h, float angle_deg, Color tint)
{
    Rectangle source = { 0, 0, (float)tex.width, (float)tex.height };   // use the whole picture
    Rectangle dest = { x, y, w, h };                                    // where and how big to draw it
    Vector2 origin = { w / 2, h / 2 };                                  // turn around the middle
    DrawTexturePro(tex, source, dest, origin, angle_deg, tint);         // draw it, rotated and tinted
}

// ======================= PART 4: PICTURES, SOUND AND SAVED BEST SCORES =======================

// Load the 10 pictures from assets/images/. Must be called AFTER the window is open.
void load_pictures(void)
{
    tex_player = LoadTexture("assets/images/player.png");
    tex_red    = LoadTexture("assets/images/red_enemy.png");
    tex_blue   = LoadTexture("assets/images/blue_enemy.png");
    tex_bullet = LoadTexture("assets/images/bullet.png");
    tex_banana = LoadTexture("assets/images/banana.png");
    char bg_file[5][40] = { "assets/images/bg_cradle.png", "assets/images/bg_graveyard.png",
        "assets/images/bg_rift.png", "assets/images/bg_bloodmoon.png", "assets/images/bg_voidcathedral.png" };
    for (int i = 0; i < 5; i++) tex_bg[i] = LoadTexture(bg_file[i]);   // load all 5 backgrounds in a loop
}

// Free the pictures when the game closes (desktop only; the browser just stops).
void unload_pictures(void)
{
    UnloadTexture(tex_player);
    UnloadTexture(tex_red);
    UnloadTexture(tex_blue);
    UnloadTexture(tex_bullet);
    UnloadTexture(tex_banana);
    for (int i = 0; i < 5; i++) UnloadTexture(tex_bg[i]);
}

char sound_file[5][30] = { "assets/shot.wav", "assets/impact.wav", "assets/pickup.wav", "assets/enemy_down.wav", "assets/surge.wav" };   // same order as SND_SHOT ... SND_LEVEL

// Start the sound system (browsers only allow sound after a click).
void start_audio(void)
{
    if (audio_ready) return;                                                 // already started
    InitAudioDevice();                                                       // open the sound device
    audio_ready = IsAudioDeviceReady();                                      // did it work?
    if (audio_ready) {                                                       // if yes, load the 5 sound files
        for (int i = 0; i < 5; i++) sounds[i] = LoadSound(sound_file[i]);
    }
}

// Play sound number n (SND_SHOT, SND_HIT ...).
void play(int n)
{
    if (audio_ready && IsSoundValid(sounds[n])) PlaySound(sounds[n]);        // only if sound works
}

#if defined(__EMSCRIPTEN__)
#define SCORE_FILE "/persist/rift_runner_scores.txt"     // web: this folder is kept by the browser
#else
#define SCORE_FILE "rift_runner_scores.txt"              // desktop: a file next to the game
#endif

// Write the 3 best scores into the score file (one number per line).
void save_best(void)
{
    FILE *f = fopen(SCORE_FILE, "w");                                        // open the file for writing (FILE * is how C names a file)
    if (f == NULL) return;                                                   // could not open: give up
    for (int i = 0; i < 3; i++) fprintf(f, "%d\n", best[i]);                 // write one score per line
    fclose(f);                                                               // close the file
#if defined(__EMSCRIPTEN__)
    EM_ASM({                                                                 // web only: copy the file into the browser's storage
        if (typeof FS !== 'undefined' && typeof IDBFS !== 'undefined')
            FS.syncfs(false, function(error) { if (error) console.warn('score save failed', error); });
    }, 0);
#endif
}

// Read the 3 best scores from the score file (if it exists).
void load_best(void)
{
    FILE *f = fopen(SCORE_FILE, "r");                                        // open the file for reading
    if (f == NULL) return;                                                   // no file yet: keep the zeros
    char text[30];                                                           // room for one line of text
    for (int i = 0; i < 3; i++) {                                            // read the 3 lines
        if (fgets(text, 30, f) != NULL) best[i] = atoi(text);                // turn the line into a number
        if (best[i] < 0) best[i] = 0;                                        // a broken file must not give negative scores
    }
    fclose(f);                                                               // close the file
}

// ======================= PART 5: SPARKS (PARTICLE EFFECTS) =======================

// An explosion: "count" sparks flying out in random directions.
void explosion(float x, float y, Color color, int count, float speed, float size)
{
    for (int k = 0; k < count; k++) {                                        // make "count" sparks
        int i = 0;                                                           // find a free spark slot
        while (i < MAX_PARTICLES && particle[i].on) i++;
        if (i == MAX_PARTICLES) return;                                      // no free slot: stop
        float angle = random_float(0, 2 * PI);                               // a random direction
        float fast = random_float(speed * 0.35f, speed);                     // a random speed
        particle[i].on = 1;                                                  // switch the spark on
        particle[i].x = x;
        particle[i].y = y;
        particle[i].vx = cosf(angle) * fast;
        particle[i].vy = sinf(angle) * fast;
        particle[i].size = random_float(size * 0.55f, size);                 // a random size
        particle[i].life = random_float(0.24f, 0.72f);                       // seconds it lives
        particle[i].max_life = particle[i].life;                             // remember the full life
        particle[i].color = color;
    }
}

// Move the sparks and count the effect timers down.
void update_effects(float dt)
{
    for (int i = 0; i < MAX_PARTICLES; i++) {                                // every spark
        if (!particle[i].on) continue;                                       // skip unused slots
        particle[i].life -= dt;                                              // get older
        if (particle[i].life <= 0) { particle[i].on = 0; continue; }         // too old: remove
        particle[i].x += particle[i].vx * dt;                                // move
        particle[i].y += particle[i].vy * dt;
        particle[i].vx -= particle[i].vx * 2.5f * dt;                        // slow down
        particle[i].vy -= particle[i].vy * 2.5f * dt;
    }
    shake = fmaxf(0, shake - dt);                                            // timers count down to 0
    screen_flash = fmaxf(0, screen_flash - 1.75f * dt);
    level_msg = fmaxf(0, level_msg - dt);
    bg_fade = fmaxf(0, bg_fade - dt);
}

// ======================= PART 6: MOUSE AND TOUCH INPUT =======================

// Once per frame: remember if the mouse or a finger was just pressed, and where.
void read_pointer(void)
{
    int finger_down = GetTouchPointCount() > 0;                              // is a finger on the screen?
    int mouse_pressed = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);             // was the mouse button just pressed?
    clicked = mouse_pressed || (finger_down && !finger_was_down);            // a new click or a new touch
    Vector2 where = (finger_down && !mouse_pressed) ? GetTouchPosition(0) : GetMousePosition();   // finger or mouse position
    pointer_x = where.x;                                                     // remember the position
    pointer_y = where.y;
    finger_was_down = finger_down;                                           // remember for the next frame
}

// Phone: the left half of the screen moves the player, the right half aims.
void read_touch(void)
{
    touch_x = 0;                                                             // no movement yet
    touch_y = 0;
    touch_aiming = 0;                                                        // not aiming yet
    for (int i = 0; i < GetTouchPointCount(); i++) {                         // look at every finger
        Vector2 finger = GetTouchPosition(i);                                // where this finger is
        if (finger.x < WIDTH * 0.47f) {                                      // left side: move
            touch_x += (finger.x - 105) / 62;                                //   the farther from the circle, the faster
            touch_y += (finger.y - (HEIGHT - 126)) / 62;
        } else {                                                             // right side: aim at the finger
            player.angle = atan2f(finger.y - player.y, finger.x - player.x);
            touch_aiming = 1;
        }
    }
}

// ======================= PART 7: PLAYER AND BULLET RULES =======================

// Put the player at a start place with full health.
void reset_player(void)
{
    player.x = WIDTH / 2;                  // start in the middle
    player.y = HEIGHT / 2;
    player.angle = 0;                      // start looking right
    player.health = max_health;            // full health
    player.shoot_timer = 0.2f;             // short wait before the first shot
    player.shield = 0;                     // no shield
    player.safe = 0.65f;                   // cannot be hurt for a moment
}

// Fire one bullet from the player.
void shoot(void)
{
    for (int i = 0; i < MAX_BULLETS; i++) {                          // look for a free bullet slot
        if (bullet[i].on) continue;                                  // slot in use: try the next one
        float speed = 620 + fminf((level - 1) * 5.0f, 90);           // bullets get faster with the level
        bullet[i].on = 1;                                            // switch the bullet on
        bullet[i].x = player.x + cosf(player.angle) * 16;            // start a little in front of the gun
        bullet[i].y = player.y + sinf(player.angle) * 16;
        bullet[i].vx = cosf(player.angle) * speed;                   // fly the way the player looks
        bullet[i].vy = sinf(player.angle) * speed;
        bullet[i].life = 1.3f;                                       // seconds before it disappears
        play(SND_SHOT);                                              // shot sound
        return;                                                      // one bullet is enough
    }
}

// Move the player with the keys, or the phone fingers, or the mouse.
void move_player(float dt)
{
    float dx = 0, dy = 0;                                            // the direction the player wants to go
    if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) dx -= 1;             // left: WASD or arrows both work
    if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) dx += 1;            // right
    if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)) dy -= 1;               // up (y gets smaller going up)
    if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) dy += 1;             // down
    touch_aiming = 0;                                                // no phone aiming yet
    read_touch();                                                    // look at the fingers on the screen
    dx += touch_x;                                                   // add the phone movement
    dy += touch_y;
    float length = sqrtf(dx * dx + dy * dy);                         // how long is the direction?
    if (length > 1) { dx = dx / length; dy = dy / length; }          // never faster than 1 (no fast diagonals)
    if (length > 0.14f && !touch_aiming) player.angle = atan2f(dy, dx);   // look the way we walk
    if (GetTouchPointCount() == 0 && (IsMouseButtonDown(MOUSE_BUTTON_LEFT) || IsMouseButtonDown(MOUSE_BUTTON_RIGHT))) {
        Vector2 mouse = GetMousePosition();                          // holding the mouse: look at the mouse
        player.angle = atan2f(mouse.y - player.y, mouse.x - player.x);
    }
    player.x += dx * player_speed[mode] * dt;                        // position = position + speed * time
    player.y += dy * player_speed[mode] * dt;
    player.x = limit(player.x, 28, WIDTH - 28);                      // stay inside the window
    player.y = limit(player.y, 34, HEIGHT - 30);
    player.shield = fmaxf(0, player.shield - dt);                    // timers count down to 0
    player.safe = fmaxf(0, player.safe - dt);
    player.shoot_timer -= dt;
}

// Move every bullet and remove old ones.
void update_bullets(float dt)
{
    for (int i = 0; i < MAX_BULLETS; i++) {                          // look at every bullet
        if (!bullet[i].on) continue;                                 // skip unused slots
        bullet[i].x += bullet[i].vx * dt;                            // move
        bullet[i].y += bullet[i].vy * dt;
        bullet[i].life -= dt;                                        // get older
        if (bullet[i].life <= 0 || bullet[i].x < -30 || bullet[i].x > WIDTH + 30
            || bullet[i].y < -30 || bullet[i].y > HEIGHT + 30) {     // too old, or left the window
            bullet[i].on = 0;                                        // remove it
        }
    }
}

// ======================= PART 8: GAME RULES: ENEMIES, BANANA, A RUN =======================

// Enemy number i was shot dead: give points and maybe go up a level.
void kill_enemy(int i)
{
    int old_level = level;                                           // remember the level before
    enemy[i].on = 0;                                                 // the enemy is gone
    score += 200;                                                    // 200 points per enemy
    explosion(enemy[i].x, enemy[i].y, C_RED, 26, 215, 3.2f);         // explosion
    play(SND_BOOM);                                                  // explosion sound
    level = 1 + score / 500;                                         // a new level every 500 points (whole-number division)
    if (level > old_level) { level_msg = 2; play(SND_LEVEL); }       // show "level increased"
}

// Check every bullet against every enemy.
void enemy_hits(void)
{
    for (int i = 0; i < MAX_BULLETS; i++) {                          // every bullet
        if (!bullet[i].on) continue;                                 // skip unused slots
        for (int j = 0; j < MAX_ENEMIES; j++) {                      // every enemy
            if (!enemy[j].on) continue;                              // skip unused slots
            if (touching(bullet[i].x, bullet[i].y, BULLET_SIZE, enemy[j].x, enemy[j].y, enemy[j].size)) {
                bullet[i].on = 0;                                    // the bullet is used up
                enemy[j].health -= 1;                                // the enemy loses 1 health
                enemy[j].flash = 0.1f;                               // flash gold for a moment
                explosion(bullet[i].x, bullet[i].y, C_CYAN, 4, 85, 1.7f);   // small sparks
                if (enemy[j].health <= 0) kill_enemy(j);             // dead enemy
                break;                                               // one bullet hits only one enemy
            }
        }
    }
}

// Create one new enemy at a random edge of the window.
void spawn_enemy(void)
{
    int alive = 0;                                                   // count the living enemies
    for (int i = 0; i < MAX_ENEMIES; i++) if (enemy[i].on) alive++;
    int most = 9 + level + mode * 3;                                 // how many may live at once
    if (most > 45) most = 45;                                        // never more than 45
    if (alive >= most) return;                                       // too many: do not spawn

    float x, y;                                                      // where the new enemy starts
    int side = rand() % 4;                                           // random edge: 0 top, 1 right, 2 bottom, 3 left
    if (side == 0) { x = random_float(25, WIDTH - 25); y = 25; }
    else if (side == 1) { x = WIDTH - 25; y = random_float(25, HEIGHT - 25); }
    else if (side == 2) { x = random_float(25, WIDTH - 25); y = HEIGHT - 25; }
    else { x = 25; y = random_float(25, HEIGHT - 25); }

    int type = (rand() % 100 < 42) ? BLUE_ENEMY : RED_ENEMY;         // 42% blue (fast, weak), the rest red (slow, tough)
    float speed = enemy_speed[mode] * (type == BLUE_ENEMY ? 0.78f : 0.70f) + fminf((level - 1) * 3.0f, 78);
    float health;                                                    // hits needed to kill it
    if (type == BLUE_ENEMY) health = 1 + (level - 1) / 6;            // whole-number division on purpose
    else health = 2 + (level - 1) / 4;
    if (health > 5) health = 5;                                      // never more than 5

    for (int i = 0; i < MAX_ENEMIES; i++) {                          // put it in the first free slot
        if (enemy[i].on) continue;                                   // slot in use
        float first = atan2f(HEIGHT / 2 - y, WIDTH / 2 - x);         // at the start walk toward the middle
        enemy[i].on = 1;                                             // switch the enemy on
        enemy[i].type = type;
        enemy[i].x = x;
        enemy[i].y = y;
        enemy[i].speed = speed;
        enemy[i].vx = cosf(first) * speed;                           // starting velocity
        enemy[i].vy = sinf(first) * speed;
        enemy[i].size = (type == BLUE_ENEMY) ? 15.5f : 21;           // blue is small, red is big
        enemy[i].health = health;
        enemy[i].max_health = health;
        enemy[i].angle = random_float(-PI, PI);                      // a random wander direction
        enemy[i].timer = random_float(0.6f, 1.3f);                   // seconds until a new wander direction
        enemy[i].flash = 0;
        return;                                                      // done
    }
}

// The run is over: save the record and show the game-over screen.
void end_run(void)
{
    state = GAME_OVER;                                               // show the game-over screen
    over_timer = 0.6f;                                               // short wait before "play again" works
    new_best = score > best[mode];                                   // 1 if this is a new record
    if (new_best) { best[mode] = score; save_best(); }               // remember and save the record
    shake = 0.4f;                                                    // shake the screen
    screen_flash = 0.3f;                                             // flash the screen
    explosion(player.x, player.y, C_PINK, 55, 260, 4);               // big explosion
    play(SND_HIT);                                                   // hit sound
}

// Move every enemy, steer it, bounce it off walls, and hurt the player on touch.
void update_enemies(float dt)
{
    for (int i = 0; i < MAX_ENEMIES; i++) {                          // every enemy
        if (!enemy[i].on) continue;                                  // skip unused slots
        enemy[i].flash = fmaxf(0, enemy[i].flash - dt);              // the flash timer counts down
        enemy[i].timer -= dt;                                        // the wander timer counts down
        if (enemy[i].timer <= 0) {                                   // time for a new random direction
            enemy[i].angle = random_float(-PI, PI);
            enemy[i].timer = random_float(0.6f, 1.3f);
        }

        float want = enemy[i].angle;                                 // EASY: wander in the random direction
        if (mode != EASY) {                                          // MEDIUM and HARD: walk toward the player
            want = atan2f(player.y - enemy[i].y, player.x - enemy[i].x);   // angle from the enemy to the player
            if (mode == HARD) want += enemy[i].angle * 0.25f;        // HARD: add a random wobble
        }
        // Slowly change the velocity toward the wanted direction (smooth turning).
        enemy[i].vx += (cosf(want) * enemy[i].speed - enemy[i].vx) * enemy_turn[mode] * dt;
        enemy[i].vy += (sinf(want) * enemy[i].speed - enemy[i].vy) * enemy_turn[mode] * dt;
        enemy[i].x += enemy[i].vx * dt;                              // move
        enemy[i].y += enemy[i].vy * dt;

        float s = enemy[i].size;                                     // bounce off the four walls
        if (enemy[i].x < s) { enemy[i].x = s; enemy[i].vx = -enemy[i].vx; }
        if (enemy[i].x > WIDTH - s) { enemy[i].x = WIDTH - s; enemy[i].vx = -enemy[i].vx; }
        if (enemy[i].y < s) { enemy[i].y = s; enemy[i].vy = -enemy[i].vy; }
        if (enemy[i].y > HEIGHT - s) { enemy[i].y = HEIGHT - s; enemy[i].vy = -enemy[i].vy; }

        // Touching the player hurts, unless the player is blinking or has a shield.
        if (touching(enemy[i].x, enemy[i].y, s, player.x, player.y, PLAYER_SIZE)
            && player.safe <= 0 && player.shield <= 0) {
            float damage = enemy_damage[mode];                       // damage of this mode
            if (enemy[i].type == BLUE_ENEMY) damage = damage * 0.55f;   // blue enemies hurt less
            player.health -= damage;                                 // the player loses health
            player.safe = 0.92f;                                     // blink for a moment
            shake = 0.2f;                                            // small shake
            screen_flash = 0.1f;                                     // small flash
            float away = atan2f(player.y - enemy[i].y, player.x - enemy[i].x);   // direction from enemy to player
            player.x += cosf(away) * 30;                             // push the player away
            player.y += sinf(away) * 30;
            enemy[i].x -= cosf(away) * 22;                           // push the enemy the other way
            enemy[i].y -= sinf(away) * 22;
            explosion(player.x, player.y, C_PINK, 16, 150, 2.3f);    // sparks
            play(SND_HIT);                                           // hit sound
            if (player.health <= 0) { player.health = 0; end_run(); return; }   // dead: the run is over
        }
    }
}

// The banana: it disappears after a while; touching it gives +20 health and an 8-second shield.
void update_banana(float dt)
{
    banana_timer -= dt;                                              // count down to the next banana
    if (banana_timer <= 0) {                                         // time for a new banana
        banana_on = 1;                                               // show it
        banana_x = random_float(55, WIDTH - 55);                     // at a random place
        banana_y = random_float(75, HEIGHT - 75);
        banana_life = 8;                                             // it stays 8 seconds
        banana_timer = random_float(22, 34);                         // seconds until the next one
    }
    if (!banana_on) return;                                          // no banana: nothing more to do
    banana_life -= dt;                                               // the banana gets older
    if (banana_life <= 0) banana_on = 0;                             // too old: it disappears
    if (touching(banana_x, banana_y, 12, player.x, player.y, PLAYER_SIZE)) {
        banana_on = 0;                                               // the player took it
        player.health = fminf(max_health, player.health + 20);       // +20 health (not more than the maximum)
        player.shield = 8;                                           // 8 seconds of shield
        explosion(banana_x, banana_y, C_GOLD, 14, 120, 2.1f);        // gold sparks
        play(SND_BANANA);                                            // banana sound
    }
}

// Everything that happens in one frame of a run. The ORDER matters.
void update_run(float dt)
{
    if (IsKeyPressed(KEY_P) || IsKeyPressed(KEY_ESCAPE)) { state = PAUSED; return; }   // P or ESC pauses

    move_player(dt);                                                 // 1. walk and aim
    if (player.shoot_timer <= 0) {                                   // 2. auto-fire when the gun is ready
        shoot();
        player.shoot_timer = shoot_delay[mode];
    }
    update_bullets(dt);                                              // 3. move the bullets
    enemy_hits();                                                    // 4. bullets against enemies
    update_enemies(dt);                                              // 5. move enemies, hurt the player
    if (state != PLAYING) return;                                    // the player may have just died
    update_banana(dt);                                               // 6. the banana
    update_effects(dt);                                              // 7. sparks and timers

    if (score / 1000 != background) {                                // a new background every 1000 score
        background = score / 1000;
        bg_fade = 0.9f;                                              // show the change effect
    }

    spawn_timer -= dt;                                               // count down to the next enemy
    if (spawn_timer <= 0) {                                          // time for a new enemy
        spawn_enemy();
        float delay = spawn_delay[mode] / (1 + (level - 1) * 0.15f); // later levels spawn faster
        spawn_timer = fmaxf(0.27f, delay) * random_float(0.84f, 1.16f);
        if (mode == HARD && level >= 3 && rand() % 100 < 34) spawn_enemy();    // HARD: sometimes two at once
        if (mode != HARD && level >= 4 && rand() % 100 < 18) spawn_enemy();    // EASY and MEDIUM: later, less often
    }
}

// ======================= PART 9: SCREENS (KEYBOARD PART) =======================

// Start a new game in the chosen mode.
void start_game(void)
{
    start_audio();                                                   // sound may start after a click
    for (int i = 0; i < MAX_BULLETS; i++) bullet[i].on = 0;          // clear everything from the last game
    for (int i = 0; i < MAX_ENEMIES; i++) enemy[i].on = 0;
    for (int i = 0; i < MAX_PARTICLES; i++) particle[i].on = 0;
    banana_on = 0;                                                   // no banana yet
    score = 0;                                                       // score starts at 0
    level = 1;                                                       // level starts at 1
    background = 0;                                                  // first background
    new_best = 0;                                                    // no record yet
    shake = 0; screen_flash = 0; level_msg = 0; bg_fade = 0;         // no effects
    spawn_timer = 1;                                                 // first enemy after 1 second
    banana_timer = random_float(12, 19);                             // first banana after 12 to 19 seconds
    max_health = start_health[mode];                                 // health of this mode
    reset_player();                                                  // start in the middle
    state = PLAYING;                                                 // show the game
    play(SND_LEVEL);                                                 // start sound
}

void update_title(void)
{
    if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A)) mode = (mode + 2) % 3;    // previous mode (wraps around)
    if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D)) mode = (mode + 1) % 3;   // next mode (wraps around)
    if (IsKeyPressed(KEY_ONE)) mode = EASY;                          // number keys pick a mode directly
    if (IsKeyPressed(KEY_TWO)) mode = MEDIUM;
    if (IsKeyPressed(KEY_THREE)) mode = HARD;
    if (IsKeyPressed(KEY_B)) { state = BEST; return; }               // B shows the best scores
    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) { start_game(); return; }   // ENTER or SPACE starts
#if !defined(__EMSCRIPTEN__)
    if (IsKeyPressed(KEY_ESCAPE)) quit_game = 1;                     // ESC quits (not in the browser)
#endif
}

void update_pause(void)
{
    if (IsKeyPressed(KEY_P) || IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_ESCAPE)) state = PLAYING;   // resume
}

void update_over(float dt)
{
    over_timer -= dt;                                                // count the waiting time down
    if (IsKeyPressed(KEY_ESCAPE)) { state = TITLE; return; }         // ESC: back to the title
    if (over_timer > 0) return;                                      // still waiting: ignore the other keys
    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) start_game();   // play again
}

void update_best(void)
{
    if (IsKeyPressed(KEY_B) || IsKeyPressed(KEY_ESCAPE)) state = TITLE;     // back to the title
}

// ======================= PART 10: SMALL DRAWING HELPERS =======================

// Draw text in the middle of the screen.
void draw_center(const char text[], int y, int size, Color color)
{
    DrawText(text, (WIDTH - MeasureText(text, size)) / 2, y, size, color);   // x = (screen width - text width) / 2
}

// Draw text that ends at right_x (right aligned).
void draw_right(const char text[], int right_x, int y, int size, Color color)
{
    DrawText(text, right_x - MeasureText(text, size), y, size, color);       // x = right edge - text width
}

// A dark rounded box with a colored border.
void panel(float x, float y, float w, float h, Color color)
{
    Rectangle box = { x, y, w, h };                                          // the box position and size
    DrawRectangleRounded(box, 0.12f, 12, C_PANEL);                           // the dark inside
    DrawRectangleRoundedLinesEx(box, 0.12f, 12, 1, Fade(color, 0.5f));       // the colored border
}

// A button: it draws itself and returns 1 when it was clicked (or touched) in this frame.
int button(float x, float y, float w, float h, const char text[], Color color, int selected)
{
    Vector2 mouse = GetMousePosition();                                      // where the mouse is
    int hover = mouse.x >= x && mouse.x <= x + w && mouse.y >= y && mouse.y <= y + h;   // is the mouse on the button?
    int lit = selected || hover;                                             // light the button up?
    Rectangle box = { x, y, w, h };                                          // the button position and size
    DrawRectangleRounded(box, 0.16f, 10, lit ? Fade(color, 0.18f) : C_PANEL);              // background
    DrawRectangleRoundedLinesEx(box, 0.16f, 10, lit ? 2 : 1, lit ? color : C_GRAY);        // border
    int size = 18;                                                           // start with big text
    while (size > 10 && MeasureText(text, size) > w - 20) size--;            // make the text smaller until it fits
    DrawText(text, x + (w - MeasureText(text, size)) / 2, y + (h - size) / 2, size, lit ? C_TEXT : C_GRAY);   // the label
    return clicked && pointer_x >= x && pointer_x <= x + w && pointer_y >= y && pointer_y <= y + h;   // clicked on it?
}

// ======================= PART 11: DRAWING THE PLAYER, ENEMIES, BACKGROUND AND THE ARENA =======================

// Draw the background picture for the current sector, stretched to fill the screen.
void draw_background(void)
{
    int n = background % 5;                                                  // which of the 5 pictures (0 to 4)
    draw_sprite(tex_bg[n], WIDTH / 2, HEIGHT / 2, WIDTH, HEIGHT, 0, WHITE);  // draw_sprite centers on (x,y), so use the middle of the screen
    if (bg_fade > 0) DrawRectangle(0, 0, WIDTH, HEIGHT, Fade((Color){ 2, 2, 12, 255 }, bg_fade * 0.3f));   // dark veil when the scene changes
}

// Draw the player, turned to face where it is looking, gold-tinted while shielded.
void draw_player(void)
{
    int blink = player.safe > 0 && ((int)(world_time * 22) % 2 == 0);        // blink while "cannot be hurt"
    Color tint = (player.shield > 0) ? C_GOLD : WHITE;                       // gold tint while shielded
    if (blink) tint = Fade(tint, 0.35f);                                     // see-through on the blink frames
    float degrees = player.angle * 180.0f / PI;                             // turn radians into degrees
    draw_sprite(tex_player, player.x, player.y, 40, 40, degrees, tint);
    if (player.shield > 0) DrawCircleLines(player.x, player.y, 27 + sinf(world_time * 8) * 2, C_GOLD);   // shield ring
}

// Draw enemy number i (red or blue), turned the way it moves, with a health bar once it has been hit.
void draw_enemy(int i)
{
    Texture2D tex = (enemy[i].type == BLUE_ENEMY) ? tex_blue : tex_red;      // which picture to use
    Color tint = (enemy[i].flash > 0) ? C_GOLD : WHITE;                      // gold flash when hit
    float degrees = atan2f(enemy[i].vy, enemy[i].vx) * 180.0f / PI;          // turn radians into degrees
    float size = enemy[i].size * 2;                                         // picture size from the hit-test size
    draw_sprite(tex, enemy[i].x, enemy[i].y, size, size, degrees, tint);
    if (enemy[i].health < enemy[i].max_health) {                             // health bar, only after the enemy was hurt
        float bar = size * 1.1f;                                             // width of the bar
        DrawRectangle(enemy[i].x - bar / 2, enemy[i].y - size / 2 - 9, bar, 4, C_DARK);   // empty bar
        DrawRectangle(enemy[i].x - bar / 2, enemy[i].y - size / 2 - 9, bar * limit(enemy[i].health / enemy[i].max_health, 0, 1), 4, C_GOLD);   // full part
    }
}

// Draw everything inside the arena: banana, bullets, enemies, sparks, player.
void draw_world(void)
{
    if (banana_on) draw_sprite(tex_banana, banana_x, banana_y, 30, 30, 0, WHITE);         // the banana
    for (int i = 0; i < MAX_BULLETS; i++) {                                  // bullets
        if (!bullet[i].on) continue;                                         // skip unused slots
        float degrees = atan2f(bullet[i].vy, bullet[i].vx) * 180.0f / PI;    // turn radians into degrees
        draw_sprite(tex_bullet, bullet[i].x, bullet[i].y, BULLET_SIZE * 4, BULLET_SIZE * 4, degrees, WHITE);
    }
    for (int i = 0; i < MAX_ENEMIES; i++) if (enemy[i].on) draw_enemy(i);    // enemies
    for (int i = 0; i < MAX_PARTICLES; i++) {                                // sparks
        if (!particle[i].on) continue;                                       // skip unused slots
        float k = particle[i].life / particle[i].max_life;                   // 1 = new, 0 = gone
        DrawCircle(particle[i].x, particle[i].y, particle[i].size * (0.4f + 0.6f * k), Fade(particle[i].color, k));
    }
    draw_player();
}

// ======================= PART 12: DRAWING THE SCREENS AND ONE WHOLE FRAME =======================

// The top bar during a run: score, level, sector and health.
void draw_hud(void)
{
    float w = WIDTH, h = HEIGHT;                                             // short names
    float ratio = limit(player.health / max_health, 0, 1);                   // health from 0 to 1
    Color health_color = (ratio > 0.5f) ? C_GREEN : ((ratio > 0.25f) ? C_GOLD : C_RED);   // green, gold or red
    float progress = (score % 1000) / 1000.0f;                               // how close to the next background

    panel(18, 16, 248, 90, C_CYAN);                                          // left box: score
    DrawText("SCORE", 34, 27, 12, C_GRAY);
    DrawText(TextFormat("%07d", score), 31, 43, 31, C_TEXT);
    DrawText("SURVIVE THE RIFT", 34, 80, 10, C_CYAN);

    float mx = w / 2 - 144;                                                  // middle box: level and sector
    panel(mx, 16, 288, 90, C_PURPLE);
    draw_center(TextFormat("RIFT LEVEL  %02d", level), 25, 15, C_TEXT);
    draw_center(TextFormat("SECTOR %02d  //  %s", background + 1, sector_name[background % 5]), 47, 10, C_CYAN);
    DrawRectangle(mx + 24, 70, 240, 6, C_DARK);                              // progress bar background
    DrawRectangle(mx + 24, 70, 240 * progress, 6, C_CYAN);                   // progress bar
    draw_center(TextFormat("%d%% TO NEXT ENVIRONMENT", (int)(progress * 100)), 82, 9, C_GRAY);

    float rx = w - 242;                                                      // right box: mode and health
    panel(rx, 16, 224, 90, mode_color[mode]);
    DrawText(mode_name[mode], rx + 18, 27, 12, mode_color[mode]);
    DrawText("INTEGRITY", rx + 18, 47, 10, C_GRAY);
    DrawRectangle(rx + 18, 63, 188, 10, C_DARK);                             // health bar background
    DrawRectangle(rx + 18, 63, 188 * ratio, 10, health_color);               // health bar
    draw_right(TextFormat("%d%%", (int)(ratio * 100)), rx + 206, 79, 11, C_TEXT);

    if (bg_fade > 0.02f) {                                                   // message: the background changes
        draw_center(TextFormat("SECTOR SHIFT  //  %s", sector_name[background % 5]), h * 0.2f, 18, Fade(C_CYAN, limit(bg_fade / 0.55f, 0, 1)));
    } else if (level_msg > 0) {                                              // message: the level went up
        draw_center("RIFT LEVEL INCREASED  //  PRESSURE RISING", h * 0.2f, 15, Fade(C_GOLD, limit(level_msg * 1.7f, 0, 1)));
    }
    if (player.shield > 0) DrawText(TextFormat("SHIELD  %.1fs", player.shield), 24, h - 30, 13, C_GOLD);   // shield time left
}

// The title screen: 3 mode cards, a play button and help text.
void draw_title(void)
{
    float w = WIDTH, h = HEIGHT;                                             // short names
    int size = (w < 720) ? 40 : 57;                                          // smaller title in narrow windows
    int top = h * 0.1f;                                                      // where the title starts
    draw_center("RIFT RUNNER", top, size, C_TEXT);
    draw_center("SIGNAL ZERO  *  SURVIVE THE SIGNAL. OUTLAST THE VOID.", top + size + 8, 12, C_GRAY);

    float gap = 16, card_w = fminf(220, (w - 40 - gap * 2) / 3);             // width of one card
    float start = (w - (card_w * 3 + gap * 2)) / 2;                          // x of the first card, so the row is centered
    float card_y = h * 0.40f;                                                // y of the cards
    draw_center("CHOOSE A MODE", card_y - 24, 12, C_CYAN);
    for (int i = 0; i < 3; i++) {                                            // the 3 cards
        float x = start + i * (card_w + gap);
        if (button(x, card_y, card_w, 100, mode_name[i], mode_color[i], mode == i)) mode = i;   // a click picks the mode
        if (card_w > 120) DrawText(mode_note[i], x + 10, card_y + 74, 10, C_GRAY);              // small description
    }
    float bw = fminf(300, w - 36);                                           // width of the play button
    if (button(w / 2 - bw / 2, card_y + 120, bw, 52, "PLAY  [ENTER]", C_CYAN, 1)) start_game();   // start the game
    if (button(w / 2 - 110, card_y + 184, 220, 34, "PERSONAL BESTS  [B]", C_PURPLE, 0)) state = BEST;   // show the records
    draw_center("WASD / ARROWS MOVE  *  HOLD MOUSE TO AIM  *  AUTO-FIRE  *  P PAUSE", card_y + 236, 11, C_TEXT);
    draw_center("NEW BACKGROUND EVERY 1000 SCORE  *  BESTS ARE SAVED ON THIS DEVICE", card_y + 254, 10, C_GRAY);
}

// The best-scores screen.
void draw_best(void)
{
    float w = WIDTH, h = HEIGHT;                                             // short names
    draw_center("PERSONAL BESTS", h * 0.08f, 30, C_TEXT);
    draw_center("ONE HIGH SCORE FOR EACH MODE", h * 0.08f + 40, 11, C_GRAY);
    float pw = fminf(600, w * 0.9f);                                         // width of the box
    float px = (w - pw) / 2, py = h * 0.08f + 70;                            // top-left corner of the box
    panel(px, py, pw, 3 * 66 + 20, C_PURPLE);
    for (int i = 0; i < 3; i++) {                                            // one row for each mode
        float ry = py + 14 + i * 66;                                         // y of this row
        DrawText(mode_name[i], px + 24, ry + 18, 18, mode_color[i]);
        if (best[i] > 0) draw_right(TextFormat("%07d", best[i]), px + pw - 24, ry + 14, 26, C_TEXT);   // the record
        else draw_right("N/A", px + pw - 24, ry + 14, 26, C_GRAY);           // no record yet
    }
    if (button(w / 2 - 110, py + 3 * 66 + 40, 220, 42, "BACK  [B]", C_CYAN, 1)) state = TITLE;   // go back
}

// The pause box.
void draw_pause(void)
{
    float w = WIDTH, h = HEIGHT;                                             // short names
    float pw = fminf(420, w - 32);                                           // width of the box
    DrawRectangle(0, 0, w, h, (Color){ 3, 4, 13, 195 });                     // darken the game
    panel(w / 2 - pw / 2, h / 2 - 110, pw, 220, C_CYAN);
    draw_center("PAUSED", h / 2 - 90, 36, C_TEXT);
    float bw = fminf(300, pw - 24);                                          // width of the buttons
    if (button(w / 2 - bw / 2, h / 2 - 20, bw, 48, "RESUME  [P / ENTER]", C_CYAN, 1)) state = PLAYING;
    if (button(w / 2 - bw / 2, h / 2 + 40, bw, 42, "RETURN TO TITLE", C_PURPLE, 0)) state = TITLE;
}

// The game-over box.
void draw_over(void)
{
    float w = WIDTH, h = HEIGHT;                                             // short names
    float pw = fminf(440, w - 24), px = (w - pw) / 2, py = h / 2 - 180;      // the box
    DrawRectangle(0, 0, w, h, (Color){ 3, 4, 13, 190 });                     // darken the game
    panel(px, py, pw, 360, C_PINK);
    draw_center("RUN TERMINATED", py + 28, 25, C_PINK);
    draw_center("FINAL SCORE", py + 80, 12, C_GRAY);
    draw_center(TextFormat("%07d", score), py + 100, 51, C_TEXT);
    draw_center(TextFormat("%s    /    RIFT LEVEL %02d", mode_name[mode], level), py + 165, 12, mode_color[mode]);
    if (new_best) draw_center("NEW PERSONAL BEST FOR THIS MODE!", py + 200, 15, C_GOLD);
    else draw_center("MODE BEST: SCORE NOT BEATEN", py + 200, 11, C_GRAY);
    float bw = pw - 48;                                                      // width of the buttons
    if (button(px + 24, py + 245, bw, 50, "PLAY AGAIN  [ENTER]", C_CYAN, 1)) start_game();
    if (button(px + 24, py + 305, bw, 40, "RETURN TO TITLE  [ESC]", C_PURPLE, 0)) state = TITLE;
}

// Phone: circles that show where to touch (only on small windows or when touched).
void draw_touch_controls(void)
{
    if (WIDTH > 820 && GetTouchPointCount() <= 0) return;                    // big window and no finger: not needed
    DrawCircleLines(105, HEIGHT - 126, 55, Fade(C_CYAN, 0.4f));              // left circle: move
    DrawCircleLines(WIDTH - 84, HEIGHT - 92, 39, Fade(C_CYAN, 0.4f));        // right circle: aim
    DrawText("AIM", WIDTH - 111, HEIGHT - 98, 11, C_CYAN);
}

// Draw one whole frame: background, world, the active screen, and the white flash.
void draw_frame(void)
{
    float sx = 0, sy = 0;                                                    // how far the camera shakes
    if (shake > 0) {                                                         // shake with sine waves
        sx = sinf(world_time * 61) * shake * 12;
        sy = cosf(world_time * 67) * shake * 12;
    }
    Camera2D camera = { 0 };                                                 // a camera that can shake the world
    camera.offset = (Vector2){ WIDTH / 2 + sx, HEIGHT / 2 + sy };            // where the camera looks at, on the screen
    camera.target = (Vector2){ WIDTH / 2, HEIGHT / 2 };                      // the middle of the world
    camera.zoom = 1;                                                         // no zoom

    BeginDrawing();                                                          // start drawing this frame
    BeginMode2D(camera);                                                     // everything until EndMode2D shakes
    draw_background();                                                       // the background picture
    if (state == PLAYING || state == PAUSED || state == GAME_OVER) draw_world();   // the arena
    EndMode2D();                                                             // the screens below do not shake

    switch (state) {                                                         // the screen on top
    case TITLE:     draw_title(); break;
    case PLAYING:   draw_hud(); draw_touch_controls(); break;
    case PAUSED:    draw_hud(); draw_pause(); break;
    case GAME_OVER: draw_over(); break;
    case BEST:      draw_best(); break;
    }

    if (screen_flash > 0) DrawRectangle(0, 0, WIDTH, HEIGHT, Fade((Color){ 188, 226, 255, 255 }, limit(screen_flash, 0, 0.4f)));   // white flash
    EndDrawing();                                                            // show the frame
}

// ======================= PART 13: THE MAIN LOOP =======================

// One frame: read input, update the active screen, then draw. Runs about 60 times a second.
void game_frame(void)
{
    float dt = GetFrameTime();                                       // seconds since the last frame
    if (dt > 0.04f) dt = 0.04f;                                      // avoid a giant jump after a lag spike
    world_time += dt;                                                // seconds since the program started
    read_pointer();                                                  // remember mouse and touch presses

    switch (state) {                                                 // do the work of the active screen
    case TITLE:     update_title();  update_effects(dt);               break;
    case PLAYING:   update_run(dt);                                     break;
    case PAUSED:    update_pause();                                     break;
    case GAME_OVER: update_over(dt); update_effects(dt);               break;
    case BEST:      update_best();                                     break;
    }
    draw_frame();                                                    // draw everything
}

// The program starts here.
int main(void)
{
#if defined(__EMSCRIPTEN__)
    SetConfigFlags(FLAG_MSAA_4X_HINT);                               // web: smooth edges
#else
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT | FLAG_MSAA_4X_HINT);   // desktop: resizable window
#endif
    InitWindow(1280, 720, GAME_NAME);                                // create the window
    if (!IsWindowReady()) return 1;                                  // could not open a window: stop
#if !defined(__EMSCRIPTEN__)
    SetWindowMinSize(800, 480);                                      // desktop: not smaller than this
#endif
    SetTargetFPS(60);                                                // about 60 frames per second
    SetExitKey(KEY_NULL);                                            // ESC must not close the window by itself

    srand((unsigned int)time(NULL));                                 // different random numbers every run
    load_pictures();                                                 // load the 10 pictures (window must be open first)
    load_best();                                                     // read the saved best scores

#if defined(__EMSCRIPTEN__)
    emscripten_set_main_loop(game_frame, 0, 1);                      // the browser calls game_frame for us
#else
    while (!WindowShouldClose() && !quit_game) game_frame();         // repeat until the window closes
    unload_pictures();                                               // free the pictures
    if (audio_ready) {                                               // clean up the sound
        for (int i = 0; i < 5; i++) if (IsSoundValid(sounds[i])) UnloadSound(sounds[i]);
        CloseAudioDevice();
    }
    CloseWindow();                                                   // close the window
#endif
    return 0;                                                        // the program ended normally
}
