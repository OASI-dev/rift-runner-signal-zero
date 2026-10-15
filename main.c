/*
 * RIFT RUNNER: SIGNAL ZERO — single-file edition
 * All gameplay, drawing, audio, scoring, persistence, and menus live in this file.
 * Built with raylib 6.x. CMake and web deployment files remain separate on purpose.
 */
#include "raylib.h"
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#if defined(__EMSCRIPTEN__)
#include <emscripten.h>
#define RR_SCORE_FILE "/persist/rift_runner_bests.txt"
#else
#define RR_SCORE_FILE "rift_runner_bests.txt"
#endif

#define RR_TITLE "RIFT RUNNER: SIGNAL ZERO"
#define SCREEN_START_W 1280
#define SCREEN_START_H 720
#define TARGET_FPS 60
#define MAX_BULLETS 240
#define MAX_ENEMIES 90
#define MAX_PARTICLES 420
#define MAX_PICKUPS 90
#define MAX_FLOATERS 32
#define MAX_STARS 120
#define SCORE_STEP 500
#define PI_F 3.14159265358979323846f

/* Palette */
#define C_BG       ((Color){  6,   8,  22, 255})
#define C_PANEL    ((Color){ 15,  21,  45, 238})
#define C_PANEL2   ((Color){ 21,  29,  59, 245})
#define C_CYAN     ((Color){  76, 231, 255, 255})
#define C_BLUE     ((Color){  78, 131, 255, 255})
#define C_PURPLE   ((Color){ 179,  91, 255, 255})
#define C_PINK     ((Color){ 255,  83, 170, 255})
#define C_GREEN    ((Color){  98, 255, 178, 255})
#define C_GOLD     ((Color){ 255, 207, 100, 255})
#define C_TEXT     ((Color){ 236, 243, 255, 255})
#define C_MUTED    ((Color){ 143, 158, 196, 255})
#define C_RED      ((Color){ 255,  83,  99, 255})


typedef enum GameState { STATE_TITLE, STATE_PLAYING, STATE_PAUSED, STATE_GAME_OVER, STATE_SCORES } GameState;
typedef enum Difficulty { DIFF_EASY = 0, DIFF_MEDIUM = 1, DIFF_HARD = 2 } Difficulty;
typedef enum PickupKind { PICKUP_DATA = 0, PICKUP_REPAIR = 1 } PickupKind;
typedef enum SoundId { SFX_SHOT, SFX_DASH, SFX_IMPACT, SFX_PICKUP, SFX_ENEMY_DOWN, SFX_SURGE, SFX_COUNT } SoundId;

typedef struct Player {
    Vector2 pos;
    Vector2 facing;
    Vector2 velocity;
    float health;
    float maxHealth;
    float fireClock;
    float dashClock;
    float dashTime;
    float invulnerable;
    float animation;
} Player;

typedef struct Bullet {
    bool active;
    Vector2 pos;
    Vector2 vel;
    float radius;
    float life;
    Color color;
} Bullet;

typedef struct Enemy {
    bool active;
    Vector2 pos, vel;
    float radius, speed, hp, maxHp;
    float heading, turnClock, turnRate, flash;
} Enemy;

typedef struct Particle {
    bool active;
    Vector2 pos;
    Vector2 vel;
    float radius;
    float life;
    float fullLife;
    Color color;
} Particle;

typedef struct Pickup {
    bool active;
    Vector2 pos;
    float phase;
    float life;
    PickupKind kind;
} Pickup;

typedef struct Floater {
    bool active;
    Vector2 pos;
    float life;
    int points;
    Color color;
} Floater;

typedef struct Star {
    float x, y, speed, size, phase;
} Star;

typedef struct DifficultyInfo {
    const char *name;
    const char *tagline;
    Color color;
    float playerSpeed;
    float fireInterval;
    float spawnInterval;
    float enemySpeed;
    float contactDamage;
    float startingHealth;
} DifficultyInfo;

static const DifficultyInfo DIFFICULTIES[3] = {
    { "EASY",   "More room to breathe", { 98, 255, 178, 255 }, 292.0f, 0.245f, 1.16f, 66.0f, 15.0f, 125.0f },
    { "MEDIUM", "The intended run",     { 76, 231, 255, 255 }, 276.0f, 0.225f, 0.91f, 83.0f, 22.0f, 100.0f },
    { "HARD",   "No wasted movement",  { 255, 83, 170, 255 }, 266.0f, 0.205f, 0.72f, 103.0f, 30.0f, 90.0f }
};

typedef struct Game {
    GameState state;
    Difficulty difficulty;
    Player player;
    Bullet bullets[MAX_BULLETS];
    Enemy enemies[MAX_ENEMIES];
    Particle particles[MAX_PARTICLES];
    Pickup pickups[MAX_PICKUPS];
    Floater floaters[MAX_FLOATERS];
    Star stars[MAX_STARS];
    int bestScores[3];
    int score;
    int kills;
    int stage;
    int combo;
    bool newPersonalBest;
    bool audioReady;
    bool musicLoaded;
    bool firstAudioGesture;
    bool quitRequested;
    float worldTime;
    float spawnClock;
    float screenShake;
    float flashAlpha;
    float stageNotice;
    float comboClock;
    int menuSelection;
    Music ambient;
    Sound sounds[SFX_COUNT];
} Game;

static Game g;

/* ---------- Small helpers ---------- */
static float ClampF(float v, float lo, float hi) { return (v < lo) ? lo : ((v > hi) ? hi : v); }
static float MaxF(float a, float b) { return (a > b) ? a : b; }
static float MinF(float a, float b) { return (a < b) ? a : b; }
static float RandF(float lo, float hi) { return lo + ((float)rand() / (float)RAND_MAX) * (hi - lo); }
static float LengthSq(Vector2 v) { return v.x * v.x + v.y * v.y; }
static float Length(Vector2 v) { return sqrtf(LengthSq(v)); }
static Vector2 V2(float x, float y) { Vector2 v = { x, y }; return v; }
static Vector2 Add(Vector2 a, Vector2 b) { return V2(a.x + b.x, a.y + b.y); }
static Vector2 Sub(Vector2 a, Vector2 b) { return V2(a.x - b.x, a.y - b.y); }
static Vector2 Scale(Vector2 a, float s) { return V2(a.x * s, a.y * s); }
static Vector2 Normalize(Vector2 v) { float len = Length(v); return (len > 0.0001f) ? Scale(v, 1.0f / len) : V2(0.0f, 0.0f); }
static Vector2 Perp(Vector2 v) { return V2(-v.y, v.x); }
static float DistSq(Vector2 a, Vector2 b) { return LengthSq(Sub(a, b)); }
static Color Alpha(Color c, float a) { c.a = (unsigned char)ClampF(a, 0.0f, 255.0f); return c; }
static Color WithOpacity(Color c, float opacity) { return Alpha(c, (float)c.a * ClampF(opacity, 0.0f, 1.0f)); }
static int SW(void) { return GetScreenWidth(); }
static int SH(void) { return GetScreenHeight(); }
static bool HitRect(Vector2 p, Rectangle r) { return p.x >= r.x && p.y >= r.y && p.x <= r.x + r.width && p.y <= r.y + r.height; }
static void DrawCentered(const char *text, int y, int size, Color color) { DrawText(text, (SW() - MeasureText(text, size)) / 2, y, size, color); }
static void DrawRight(const char *text, int rightX, int y, int size, Color color) { DrawText(text, rightX - MeasureText(text, size), y, size, color); }
static const DifficultyInfo *GetDifficulty(void) { return &DIFFICULTIES[(int)g.difficulty]; }
static int CurrentStageForScore(int score) { return 1 + score / SCORE_STEP; }

static void FirstAudioGesture(void)
{
    if (g.firstAudioGesture) return;
    /* Initialize only after a real key/mouse gesture so browser audio is not autoplay-blocked. */
    if (!g.audioReady) {
        InitAudioDevice();
        g.audioReady = IsAudioDeviceReady();
        if (g.audioReady) {
            g.sounds[SFX_SHOT] = LoadSound("assets/shot.wav");
            g.sounds[SFX_DASH] = LoadSound("assets/dash.wav");
            g.sounds[SFX_IMPACT] = LoadSound("assets/impact.wav");
            g.sounds[SFX_PICKUP] = LoadSound("assets/pickup.wav");
            g.sounds[SFX_ENEMY_DOWN] = LoadSound("assets/enemy_down.wav");
            g.sounds[SFX_SURGE] = LoadSound("assets/surge.wav");
            g.ambient = LoadMusicStream("assets/ambient_loop.wav");
            g.musicLoaded = IsMusicValid(g.ambient);
            if (g.musicLoaded) SetMusicVolume(g.ambient, 0.22f);
        }
    }
    if (g.audioReady && g.musicLoaded) PlayMusicStream(g.ambient);
    g.firstAudioGesture = true;
}

static void PlaySfx(SoundId id)
{
    if (g.audioReady && id >= 0 && id < SFX_COUNT && IsSoundValid(g.sounds[id])) PlaySound(g.sounds[id]);
}

/* ---------- Three personal bests: one score per difficulty ---------- */
static void SaveBestScores(void)
{
    FILE *f = fopen(RR_SCORE_FILE, "w");
    if (!f) return;
    fprintf(f, "%d %d %d\n", g.bestScores[0], g.bestScores[1], g.bestScores[2]);
    fclose(f);
#if defined(__EMSCRIPTEN__)
    EM_ASM({
        if (typeof FS !== 'undefined' && typeof IDBFS !== 'undefined')
            FS.syncfs(false, function(error) { if (error) console.warn('RIFT RUNNER: score save failed', error); });
    }, 0);
#endif
}

static void LoadBestScores(void)
{
    FILE *f = fopen(RR_SCORE_FILE, "r");
    if (!f) return;
    if (fscanf(f, "%d %d %d", &g.bestScores[0], &g.bestScores[1], &g.bestScores[2]) != 3)
        for (int i = 0; i < 3; ++i) g.bestScores[i] = -1;
    for (int i = 0; i < 3; ++i) if (g.bestScores[i] < -1) g.bestScores[i] = -1;
    fclose(f);
}

static void RecordPersonalBest(void)
{
    int mode = (int)g.difficulty;
    g.newPersonalBest = g.bestScores[mode] < 0 || g.score > g.bestScores[mode];
    if (g.newPersonalBest) {
        g.bestScores[mode] = g.score;
        SaveBestScores();
    }
}

/* ---------- Tiny visual effects ---------- */
static void AddParticle(Vector2 pos, Vector2 vel, float radius, float life, Color color)
{
    for (int i = 0; i < MAX_PARTICLES; ++i) {
        if (!g.particles[i].active) {
            g.particles[i] = (Particle){ true, pos, vel, radius, life, life, color };
            return;
        }
    }
}

static void Burst(Vector2 pos, Color color, int count, float speed, float radius)
{
    for (int i = 0; i < count; ++i) {
        float angle = RandF(0.0f, 2.0f * PI_F);
        float spd = RandF(speed * 0.35f, speed);
        AddParticle(pos, V2(cosf(angle) * spd, sinf(angle) * spd), RandF(radius * 0.55f, radius), RandF(0.24f, 0.72f), color);
    }
}

static void AddFloater(Vector2 pos, int points, Color color)
{
    for (int i = 0; i < MAX_FLOATERS; ++i) {
        if (!g.floaters[i].active) {
            g.floaters[i] = (Floater){ true, pos, 0.88f, points, color };
            return;
        }
    }
}

static void UpdateEffects(float dt)
{
    for (int i = 0; i < MAX_PARTICLES; ++i) {
        Particle *p = &g.particles[i];
        if (!p->active) continue;
        p->life -= dt;
        if (p->life <= 0.0f) { p->active = false; continue; }
        p->pos = Add(p->pos, Scale(p->vel, dt));
        p->vel = Scale(p->vel, 1.0f - MinF(2.8f * dt, 0.22f));
        p->radius *= (1.0f - MinF(0.5f * dt, 0.06f));
    }
    for (int i = 0; i < MAX_FLOATERS; ++i) {
        Floater *f = &g.floaters[i];
        if (!f->active) continue;
        f->life -= dt;
        f->pos.y -= 27.0f * dt;
        if (f->life <= 0.0f) f->active = false;
    }
    g.screenShake = MaxF(0.0f, g.screenShake - dt);
    g.flashAlpha = MaxF(0.0f, g.flashAlpha - 1.75f * dt);
    g.stageNotice = MaxF(0.0f, g.stageNotice - dt);
    g.comboClock = MaxF(0.0f, g.comboClock - dt);
    if (g.comboClock <= 0.0f) g.combo = 0;
}

/* ---------- Player, bullets, enemies, pickups ---------- */
static void ResetWorld(void)
{
    memset(g.bullets, 0, sizeof(g.bullets));
    memset(g.enemies, 0, sizeof(g.enemies));
    memset(g.particles, 0, sizeof(g.particles));
    memset(g.pickups, 0, sizeof(g.pickups));
    memset(g.floaters, 0, sizeof(g.floaters));
    memset(&g.player, 0, sizeof(g.player));
    g.score = 0;
    g.kills = 0;
    g.stage = 1;
    g.combo = 0;
    g.comboClock = 0.0f;
    g.spawnClock = 1.0f;
    g.screenShake = 0.0f;
    g.flashAlpha = 0.0f;
    g.stageNotice = 0.0f;
    g.player.pos = V2((float)SW() * 0.5f, (float)SH() * 0.5f);
    g.player.facing = V2(1.0f, 0.0f);
    g.player.health = GetDifficulty()->startingHealth;
    g.player.maxHealth = GetDifficulty()->startingHealth;
    g.player.dashClock = 0.0f;
    g.player.fireClock = 0.12f;
    g.player.invulnerable = 0.65f;
    g.player.animation = 0.0f;
    g.newPersonalBest = false;
}

static void StartRun(void)
{
    FirstAudioGesture();
    ResetWorld();
    g.state = STATE_PLAYING;
    Burst(g.player.pos, C_CYAN, 34, 190.0f, 3.3f);
    PlaySfx(SFX_SURGE);
}

static void AddBullet(Vector2 pos, Vector2 dir)
{
    dir = Normalize(dir);
    if (LengthSq(dir) < 0.5f) dir = g.player.facing;
    for (int i = 0; i < MAX_BULLETS; ++i) {
        if (!g.bullets[i].active) {
            float stageBoost = MinF((float)(g.stage - 1) * 5.0f, 90.0f);
            float spd = 620.0f + stageBoost;
            g.bullets[i] = (Bullet){ true, Add(pos, Scale(dir, 16.0f)), Scale(dir, spd), 4.0f, 1.32f, (g.combo >= 5) ? C_GOLD : C_CYAN };
            PlaySfx(SFX_SHOT);
            return;
        }
    }
}

static void SpawnEnemy(void)
{
    int alive = 0;
    for (int i = 0; i < MAX_ENEMIES; ++i) if (g.enemies[i].active) ++alive;
    int limit = 8 + g.stage + (int)g.difficulty * 2;
    if (limit > 38) limit = 38;
    if (alive >= limit) return;

    /* Spawn visible red enemies at an edge, initially travelling inward toward
       a random point—not toward the player. After spawning they roam and bounce. */
    const float w = (float)SW(), h = (float)SH(), edge = 25.0f;
    Vector2 pos;
    switch (rand() % 4) {
        case 0: pos = V2(RandF(edge, w - edge), edge); break;
        case 1: pos = V2(w - edge, RandF(edge, h - edge)); break;
        case 2: pos = V2(RandF(edge, w - edge), h - edge); break;
        default: pos = V2(edge, RandF(edge, h - edge)); break;
    }
    Vector2 waypoint = V2(RandF(w * 0.18f, w * 0.82f), RandF(h * 0.18f, h * 0.82f));
    float heading = atan2f(waypoint.y - pos.y, waypoint.x - pos.x) + RandF(-0.55f, 0.55f);
    float stageSpeed = MinF((float)(g.stage - 1) * 3.0f, 78.0f);
    float speed = GetDifficulty()->enemySpeed * 0.70f + stageSpeed;
    float hp = 2.0f + (float)((g.stage - 1) / 4);
    if (hp > 5.0f) hp = 5.0f;

    for (int i = 0; i < MAX_ENEMIES; ++i) {
        if (!g.enemies[i].active) {
            g.enemies[i] = (Enemy){ true, pos, V2(0, 0), 21.0f, speed, hp, hp,
                                    heading, RandF(0.55f, 1.25f), RandF(-0.9f, 0.9f), 0.0f };
            return;
        }
    }
}

static void AddPickup(Vector2 pos, PickupKind kind)
{
    for (int i = 0; i < MAX_PICKUPS; ++i) {
        if (!g.pickups[i].active) {
            g.pickups[i] = (Pickup){ true, pos, RandF(0.0f, 6.28f), 8.0f, kind };
            return;
        }
    }
}

static void KillEnemy(Enemy *e)
{
    const int oldStage = g.stage;
    e->active = false;
    ++g.kills;
    g.combo = (g.comboClock > 0.0f) ? g.combo + 1 : 1;
    g.comboClock = 3.2f;
    int bonus = (g.combo > 1) ? ((g.combo > 12) ? 60 : g.combo * 5) : 0;
    int points = 200 + bonus;
    g.score += points;
    AddFloater(e->pos, points, (g.combo >= 5) ? C_GOLD : C_CYAN);
    Burst(e->pos, C_RED, 26, 215.0f, 3.2f);
    PlaySfx(SFX_ENEMY_DOWN);
    if ((rand() % 100) < 28) AddPickup(e->pos, PICKUP_DATA);
    else if ((rand() % 100) < 9) AddPickup(e->pos, PICKUP_REPAIR);
    g.stage = CurrentStageForScore(g.score);
    if (g.stage > oldStage) {
        g.stageNotice = 2.0f;
        g.flashAlpha = 0.15f;
        PlaySfx(SFX_SURGE);
        Burst(g.player.pos, C_GOLD, 24, 170.0f, 2.9f);
    }
}

static void EndRun(void)
{
    if (g.state != STATE_PLAYING) return;
    g.state = STATE_GAME_OVER;
    RecordPersonalBest();
    g.screenShake = 0.42f;
    g.flashAlpha = 0.32f;
    Burst(g.player.pos, C_PINK, 55, 260.0f, 4.0f);
    PlaySfx(SFX_IMPACT);
}

static void UpdatePlayer(float dt)
{
    Player *p = &g.player;
    const DifficultyInfo *d = GetDifficulty();
    Vector2 input = V2(0.0f, 0.0f);
    if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) input.x -= 1.0f;
    if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) input.x += 1.0f;
    if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)) input.y -= 1.0f;
    if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) input.y += 1.0f;

    /* Optional touch controls: drag in the left half to move; touch the right half to aim. */
    int touchCount = GetTouchPointCount();
    if (touchCount > 0) {
        for (int i = 0; i < touchCount; ++i) {
            Vector2 touch = GetTouchPosition(i);
            if (touch.x < (float)SW() * 0.47f) {
                Vector2 anchor = V2(105.0f, (float)SH() - 126.0f);
                Vector2 drag = Scale(Sub(touch, anchor), 1.0f / 62.0f);
                float dragLen = Length(drag);
                if (dragLen > 1.0f) drag = Scale(drag, 1.0f / dragLen);
                input = Add(input, drag);
            } else {
                Vector2 aim = Normalize(Sub(touch, p->pos));
                if (LengthSq(aim) > 0.1f) p->facing = aim;
            }
        }
    }
    float inLen = Length(input);
    if (inLen > 1.0f) input = Scale(input, 1.0f / inLen);
    if (LengthSq(input) > 0.02f) {
        p->facing = Normalize(input);
        p->animation += dt * 14.0f;
    }

    Vector2 mouse = GetMousePosition();
    bool mouseAiming = IsMouseButtonDown(MOUSE_BUTTON_LEFT) || IsMouseButtonDown(MOUSE_BUTTON_RIGHT);
    bool touchDashRequested = false;
    if (mouseAiming) {
        Vector2 mouseAim = Normalize(Sub(mouse, p->pos));
        if (LengthSq(mouseAim) > 0.1f) p->facing = mouseAim;
    }
    if (touchCount > 0) {
        for (int i = 0; i < touchCount; ++i) {
            Vector2 touch = GetTouchPosition(i);
            if (touch.x > (float)SW() - 160.0f && touch.y > (float)SH() - 160.0f) touchDashRequested = true;
        }
    }
    bool dashPressed = IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_LEFT_SHIFT) || IsKeyPressed(KEY_RIGHT_SHIFT) || touchDashRequested;
    if (dashPressed && p->dashClock <= 0.0f) {
        Vector2 dashDir = (LengthSq(input) > 0.02f) ? Normalize(input) : p->facing;
        if (LengthSq(dashDir) < 0.1f) dashDir = V2(1, 0);
        p->facing = dashDir;
        p->velocity = Scale(dashDir, 720.0f);
        p->dashTime = 0.17f;
        p->dashClock = 1.12f;
        p->invulnerable = MaxF(p->invulnerable, 0.24f);
        PlaySfx(SFX_DASH);
        Burst(p->pos, C_CYAN, 17, 145.0f, 2.5f);
    } else if (p->dashTime > 0.0f) {
        /* Keep the dash vector for its entire short burst, even while a direction key is held. */
        p->velocity = Scale(Normalize(p->velocity), 720.0f);
    } else if (LengthSq(input) > 0.02f) {
        p->velocity = Scale(input, d->playerSpeed);
    } else {
        p->velocity = Scale(p->velocity, MaxF(0.0f, 1.0f - 7.5f * dt));
    }

    p->dashClock = MaxF(0.0f, p->dashClock - dt);
    p->dashTime = MaxF(0.0f, p->dashTime - dt);
    p->invulnerable = MaxF(0.0f, p->invulnerable - dt);
    p->fireClock -= dt;
    p->pos = Add(p->pos, Scale(p->velocity, dt));
    p->pos.x = ClampF(p->pos.x, 28.0f, (float)SW() - 28.0f);
    p->pos.y = ClampF(p->pos.y, 34.0f, (float)SH() - 30.0f);

    /* Automatic blaster fire follows the runner facing; hold mouse to aim. */
    if (p->fireClock <= 0.0f) {
        AddBullet(p->pos, p->facing);
        p->fireClock = d->fireInterval;
    }
}

static void UpdateBullets(float dt)
{
    for (int i = 0; i < MAX_BULLETS; ++i) {
        Bullet *b = &g.bullets[i];
        if (!b->active) continue;
        b->life -= dt;
        b->pos = Add(b->pos, Scale(b->vel, dt));
        if (b->life <= 0.0f || b->pos.x < -30 || b->pos.x > SW() + 30 || b->pos.y < -30 || b->pos.y > SH() + 30) {
            b->active = false;
            continue;
        }
        for (int j = 0; j < MAX_ENEMIES; ++j) {
            Enemy *e = &g.enemies[j];
            if (!e->active) continue;
            float r = e->radius + b->radius;
            if (DistSq(e->pos, b->pos) <= r * r) {
                b->active = false;
                e->hp -= 1.0f;
                e->flash = 0.10f;
                Burst(b->pos, b->color, 4, 85.0f, 1.7f);
                if (e->hp <= 0.0f) KillEnemy(e);
                break;
            }
        }
    }
}

static void UpdateEnemies(float dt)
{
    const float w = (float)SW(), h = (float)SH();
    float damage = GetDifficulty()->contactDamage;
    for (int i = 0; i < MAX_ENEMIES; ++i) {
        Enemy *e = &g.enemies[i];
        if (!e->active) continue;

        e->flash = MaxF(0.0f, e->flash - dt);
        e->turnClock -= dt;
        if (e->turnClock <= 0.0f) {
            e->turnRate = RandF(-0.9f, 0.9f);
            e->turnClock = RandF(0.55f, 1.25f);
        }
        e->heading += e->turnRate * dt;
        float stageFactor = 1.0f + MinF((float)(g.stage - 1) * 0.028f, 0.48f);
        e->vel = Scale(V2(cosf(e->heading), sinf(e->heading)), e->speed * stageFactor);
        e->pos = Add(e->pos, Scale(e->vel, dt));

        /* Bounce off the arena edges so the red enemies keep roaming on-screen. */
        if (e->pos.x < e->radius) { e->pos.x = e->radius; e->heading = PI_F - e->heading; e->turnRate = -e->turnRate; }
        if (e->pos.x > w - e->radius) { e->pos.x = w - e->radius; e->heading = PI_F - e->heading; e->turnRate = -e->turnRate; }
        if (e->pos.y < e->radius) { e->pos.y = e->radius; e->heading = -e->heading; e->turnRate = -e->turnRate; }
        if (e->pos.y > h - e->radius) { e->pos.y = h - e->radius; e->heading = -e->heading; e->turnRate = -e->turnRate; }

        float r = e->radius + 15.0f;
        if (DistSq(e->pos, g.player.pos) < r * r && g.player.invulnerable <= 0.0f && g.player.dashTime <= 0.0f) {
            g.player.health -= damage;
            g.player.invulnerable = 0.92f;
            g.screenShake = 0.20f;
            g.flashAlpha = 0.10f;
            Vector2 knock = Normalize(Sub(g.player.pos, e->pos));
            g.player.pos = Add(g.player.pos, Scale(knock, 30.0f));
            e->pos = Add(e->pos, Scale(knock, -22.0f));
            Burst(g.player.pos, C_PINK, 16, 150.0f, 2.3f);
            PlaySfx(SFX_IMPACT);
            if (g.player.health <= 0.0f) { g.player.health = 0.0f; EndRun(); return; }
        }
    }
}

static void UpdatePickups(float dt)
{
    for (int i = 0; i < MAX_PICKUPS; ++i) {
        Pickup *p = &g.pickups[i];
        if (!p->active) continue;
        p->life -= dt;
        p->phase += dt * 3.0f;
        if (p->life <= 0.0f) { p->active = false; continue; }
        float r = 18.0f;
        if (DistSq(p->pos, g.player.pos) <= r * r) {
            p->active = false;
            if (p->kind == PICKUP_DATA) {
                g.score += 50;
                AddFloater(p->pos, 50, C_CYAN);
            } else {
                float before = g.player.health;
                g.player.health = MinF(g.player.maxHealth, g.player.health + 22.0f);
                int actual = (int)(g.player.health - before);
                if (actual > 0) AddFloater(p->pos, actual, C_GREEN);
                g.score += 25;
                AddFloater(Add(p->pos, V2(0, -17)), 25, C_GOLD);
            }
            int oldStage = g.stage;
            g.stage = CurrentStageForScore(g.score);
            if (g.stage > oldStage) { g.stageNotice = 2.0f; PlaySfx(SFX_SURGE); }
            Burst(p->pos, p->kind == PICKUP_DATA ? C_CYAN : C_GREEN, 14, 120.0f, 2.1f);
            PlaySfx(SFX_PICKUP);
        }
    }
}

static void UpdatePlaying(float dt)
{
    if (IsKeyPressed(KEY_P) || IsKeyPressed(KEY_ESCAPE)) { g.state = STATE_PAUSED; return; }

    UpdatePlayer(dt);
    UpdateBullets(dt);
    if (g.state != STATE_PLAYING) return;
    UpdateEnemies(dt);
    if (g.state != STATE_PLAYING) return;
    UpdatePickups(dt);
    UpdateEffects(dt);
    g.spawnClock -= dt;
    if (g.spawnClock <= 0.0f) {
        SpawnEnemy();
        float scoreRate = 1.0f + (float)(g.stage - 1) * 0.15f;
        float interval = GetDifficulty()->spawnInterval / scoreRate;
        g.spawnClock = MaxF(0.27f, interval) * RandF(0.84f, 1.16f);
        if (g.stage >= 4 && (rand() % 100) < 18) SpawnEnemy();
    }
}

/* ---------- Background and drawing ---------- */
static void DrawBackground(void)
{
    int w = SW(), h = SH();
    ClearBackground(C_BG);
    DrawRectangleGradientV(0, 0, w, h, (Color){ 15, 17, 47, 255 }, C_BG);
    Vector2 center = V2(w * 0.5f + sinf(g.worldTime * 0.11f) * 70.0f, h * 0.48f + cosf(g.worldTime * 0.09f) * 25.0f);
    DrawCircleGradient(center, (float)MinF(w, h) * 0.54f, (Color){ 53, 44, 128, 42 }, (Color){ 11, 11, 28, 0 });
    DrawCircleLines((int)center.x, (int)center.y, (float)MinF(w, h) * 0.19f, (Color){ 90, 108, 228, 33 });
    DrawCircleLines((int)center.x, (int)center.y, (float)MinF(w, h) * 0.28f, (Color){ 69, 223, 255, 22 });
    DrawCircleLines((int)center.x, (int)center.y, (float)MinF(w, h) * 0.37f, (Color){ 168, 88, 255, 18 });

    float gridShift = fmodf(g.worldTime * 9.0f, 54.0f);
    for (int x = -54; x < w + 54; x += 54) DrawLine((int)(x + gridShift), 0, (int)(x + gridShift), h, (Color){ 86, 116, 202, 15 });
    for (int y = -54; y < h + 54; y += 54) DrawLine(0, (int)(y + gridShift), w, (int)(y + gridShift), (Color){ 86, 116, 202, 13 });

    for (int i = 0; i < MAX_STARS; ++i) {
        Star *s = &g.stars[i];
        float x = fmodf(s->x + g.worldTime * s->speed * 0.16f, (float)w);
        float y = fmodf(s->y + g.worldTime * s->speed, (float)h);
        float pulse = 0.58f + 0.42f * sinf(g.worldTime * 1.6f + s->phase);
        Color starColor = (i % 7 == 0) ? C_CYAN : (i % 9 == 0 ? C_PURPLE : C_TEXT);
        DrawCircleV(V2(x, y), s->size * pulse, WithOpacity(starColor, 0.52f + 0.42f * pulse));
    }
}

static void DrawRunner(void)
{
    Player *p = &g.player;
    Vector2 f = Normalize(p->facing);
    Vector2 side = Perp(f);
    bool blinking = p->invulnerable > 0.0f && ((int)(g.worldTime * 22.0f) % 2 == 0);
    Color armor = (p->dashTime > 0.0f) ? C_CYAN : C_TEXT;
    if (blinking) armor = WithOpacity(armor, 0.35f);
    float stride = (LengthSq(p->velocity) > 80.0f) ? sinf(p->animation) * 3.0f : 0.0f;

    Vector2 head = Add(p->pos, Scale(f, 9.0f));
    Vector2 shoulderL = Add(Add(p->pos, Scale(f, 3.0f)), Scale(side, 8.0f));
    Vector2 shoulderR = Add(Add(p->pos, Scale(f, 3.0f)), Scale(side, -8.0f));
    Vector2 hipL = Add(Add(p->pos, Scale(f, -8.0f)), Scale(side, 5.5f));
    Vector2 hipR = Add(Add(p->pos, Scale(f, -8.0f)), Scale(side, -5.5f));
    Vector2 leftFoot = Add(Add(p->pos, Scale(f, -17.0f + stride)), Scale(side, 9.0f));
    Vector2 rightFoot = Add(Add(p->pos, Scale(f, -17.0f - stride)), Scale(side, -9.0f));
    Vector2 blasterBase = Add(Add(p->pos, Scale(f, 2.5f)), Scale(side, -8.0f));
    Vector2 blasterTip = Add(Add(p->pos, Scale(f, 19.5f)), Scale(side, -8.0f));

    DrawCircleGradient(p->pos, 36.0f, (Color){ 63, 220, 255, 49 }, (Color){ 63, 220, 255, 0 });
    DrawCircleV(Add(p->pos, V2(0, 8)), 13.0f, (Color){ 1, 3, 15, 190 });
    if (p->dashTime > 0.0f) {
        for (int i = 1; i <= 4; ++i) {
            Vector2 ghost = Sub(p->pos, Scale(f, i * 9.0f));
            DrawLineEx(ghost, Sub(ghost, Scale(f, 7.0f)), (float)(5 - i), WithOpacity(C_CYAN, 0.42f - i * 0.08f));
        }
    }

    /* Back legs and boots make the avatar read as a small armored runner, not an orb. */
    DrawLineEx(hipL, leftFoot, 4.2f, (Color){ 67, 117, 224, 255 });
    DrawLineEx(hipR, rightFoot, 4.2f, (Color){ 67, 117, 224, 255 });
    DrawLineEx(leftFoot, Add(leftFoot, Scale(f, 4.0f)), 5.2f, C_PURPLE);
    DrawLineEx(rightFoot, Add(rightFoot, Scale(f, 4.0f)), 5.2f, C_PURPLE);

    /* Chest armour is drawn as two triangles so it rotates naturally with facing. */
    DrawTriangle(shoulderL, shoulderR, hipR, armor);
    DrawTriangle(shoulderL, hipR, hipL, (Color){ 44, 77, 133, 255 });
    DrawLineEx(shoulderL, shoulderR, 2.0f, C_CYAN);
    DrawLineEx(shoulderL, hipL, 1.5f, (Color){ 96, 163, 255, 255 });
    DrawLineEx(shoulderR, hipR, 1.5f, (Color){ 96, 163, 255, 255 });

    /* One arm braces the blaster; the other keeps a visible shoulder silhouette. */
    DrawLineEx(Add(p->pos, Scale(side, 6.5f)), Add(Add(p->pos, Scale(f, -1.0f)), Scale(side, 14.0f)), 3.4f, C_BLUE);
    DrawLineEx(Add(p->pos, Scale(side, -6.5f)), blasterBase, 4.1f, (Color){ 108, 165, 255, 255 });
    DrawLineEx(blasterBase, blasterTip, 4.0f, C_CYAN);
    DrawCircleV(blasterTip, 3.0f, C_TEXT);

    DrawCircleGradient(head, 14.0f, (Color){ 76, 231, 255, 52 }, (Color){ 76, 231, 255, 0 });
    DrawCircleV(head, 6.4f, armor);
    DrawCircleLines((int)head.x, (int)head.y, 6.4f, C_CYAN);
    DrawLineEx(Add(head, Scale(f, 1.0f)), Add(head, Scale(f, 5.0f)), 3.1f, (Color){ 21, 44, 80, 255 });
    DrawCircleV(Add(head, Scale(f, 3.2f)), 1.6f, C_TEXT);
    DrawCircleV(Sub(p->pos, Scale(f, 10.5f)), 2.6f, C_GOLD);
}

static void DrawEnemy(const Enemy *e)
{
    Color red = e->flash > 0.0f ? C_GOLD : C_RED;
    float r = e->radius;
    Vector2 forward = V2(cosf(e->heading), sinf(e->heading));
    DrawCircleGradient(e->pos, r * 2.5f, WithOpacity(C_RED, 0.34f), (Color){ 255, 83, 99, 0 });
    DrawCircleV(e->pos, r, red);
    DrawCircleLines((int)e->pos.x, (int)e->pos.y, r, (Color){ 255, 172, 180, 255 });
    DrawLineEx(Add(e->pos, Scale(forward, -r * 0.48f)), Add(e->pos, Scale(forward, r * 0.50f)), 5.0f, (Color){ 76, 12, 28, 255 });
    DrawCircleV(Add(e->pos, Scale(forward, r * 0.22f)), 3.2f, C_GOLD);
    if (e->hp < e->maxHp) {
        float barW = r * 2.2f;
        DrawRectangleRounded((Rectangle){ e->pos.x - barW * 0.5f, e->pos.y - r - 9.0f, barW, 4.0f }, 0.5f, 4, (Color){ 20, 25, 46, 230 });
        DrawRectangleRounded((Rectangle){ e->pos.x - barW * 0.5f, e->pos.y - r - 9.0f, barW * ClampF(e->hp / e->maxHp, 0, 1), 4.0f }, 0.5f, 4, C_GOLD);
    }
}

static void DrawWorld(void)
{
    for (int i = 0; i < MAX_PICKUPS; ++i) {
        Pickup *p = &g.pickups[i];
        if (!p->active) continue;
        Color c = p->kind == PICKUP_DATA ? C_CYAN : C_GREEN;
        float r = 6.2f + sinf(p->phase) * 1.6f;
        DrawCircleGradient(p->pos, 24.0f, WithOpacity(c, 0.27f), (Color){ c.r, c.g, c.b, 0 });
        DrawPoly(p->pos, 4, r + 2.0f, 45.0f, WithOpacity(c, 0.44f));
        DrawPoly(p->pos, 4, r, 45.0f, c);
        DrawCircleV(p->pos, 2.0f, C_TEXT);
    }
    for (int i = 0; i < MAX_BULLETS; ++i) {
        Bullet *b = &g.bullets[i];
        if (!b->active) continue;
        Vector2 tail = Sub(b->pos, Scale(Normalize(b->vel), 16.0f));
        DrawLineEx(tail, b->pos, 3.0f, WithOpacity(b->color, 0.45f));
        DrawCircleGradient(b->pos, 10.0f, WithOpacity(b->color, 0.56f), (Color){ b->color.r, b->color.g, b->color.b, 0 });
        DrawCircleV(b->pos, b->radius, C_CYAN);
    }
    for (int i = 0; i < MAX_ENEMIES; ++i) if (g.enemies[i].active) DrawEnemy(&g.enemies[i]);
    for (int i = 0; i < MAX_PARTICLES; ++i) {
        Particle *p = &g.particles[i];
        if (!p->active) continue;
        float a = ClampF(p->life / p->fullLife, 0.0f, 1.0f);
        DrawCircleV(p->pos, p->radius * (0.36f + 0.64f * a), WithOpacity(p->color, a));
    }
    if (g.state == STATE_PLAYING || g.state == STATE_PAUSED || g.state == STATE_GAME_OVER) DrawRunner();
}

static void DrawPanel(Rectangle r, Color accent)
{
    DrawRectangleRounded(r, 0.12f, 12, C_PANEL);
    DrawRectangleRoundedLinesEx(r, 0.12f, 12, 1.0f, WithOpacity(accent, 0.48f));
}

static void DrawHUD(void)
{
    char buf[96];
    int w = SW();
    const DifficultyInfo *d = GetDifficulty();
    bool compactHud = w < 760;
    if (compactHud) {
        float sideW = MinF(180.0f, (float)w * 0.40f);
        DrawPanel((Rectangle){ 8, 8, sideW, 66 }, C_CYAN);
        DrawText("SCORE", 18, 16, 9, C_MUTED);
        snprintf(buf, sizeof(buf), "%d", g.score);
        DrawText(buf, 16, 30, 23, C_TEXT);
        snprintf(buf, sizeof(buf), "STAGE %02d / %d K", g.stage, g.kills);
        DrawText(buf, 18, 56, 9, C_CYAN);

        float healthW = MinF(170.0f, (float)w * 0.39f);
        float healthX = (float)w - healthW - 8.0f;
        DrawPanel((Rectangle){ healthX, 8, healthW, 66 }, d->color);
        DrawText(d->name, (int)healthX + 10, 16, 9, d->color);
        DrawText("INTEGRITY", (int)healthX + 10, 33, 9, C_MUTED);
        DrawRectangleRounded((Rectangle){ healthX + 10, 48, healthW - 20, 8 }, 0.5f, 6, (Color){ 38, 47, 74, 255 });
        Color healthColor = (g.player.health > g.player.maxHealth * 0.5f) ? C_GREEN : (g.player.health > g.player.maxHealth * 0.25f ? C_GOLD : C_RED);
        DrawRectangleRounded((Rectangle){ healthX + 10, 48, (healthW - 20) * ClampF(g.player.health / g.player.maxHealth, 0, 1), 8 }, 0.5f, 6, healthColor);
        DrawCentered("SCORE MILESTONES DRIVE THE PRESSURE", 84, 9, C_MUTED);
        DrawRectangleRounded((Rectangle){ (float)w * 0.5f - 66.0f, 98, 132, 3 }, 0.5f, 4, (Color){ 31, 38, 70, 220 });
        DrawRectangleRounded((Rectangle){ (float)w * 0.5f - 66.0f, 98, 132.0f * ((float)(g.score % SCORE_STEP) / (float)SCORE_STEP), 3 }, 0.5f, 4, C_PURPLE);
    } else {
        DrawPanel((Rectangle){ 18, 16, 248, 90 }, C_CYAN);
        DrawText("SCORE", 34, 27, 12, C_MUTED);
        snprintf(buf, sizeof(buf), "%07d", g.score);
        DrawText(buf, 31, 43, 31, C_TEXT);
        snprintf(buf, sizeof(buf), "STAGE %02d  /  %d KILLS", g.stage, g.kills);
        DrawText(buf, 34, 80, 11, C_CYAN);

        DrawPanel((Rectangle){ (float)w - 242.0f, 16, 224, 90 }, d->color);
        DrawText(d->name, w - 224, 27, 12, d->color);
        DrawText("INTEGRITY", w - 224, 47, 10, C_MUTED);
        DrawRectangleRounded((Rectangle){ (float)w - 224, 63, 188, 10 }, 0.5f, 6, (Color){ 38, 47, 74, 255 });
        Color healthColor = (g.player.health > g.player.maxHealth * 0.5f) ? C_GREEN : (g.player.health > g.player.maxHealth * 0.25f ? C_GOLD : C_RED);
        DrawRectangleRounded((Rectangle){ (float)w - 224, 63, 188.0f * ClampF(g.player.health / g.player.maxHealth, 0, 1), 10 }, 0.5f, 6, healthColor);
        snprintf(buf, sizeof(buf), "%d%%", (int)(g.player.health / g.player.maxHealth * 100.0f));
        DrawRight(buf, w - 36, 79, 11, C_TEXT);

        DrawPanel((Rectangle){ (float)w * 0.5f - 125.0f, 18, 250, 43 }, C_PURPLE);
        snprintf(buf, sizeof(buf), "RIFT STAGE %02d", g.stage);
        DrawCentered(buf, 31, 17, C_TEXT);
        float progress = (float)(g.score % SCORE_STEP) / (float)SCORE_STEP;
        DrawRectangleRounded((Rectangle){ (float)w * 0.5f - 110.0f, 66, 220, 3 }, 0.5f, 4, (Color){ 31, 38, 70, 220 });
        DrawRectangleRounded((Rectangle){ (float)w * 0.5f - 110.0f, 66, 220.0f * progress, 3 }, 0.5f, 4, C_PURPLE);
        snprintf(buf, sizeof(buf), "%d / %d TO NEXT STAGE", g.score % SCORE_STEP, SCORE_STEP);
        DrawCentered(buf, 76, 9, C_MUTED);
    }
    /* Dash recharge indicator: deliberately no survival-time counter. */
    float dashFill = 1.0f - ClampF(g.player.dashClock / 1.12f, 0.0f, 1.0f);
    DrawText("PHASE DASH [SPACE]", 24, SH() - 38, 10, C_MUTED);
    DrawRectangleRounded((Rectangle){ 24, SH() - 22.0f, 164, 7 }, 0.5f, 4, (Color){ 29, 38, 67, 255 });
    DrawRectangleRounded((Rectangle){ 24, SH() - 22.0f, 164.0f * dashFill, 7 }, 0.5f, 4, C_CYAN);
    if (g.combo >= 2 && g.comboClock > 0.0f) {
        snprintf(buf, sizeof(buf), "COMBO x%d", g.combo);
        DrawRight(buf, w - 26, SH() - 42, 15, C_GOLD);
    }
    for (int i = 0; i < MAX_FLOATERS; ++i) {
        Floater *f = &g.floaters[i];
        if (!f->active) continue;
        snprintf(buf, sizeof(buf), "+%d", f->points);
        int alpha = (int)(255.0f * ClampF(f->life / 0.88f, 0, 1));
        DrawText(buf, (int)f->pos.x - MeasureText(buf, 15) / 2, (int)f->pos.y, 15, Alpha(f->color, (float)alpha));
    }
    if (g.stageNotice > 0.0f) {
        float a = ClampF(g.stageNotice * 2.0f, 0, 1);
        snprintf(buf, sizeof(buf), "RIFT STAGE %02d  //  PRESSURE INCREASED", g.stage);
        int size = (SW() < 760) ? 16 : 21;
        DrawCentered(buf, (int)(SH() * 0.23f), size, Alpha(C_GOLD, a * 255.0f));
    }
}

static void DrawButton(Rectangle r, const char *label, Color accent, bool selected)
{
    Vector2 mouse = GetMousePosition();
    bool hover = HitRect(mouse, r);
    Color fill = selected || hover ? WithOpacity(accent, 0.16f) : C_PANEL;
    Color border = selected || hover ? accent : (Color){ 67, 82, 126, 220 };
    DrawRectangleRounded(r, 0.16f, 10, fill);
    DrawRectangleRoundedLinesEx(r, 0.16f, 10, selected || hover ? 2.0f : 1.0f, border);
    DrawText(label, (int)(r.x + (r.width - MeasureText(label, 18)) * 0.5f), (int)(r.y + (r.height - 18) * 0.5f - 1), 18, selected || hover ? C_TEXT : C_MUTED);
}

static Rectangle DifficultyCard(int i)
{
    float gap = (SW() < 600) ? 6.0f : 16.0f;
    float cardW = MinF(218.0f, ((float)SW() - 40.0f - gap * 2.0f) / 3.0f);
    float total = cardW * 3.0f + gap * 2.0f;
    float start = ((float)SW() - total) * 0.5f;
    return (Rectangle){ start + i * (cardW + gap), (float)SH() * 0.455f, cardW, 104.0f };
}

static void DrawTitle(void)
{
    int h = SH(), w = SW();
    DrawCentered("SIGNAL ZERO", (int)(h * 0.135f), 17, C_CYAN);
    DrawCentered("RIFT RUNNER", (int)(h * 0.17f), (w < 440) ? 32 : ((w < 720) ? 40 : 57), C_TEXT);
    DrawCentered("A SCORE-DRIVEN NEON SURVIVAL RUN", (int)(h * 0.265f), 13, C_MUTED);
    DrawCentered("CHOOSE YOUR PRESSURE LEVEL", (int)(h * 0.395f), 13, C_CYAN);

    for (int i = 0; i < 3; ++i) {
        Rectangle r = DifficultyCard(i);
        bool selected = g.menuSelection == i;
        const DifficultyInfo *d = &DIFFICULTIES[i];
        DrawRectangleRounded(r, 0.12f, 10, selected ? (Color){ 25, 34, 69, 245 } : C_PANEL);
        DrawRectangleRoundedLinesEx(r, 0.12f, 10, selected ? 2.0f : 1.0f, selected ? d->color : (Color){ 60, 73, 112, 220 });
        DrawText(d->name, (int)(r.x + (r.width - MeasureText(d->name, 19)) * 0.5f), (int)r.y + 18, 19, d->color);
        if (r.width > 150.0f) DrawText(d->tagline, (int)(r.x + 12), (int)r.y + 54, 11, C_MUTED);
        DrawText(i == 0 ? "1" : (i == 1 ? "2" : "3"), (int)(r.x + r.width - 25), (int)r.y + 12, 11, selected ? C_TEXT : C_MUTED);
        if (selected) DrawRectangleRounded((Rectangle){ r.x + 16, r.y + r.height - 13, r.width - 32, 3 }, 0.5f, 5, d->color);
    }
    float startW = MinF(300.0f, (float)w - 36.0f);
    Rectangle startBtn = { (float)w * 0.5f - startW * 0.5f, (float)h * 0.635f, startW, 57.0f };
    DrawButton(startBtn, "ENTER THE RIFT  [ENTER]", C_CYAN, true);
    DrawCentered("WASD / ARROWS MOVE  ·  AUTO-FIRE FOLLOWS FACING", (int)(h * 0.755f), 12, C_TEXT);
    DrawCentered("HOLD LEFT MOUSE TO AIM  ·  SPACE / SHIFT TO PHASE DASH", (int)(h * 0.79f), 11, C_MUTED);
    DrawCentered("B  PERSONAL BESTS       ESC  QUIT", (int)(h * 0.85f), 11, C_MUTED);
    DrawCentered("EVERY STAGE ADVANCES AT SCORE MILESTONES — NOT BY ELAPSED TIME", (int)(h * 0.91f), 10, (Color){ 111, 128, 176, 255 });
}

static void DrawScores(void)
{
    int w = SW(), h = SH();
    int titleSize = (w < 440) ? 21 : 30;
    DrawCentered("PERSONAL BESTS", (int)(h * 0.10f), titleSize, C_TEXT);
    DrawCentered("ONE HIGH SCORE PER MODE  ·  SAVED ON THIS DEVICE", (int)(h * 0.16f), 11, C_MUTED);

    float panelW = MinF(700.0f, (float)w * 0.90f);
    float rowH = MinF(76.0f, (float)h * 0.105f);
    float panelH = rowH * 3.0f + 54.0f;
    Rectangle panel = { ((float)w - panelW) * 0.5f, ((float)h - panelH) * 0.51f, panelW, panelH };
    DrawPanel(panel, C_PURPLE);

    for (int i = 0; i < 3; ++i) {
        const DifficultyInfo *d = &DIFFICULTIES[i];
        float y = panel.y + 18.0f + i * rowH;
        Rectangle row = { panel.x + 12.0f, y, panel.width - 24.0f, rowH - 7.0f };
        DrawRectangleRounded(row, 0.16f, 8, (Color){ 21, 29, 59, 235 });
        DrawRectangleRoundedLinesEx(row, 0.16f, 8, 1.2f, WithOpacity(d->color, 0.65f));
        DrawText(d->name, (int)row.x + 16, (int)(row.y + row.height * 0.5f - 10), 16, d->color);
        DrawText("HIGH SCORE", (int)row.x + (w < 480 ? 90 : 170), (int)row.y + 9, 9, C_MUTED);
        char scoreText[32];
        if (g.bestScores[i] >= 0) snprintf(scoreText, sizeof(scoreText), "%07d", g.bestScores[i]);
        else snprintf(scoreText, sizeof(scoreText), "N/A");
        int scoreSize = (w < 480) ? 19 : 24;
        DrawRight(scoreText, (int)(row.x + row.width - 16.0f), (int)(row.y + row.height * 0.5f - scoreSize * 0.5f), scoreSize,
                  g.bestScores[i] >= 0 ? C_TEXT : C_MUTED);
        if (g.bestScores[i] < 0) DrawText("PLAY THIS MODE TO SET A RECORD", (int)row.x + (w < 480 ? 90 : 170), (int)row.y + 24, 9, C_MUTED);
    }
    DrawCentered("B / ESC  BACK", (int)(h * 0.88f), 13, C_CYAN);
}

static void DrawGameOver(void)
{
    int w = SW(), h = SH();
    DrawRectangle(0, 0, w, h, (Color){ 3, 4, 13, 190 });
    float panelW = MinF(476.0f, (float)w - 24.0f);
    float panelH = MinF(410.0f, (float)h - 24.0f);
    Rectangle p = { (float)w * 0.5f - panelW * 0.5f, (float)h * 0.5f - panelH * 0.5f, panelW, panelH };
    DrawPanel(p, C_PINK);
    float scaleY = panelH / 410.0f;
    DrawCentered("RUN TERMINATED", (int)p.y + (int)(31 * scaleY), 25, C_PINK);
    DrawCentered("FINAL SCORE", (int)p.y + (int)(83 * scaleY), 12, C_MUTED);
    char buf[100];
    snprintf(buf, sizeof(buf), "%07d", g.score);
    DrawCentered(buf, (int)p.y + (int)(105 * scaleY), (w < 440) ? 38 : 51, C_TEXT);
    snprintf(buf, sizeof(buf), "%s    /    RIFT %02d    /    %d KILLS", GetDifficulty()->name, g.stage, g.kills);
    DrawCentered(buf, (int)p.y + (int)(167 * scaleY), (w < 440) ? 9 : 12, GetDifficulty()->color);
    if (g.newPersonalBest) DrawCentered("NEW PERSONAL BEST FOR THIS MODE!", (int)p.y + (int)(207 * scaleY), (w < 440) ? 11 : 15, C_GOLD);
    else DrawCentered("MODE BEST: SCORE NOT BEATEN", (int)p.y + (int)(207 * scaleY), (w < 440) ? 9 : 11, C_MUTED);
    Rectangle again = { p.x + 24, p.y + panelH * 0.64f, p.width - 48, 50 };
    DrawButton(again, "RUN AGAIN  [ENTER]", C_CYAN, true);
    DrawCentered("ESC  TITLE     B  PERSONAL BESTS", (int)p.y + (int)(348 * scaleY), 11, C_MUTED);
}

static void DrawPause(void)
{
    int w = SW(), h = SH();
    DrawRectangle(0, 0, w, h, (Color){ 3, 4, 13, 178 });
    DrawCentered("PAUSED", (int)(h * 0.40f), 42, C_TEXT);
    DrawCentered("P / ENTER  RESUME", (int)(h * 0.50f), 15, C_CYAN);
    DrawCentered("B  PERSONAL BESTS       ENTER / P  RESUME", (int)(h * 0.55f), 12, C_MUTED);
}

static void DrawTouchControls(void)
{
    if (g.state != STATE_PLAYING) return;
    if (SW() > 820 && GetTouchPointCount() <= 0) return;
    Vector2 anchor = V2(105.0f, (float)SH() - 126.0f);
    DrawCircleLines((int)anchor.x, (int)anchor.y, 55.0f, (Color){ 126, 206, 255, 95 });
    DrawCircleV(anchor, 22.0f, (Color){ 77, 123, 195, 50 });
    DrawCircleLines(SW() - 84, SH() - 92, 39.0f, (Color){ 126, 206, 255, 95 });
    DrawText("DASH", SW() - 111, SH() - 98, 11, C_CYAN);
}

static void DrawFrame(void)
{
    int w = SW(), h = SH();
    Camera2D camera = { 0 };
    float sx = 0.0f, sy = 0.0f;
    if (g.screenShake > 0.0f) {
        float amount = g.screenShake * 19.0f;
        sx = RandF(-amount, amount);
        sy = RandF(-amount, amount);
    }
    camera.offset = V2(w * 0.5f + sx, h * 0.5f + sy);
    camera.target = V2(w * 0.5f, h * 0.5f);
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;

    BeginDrawing();
    BeginMode2D(camera);
    DrawBackground();
    if (g.state == STATE_PLAYING || g.state == STATE_PAUSED || g.state == STATE_GAME_OVER) DrawWorld();
    else {
        /* Decorative rift rings and sparks on menus, without starting gameplay. */
        Vector2 c = V2(w * 0.5f, h * 0.52f);
        float pulse = 1.0f + sinf(g.worldTime * 1.2f) * 0.025f;
        DrawCircleGradient(c, 118.0f * pulse, (Color){ 75, 44, 194, 26 }, (Color){ 75, 44, 194, 0 });
        DrawCircleLines((int)c.x, (int)c.y, 80.0f * pulse, (Color){ 89, 222, 255, 90 });
        DrawCircleLines((int)c.x, (int)c.y, 100.0f * pulse, (Color){ 175, 91, 255, 68 });
        for (int i = 0; i < 10; ++i) {
            float angle = g.worldTime * 0.55f + (float)i * (2.0f * PI_F / 10.0f);
            Vector2 p = Add(c, V2(cosf(angle) * 90.0f, sinf(angle) * 90.0f));
            DrawCircleGradient(p, 9.0f, WithOpacity(C_CYAN, 0.35f), (Color){ 76, 231, 255, 0 });
        }
    }
    EndMode2D();

    if (g.state == STATE_TITLE) DrawTitle();
    if (g.state == STATE_PLAYING) { DrawHUD(); DrawTouchControls(); }
    if (g.state == STATE_PAUSED) { DrawHUD(); DrawPause(); }
    if (g.state == STATE_GAME_OVER) DrawGameOver();
    if (g.state == STATE_SCORES) DrawScores();
    if (g.flashAlpha > 0.0f) DrawRectangle(0, 0, w, h, (Color){ 188, 226, 255, (unsigned char)(ClampF(g.flashAlpha, 0, 0.4f) * 255.0f) });
    EndDrawing();
}

/* ---------- Menu navigation and initialization ---------- */
static void SetDifficulty(int index)
{
    if (index < 0) index = 2;
    if (index > 2) index = 0;
    g.menuSelection = index;
    g.difficulty = (Difficulty)index;
}

static void InitGame(void)
{
    memset(&g, 0, sizeof(g));
    g.state = STATE_TITLE;
    g.difficulty = DIFF_MEDIUM;
    g.menuSelection = DIFF_MEDIUM;
    g.worldTime = 0.0f;
    for (int i = 0; i < 3; ++i) g.bestScores[i] = -1;
    srand((unsigned int)time(NULL));
    for (int i = 0; i < MAX_STARS; ++i) {
        g.stars[i] = (Star){ RandF(0.0f, (float)SCREEN_START_W), RandF(0.0f, (float)SCREEN_START_H), RandF(5.0f, 25.0f), RandF(0.6f, 1.8f), RandF(0.0f, 6.28f) };
    }
    LoadBestScores();

}

static void UpdateTitle(void)
{
    if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A)) SetDifficulty(g.menuSelection - 1);
    if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D)) SetDifficulty(g.menuSelection + 1);
    if (IsKeyPressed(KEY_ONE) || IsKeyPressed(KEY_KP_1)) SetDifficulty(DIFF_EASY);
    if (IsKeyPressed(KEY_TWO) || IsKeyPressed(KEY_KP_2)) SetDifficulty(DIFF_MEDIUM);
    if (IsKeyPressed(KEY_THREE) || IsKeyPressed(KEY_KP_3)) SetDifficulty(DIFF_HARD);
    if (IsKeyPressed(KEY_B)) g.state = STATE_SCORES;
    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) { StartRun(); return; }

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        Vector2 m = GetMousePosition();
        for (int i = 0; i < 3; ++i) if (HitRect(m, DifficultyCard(i))) SetDifficulty(i);
        float startW = MinF(300.0f, (float)SW() - 36.0f);
        Rectangle startBtn = { (float)SW() * 0.5f - startW * 0.5f, (float)SH() * 0.635f, startW, 57.0f };
        if (HitRect(m, startBtn)) StartRun();
    }
    if (IsKeyPressed(KEY_ESCAPE)) {
#if !defined(__EMSCRIPTEN__)
        g.quitRequested = true;
#endif
    }
}

static void UpdatePaused(void)
{
    if (IsKeyPressed(KEY_P) || IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_ESCAPE)) g.state = STATE_PLAYING;
    if (IsKeyPressed(KEY_B)) g.state = STATE_SCORES;
}

static void UpdateGameOver(void)
{
    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) StartRun();
    if (IsKeyPressed(KEY_B)) g.state = STATE_SCORES;
    if (IsKeyPressed(KEY_ESCAPE)) { g.state = STATE_TITLE; }
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        float panelW = MinF(476.0f, (float)SW() - 24.0f);
        float panelH = MinF(410.0f, (float)SH() - 24.0f);
        Rectangle again = { (float)SW() * 0.5f - panelW * 0.5f + 24.0f, (float)SH() * 0.5f - panelH * 0.5f + panelH * 0.64f, panelW - 48.0f, 50.0f };
        if (HitRect(GetMousePosition(), again)) StartRun();
    }
}

static void UpdateScores(void)
{
    if (IsKeyPressed(KEY_B) || IsKeyPressed(KEY_ESCAPE)) g.state = STATE_TITLE;
}

static void Frame(void)
{
    float dt = GetFrameTime();
    if (!isfinite(dt) || dt < 0.0f) dt = 0.0f;
    if (dt > 1.0f / 25.0f) dt = 1.0f / 25.0f;
    g.worldTime += dt;

    if (g.audioReady && g.musicLoaded && g.firstAudioGesture) {
        UpdateMusicStream(g.ambient);
        if (!IsMusicStreamPlaying(g.ambient)) PlayMusicStream(g.ambient);
    }
    switch (g.state) {
        case STATE_TITLE: UpdateTitle(); UpdateEffects(dt); break;
        case STATE_PLAYING: UpdatePlaying(dt); break;
        case STATE_PAUSED: UpdatePaused(); UpdateEffects(dt); break;
        case STATE_GAME_OVER: UpdateGameOver(); UpdateEffects(dt); break;
        case STATE_SCORES: UpdateScores(); UpdateEffects(dt); break;
    }
    DrawFrame();
}

#if !defined(__EMSCRIPTEN__)
static void ShutdownGame(void)
{
    if (g.audioReady) {
        if (g.musicLoaded) { StopMusicStream(g.ambient); UnloadMusicStream(g.ambient); }
        for (int i = 0; i < SFX_COUNT; ++i) if (IsSoundValid(g.sounds[i])) UnloadSound(g.sounds[i]);
        CloseAudioDevice();
    }
}
#endif

int main(void)
{
#if defined(__EMSCRIPTEN__)
    SetConfigFlags(FLAG_MSAA_4X_HINT);
#else
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT | FLAG_MSAA_4X_HINT);
#endif
    InitWindow(SCREEN_START_W, SCREEN_START_H, RR_TITLE);
    if (!IsWindowReady()) {
        fprintf(stderr, "RIFT RUNNER: could not initialize the raylib window.\n");
        return 1;
    }
#if !defined(__EMSCRIPTEN__)
    SetWindowMinSize(800, 480);
#endif
    SetTargetFPS(TARGET_FPS);
    SetExitKey(KEY_NULL);
    InitGame();

#if defined(__EMSCRIPTEN__)
    emscripten_set_main_loop(Frame, 0, 1);
#else
    while (!WindowShouldClose() && !g.quitRequested) Frame();
    ShutdownGame();
    CloseWindow();
#endif
    return 0;
}
