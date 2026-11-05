/*
 * RIFT RUNNER: SIGNAL ZERO  -  main.c   (single file, picture edition)
 *
 * This is the complete game source. It uses beginner C topics such as
 * variables, arrays, structs, conditions, loops and functions.
 * The code is grouped by job: data, input, game rules, drawing and main loop.
 *
 * Modes: EASY, MEDIUM, HARD. You run and shoot alien enemies, grab the
 * banana for health and a shield. The same space background stays on screen
 * for the whole game, so the player can focus on surviving and scoring.
 *
 * PICTURES: the player, the two enemy types, the bullet, the banana and
 * one background are real image files, not drawn shapes. They must sit
 * in a folder called "assets/images/" next to this program, using these
 * exact names:
 *   player.png, red_enemy.png, blue_enemy.png, bullet.png, banana.png,
 *   bg_cradle.png
 * If a picture is missing, the game still runs - that one thing is just
 * invisible, nothing crashes.
 *
 * Function names describe the job each section performs.
 */

// ======================= PART 1: LIBRARIES AND SETTINGS =======================

#include <raylib.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#if defined(__EMSCRIPTEN__)
#include <emscripten.h>
#endif

#define GAME_NAME "RIFT RUNNER: SIGNAL ZERO"
#define WIDTH GetScreenWidth()
#define HEIGHT GetScreenHeight()
#define MAX_BULLETS 200
#define MAX_ENEMIES 60
#define MAX_PARTICLES 300
#define PLAYER_SIZE 16
#define BULLET_SIZE 4

// Names for numbers. EASY = 0, MEDIUM = 1, HARD = 2.
enum { TITLE, PLAYING, PAUSED, GAME_OVER, BEST };
enum { EASY, MEDIUM, HARD };
enum { RED_ENEMY, BLUE_ENEMY };
enum { SND_SHOT, SND_HIT, SND_BANANA, SND_BOOM, SND_LEVEL };

// ======================= PART 2: GAME DATA =======================

#define C_PANEL  (Color){ 15, 21, 45, 238 }
#define C_CYAN   (Color){ 76, 231, 255, 255 }
#define C_PURPLE (Color){ 179, 91, 255, 255 }
#define C_PINK   (Color){ 255, 83, 170, 255 }
#define C_GREEN  (Color){ 98, 255, 178, 255 }
#define C_GOLD   (Color){ 255, 207, 100, 255 }
#define C_TEXT   (Color){ 236, 243, 255, 255 }
#define C_GRAY   (Color){ 143, 158, 196, 255 }
#define C_RED    (Color){ 255, 83, 99, 255 }
#define C_DARK   (Color){ 38, 47, 74, 255 }

// These three arrays use the same order as EASY, MEDIUM and HARD.
static const char *mode_names[3] = { "EASY", "MEDIUM", "HARD" };
static const char *mode_notes[3] = { "Learn the arena", "Stay moving", "Overwhelming pursuit" };

// A struct groups the information about ONE thing.
typedef struct { float x, y, angle, health, shoot_timer, shield, safe; } Player;
typedef struct { int on; float x, y, vx, vy, life; } Bullet;
typedef struct { int on, type; float x, y, vx, vy, size, speed, health, max_health, angle, timer, flash; } Enemy;
typedef struct { int on; float x, y, vx, vy, size, life, max_life; Color color; } Particle;

Player player;
Bullet bullet[MAX_BULLETS];
Enemy enemy[MAX_ENEMIES];
Particle particle[MAX_PARTICLES];

// Variables any function can use.
int state = TITLE;
int mode = MEDIUM;
int best[3] = { 0, 0, 0 };
int score = 0, level = 1, new_best = 0;
float max_health = 100;
float spawn_timer = 0, banana_timer = 0, over_timer = 0;
int banana_on = 0;
float banana_x = 0, banana_y = 0, banana_life = 0;
float shake = 0, screen_flash = 0, level_msg = 0, world_time = 0;
int audio_ready = 0, quit_game = 0;
int clicked = 0, finger_was_down = 0;
float pointer_x = 0, pointer_y = 0;
float touch_x = 0, touch_y = 0;
int touch_aiming = 0;
Sound sounds[5];

// Difficulty settings are stored in arrays using the same EASY, MEDIUM, HARD order.
float player_speed[3]  = { 292, 276, 266 };
float shoot_delay[3]   = { 0.245f, 0.225f, 0.205f };
float spawn_delay[3]   = { 1.02f, 0.78f, 0.58f };
float enemy_speed[3]   = { 72, 95, 120 };
float enemy_damage[3]  = { 13, 25, 34 };
float start_health[3]  = { 130, 100, 85 };
float enemy_turn[3]    = { 1.2f, 0.8f, 1.1f };

// The five game sprites and the one permanent background.
Texture2D tex_player;
Texture2D tex_red;
Texture2D tex_blue;
Texture2D tex_bullet;
Texture2D tex_banana;
Texture2D tex_bg;

// ======================= PART 3: SMALL HELPER FUNCTIONS =======================

float random_float(float low, float high)
{
    return low + ((float)rand() / (float)RAND_MAX) * (high - low);
}

float limit(float value, float low, float high)
{
    if (value < low) return low;
    if (value > high) return high;
    return value;
}

// Circle collision: compare squared distances so no square root is needed.
int touching(float x1, float y1, float size1, float x2, float y2, float size2)
{
    float dx = x1 - x2, dy = y1 - y2;
    float together = size1 + size2;
    return dx * dx + dy * dy <= together * together;
}

Color mode_color(int selected_mode)
{
    if (selected_mode == EASY) return C_GREEN;
    if (selected_mode == MEDIUM) return C_CYAN;
    return C_PINK;
}

void draw_sprite(Texture2D tex, float x, float y, float w, float h, float angle_deg, Color tint)
{
    Rectangle source = { 0, 0, (float)tex.width, (float)tex.height };
    Rectangle dest = { x, y, w, h };
    Vector2 origin = { w / 2, h / 2 };
    DrawTexturePro(tex, source, dest, origin, angle_deg, tint);
}

// ======================= PART 4: PICTURES, SOUND AND SAVED BEST SCORES =======================

void load_pictures(void)
{
    tex_player = LoadTexture("assets/images/player.png");
    tex_red    = LoadTexture("assets/images/red_enemy.png");
    tex_blue   = LoadTexture("assets/images/blue_enemy.png");
    tex_bullet = LoadTexture("assets/images/bullet.png");
    tex_banana = LoadTexture("assets/images/banana.png");
    tex_bg = LoadTexture("assets/images/bg_cradle.png");
}

void unload_pictures(void)
{
    UnloadTexture(tex_player);
    UnloadTexture(tex_red);
    UnloadTexture(tex_blue);
    UnloadTexture(tex_bullet);
    UnloadTexture(tex_banana);
    UnloadTexture(tex_bg);
}

void start_audio(void)
{
    if (audio_ready) return;
    InitAudioDevice();
    audio_ready = IsAudioDeviceReady();
    if (audio_ready) {
        const char *files[5] = {
            "assets/shot.wav", "assets/impact.wav", "assets/pickup.wav",
            "assets/enemy_down.wav", "assets/surge.wav"
        };
        for (int i = 0; i < 5; i++) sounds[i] = LoadSound(files[i]);
    }
}

void play(int n)
{
    if (audio_ready && IsSoundValid(sounds[n])) PlaySound(sounds[n]);
}

#if defined(__EMSCRIPTEN__)
#define SCORE_FILE "/persist/rift_runner_scores.txt"
#else
#define SCORE_FILE "rift_runner_scores.txt"
#endif

// Write the 3 best scores into the score file (one number per line).
void save_best(void)
{
    FILE *f = fopen(SCORE_FILE, "w");
    if (f == NULL) return;
    for (int i = 0; i < 3; i++) fprintf(f, "%d\n", best[i]);
    fclose(f);
#if defined(__EMSCRIPTEN__)
    EM_ASM({
        if (typeof FS !== 'undefined' && typeof IDBFS !== 'undefined')
            FS.syncfs(false, function(error) { if (error) console.warn('score save failed', error); });
    }, 0);
#endif
}

// Read the 3 best scores from the score file (if it exists).
void load_best(void)
{
    FILE *f = fopen(SCORE_FILE, "r");
    if (f == NULL) return;
    char text[30];
    for (int i = 0; i < 3; i++) {
        if (fgets(text, 30, f) != NULL) best[i] = atoi(text);
        if (best[i] < 0) best[i] = 0;
    }
    fclose(f);
}

// ======================= PART 5: SPARKS (PARTICLE EFFECTS) =======================

void explosion(float x, float y, Color color, int count, float speed, float size)
{
    for (int k = 0; k < count; k++) {
        int i = 0;
        while (i < MAX_PARTICLES && particle[i].on) i++;
        if (i == MAX_PARTICLES) return;
        float angle = random_float(0, 2 * PI);
        float fast = random_float(speed * 0.35f, speed);
        particle[i].on = 1;
        particle[i].x = x;
        particle[i].y = y;
        particle[i].vx = cosf(angle) * fast;
        particle[i].vy = sinf(angle) * fast;
        particle[i].size = random_float(size * 0.55f, size);
        particle[i].life = random_float(0.24f, 0.72f);
        particle[i].max_life = particle[i].life;
        particle[i].color = color;
    }
}

void update_effects(float dt)
{
    for (int i = 0; i < MAX_PARTICLES; i++) {
        if (!particle[i].on) continue;
        particle[i].life -= dt;
        if (particle[i].life <= 0) { particle[i].on = 0; continue; }
        particle[i].x += particle[i].vx * dt;
        particle[i].y += particle[i].vy * dt;
        particle[i].vx -= particle[i].vx * 2.5f * dt;
        particle[i].vy -= particle[i].vy * 2.5f * dt;
    }
    shake = fmaxf(0, shake - dt);
    screen_flash = fmaxf(0, screen_flash - 1.75f * dt);
    level_msg = fmaxf(0, level_msg - dt);
}

// ======================= PART 6: INPUT =======================

// On a phone, left-side touches move the player and right-side touches aim.
void read_touch(void)
{
    touch_x = 0;
    touch_y = 0;
    touch_aiming = 0;
    for (int i = 0; i < GetTouchPointCount(); i++) {
        Vector2 finger = GetTouchPosition(i);
        if (finger.x < WIDTH * 0.47f) {
            touch_x += (finger.x - 105) / 62;
            touch_y += (finger.y - (HEIGHT - 126)) / 62;
        } else {
            player.angle = atan2f(finger.y - player.y, finger.x - player.x);
            touch_aiming = 1;
        }
    }
}

// ======================= PART 7: PLAYER AND BULLET RULES =======================

void reset_player(void)
{
    player.x = WIDTH / 2;
    player.y = HEIGHT / 2;
    player.angle = 0;
    player.health = max_health;
    player.shoot_timer = 0.2f;
    player.shield = 0;
    player.safe = 0.65f;
}

void shoot(void)
{
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (bullet[i].on) continue;
        float speed = 620 + fminf((level - 1) * 5.0f, 90);
        bullet[i].on = 1;
        bullet[i].x = player.x + cosf(player.angle) * 16;
        bullet[i].y = player.y + sinf(player.angle) * 16;
        bullet[i].vx = cosf(player.angle) * speed;
        bullet[i].vy = sinf(player.angle) * speed;
        bullet[i].life = 1.3f;
        play(SND_SHOT);
        return;
    }
}

// Read keyboard, mouse and touch input, then keep the player inside the screen.
void move_player(float dt)
{
    float dx = 0, dy = 0;
    if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) dx -= 1;
    if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) dx += 1;
    if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)) dy -= 1;
    if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) dy += 1;
    touch_aiming = 0;
    read_touch();
    dx += touch_x;
    dy += touch_y;
    float length = sqrtf(dx * dx + dy * dy);
    if (length > 1) { dx = dx / length; dy = dy / length; }
    if (length > 0.14f && !touch_aiming) player.angle = atan2f(dy, dx);
    if (GetTouchPointCount() == 0 && (IsMouseButtonDown(MOUSE_BUTTON_LEFT) || IsMouseButtonDown(MOUSE_BUTTON_RIGHT))) {
        Vector2 mouse = GetMousePosition();
        player.angle = atan2f(mouse.y - player.y, mouse.x - player.x);
    }
    player.x += dx * player_speed[mode] * dt;
    player.y += dy * player_speed[mode] * dt;
    player.x = limit(player.x, 28, WIDTH - 28);
    player.y = limit(player.y, 34, HEIGHT - 30);
    player.shield = fmaxf(0, player.shield - dt);
    player.safe = fmaxf(0, player.safe - dt);
    player.shoot_timer -= dt;
}

// Move bullets and deactivate them when their lifetime ends or they leave the screen.
void update_bullets(float dt)
{
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (!bullet[i].on) continue;
        bullet[i].x += bullet[i].vx * dt;
        bullet[i].y += bullet[i].vy * dt;
        bullet[i].life -= dt;
        if (bullet[i].life <= 0 || bullet[i].x < -30 || bullet[i].x > WIDTH + 30
            || bullet[i].y < -30 || bullet[i].y > HEIGHT + 30) {
            bullet[i].on = 0;
        }
    }
}

// ======================= PART 8: GAME RULES =======================

// Give points, create an explosion and raise the level when the score reaches 500-point steps.
void kill_enemy(int i)
{
    int old_level = level;
    enemy[i].on = 0;
    score += 200;
    explosion(enemy[i].x, enemy[i].y, C_RED, 26, 215, 3.2f);
    play(SND_BOOM);
    level = 1 + score / 500;
    if (level > old_level) { level_msg = 2; play(SND_LEVEL); }
}

void enemy_hits(void)
{
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (!bullet[i].on) continue;
        for (int j = 0; j < MAX_ENEMIES; j++) {
            if (!enemy[j].on) continue;
            if (touching(bullet[i].x, bullet[i].y, BULLET_SIZE, enemy[j].x, enemy[j].y, enemy[j].size)) {
                bullet[i].on = 0;
                enemy[j].health -= 1;
                enemy[j].flash = 0.1f;
                explosion(bullet[i].x, bullet[i].y, C_CYAN, 4, 85, 1.7f);
                if (enemy[j].health <= 0) kill_enemy(j);
                break;
            }
        }
    }
}

// Choose a random edge, then fill the first unused enemy slot.
void spawn_enemy(void)
{
    int alive = 0;
    for (int i = 0; i < MAX_ENEMIES; i++) if (enemy[i].on) alive++;
    int most = 9 + level + mode * 3;
    if (most > 45) most = 45;
    if (alive >= most) return;

    float x, y;
    int side = rand() % 4;
    switch (side) {
    case 0: x = random_float(25, WIDTH - 25); y = 25; break;
    case 1: x = WIDTH - 25; y = random_float(25, HEIGHT - 25); break;
    case 2: x = random_float(25, WIDTH - 25); y = HEIGHT - 25; break;
    default: x = 25; y = random_float(25, HEIGHT - 25); break;
    }

    int type = (rand() % 100 < 42) ? BLUE_ENEMY : RED_ENEMY;
    float speed = enemy_speed[mode] * (type == BLUE_ENEMY ? 0.78f : 0.70f) + fminf((level - 1) * 3.0f, 78);
    float health;
    if (type == BLUE_ENEMY) health = 1 + (level - 1) / 6;
    else health = 2 + (level - 1) / 4;
    if (health > 5) health = 5;

    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (enemy[i].on) continue;
        float first = atan2f(HEIGHT / 2 - y, WIDTH / 2 - x);
        enemy[i].on = 1;
        enemy[i].type = type;
        enemy[i].x = x;
        enemy[i].y = y;
        enemy[i].speed = speed;
        enemy[i].vx = cosf(first) * speed;
        enemy[i].vy = sinf(first) * speed;
        enemy[i].size = (type == BLUE_ENEMY) ? 15.5f : 21;
        enemy[i].health = health;
        enemy[i].max_health = health;
        enemy[i].angle = random_float(-PI, PI);
        enemy[i].timer = random_float(0.6f, 1.3f);
        enemy[i].flash = 0;
        return;
    }
}

void end_run(void)
{
    state = GAME_OVER;
    over_timer = 0.6f;
    new_best = score > best[mode];
    if (new_best) { best[mode] = score; save_best(); }
    shake = 0.4f;
    screen_flash = 0.3f;
    explosion(player.x, player.y, C_PINK, 55, 260, 4);
    play(SND_HIT);
}

// Easy enemies wander; Medium and Hard enemies steer toward the player.
void update_enemies(float dt)
{
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (!enemy[i].on) continue;
        enemy[i].flash = fmaxf(0, enemy[i].flash - dt);
        enemy[i].timer -= dt;
        if (enemy[i].timer <= 0) {
            enemy[i].angle = random_float(-PI, PI);
            enemy[i].timer = random_float(0.6f, 1.3f);
        }

        float want = enemy[i].angle;
        if (mode != EASY) {
            want = atan2f(player.y - enemy[i].y, player.x - enemy[i].x);
            if (mode == HARD) want += enemy[i].angle * 0.25f;
        }
        // Slowly change the velocity toward the wanted direction (smooth turning).
        enemy[i].vx += (cosf(want) * enemy[i].speed - enemy[i].vx) * enemy_turn[mode] * dt;
        enemy[i].vy += (sinf(want) * enemy[i].speed - enemy[i].vy) * enemy_turn[mode] * dt;
        enemy[i].x += enemy[i].vx * dt;
        enemy[i].y += enemy[i].vy * dt;

        float s = enemy[i].size;
        if (enemy[i].x < s) { enemy[i].x = s; enemy[i].vx = -enemy[i].vx; }
        if (enemy[i].x > WIDTH - s) { enemy[i].x = WIDTH - s; enemy[i].vx = -enemy[i].vx; }
        if (enemy[i].y < s) { enemy[i].y = s; enemy[i].vy = -enemy[i].vy; }
        if (enemy[i].y > HEIGHT - s) { enemy[i].y = HEIGHT - s; enemy[i].vy = -enemy[i].vy; }

        // Touching the player hurts, unless the player is blinking or has a shield.
        if (touching(enemy[i].x, enemy[i].y, s, player.x, player.y, PLAYER_SIZE)
            && player.safe <= 0 && player.shield <= 0) {
            float damage = enemy_damage[mode];
            if (enemy[i].type == BLUE_ENEMY) damage = damage * 0.55f;
            player.health -= damage;
            player.safe = 0.92f;
            shake = 0.2f;
            screen_flash = 0.1f;
            float away = atan2f(player.y - enemy[i].y, player.x - enemy[i].x);
            player.x += cosf(away) * 30;
            player.y += sinf(away) * 30;
            enemy[i].x -= cosf(away) * 22;
            enemy[i].y -= sinf(away) * 22;
            explosion(player.x, player.y, C_PINK, 16, 150, 2.3f);
            play(SND_HIT);
            if (player.health <= 0) { player.health = 0; end_run(); return; }
        }
    }
}

// Spawn a banana from time to time. It heals the player and gives temporary protection.
void update_banana(float dt)
{
    banana_timer -= dt;
    if (banana_timer <= 0) {
        banana_on = 1;
        banana_x = random_float(55, WIDTH - 55);
        banana_y = random_float(75, HEIGHT - 75);
        banana_life = 8;
        banana_timer = random_float(22, 34);
    }
    if (!banana_on) return;
    banana_life -= dt;
    if (banana_life <= 0) {
        banana_on = 0;
        return;
    }
    if (touching(banana_x, banana_y, 12, player.x, player.y, PLAYER_SIZE)) {
        banana_on = 0;
        player.health = fminf(max_health, player.health + 20);
        player.shield = 8;
        explosion(banana_x, banana_y, C_GOLD, 14, 120, 2.1f);
        play(SND_BANANA);
    }
}

void update_run(float dt)
{
    if (IsKeyPressed(KEY_P) || IsKeyPressed(KEY_ESCAPE)) { state = PAUSED; return; }

    // Run the frame in a fixed order: player, bullets, enemies, pickups, effects.
    move_player(dt);
    if (player.shoot_timer <= 0) {
        shoot();
        player.shoot_timer = shoot_delay[mode];
    }
    update_bullets(dt);
    enemy_hits();
    update_enemies(dt);
    if (state != PLAYING) return;
    update_banana(dt);
    update_effects(dt);

    spawn_timer -= dt;
    if (spawn_timer <= 0) {
        spawn_enemy();
        float delay = spawn_delay[mode] / (1 + (level - 1) * 0.15f);
        spawn_timer = fmaxf(0.27f, delay) * random_float(0.84f, 1.16f);
        if (mode == HARD && level >= 3 && rand() % 100 < 34) spawn_enemy();
        if (mode != HARD && level >= 4 && rand() % 100 < 18) spawn_enemy();
    }
}

// ======================= PART 9: SCREEN INPUT AND GAME STATES =======================

void start_game(void)
{
    start_audio();
    for (int i = 0; i < MAX_BULLETS; i++) bullet[i].on = 0;
    for (int i = 0; i < MAX_ENEMIES; i++) enemy[i].on = 0;
    for (int i = 0; i < MAX_PARTICLES; i++) particle[i].on = 0;
    banana_on = 0;
    score = 0;
    level = 1;
    new_best = 0;
    shake = 0; screen_flash = 0; level_msg = 0;
    spawn_timer = 1;
    banana_timer = random_float(12, 19);
    max_health = start_health[mode];
    reset_player();
    state = PLAYING;
    play(SND_LEVEL);
}

void update_title(void)
{
    if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A)) mode = (mode + 2) % 3;
    if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D)) mode = (mode + 1) % 3;
    if (IsKeyPressed(KEY_ONE)) mode = EASY;
    if (IsKeyPressed(KEY_TWO)) mode = MEDIUM;
    if (IsKeyPressed(KEY_THREE)) mode = HARD;
    if (IsKeyPressed(KEY_B)) { state = BEST; return; }
    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) { start_game(); return; }
#if !defined(__EMSCRIPTEN__)
    if (IsKeyPressed(KEY_ESCAPE)) quit_game = 1;
#endif
}

void update_over(float dt)
{
    over_timer -= dt;
    if (IsKeyPressed(KEY_ESCAPE)) { state = TITLE; return; }
    if (over_timer > 0) return;
    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) start_game();
}

// ======================= PART 10: SMALL DRAWING HELPERS =======================

void draw_center(const char text[], int y, int size, Color color)
{
    DrawText(text, (WIDTH - MeasureText(text, size)) / 2, y, size, color);
}

void draw_right(const char text[], int right_x, int y, int size, Color color)
{
    DrawText(text, right_x - MeasureText(text, size), y, size, color);
}

void panel(float x, float y, float w, float h, Color color)
{
    Rectangle box = { x, y, w, h };
    DrawRectangleRounded(box, 0.12f, 12, C_PANEL);
    DrawRectangleRoundedLinesEx(box, 0.12f, 12, 1, Fade(color, 0.5f));
}

int button(float x, float y, float w, float h, const char text[], Color color, int selected)
{
    Vector2 mouse = GetMousePosition();
    int hover = mouse.x >= x && mouse.x <= x + w && mouse.y >= y && mouse.y <= y + h;
    int lit = selected || hover;
    Rectangle box = { x, y, w, h };
    DrawRectangleRounded(box, 0.16f, 10, lit ? Fade(color, 0.18f) : C_PANEL);
    DrawRectangleRoundedLinesEx(box, 0.16f, 10, lit ? 2 : 1, lit ? color : C_GRAY);
    int size = 18;
    while (size > 10 && MeasureText(text, size) > w - 20) size--;
    DrawText(text, x + (w - MeasureText(text, size)) / 2, y + (h - size) / 2, size, lit ? C_TEXT : C_GRAY);
    return clicked && pointer_x >= x && pointer_x <= x + w && pointer_y >= y && pointer_y <= y + h;
}

// ======================= PART 11: DRAWING THE PLAYER, ENEMIES, BACKGROUND AND THE ARENA =======================

void draw_player(void)
{
    int blink = player.safe > 0 && ((int)(world_time * 22) % 2 == 0);
    Color tint = (player.shield > 0) ? C_GOLD : WHITE;
    if (blink) tint = Fade(tint, 0.35f);
    float degrees = player.angle * 180.0f / PI;
    draw_sprite(tex_player, player.x, player.y, 40, 40, degrees, tint);
    if (player.shield > 0) DrawCircleLines(player.x, player.y, 27 + sinf(world_time * 8) * 2, C_GOLD);
}

void draw_enemy(int i)
{
    Texture2D tex = (enemy[i].type == BLUE_ENEMY) ? tex_blue : tex_red;
    Color tint = (enemy[i].flash > 0) ? C_GOLD : WHITE;
    float degrees = atan2f(enemy[i].vy, enemy[i].vx) * 180.0f / PI;
    float size = enemy[i].size * 2;
    draw_sprite(tex, enemy[i].x, enemy[i].y, size, size, degrees, tint);
    if (enemy[i].health < enemy[i].max_health) {
        float bar = size * 1.1f;
        DrawRectangle(enemy[i].x - bar / 2, enemy[i].y - size / 2 - 9, bar, 4, C_DARK);
        DrawRectangle(enemy[i].x - bar / 2, enemy[i].y - size / 2 - 9, bar * limit(enemy[i].health / enemy[i].max_health, 0, 1), 4, C_GOLD);
    }
}

// Draw all objects that belong inside the moving game world.
void draw_world(void)
{
    if (banana_on) draw_sprite(tex_banana, banana_x, banana_y, 30, 30, 0, WHITE);
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (!bullet[i].on) continue;
        float degrees = atan2f(bullet[i].vy, bullet[i].vx) * 180.0f / PI;
        draw_sprite(tex_bullet, bullet[i].x, bullet[i].y, BULLET_SIZE * 4, BULLET_SIZE * 4, degrees, WHITE);
    }
    for (int i = 0; i < MAX_ENEMIES; i++) if (enemy[i].on) draw_enemy(i);
    for (int i = 0; i < MAX_PARTICLES; i++) {
        if (!particle[i].on) continue;
        float k = particle[i].life / particle[i].max_life;
        DrawCircle(particle[i].x, particle[i].y, particle[i].size * (0.4f + 0.6f * k), Fade(particle[i].color, k));
    }
    draw_player();
}

// ======================= PART 12: DRAWING THE SCREENS AND ONE WHOLE FRAME =======================

// HUD: the score, level, difficulty and player health.
void draw_hud(void)
{
    float w = WIDTH, h = HEIGHT;
    float ratio = limit(player.health / max_health, 0, 1);
    Color health_color = (ratio > 0.5f) ? C_GREEN : ((ratio > 0.25f) ? C_GOLD : C_RED);
    float progress = (score % 500) / 500.0f;

    panel(18, 16, 248, 90, C_CYAN);
    DrawText("SCORE", 34, 27, 12, C_GRAY);
    DrawText(TextFormat("%07d", score), 31, 43, 31, C_TEXT);
    DrawText("SURVIVE THE RIFT", 34, 80, 10, C_CYAN);

    float mx = w / 2 - 144;
    panel(mx, 16, 288, 90, C_PURPLE);
    draw_center(TextFormat("RIFT LEVEL  %02d", level), 25, 15, C_TEXT);
    draw_center("THE CRADLE", 47, 10, C_CYAN);
    DrawRectangle(mx + 24, 70, 240, 6, C_DARK);
    DrawRectangle(mx + 24, 70, 240 * progress, 6, C_CYAN);
    draw_center(TextFormat("%d%% TO NEXT LEVEL", (int)(progress * 100)), 82, 9, C_GRAY);
    Color mcolor = mode_color(mode);
    float rx = w - 242;
    panel(rx, 16, 224, 90, mcolor);
    DrawText(mode_names[mode], rx + 18, 27, 12, mcolor);
    DrawText("INTEGRITY", rx + 18, 47, 10, C_GRAY);
    DrawRectangle(rx + 18, 63, 188, 10, C_DARK);
    DrawRectangle(rx + 18, 63, 188 * ratio, 10, health_color);
    draw_right(TextFormat("%d%%", (int)(ratio * 100)), rx + 206, 79, 11, C_TEXT);

    if (level_msg > 0) {
        draw_center("RIFT LEVEL INCREASED  //  PRESSURE RISING", h * 0.2f, 15, Fade(C_GOLD, limit(level_msg * 1.7f, 0, 1)));
    }
    if (player.shield > 0) DrawText(TextFormat("SHIELD  %.1fs", player.shield), 24, h - 30, 13, C_GOLD);
}

void draw_title(void)
{
    float w = WIDTH, h = HEIGHT;
    int size = (w < 720) ? 40 : 57;
    int top = h * 0.1f;
    draw_center("RIFT RUNNER", top, size, C_TEXT);
    draw_center("SIGNAL ZERO  *  SURVIVE THE SIGNAL. OUTLAST THE VOID.", top + size + 8, 12, C_GRAY);

    float gap = 16, card_w = fminf(220, (w - 40 - gap * 2) / 3);
    float start = (w - (card_w * 3 + gap * 2)) / 2;
    float card_y = h * 0.40f;
    draw_center("CHOOSE A MODE", card_y - 24, 12, C_CYAN);
    for (int i = 0; i < 3; i++) {
        float x = start + i * (card_w + gap);
        if (button(x, card_y, card_w, 100, mode_names[i], mode_color(i), mode == i)) mode = i;
        if (card_w > 120) { DrawText(mode_notes[i], x + 10, card_y + 74, 10, C_GRAY); }
    }
    float bw = fminf(300, w - 36);
    if (button(w / 2 - bw / 2, card_y + 120, bw, 52, "PLAY  [ENTER]", C_CYAN, 1)) start_game();
    if (button(w / 2 - 110, card_y + 184, 220, 34, "PERSONAL BESTS  [B]", C_PURPLE, 0)) state = BEST;
    draw_center("WASD / ARROWS MOVE  *  HOLD MOUSE TO AIM  *  AUTO-FIRE  *  P PAUSE", card_y + 236, 11, C_TEXT);
    draw_center("ONE SPACE BACKGROUND  *  BEST SCORES ARE SAVED ON THIS DEVICE", card_y + 254, 10, C_GRAY);
}

void draw_best(void)
{
    float w = WIDTH, h = HEIGHT;
    draw_center("PERSONAL BESTS", h * 0.08f, 30, C_TEXT);
    draw_center("ONE HIGH SCORE FOR EACH MODE", h * 0.08f + 40, 11, C_GRAY);
    float pw = fminf(600, w * 0.9f);
    float px = (w - pw) / 2, py = h * 0.08f + 70;
    panel(px, py, pw, 3 * 66 + 20, C_PURPLE);
    for (int i = 0; i < 3; i++) {
        float ry = py + 14 + i * 66;
        DrawText(mode_names[i], px + 24, ry + 18, 18, mode_color(i));
        if (best[i] > 0) draw_right(TextFormat("%07d", best[i]), px + pw - 24, ry + 14, 26, C_TEXT);
        else draw_right("N/A", px + pw - 24, ry + 14, 26, C_GRAY);
    }
    if (button(w / 2 - 110, py + 3 * 66 + 40, 220, 42, "BACK  [B]", C_CYAN, 1)) state = TITLE;
}

void draw_pause(void)
{
    float w = WIDTH, h = HEIGHT;
    float pw = fminf(420, w - 32);
    DrawRectangle(0, 0, w, h, (Color){ 3, 4, 13, 195 });
    panel(w / 2 - pw / 2, h / 2 - 110, pw, 220, C_CYAN);
    draw_center("PAUSED", h / 2 - 90, 36, C_TEXT);
    float bw = fminf(300, pw - 24);
    if (button(w / 2 - bw / 2, h / 2 - 20, bw, 48, "RESUME  [P / ENTER]", C_CYAN, 1)) state = PLAYING;
    if (button(w / 2 - bw / 2, h / 2 + 40, bw, 42, "RETURN TO TITLE", C_PURPLE, 0)) state = TITLE;
}

void draw_over(void)
{
    float w = WIDTH, h = HEIGHT;
    float pw = fminf(440, w - 24), px = (w - pw) / 2, py = h / 2 - 180;
    DrawRectangle(0, 0, w, h, (Color){ 3, 4, 13, 190 });
    panel(px, py, pw, 360, C_PINK);
    draw_center("RUN TERMINATED", py + 28, 25, C_PINK);
    draw_center("FINAL SCORE", py + 80, 12, C_GRAY);
    draw_center(TextFormat("%07d", score), py + 100, 51, C_TEXT);
    draw_center(TextFormat("%s    /    RIFT LEVEL %02d", mode_names[mode], level), py + 165, 12, mode_color(mode));
    if (new_best) draw_center("NEW PERSONAL BEST FOR THIS MODE!", py + 200, 15, C_GOLD);
    else draw_center("MODE BEST: SCORE NOT BEATEN", py + 200, 11, C_GRAY);
    float bw = pw - 48;
    if (button(px + 24, py + 245, bw, 50, "PLAY AGAIN  [ENTER]", C_CYAN, 1)) start_game();
    if (button(px + 24, py + 305, bw, 40, "RETURN TO TITLE  [ESC]", C_PURPLE, 0)) state = TITLE;
}

// Draw the background and game world first, then draw the active menu or HUD.
void draw_frame(void)
{
    float sx = 0, sy = 0;
    if (shake > 0) {
        sx = sinf(world_time * 61) * shake * 12;
        sy = cosf(world_time * 67) * shake * 12;
    }
    Camera2D camera = { 0 };
    camera.offset = (Vector2){ WIDTH / 2 + sx, HEIGHT / 2 + sy };
    camera.target = (Vector2){ WIDTH / 2, HEIGHT / 2 };
    camera.zoom = 1;

    BeginDrawing();
    BeginMode2D(camera);
    draw_sprite(tex_bg, WIDTH / 2, HEIGHT / 2, WIDTH, HEIGHT, 0, WHITE);
    if (state == PLAYING || state == PAUSED || state == GAME_OVER) draw_world();
    EndMode2D();

    switch (state) {
    case TITLE:     draw_title(); break;
    case PLAYING:
        draw_hud();
        if (WIDTH <= 820 || GetTouchPointCount() > 0) {
            DrawCircleLines(105, HEIGHT - 126, 55, Fade(C_CYAN, 0.4f));
            DrawCircleLines(WIDTH - 84, HEIGHT - 92, 39, Fade(C_CYAN, 0.4f));
            DrawText("AIM", WIDTH - 111, HEIGHT - 98, 11, C_CYAN);
        }
        break;
    case PAUSED:    draw_hud(); draw_pause(); break;
    case GAME_OVER: draw_over(); break;
    case BEST:      draw_best(); break;
    }

    if (screen_flash > 0) DrawRectangle(0, 0, WIDTH, HEIGHT, Fade((Color){ 188, 226, 255, 255 }, limit(screen_flash, 0, 0.4f)));
    EndDrawing();
}

// ======================= PART 13: THE MAIN LOOP =======================

// The game loop calls this function once for every displayed frame.
void game_frame(void)
{
    float dt = GetFrameTime();
    if (dt > 0.04f) dt = 0.04f;
    world_time += dt;
    // Detect a new mouse click or finger tap and remember its position.
    int finger_down = GetTouchPointCount() > 0;
    int mouse_pressed = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
    clicked = mouse_pressed || (finger_down && !finger_was_down);
    Vector2 pointer = (finger_down && !mouse_pressed) ? GetTouchPosition(0) : GetMousePosition();
    pointer_x = pointer.x;
    pointer_y = pointer.y;
    finger_was_down = finger_down;

    switch (state) {
    case TITLE:     update_title();  update_effects(dt);               break;
    case PLAYING:   update_run(dt);                                     break;
    case PAUSED:
        if (IsKeyPressed(KEY_P) || IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_ESCAPE)) state = PLAYING;
        break;
    case GAME_OVER: update_over(dt); update_effects(dt); break;
    case BEST:
        if (IsKeyPressed(KEY_B) || IsKeyPressed(KEY_ESCAPE)) state = TITLE;
        break;
    }
    draw_frame();
}

int main(void)
{
#if defined(__EMSCRIPTEN__)
    SetConfigFlags(FLAG_MSAA_4X_HINT);
#else
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT | FLAG_MSAA_4X_HINT);
#endif
    InitWindow(1280, 720, GAME_NAME);
    if (!IsWindowReady()) return 1;
#if !defined(__EMSCRIPTEN__)
    SetWindowMinSize(800, 480);
#endif
    SetTargetFPS(60);
    SetExitKey(KEY_NULL);

    srand((unsigned int)time(NULL));
    load_pictures();
    load_best();

#if defined(__EMSCRIPTEN__)
    emscripten_set_main_loop(game_frame, 0, 1);
#else
    while (!WindowShouldClose() && !quit_game) game_frame();
    unload_pictures();
    if (audio_ready) {
        for (int i = 0; i < 5; i++) if (IsSoundValid(sounds[i])) UnloadSound(sounds[i]);
        CloseAudioDevice();
    }
    CloseWindow();
#endif
    return 0;
}
