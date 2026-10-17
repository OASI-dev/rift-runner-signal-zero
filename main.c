/*
 * RIFT RUNNER: SIGNAL ZERO — SINGLE-FILE EDITION v0.26.0
 *
 * All game logic is in this file. raylib supplies the window, drawing,
 * keyboard/mouse/touch input, frame timing, and sound API; the game rules,
 * enemy AI, effects, menus, backgrounds and score storage are implemented below.
 *
 * This edition combines the previous small .c/.h modules into one main.c to
 * make the whole project easier to find, read, present and compile as one unit.
 */
#include <raylib.h>
#include <stdbool.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#if defined(__EMSCRIPTEN__)
#include <emscripten.h>
#endif

/* Definitions originally in audio.h; now kept in this one source file. */
/* Original one-shot sound effects only; no music or background ambience. */
typedef enum SoundId { SFX_SHOT, SFX_IMPACT, SFX_PICKUP, SFX_ENEMY_DOWN, SFX_SURGE, SFX_COUNT } SoundId;

/* Definitions originally in background.h; now kept in this one source file. */
/* Star position and animation data used by the background renderer. */
typedef struct Star {
    float x, y, speed, size, phase;
} Star;

/* Friendly sector names shared by the background and HUD. */
const char *GetSectorName(void);

/* Definitions originally in bullets.h; now kept in this one source file. */
/* A player blaster shot. */
typedef struct Bullet {
    bool active;
    Vector2 pos;
    Vector2 vel;
    float radius;
    float life;
    Color color;
} Bullet;

/* Definitions originally in colors.h; now kept in this one source file. */
/* The shared game palette is defined once in colors.c. */
extern const Color C_BG, C_PANEL, C_PANEL2;
extern const Color C_CYAN, C_BLUE, C_PURPLE, C_PINK, C_GREEN, C_GOLD;
extern const Color C_TEXT, C_MUTED, C_RED;

/* Definitions originally in effects.h; now kept in this one source file. */
/* Lightweight visual particles for hits, impacts, and transitions. */
typedef struct Particle {
    bool active;
    Vector2 pos;
    Vector2 vel;
    float radius;
    float life;
    float fullLife;
    Color color;
} Particle;

/* Definitions originally in enemy.h; now kept in this one source file. */
/* One enemy, including movement/turning state and red-wraith/blue-skitter type. */
typedef struct Enemy {
    bool active;
    Vector2 pos, vel;
    float radius, speed, hp, maxHp;
    float heading, turnClock, turnRate, flash;
    bool blue;
} Enemy;

/* Definitions originally in pickups.h; now kept in this one source file. */
/* The banana is the only pickup: it restores up to 20 health and grants an 8-second shield. */
typedef struct Pickup {
    bool active;
    Vector2 pos;
    float phase;
    float life;
} Pickup;

/* Definitions originally in player.h; now kept in this one source file. */
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

/* Shared state, constants, and function declarations formerly in game.h. */
#define RR_TITLE "RIFT RUNNER: SIGNAL ZERO"
#define SCREEN_START_W 1280
#define SCREEN_START_H 720
#define TARGET_FPS 60
#define MAX_BULLETS 240
#define MAX_ENEMIES 90
#define MAX_PARTICLES 420
#define MAX_PICKUPS 4
#define MAX_STARS 120
#define SCORE_STEP 500
#define PI_F 3.14159265358979323846f

typedef enum GameState { STATE_TITLE, STATE_PLAYING, STATE_PAUSED, STATE_GAME_OVER, STATE_SCORES } GameState;
typedef enum Difficulty { DIFF_EASY = 0, DIFF_MEDIUM = 1, DIFF_HARD = 2 } Difficulty;
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

/* Shared live game state. Systems are split into small files but work on this state. */
typedef struct Game {
    GameState state;
    GameState scoresReturnState; /* Return to title, pause, or game-over from records. */
    Difficulty difficulty;
    Player player;
    Bullet bullets[MAX_BULLETS];
    Enemy enemies[MAX_ENEMIES];
    Particle particles[MAX_PARTICLES];
    Pickup pickups[MAX_PICKUPS];
    Star stars[MAX_STARS];
    int bestScores[3];
    int score, stage;
    bool newPersonalBest;
    bool audioReady, firstAudioGesture, quitRequested;
    float worldTime, spawnClock, screenShake, flashAlpha, stageNotice;
    float bananaClock, backgroundTransition;
    int backgroundTheme, menuSelection;
    Sound sounds[SFX_COUNT];
} Game;

extern Game g;
extern const DifficultyInfo DIFFICULTIES[3];

/* Shared math, geometry and text helpers. */
float ClampF(float v, float lo, float hi);
float MaxF(float a, float b);
float MinF(float a, float b);
float RandF(float lo, float hi);
float LengthSq(Vector2 v);
float Length(Vector2 v);
Vector2 V2(float x, float y);
Vector2 Add(Vector2 a, Vector2 b);
Vector2 Sub(Vector2 a, Vector2 b);
Vector2 Scale(Vector2 a, float s);
Vector2 RRNormalizeVector2(Vector2 v);
Vector2 Perp(Vector2 v);
float DistSq(Vector2 a, Vector2 b);
Color Alpha(Color c, float a);
Color WithOpacity(Color c, float opacity);
int SW(void);
int SH(void);
bool HitRect(Vector2 p, Rectangle r);
void DrawCentered(const char *text, int y, int size, Color color);
void DrawRight(const char *text, int rightX, int y, int size, Color color);
const DifficultyInfo *GetDifficulty(void);
int CurrentStageForScore(int score);

/* Audio and personal-best storage. */
void FirstAudioGesture(void);
void PlaySfx(SoundId id);
void SaveBestScores(void);
void LoadBestScores(void);
void RecordPersonalBest(void);
void ShutdownAudio(void);

/* Gameplay systems. */
void ResetWorld(void);
void StartRun(void);
void EndRun(void);
void UpdatePlayer(float dt);
void AddBullet(Vector2 pos, Vector2 dir);
void UpdateBullets(float dt);
void SpawnEnemy(void);
void KillEnemy(Enemy *e);
void UpdateEnemies(float dt);
void AddPickup(Vector2 pos);
void UpdatePickups(float dt);
void AddParticle(Vector2 pos, Vector2 vel, float radius, float life, Color color);
void Burst(Vector2 pos, Color color, int count, float speed, float radius);
void UpdateEffects(float dt);

/* Rendering: each module draws its own visual layer. */
void DrawBackground(void);
void DrawRunner(void);
void DrawEnemy(const Enemy *e);
void DrawWorld(void);
void DrawPanel(Rectangle r, Color accent);
void DrawHUD(void);
void DrawButton(Rectangle r, const char *label, Color accent, bool selected);
Rectangle DifficultyCard(int i);
void DrawTitle(void);
void DrawScores(void);
void DrawGameOver(void);
void DrawPause(void);
void DrawTouchControls(void);
void PollUIPointer(void);
bool GetUIPointerPress(Vector2 *position);
void DrawFrame(void);

/* Screen lifecycle and input, implemented in game.c / ui.c. */
void InitGame(void);
void SetDifficulty(int index);
void UpdateTitle(void);
void UpdatePaused(void);
void UpdateGameOver(void);
void UpdateScores(void);
void UpdatePlaying(float dt);
void GameFrame(void);



/* ======================================================================
 * SECTION: helpers.c — merged into main.c from the v0.26.0 modular source.
 * ====================================================================== */

/* Shared math, screen and text helpers. */


float ClampF(float v, float lo, float hi) { return (v < lo) ? lo : ((v > hi) ? hi : v); }

float MaxF(float a, float b) { return (a > b) ? a : b; }

float MinF(float a, float b) { return (a < b) ? a : b; }

float RandF(float lo, float hi) { return lo + ((float)rand() / (float)RAND_MAX) * (hi - lo); }

float LengthSq(Vector2 v) { return v.x * v.x + v.y * v.y; }

float Length(Vector2 v) { return sqrtf(LengthSq(v)); }

Vector2 V2(float x, float y) { Vector2 v = { x, y }; return v; }

Vector2 Add(Vector2 a, Vector2 b) { return V2(a.x + b.x, a.y + b.y); }

Vector2 Sub(Vector2 a, Vector2 b) { return V2(a.x - b.x, a.y - b.y); }

Vector2 Scale(Vector2 a, float s) { return V2(a.x * s, a.y * s); }

Vector2 RRNormalizeVector2(Vector2 v) { float len = Length(v); return (len > 0.0001f) ? Scale(v, 1.0f / len) : V2(0.0f, 0.0f); }

Vector2 Perp(Vector2 v) { return V2(-v.y, v.x); }

float DistSq(Vector2 a, Vector2 b) { return LengthSq(Sub(a, b)); }

Color Alpha(Color c, float a) { c.a = (unsigned char)ClampF(a, 0.0f, 255.0f); return c; }

Color WithOpacity(Color c, float opacity) { return Alpha(c, (float)c.a * ClampF(opacity, 0.0f, 1.0f)); }

int SW(void) { return GetScreenWidth(); }

int SH(void) { return GetScreenHeight(); }

bool HitRect(Vector2 p, Rectangle r) { return p.x >= r.x && p.y >= r.y && p.x <= r.x + r.width && p.y <= r.y + r.height; }

void DrawCentered(const char *text, int y, int size, Color color) { DrawText(text, (SW() - MeasureText(text, size)) / 2, y, size, color); }

void DrawRight(const char *text, int rightX, int y, int size, Color color) { DrawText(text, rightX - MeasureText(text, size), y, size, color); }

const DifficultyInfo *GetDifficulty(void) { return &DIFFICULTIES[(int)g.difficulty]; }

int CurrentStageForScore(int score) { return 1 + score / SCORE_STEP; }


/* ======================================================================
 * SECTION: colors.c — merged into main.c from the v0.26.0 modular source.
 * ====================================================================== */

/* Shared palette: change colors here instead of hunting through gameplay files. */

const Color C_BG     = {  6,   8,  22, 255};
const Color C_PANEL  = { 15,  21,  45, 238};
const Color C_PANEL2 = { 21,  29,  59, 245};
const Color C_CYAN   = { 76, 231, 255, 255};
const Color C_BLUE   = { 78, 131, 255, 255};
const Color C_PURPLE = {179,  91, 255, 255};
const Color C_PINK   = {255,  83, 170, 255};
const Color C_GREEN  = { 98, 255, 178, 255};
const Color C_GOLD   = {255, 207, 100, 255};
const Color C_TEXT   = {236, 243, 255, 255};
const Color C_MUTED  = {143, 158, 196, 255};
const Color C_RED    = {255,  83,  99, 255};


/* ======================================================================
 * SECTION: background.c — merged into main.c from the v0.26.0 modular source.
 * ====================================================================== */

/*
 * Procedural space environments.
 * Every 1,000 score, the scene changes its actual landmarks and geometry—not
 * only its colors. The transition is a brief quiet dim-and-reveal, not an
 * explosion or radial burst.
 */

const char *GetSectorName(void)
{
    static const char *names[5] = {
        "THE CRADLE", "GRAVEYARD BELT", "THE RIFT", "BLOOD MOON", "VOID CATHEDRAL"
    };
    int theme = g.backgroundTheme % 5;
    if (theme < 0) theme += 5;
    return names[theme];
}

static Color FadeColor(Color c, float opacity)
{
    c.a = (unsigned char)(ClampF(opacity, 0.0f, 1.0f) * (float)c.a);
    return c;
}

static void Nebula(Vector2 center, float radius, Color color, float drift)
{
    Vector2 p = V2(center.x + sinf(g.worldTime * 0.035f + drift) * 24.0f,
                   center.y + cosf(g.worldTime * 0.027f + drift) * 15.0f);
    DrawCircleGradient(p, radius, FadeColor(color, 0.22f), FadeColor(color, 0.0f));
    DrawCircleGradient(V2(p.x + radius * 0.18f, p.y - radius * 0.08f), radius * 0.62f,
                       FadeColor(color, 0.13f), FadeColor(color, 0.0f));
}

static void RingedPlanet(Vector2 p, float radius, Color body, Color ring)
{
    DrawCircleGradient(p, radius * 1.48f, FadeColor(ring, 0.12f), FadeColor(ring, 0.0f));
    /* A slanted-looking segmented ring, layered behind and in front of the planet. */
    for (int i = 0; i < 52; ++i) {
        float a0 = (float)i * (2.0f * PI_F / 52.0f);
        float a1 = (float)(i + 1) * (2.0f * PI_F / 52.0f);
        Vector2 a = V2(p.x + cosf(a0) * radius * 1.65f,
                       p.y + sinf(a0) * radius * 0.48f);
        Vector2 b = V2(p.x + cosf(a1) * radius * 1.65f,
                       p.y + sinf(a1) * radius * 0.48f);
        DrawLineEx(a, b, 2.2f, FadeColor(ring, 0.44f));
    }
    DrawCircleGradient(p, radius, body, FadeColor(body, 0.60f));
    DrawCircleV(V2(p.x - radius * 0.23f, p.y - radius * 0.16f), radius * 0.17f,
                FadeColor((Color){ 220, 230, 255, 255 }, 0.14f));
    DrawCircleLines((int)p.x, (int)p.y, radius, FadeColor(ring, 0.60f));
    for (int i = 0; i < 52; ++i) {
        float a0 = (float)i * (2.0f * PI_F / 52.0f);
        float a1 = (float)(i + 1) * (2.0f * PI_F / 52.0f);
        if (sinf((a0 + a1) * 0.5f) < -0.10f) continue;
        Vector2 a = V2(p.x + cosf(a0) * radius * 1.65f,
                       p.y + sinf(a0) * radius * 0.48f);
        Vector2 b = V2(p.x + cosf(a1) * radius * 1.65f,
                       p.y + sinf(a1) * radius * 0.48f);
        DrawLineEx(a, b, 2.0f, FadeColor(ring, 0.72f));
    }
}

static void DrawCraterMoon(Vector2 p, float radius, Color base, Color edge)
{
    DrawCircleGradient(p, radius * 1.30f, FadeColor(edge, 0.20f), FadeColor(edge, 0.0f));
    DrawCircleGradient(p, radius, base, FadeColor(base, 0.78f));
    for (int i = 0; i < 9; ++i) {
        float angle = (float)i * 2.39996f + 0.6f;
        float distance = radius * (0.22f + 0.44f * (float)((i * 7) % 9) / 9.0f);
        Vector2 c = V2(p.x + cosf(angle) * distance, p.y + sinf(angle) * distance);
        float cr = radius * (0.035f + 0.022f * (float)(i % 3));
        DrawCircleV(c, cr * 1.55f, FadeColor(edge, 0.23f));
        DrawCircleV(c, cr, FadeColor((Color){ 9, 8, 20, 255 }, 0.38f));
        DrawCircleLines((int)c.x, (int)c.y, cr, FadeColor(edge, 0.26f));
    }
    DrawCircleLines((int)p.x, (int)p.y, radius, FadeColor(edge, 0.44f));
}

static void DrawAsteroid(float x, float y, float radius, float rotation, int seed)
{
    Color rock = (seed % 2) ? (Color){ 47, 51, 79, 255 } : (Color){ 34, 39, 61, 255 };
    Color edge = (Color){ 105, 120, 157, 255 };
    DrawCircleGradient(V2(x, y), radius * 1.8f, FadeColor(C_PURPLE, 0.07f), FadeColor(C_PURPLE, 0.0f));
    DrawPoly(V2(x, y), 7, radius, rotation, rock);
    DrawPolyLines(V2(x, y), 7, radius, rotation, FadeColor(edge, 0.38f));
    for (int k = 0; k < 3; ++k) {
        float a = rotation * PI_F / 180.0f + (float)k * 2.1f;
        Vector2 c = V2(x + cosf(a) * radius * 0.34f, y + sinf(a) * radius * 0.30f);
        DrawCircleV(c, radius * (0.09f + 0.025f * (float)(k % 2)), FadeColor((Color){ 6, 9, 24, 255 }, 0.6f));
    }
}

static void DrawVoidSpire(float x, float baseY, float width, float height, float pulse)
{
    Vector2 tip = V2(x + sinf(g.worldTime * 0.16f + x) * width * 0.12f, baseY - height);
    Vector2 left = V2(x - width * 0.5f, baseY);
    Vector2 right = V2(x + width * 0.5f, baseY);
    DrawTriangle(tip, left, right, (Color){ 11, 13, 34, 255 });
    DrawTriangle(tip, V2(x, baseY - height * 0.62f), right, FadeColor(C_PURPLE, 0.12f));
    DrawLineEx(tip, left, 1.3f, FadeColor(C_CYAN, 0.15f + pulse * 0.08f));
    DrawLineEx(tip, right, 1.3f, FadeColor(C_PURPLE, 0.19f));
}

static void DrawEnvironment(int theme, int w, int h)
{
    Vector2 center = V2(w * 0.5f, h * 0.48f);
    switch (theme) {
        case 0: /* THE CRADLE — distant ringworld and a soft violet nebula. */
            Nebula(V2(w * 0.28f, h * 0.40f), (float)h * 0.56f, C_PURPLE, 0.4f);
            Nebula(V2(w * 0.60f, h * 0.72f), (float)h * 0.38f, C_CYAN, 2.1f);
            RingedPlanet(V2(w * 0.79f, h * 0.25f), (float)h * 0.105f,
                         (Color){ 43, 50, 91, 255 }, (Color){ 93, 226, 255, 255 });
            DrawCircleLines((int)(w * 0.22f), (int)(h * 0.72f), (float)h * 0.07f, FadeColor(C_PURPLE, 0.16f));
            break;
        case 1: /* GRAVEYARD BELT — huge slow debris crossing the starfield. */
            Nebula(V2(w * 0.52f, h * 0.34f), (float)h * 0.52f, C_BLUE, 1.0f);
            DrawAsteroid(w * 0.13f, h * 0.24f, h * 0.090f, 18.0f + sinf(g.worldTime * 0.05f) * 8.0f, 1);
            DrawAsteroid(w * 0.89f, h * 0.60f, h * 0.125f, -26.0f + sinf(g.worldTime * 0.04f) * 6.0f, 2);
            DrawAsteroid(w * 0.67f, h * 0.11f, h * 0.045f, 42.0f + g.worldTime * 0.25f, 3);
            DrawAsteroid(w * 0.39f, h * 0.86f, h * 0.060f, -9.0f - g.worldTime * 0.19f, 4);
            for (int i = 0; i < 9; ++i) {
                float x = (float)((i * 137 + 51) % 1000) / 1000.0f * w;
                float y = fmodf((float)((i * 193 + 71) % 700) / 700.0f * h + g.worldTime * (3.0f + i % 4), (float)h);
                DrawCircleV(V2(x, y), 1.5f + (float)(i % 3), FadeColor(C_MUTED, 0.25f));
            }
            break;
        case 2: /* THE RIFT — nested dimensional rings around a dark void. */
            Nebula(V2(w * 0.50f, h * 0.48f), (float)h * 0.70f, C_PURPLE, 1.9f);
            Nebula(V2(w * 0.47f, h * 0.54f), (float)h * 0.46f, C_PINK, 0.7f);
            for (int i = 0; i < 7; ++i) {
                float r = (float)h * (0.13f + (float)i * 0.085f);
                float phase = g.worldTime * (i % 2 ? 2.4f : -1.7f) + (float)i * 19.0f;
                DrawCircleLines((int)(center.x + sinf(phase * 0.08f) * 13.0f),
                                (int)(center.y + cosf(phase * 0.06f) * 9.0f), r,
                                FadeColor((i % 2) ? C_CYAN : C_PURPLE, 0.08f + 0.035f * (float)(i % 3)));
            }
            DrawCircleGradient(center, (float)h * 0.115f, (Color){ 2, 3, 15, 255 }, FadeColor(C_PINK, 0.08f));
            DrawCircleLines((int)center.x, (int)center.y, (float)h * 0.115f, FadeColor(C_PINK, 0.25f));
            break;
        case 3: /* BLOOD MOON — cold red moon with craters and drifting haze. */
            Nebula(V2(w * 0.72f, h * 0.65f), (float)h * 0.62f, C_RED, 0.2f);
            Nebula(V2(w * 0.28f, h * 0.24f), (float)h * 0.35f, C_PURPLE, 2.7f);
            DrawCraterMoon(V2(w * 0.78f, h * 0.67f), (float)h * 0.22f,
                           (Color){ 67, 30, 56, 255 }, (Color){ 248, 85, 135, 255 });
            for (int i = 0; i < 8; ++i) {
                float x = (float)i * (float)w / 7.0f;
                float spike = (float)(i % 3 + 1) * h * 0.045f;
                DrawTriangle(V2(x - w * 0.045f, (float)h), V2(x + w * 0.015f, (float)h - spike),
                             V2(x + w * 0.06f, (float)h), (Color){ 10, 10, 27, 255 });
            }
            break;
        default: /* VOID CATHEDRAL — distant obsidian spires and ghost-light. */
            Nebula(V2(w * 0.50f, h * 0.50f), (float)h * 0.66f, C_BLUE, 0.8f);
            Nebula(V2(w * 0.80f, h * 0.29f), (float)h * 0.34f, C_CYAN, 2.4f);
            for (int i = 0; i < 12; ++i) {
                float x = (float)i * (float)w / 11.0f;
                float tall = (float)(2 + (i * 7) % 6) * (float)h * 0.075f;
                DrawVoidSpire(x, (float)h, (float)w * (0.035f + 0.01f * (float)(i % 3)), tall,
                              0.5f + 0.5f * sinf(g.worldTime * 0.7f + (float)i));
            }
            for (int i = 0; i < 5; ++i) {
                float y = h * (0.18f + 0.14f * (float)i) + sinf(g.worldTime * 0.22f + (float)i) * 10.0f;
                DrawLineEx(V2(w * 0.14f, y), V2(w * 0.86f, y - 16.0f), 1.0f, FadeColor(C_CYAN, 0.06f));
            }
            break;
    }
}

void DrawBackground(void)
{
    int w = SW(), h = SH();
    static const Color baseTop[5] = {
        { 16, 14, 42, 255 }, { 9, 19, 36, 255 }, { 19, 8, 38, 255 },
        { 35, 9, 28, 255 }, { 5, 15, 34, 255 }
    };
    static const Color baseBottom[5] = {
        { 4, 6, 19, 255 }, { 3, 7, 19, 255 }, { 4, 3, 17, 255 },
        { 13, 3, 19, 255 }, { 2, 4, 16, 255 }
    };
    int theme = g.backgroundTheme % 5;
    ClearBackground(baseBottom[theme]);
    DrawRectangleGradientV(0, 0, w, h, baseTop[theme], baseBottom[theme]);

    /* Landmark layers make each score band a distinct place in space. */
    DrawEnvironment(theme, w, h);

    /* Three subtle parallax star layers keep the space moving behind combat. */
    for (int i = 0; i < MAX_STARS; ++i) {
        Star *s = &g.stars[i];
        float x = fmodf(s->x + g.worldTime * s->speed * (i % 3 == 0 ? 0.11f : 0.025f), (float)w);
        float y = fmodf(s->y + g.worldTime * s->speed * (i % 3 == 0 ? 0.17f : 0.045f), (float)h);
        float pulse = 0.58f + 0.42f * sinf(g.worldTime * 1.4f + s->phase);
        Color star = (i % 11 == 0) ? C_CYAN : ((i % 13 == 0) ? C_PURPLE : C_TEXT);
        DrawCircleV(V2(x, y), s->size * pulse, FadeColor(star, 0.28f + 0.48f * pulse));
        if (i % 19 == 0) {
            DrawLineEx(V2(x - 3.0f, y), V2(x + 3.0f, y), 0.8f, FadeColor(star, 0.22f));
        }
    }

    /* Every 1,000 score: fade the new scene in through a soft dark veil. */
    if (g.backgroundTransition > 0.0f) {
        float amount = ClampF(g.backgroundTransition / 0.90f, 0.0f, 1.0f);
        float progress = 1.0f - amount;
        float sweepX = progress * (float)w;
        /* The new scene is revealed by a quiet scanning veil—not an explosion. */
        DrawRectangle(0, 0, w, h, (Color){ 2, 2, 12, (unsigned char)(72.0f * amount) });
        DrawRectangle((int)(sweepX - 42.0f), 0, 84, h,
                      (Color){ 65, 96, 190, (unsigned char)(18.0f * amount) });
        DrawLineEx(V2(sweepX, 0.0f), V2(sweepX, (float)h), 2.0f,
                   (Color){ 92, 225, 255, (unsigned char)(58.0f * amount) });
        DrawLineEx(V2(sweepX + 4.0f, 0.0f), V2(sweepX + 4.0f, (float)h), 1.0f,
                   (Color){ 179, 112, 255, (unsigned char)(34.0f * amount) });
    }
}


/* ======================================================================
 * SECTION: player.c — merged into main.c from the v0.26.0 modular source.
 * ====================================================================== */

/* Player movement, aiming, auto-fire timing and player rendering. */


void UpdatePlayer(float dt)
{
    Player *p = &g.player;
    const DifficultyInfo *d = GetDifficulty();
    Vector2 input = V2(0.0f, 0.0f);
    if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) input.x -= 1.0f;
    if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) input.x += 1.0f;
    if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)) input.y -= 1.0f;
    if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) input.y += 1.0f;

    /* Optional touch controls: drag in the left half to move; touch the right half to aim. */
    bool touchAiming = false;
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
                Vector2 aim = RRNormalizeVector2(Sub(touch, p->pos));
                if (LengthSq(aim) > 0.1f) {
                    p->facing = aim;
                    touchAiming = true;
                }
            }
        }
    }
    float inLen = Length(input);
    if (inLen > 1.0f) input = Scale(input, 1.0f / inLen);
    if (LengthSq(input) > 0.02f) {
        /* Movement sets facing unless the player is explicitly aiming by touch. */
        if (!touchAiming) p->facing = RRNormalizeVector2(input);
        p->animation += dt * 14.0f;
    }

    Vector2 mouse = GetMousePosition();
    bool mouseAiming = IsMouseButtonDown(MOUSE_BUTTON_LEFT) || IsMouseButtonDown(MOUSE_BUTTON_RIGHT);
    if (mouseAiming) {
        Vector2 mouseAim = RRNormalizeVector2(Sub(mouse, p->pos));
        if (LengthSq(mouseAim) > 0.1f) p->facing = mouseAim;
    }
    if (LengthSq(input) > 0.02f) p->velocity = Scale(input, d->playerSpeed);
    else p->velocity = Scale(p->velocity, MaxF(0.0f, 1.0f - 7.5f * dt));

    p->shieldTime = MaxF(0.0f, p->shieldTime - dt);
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

void DrawRunner(void)
{
    Player *p = &g.player;
    Vector2 f = RRNormalizeVector2(p->facing);
    Vector2 side = Perp(f);
    bool blinking = p->invulnerable > 0.0f && ((int)(g.worldTime * 22.0f) % 2 == 0);
    Color armor = (p->shieldTime > 0.0f) ? C_GOLD : C_TEXT;
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

    /* Three short wake lines make movement readable without filling the screen with particles. */
    float movement = ClampF(Length(p->velocity) / GetDifficulty()->playerSpeed, 0.0f, 1.0f);
    if (movement > 0.08f) {
        for (int i = 0; i < 3; ++i) {
            float back = 21.0f + (float)i * 5.0f;
            float sideOffset = sinf(g.worldTime * 8.0f + (float)i * 1.9f) * 2.2f;
            Vector2 a = Add(Add(p->pos, Scale(f, -back)), Scale(side, sideOffset));
            Vector2 b = Add(a, Scale(f, -(6.0f + movement * 8.0f)));
            DrawLineEx(a, b, 1.0f + movement, WithOpacity(i == 1 ? C_CYAN : C_BLUE, movement * 0.34f));
        }
    }
    if (p->shieldTime > 0.0f) {
        DrawCircleLines((int)p->pos.x, (int)p->pos.y, 27.0f + sinf(g.worldTime * 8.0f) * 2.0f, C_GOLD);
        DrawCircleLines((int)p->pos.x, (int)p->pos.y, 31.0f, WithOpacity(C_GOLD, 0.5f));
    }
    DrawCircleV(Add(p->pos, V2(0, 8)), 13.0f, (Color){ 1, 3, 15, 190 });

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


/* ======================================================================
 * SECTION: enemies.c — merged into main.c from the v0.26.0 modular source.
 * ====================================================================== */

/* Enemy spawning, AI, contact damage, fixed score awards and rendering. */


void SpawnEnemy(void)
{
    int alive = 0;
    for (int i = 0; i < MAX_ENEMIES; ++i) if (g.enemies[i].active) ++alive;
    /* Higher modes permit more simultaneous threats, but retain a hard cap. */
    int limit = 9 + g.stage + (int)g.difficulty * 3;
    int cap = (g.difficulty == DIFF_HARD) ? 48 : 42;
    if (limit > cap) limit = cap;
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
    bool blue = (rand() % 100) < 42;
    float speed = GetDifficulty()->enemySpeed * (blue ? 0.78f : 0.70f) + stageSpeed;
    float hp = blue ? 1.0f + (float)((g.stage - 1) / 6) : 2.0f + (float)((g.stage - 1) / 4);
    if (hp > 5.0f) hp = 5.0f;

    for (int i = 0; i < MAX_ENEMIES; ++i) {
        if (!g.enemies[i].active) {
            g.enemies[i] = (Enemy){ true, pos, V2(0, 0), blue ? 15.5f : 21.0f, speed, hp, hp,
                                    heading, RandF(0.55f, 1.25f), RandF(-0.9f, 0.9f), 0.0f, blue };
            return;
        }
    }
}

void KillEnemy(Enemy *e)
{
    const int oldStage = g.stage;
    e->active = false;
    g.score += 200;
    Burst(e->pos, C_RED, 26, 215.0f, 3.2f);
    PlaySfx(SFX_ENEMY_DOWN);

    g.stage = CurrentStageForScore(g.score);
    if (g.stage > oldStage) {
        /* Stage changes are announced cleanly; no burst is emitted at the player. */
        g.stageNotice = 2.0f;
        PlaySfx(SFX_SURGE);
    }
}

void UpdateEnemies(float dt)
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
        if (g.difficulty == DIFF_EASY) {
            /* EASY: pure roaming. Random steering changes keep enemies unpredictable. */
            e->heading += e->turnRate * dt * 1.35f;
        } else {
            Vector2 toPlayer = RRNormalizeVector2(Sub(g.player.pos, e->pos));
            float desired = atan2f(toPlayer.y, toPlayer.x);
            float delta = atan2f(sinf(desired - e->heading), cosf(desired - e->heading));

            if (g.difficulty == DIFF_MEDIUM) {
                /* MEDIUM: steady pursuit, without the random steering drift. */
                e->heading += ClampF(delta, -0.52f * dt, 0.52f * dt);
            } else {
                /* HARD: pursuit plus stronger random drift for mixed behavior. */
                e->heading += ClampF(delta, -0.68f * dt, 0.68f * dt);
                e->heading += e->turnRate * dt * 1.50f;
            }
        }
        float stageFactor = 1.0f + MinF((float)(g.stage - 1) * 0.028f, 0.48f);
        e->vel = Scale(V2(cosf(e->heading), sinf(e->heading)), e->speed * stageFactor);
        e->pos = Add(e->pos, Scale(e->vel, dt));

        /* Bounce off the arena edges so the red enemies keep roaming on-screen. */
        if (e->pos.x < e->radius) { e->pos.x = e->radius; e->heading = PI_F - e->heading; e->turnRate = -e->turnRate; }
        if (e->pos.x > w - e->radius) { e->pos.x = w - e->radius; e->heading = PI_F - e->heading; e->turnRate = -e->turnRate; }
        if (e->pos.y < e->radius) { e->pos.y = e->radius; e->heading = -e->heading; e->turnRate = -e->turnRate; }
        if (e->pos.y > h - e->radius) { e->pos.y = h - e->radius; e->heading = -e->heading; e->turnRate = -e->turnRate; }

        float r = e->radius + 15.0f;
        if (DistSq(e->pos, g.player.pos) < r * r && g.player.invulnerable <= 0.0f && g.player.shieldTime <= 0.0f) {
            g.player.health -= e->blue ? damage * 0.55f : damage;
            g.player.invulnerable = 0.92f;
            g.screenShake = 0.20f;
            g.flashAlpha = 0.10f;
            Vector2 knock = RRNormalizeVector2(Sub(g.player.pos, e->pos));
            g.player.pos = Add(g.player.pos, Scale(knock, 30.0f));
            e->pos = Add(e->pos, Scale(knock, -22.0f));
            Burst(g.player.pos, C_PINK, 16, 150.0f, 2.3f);
            PlaySfx(SFX_IMPACT);
            if (g.player.health <= 0.0f) { g.player.health = 0.0f; EndRun(); return; }
        }
    }
}

static Color EnemyFade(Color c, float opacity)
{
    c.a = (unsigned char)(ClampF(opacity, 0.0f, 1.0f) * (float)c.a);
    return c;
}

/* Convert an enemy-local point (forward, sideways) into arena coordinates. */
static Vector2 EnemyPoint(const Enemy *e, float forward, float sideways)
{
    float c = cosf(e->heading), s = sinf(e->heading);
    return V2(e->pos.x + forward * c - sideways * s,
              e->pos.y + forward * s + sideways * c);
}

/* Tendrils are drawn as bent, tapered-looking segments so the silhouette feels
   organic instead of like a glowing ball. The movement is purely cosmetic. */
static void DrawTendril(const Enemy *e, int index, float side, float length, Color shade, Color glint)
{
    float r = e->radius;
    float sway = sinf(g.worldTime * (e->blue ? 4.7f : 3.1f) + (float)index * 1.4f + e->pos.x * 0.01f);
    float rootX = -r * (0.18f + 0.16f * (float)(index % 3));
    float rootY = side * r * (0.24f + 0.11f * (float)(index % 2));
    float reachX = rootX - length * (0.55f + 0.08f * (float)(index % 2));
    float reachY = side * (r * (0.95f + 0.19f * (float)(index % 3)) + sway * r * 0.18f);
    Vector2 a = EnemyPoint(e, rootX, rootY);
    Vector2 b = EnemyPoint(e, rootX - length * 0.22f, rootY + side * length * 0.50f + sway * r * 0.10f);
    Vector2 c = EnemyPoint(e, reachX, reachY);
    DrawLineEx(a, b, e->blue ? 3.1f : 4.6f, shade);
    DrawLineEx(b, c, e->blue ? 1.8f : 2.7f, shade);
    DrawLineEx(b, c, 0.8f, EnemyFade(glint, 0.72f));
    DrawCircleV(c, e->blue ? 1.7f : 2.2f, glint);
}

static void DrawAlienEye(Vector2 p, float radius, Color glow, Color pupil)
{
    DrawCircleGradient(p, radius * 3.8f, EnemyFade(glow, 0.40f), EnemyFade(glow, 0.0f));
    DrawCircleV(p, radius, glow);
    DrawCircleV(V2(p.x + radius * 0.18f, p.y), radius * 0.48f, pupil);
    DrawCircleV(V2(p.x + radius * 0.25f, p.y - radius * 0.10f), radius * 0.16f, C_TEXT);
}

static void DrawRedWraith(const Enemy *e, float r, Color body, Color glow)
{
    /* Six animated hooked tendrils hang behind a many-pointed armored shell. */
    for (int i = 0; i < 3; ++i) {
        DrawTendril(e, i, -1.0f, r * (0.88f + 0.14f * (float)i), (Color){ 34, 11, 34, 255 }, glow);
        DrawTendril(e, i + 3, 1.0f, r * (0.88f + 0.14f * (float)i), (Color){ 34, 11, 34, 255 }, glow);
    }

    /* Bone-like dorsal spikes and side blades frame the silhouette. */
    for (int sideIndex = -1; sideIndex <= 1; sideIndex += 2) {
        DrawTriangle(EnemyPoint(e, -r * 0.23f, sideIndex * r * 0.48f),
                     EnemyPoint(e, r * 0.14f, sideIndex * r * 1.03f),
                     EnemyPoint(e, r * 0.33f, sideIndex * r * 0.43f),
                     (Color){ 67, 18, 43, 255 });
        DrawTriangle(EnemyPoint(e, -r * 0.58f, sideIndex * r * 0.34f),
                     EnemyPoint(e, -r * 0.90f, sideIndex * r * 0.78f),
                     EnemyPoint(e, -r * 0.16f, sideIndex * r * 0.58f),
                     (Color){ 42, 14, 35, 255 });
        DrawLineEx(EnemyPoint(e, -r * 0.58f, sideIndex * r * 0.34f),
                   EnemyPoint(e, -r * 0.90f, sideIndex * r * 0.78f), 1.2f,
                   EnemyFade(glow, 0.42f));
    }

    /* Split carapace: layered pointed plates, never a circular body. */
    Vector2 tip = EnemyPoint(e, r * 1.05f, 0.0f);
    Vector2 crown = EnemyPoint(e, r * 0.20f, -r * 0.58f);
    Vector2 shoulder = EnemyPoint(e, -r * 0.55f, -r * 0.43f);
    Vector2 tail = EnemyPoint(e, -r * 0.82f, 0.0f);
    Vector2 lower = EnemyPoint(e, -r * 0.55f, r * 0.43f);
    Vector2 jaw = EnemyPoint(e, r * 0.20f, r * 0.58f);
    Vector2 core = EnemyPoint(e, -r * 0.13f, 0.0f);
    DrawTriangle(tip, crown, core, body);
    DrawTriangle(tip, core, jaw, EnemyFade(body, 0.88f));
    DrawTriangle(crown, shoulder, core, (Color){ 60, 17, 42, 255 });
    DrawTriangle(jaw, core, lower, (Color){ 78, 18, 39, 255 });
    DrawTriangle(shoulder, tail, core, (Color){ 25, 12, 31, 255 });
    DrawTriangle(lower, core, tail, (Color){ 25, 12, 31, 255 });

    /* Sharp mandibles and a vertical, furnace-like central slit. */
    DrawTriangle(EnemyPoint(e, r * 0.66f, -r * 0.08f),
                 EnemyPoint(e, r * 1.00f, -r * 0.27f),
                 EnemyPoint(e, r * 0.79f, r * 0.02f), (Color){ 19, 8, 24, 255 });
    DrawTriangle(EnemyPoint(e, r * 0.66f, r * 0.08f),
                 EnemyPoint(e, r * 1.00f, r * 0.27f),
                 EnemyPoint(e, r * 0.79f, -r * 0.02f), (Color){ 19, 8, 24, 255 });
    DrawLineEx(EnemyPoint(e, r * 0.25f, 0.0f), EnemyPoint(e, r * 0.78f, 0.0f), r * 0.105f,
               EnemyFade(glow, 0.90f));
    DrawLineEx(EnemyPoint(e, r * 0.31f, 0.0f), EnemyPoint(e, r * 0.68f, 0.0f), 1.0f,
               (Color){ 255, 245, 222, 255 });

    DrawAlienEye(EnemyPoint(e, r * 0.23f, -r * 0.23f), r * 0.105f,
                 (Color){ 255, 83, 104, 255 }, (Color){ 30, 3, 15, 255 });
    DrawAlienEye(EnemyPoint(e, r * 0.23f, r * 0.23f), r * 0.105f,
                 (Color){ 255, 83, 104, 255 }, (Color){ 30, 3, 15, 255 });

    /* Etched seams catch light as the creature pulses. */
    DrawLineEx(crown, core, 1.0f, EnemyFade(glow, 0.56f));
    DrawLineEx(jaw, core, 1.0f, EnemyFade(glow, 0.38f));
    DrawLineEx(shoulder, tail, 1.1f, EnemyFade((Color){ 255, 159, 177, 255 }, 0.34f));
    DrawLineEx(lower, tail, 1.1f, EnemyFade((Color){ 255, 159, 177, 255 }, 0.34f));
}

static void DrawBlueSkitter(const Enemy *e, float r, Color body, Color glow)
{
    /* Smaller scout: four blade-like limbs, swept wings and a triple eye cluster. */
    for (int i = 0; i < 2; ++i) {
        DrawTendril(e, i, -1.0f, r * 0.92f, (Color){ 12, 26, 57, 255 }, glow);
        DrawTendril(e, i + 2, 1.0f, r * 0.92f, (Color){ 12, 26, 57, 255 }, glow);
    }
    for (int sideIndex = -1; sideIndex <= 1; sideIndex += 2) {
        DrawTriangle(EnemyPoint(e, r * 0.24f, sideIndex * r * 0.34f),
                     EnemyPoint(e, -r * 0.10f, sideIndex * r * 1.10f),
                     EnemyPoint(e, -r * 0.31f, sideIndex * r * 0.31f),
                     (Color){ 19, 47, 91, 255 });
        DrawLineEx(EnemyPoint(e, r * 0.18f, sideIndex * r * 0.40f),
                   EnemyPoint(e, -r * 0.11f, sideIndex * r * 1.03f), 1.2f, EnemyFade(glow, 0.76f));
    }

    Vector2 tip = EnemyPoint(e, r * 1.15f, 0.0f);
    Vector2 top = EnemyPoint(e, r * 0.08f, -r * 0.46f);
    Vector2 rear = EnemyPoint(e, -r * 0.85f, 0.0f);
    Vector2 bottom = EnemyPoint(e, r * 0.08f, r * 0.46f);
    Vector2 core = EnemyPoint(e, -r * 0.20f, 0.0f);
    DrawTriangle(tip, top, core, body);
    DrawTriangle(tip, core, bottom, EnemyFade(body, 0.88f));
    DrawTriangle(top, rear, core, (Color){ 11, 27, 59, 255 });
    DrawTriangle(bottom, core, rear, (Color){ 11, 27, 59, 255 });
    DrawTriangleLines(tip, top, bottom, EnemyFade(glow, 0.85f));
    DrawTriangleLines(top, rear, bottom, EnemyFade(glow, 0.48f));

    DrawLineEx(EnemyPoint(e, r * 0.29f, -r * 0.12f), EnemyPoint(e, r * 0.87f, 0.0f), r * 0.10f, glow);
    DrawAlienEye(EnemyPoint(e, r * 0.30f, 0.0f), r * 0.11f,
                 (Color){ 83, 236, 255, 255 }, (Color){ 2, 19, 42, 255 });
    DrawCircleV(EnemyPoint(e, r * 0.06f, -r * 0.20f), r * 0.055f, C_TEXT);
    DrawCircleV(EnemyPoint(e, r * 0.06f, r * 0.20f), r * 0.055f, C_TEXT);
}

void DrawEnemy(const Enemy *e)
{
    Color base = e->blue ? (Color){ 34, 113, 175, 255 } : (Color){ 123, 27, 48, 255 };
    Color glow = e->blue ? C_CYAN : (Color){ 255, 62, 103, 255 };
    float r = e->radius;
    float pulse = 1.0f + 0.045f * sinf(g.worldTime * (e->blue ? 5.6f : 3.2f) + e->pos.x * 0.02f);
    Color body = e->flash > 0.0f ? C_GOLD : base;

    /* Keep the aura soft and narrow; the body silhouette does the visual work. */
    DrawCircleGradient(e->pos, r * (e->blue ? 2.0f : 1.85f), EnemyFade(glow, 0.25f), EnemyFade(glow, 0.0f));
    if (e->blue) DrawBlueSkitter(e, r * pulse, body, glow);
    else DrawRedWraith(e, r * pulse, body, glow);

    /* Enemy health is shown only when damaged, keeping the arena uncluttered. */
    if (e->hp < e->maxHp) {
        float barW = r * 2.2f;
        DrawRectangleRounded((Rectangle){ e->pos.x - barW * 0.5f, e->pos.y - r - 9.0f, barW, 4.0f }, 0.5f, 4, (Color){ 20, 25, 46, 230 });
        DrawRectangleRounded((Rectangle){ e->pos.x - barW * 0.5f, e->pos.y - r - 9.0f, barW * ClampF(e->hp / e->maxHp, 0, 1), 4.0f }, 0.5f, 4, C_GOLD);
    }
}


/* ======================================================================
 * SECTION: bullets.c — merged into main.c from the v0.26.0 modular source.
 * ====================================================================== */

void AddBullet(Vector2 pos, Vector2 dir)
{
    dir = RRNormalizeVector2(dir);
    if (LengthSq(dir) < 0.5f) dir = g.player.facing;
    for (int i = 0; i < MAX_BULLETS; ++i) {
        if (!g.bullets[i].active) {
            float stageBoost = MinF((float)(g.stage - 1) * 5.0f, 90.0f);
            float spd = 620.0f + stageBoost;
            g.bullets[i] = (Bullet){ true, Add(pos, Scale(dir, 16.0f)), Scale(dir, spd), 4.0f, 1.32f, C_CYAN };
            PlaySfx(SFX_SHOT);
            return;
        }
    }
}

void UpdateBullets(float dt)
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


/* ======================================================================
 * SECTION: pickups.c — merged into main.c from the v0.26.0 modular source.
 * ====================================================================== */

/* Banana-only pickup behavior: a little health plus temporary protection. */


void AddPickup(Vector2 pos)
{
    for (int i = 0; i < MAX_PICKUPS; ++i) {
        if (!g.pickups[i].active) {
            g.pickups[i] = (Pickup){ true, pos, RandF(0.0f, 6.28f), 8.0f };
            return;
        }
    }
}

void UpdatePickups(float dt)
{
    for (int i = 0; i < MAX_PICKUPS; ++i) {
        Pickup *p = &g.pickups[i];
        if (!p->active) continue;

        p->life -= dt;
        p->phase += dt * 3.0f;
        if (p->life <= 0.0f) {
            p->active = false;
            continue;
        }

        const float collectRadius = 18.0f;
        if (DistSq(p->pos, g.player.pos) <= collectRadius * collectRadius) {
            p->active = false;
            g.player.health = MinF(g.player.maxHealth, g.player.health + 20.0f);
            g.player.shieldTime = 8.0f;
            Burst(p->pos, C_GOLD, 14, 120.0f, 2.1f);
            PlaySfx(SFX_PICKUP);
        }
    }
}


/* ======================================================================
 * SECTION: effects.c — merged into main.c from the v0.26.0 modular source.
 * ====================================================================== */

/* Particle bursts and small screen feedback effects. */


void AddParticle(Vector2 pos, Vector2 vel, float radius, float life, Color color)
{
    for (int i = 0; i < MAX_PARTICLES; ++i) {
        if (!g.particles[i].active) {
            g.particles[i] = (Particle){ true, pos, vel, radius, life, life, color };
            return;
        }
    }
}

void Burst(Vector2 pos, Color color, int count, float speed, float radius)
{
    for (int i = 0; i < count; ++i) {
        float angle = RandF(0.0f, 2.0f * PI_F);
        float spd = RandF(speed * 0.35f, speed);
        AddParticle(pos, V2(cosf(angle) * spd, sinf(angle) * spd),
                    RandF(radius * 0.55f, radius), RandF(0.24f, 0.72f), color);
    }
}

void UpdateEffects(float dt)
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
    g.screenShake = MaxF(0.0f, g.screenShake - dt);
    g.flashAlpha = MaxF(0.0f, g.flashAlpha - 1.75f * dt);
    g.stageNotice = MaxF(0.0f, g.stageNotice - dt);
    g.backgroundTransition = MaxF(0.0f, g.backgroundTransition - dt);
}


/* ======================================================================
 * SECTION: world.c — merged into main.c from the v0.26.0 modular source.
 * ====================================================================== */

/* Composition of pickups, bullets, enemies, effects and the player. */


static Vector2 RotateLocal(Vector2 center, Vector2 local, float angle)
{
    float c = cosf(angle), s = sinf(angle);
    return V2(center.x + local.x * c - local.y * s,
              center.y + local.x * s + local.y * c);
}

/* Draw a recognizable golden crescent, not the old generic diamond icon. */
static void DrawBananaPickup(const Pickup *p, float r)
{
    const int segments = 12;
    float angle = -0.48f + 0.10f * sinf(p->phase * 0.5f);
    Vector2 outer[segments + 1], inner[segments + 1];
    for (int i = 0; i <= segments; ++i) {
        float t = (float)i / (float)segments;
        float x = (t - 0.5f) * 2.0f * r;
        float curve = sinf(PI_F * t);
        float yOuter = -0.10f * r - 0.52f * r * curve;
        float yInner = yOuter + 0.34f * r * curve;
        outer[i] = RotateLocal(p->pos, V2(x, yOuter), angle);
        inner[i] = RotateLocal(p->pos, V2(x, yInner), angle);
    }
    DrawCircleGradient(p->pos, r * 2.5f, WithOpacity(C_GOLD, 0.32f), WithOpacity(C_GOLD, 0.0f));
    for (int i = 0; i < segments; ++i) {
        DrawTriangle(outer[i], outer[i + 1], inner[i + 1], (Color){ 255, 191, 54, 255 });
        DrawTriangle(outer[i], inner[i + 1], inner[i], (Color){ 255, 226, 104, 255 });
        DrawLineEx(outer[i], outer[i + 1], 1.7f, (Color){ 255, 246, 173, 255 });
        DrawLineEx(inner[i], inner[i + 1], 1.0f, (Color){ 204, 130, 28, 255 });
    }
    DrawCircleV(outer[0], 2.0f, (Color){ 255, 238, 143, 255 });
    DrawCircleV(outer[segments], 2.0f, (Color){ 255, 238, 143, 255 });
}

static void DrawPickup(const Pickup *p)
{
    float pulse = 1.0f + sinf(p->phase * 1.6f) * 0.08f;
    DrawBananaPickup(p, 12.0f * pulse);
}

void DrawWorld(void)
{
    for (int i = 0; i < MAX_PICKUPS; ++i) {
        const Pickup *p = &g.pickups[i];
        if (p->active) DrawPickup(p);
    }

    for (int i = 0; i < MAX_BULLETS; ++i) {
        Bullet *b = &g.bullets[i];
        if (!b->active) continue;
        Vector2 tail = Sub(b->pos, Scale(RRNormalizeVector2(b->vel), 16.0f));
        DrawLineEx(tail, b->pos, 3.0f, WithOpacity(b->color, 0.45f));
        DrawCircleGradient(b->pos, 10.0f, WithOpacity(b->color, 0.56f),
                           (Color){ b->color.r, b->color.g, b->color.b, 0 });
        DrawCircleV(b->pos, b->radius, C_CYAN);
    }

    for (int i = 0; i < MAX_ENEMIES; ++i) {
        if (g.enemies[i].active) DrawEnemy(&g.enemies[i]);
    }

    for (int i = 0; i < MAX_PARTICLES; ++i) {
        Particle *p = &g.particles[i];
        if (!p->active) continue;
        float a = ClampF(p->life / p->fullLife, 0.0f, 1.0f);
        DrawCircleV(p->pos, p->radius * (0.36f + 0.64f * a), WithOpacity(p->color, a));
    }

    if (g.state == STATE_PLAYING || g.state == STATE_PAUSED || g.state == STATE_GAME_OVER) DrawRunner();
}


/* ======================================================================
 * SECTION: audio.c — merged into main.c from the v0.26.0 modular source.
 * ====================================================================== */

/* Original one-shot game sound effects. No music or ambience loop. */

void FirstAudioGesture(void)
{
    if (g.firstAudioGesture) return;

    /* Initialize only after a user gesture so browser audio is not autoplay-blocked. */
    if (!g.audioReady) {
        InitAudioDevice();
        g.audioReady = IsAudioDeviceReady();
        if (g.audioReady) {
            g.sounds[SFX_SHOT] = LoadSound("assets/shot.wav");
            g.sounds[SFX_IMPACT] = LoadSound("assets/impact.wav");
            g.sounds[SFX_PICKUP] = LoadSound("assets/pickup.wav");
            g.sounds[SFX_ENEMY_DOWN] = LoadSound("assets/enemy_down.wav");
            g.sounds[SFX_SURGE] = LoadSound("assets/surge.wav");
        }
    }
    g.firstAudioGesture = true;
}

void PlaySfx(SoundId id)
{
    if (g.audioReady && id >= 0 && id < SFX_COUNT && IsSoundValid(g.sounds[id])) {
        PlaySound(g.sounds[id]);
    }
}

void ShutdownAudio(void)
{
#if !defined(__EMSCRIPTEN__)
    if (g.audioReady) {
        for (int i = 0; i < SFX_COUNT; ++i) {
            if (IsSoundValid(g.sounds[i])) UnloadSound(g.sounds[i]);
        }
        CloseAudioDevice();
    }
#endif
}


/* ======================================================================
 * SECTION: leaderboard.c — merged into main.c from the v0.26.0 modular source.
 * ====================================================================== */

/* Three personal-best scores, one per difficulty. */

#if defined(__EMSCRIPTEN__)
#define RR_SCORE_FILE "/persist/rift_runner_bests.txt"
#else
#define RR_SCORE_FILE "rift_runner_bests.txt"
#endif

void SaveBestScores(void)
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

void LoadBestScores(void)
{
    FILE *f = fopen(RR_SCORE_FILE, "r");
    if (!f) return;
    if (fscanf(f, "%d %d %d", &g.bestScores[0], &g.bestScores[1], &g.bestScores[2]) != 3)
        for (int i = 0; i < 3; ++i) g.bestScores[i] = -1;
    for (int i = 0; i < 3; ++i) if (g.bestScores[i] < -1) g.bestScores[i] = -1;
    fclose(f);
}

void RecordPersonalBest(void)
{
    int mode = (int)g.difficulty;
    g.newPersonalBest = g.bestScores[mode] < 0 || g.score > g.bestScores[mode];
    if (g.newPersonalBest) {
        g.bestScores[mode] = g.score;
        SaveBestScores();
    }
}


/* ======================================================================
 * SECTION: game.c — merged into main.c from the v0.26.0 modular source.
 * ====================================================================== */

/* Game lifecycle: reset a run, start it, and finish it. */

const DifficultyInfo DIFFICULTIES[3] = {
    { "EASY",   "Learn the arena",       { 98, 255, 178, 255 }, 292.0f, 0.245f, 1.02f, 72.0f, 13.0f, 130.0f },
    { "MEDIUM", "Stay moving",           { 76, 231, 255, 255 }, 276.0f, 0.225f, 0.78f, 95.0f, 25.0f, 100.0f },
    { "HARD",   "Overwhelming pursuit", { 255, 83, 170, 255 }, 266.0f, 0.205f, 0.58f, 120.0f, 34.0f, 85.0f }
};

void ResetWorld(void)
{
    memset(g.bullets, 0, sizeof(g.bullets));
    memset(g.enemies, 0, sizeof(g.enemies));
    memset(g.particles, 0, sizeof(g.particles));
    memset(g.pickups, 0, sizeof(g.pickups));
    memset(&g.player, 0, sizeof(g.player));
    g.score = 0;
    g.stage = 1;
    g.spawnClock = 1.0f;
    g.screenShake = 0.0f;
    g.flashAlpha = 0.0f;
    g.stageNotice = 0.0f;
    g.bananaClock = RandF(12.0f, 19.0f);
    g.backgroundTransition = 0.0f;
    g.backgroundTheme = 0;
    g.player.pos = V2((float)SW() * 0.5f, (float)SH() * 0.5f);
    g.player.facing = V2(1.0f, 0.0f);
    g.player.health = GetDifficulty()->startingHealth;
    g.player.maxHealth = GetDifficulty()->startingHealth;
    g.player.fireClock = 0.12f;
    g.player.invulnerable = 0.65f;
    g.player.animation = 0.0f;
    g.newPersonalBest = false;
}

void StartRun(void)
{
    FirstAudioGesture();
    ResetWorld();
    g.state = STATE_PLAYING;
    Burst(g.player.pos, C_CYAN, 34, 190.0f, 3.3f);
    PlaySfx(SFX_SURGE);
}

void EndRun(void)
{
    if (g.state != STATE_PLAYING) return;
    g.state = STATE_GAME_OVER;
    RecordPersonalBest();
    g.screenShake = 0.42f;
    g.flashAlpha = 0.32f;
    Burst(g.player.pos, C_PINK, 55, 260.0f, 4.0f);
    PlaySfx(SFX_IMPACT);
}

void SetDifficulty(int index)
{
    if (index < 0) index = 2;
    if (index > 2) index = 0;
    g.menuSelection = index;
    g.difficulty = (Difficulty)index;
}

void InitGame(void)
{
    memset(&g, 0, sizeof(g));
    g.state = STATE_TITLE;
    g.scoresReturnState = STATE_TITLE;
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


/* ======================================================================
 * SECTION: ui.c — merged into main.c from the v0.26.0 modular source.
 * ====================================================================== */

/* Menus, HUD, screen rendering and menu input. */


/* Poll once per frame so touch taps work for menus as reliably as mouse clicks. */
static bool uiPointerPressed = false;
static bool uiTouchWasDown = false;
static Vector2 uiPointerPosition = { 0.0f, 0.0f };

void PollUIPointer(void)
{
    int touchCount = GetTouchPointCount();
    bool touchDown = touchCount > 0;
    bool mousePressed = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
    uiPointerPressed = mousePressed || (touchDown && !uiTouchWasDown);
    if (touchDown && !mousePressed) uiPointerPosition = GetTouchPosition(0);
    else uiPointerPosition = GetMousePosition();
    uiTouchWasDown = touchDown;
}

bool GetUIPointerPress(Vector2 *position)
{
    if (!uiPointerPressed) return false;
    if (position) *position = uiPointerPosition;
    uiPointerPressed = false;
    return true;
}

void DrawPanel(Rectangle r, Color accent)
{
    DrawRectangleRounded(r, 0.12f, 12, C_PANEL);
    /* A restrained top-edge glint gives the HUD a layered glass-panel finish. */
    DrawLineEx(V2(r.x + 12.0f, r.y + 1.0f),
               V2(r.x + r.width - 12.0f, r.y + 1.0f), 1.0f,
               WithOpacity(C_TEXT, 0.055f));
    DrawRectangleRoundedLinesEx(r, 0.12f, 12, 1.0f, WithOpacity(accent, 0.48f));
}

static Color GetHealthColor(void)
{
    float ratio = (g.player.maxHealth > 0.0f) ? g.player.health / g.player.maxHealth : 0.0f;
    if (ratio > 0.50f) return C_GREEN;
    if (ratio > 0.25f) return C_GOLD;
    return C_RED;
}

static void DrawSectorProgress(float x, float y, float width, float height)
{
    float progress = (float)(g.score % 1000) / 1000.0f;
    DrawRectangleRounded((Rectangle){ x, y, width, height }, 0.5f, 6,
                         (Color){ 35, 45, 75, 235 });
    if (progress > 0.0f) {
        DrawRectangleRounded((Rectangle){ x, y, width * progress, height }, 0.5f, 6, C_CYAN);
    }
}

void DrawHUD(void)
{
    char buf[112];
    const int w = SW(), h = SH();
    const DifficultyInfo *d = GetDifficulty();
    const bool compactHud = w < 900 || h < 560;
    const float healthRatio = (g.player.maxHealth > 0.0f)
        ? ClampF(g.player.health / g.player.maxHealth, 0.0f, 1.0f) : 0.0f;
    const Color healthColor = GetHealthColor();
    const float environmentProgress = (float)(g.score % 1000) / 1000.0f;

    if (compactHud) {
        float sideW = MinF(180.0f, (float)w * 0.40f);
        float healthW = MinF(170.0f, (float)w * 0.39f);
        float healthX = (float)w - healthW - 8.0f;
        DrawPanel((Rectangle){ 8, 8, sideW, 66 }, C_CYAN);
        DrawText("SCORE", 18, 16, 9, C_MUTED);
        snprintf(buf, sizeof(buf), "%d", g.score);
        DrawText(buf, 16, 29, 23, C_TEXT);
        snprintf(buf, sizeof(buf), "RIFT LEVEL %02d", g.stage);
        DrawText(buf, 18, 56, 9, C_CYAN);

        DrawPanel((Rectangle){ healthX, 8, healthW, 66 }, d->color);
        DrawText(d->name, (int)healthX + 10, 16, 9, d->color);
        DrawText("INTEGRITY", (int)healthX + 10, 32, 9, C_MUTED);
        DrawRectangleRounded((Rectangle){ healthX + 10, 48, healthW - 20, 8 }, 0.5f, 6,
                             (Color){ 38, 47, 74, 255 });
        if (healthRatio > 0.0f) {
            DrawRectangleRounded((Rectangle){ healthX + 10, 48, (healthW - 20) * healthRatio, 8 },
                                 0.5f, 6, healthColor);
        }

        /* Full-width micro progress bar stays clear of the mobile touch zones. */
        DrawSectorProgress(8.0f, 80.0f, (float)w - 16.0f, 3.0f);
        snprintf(buf, sizeof(buf), "SECTOR %02d  //  %s", g.backgroundTheme + 1, GetSectorName());
        DrawText(buf, 10, 87, 9, WithOpacity(C_TEXT, 0.78f));
        snprintf(buf, sizeof(buf), "%d%%", (int)(environmentProgress * 100.0f));
        DrawRight(buf, w - 10, 87, 9, C_CYAN);
    } else {
        /* Keep the score panel focused on the only tracked run metric. */
        DrawPanel((Rectangle){ 18, 16, 248, 90 }, C_CYAN);
        DrawText("SCORE", 34, 27, 12, C_MUTED);
        snprintf(buf, sizeof(buf), "%07d", g.score);
        DrawText(buf, 31, 43, 31, C_TEXT);
        DrawText("SURVIVE THE RIFT", 34, 80, 10, C_CYAN);

        /* Center sector card: level, named environment and progress to next shift. */
        float sectorX = (float)w * 0.5f - 144.0f;
        DrawPanel((Rectangle){ sectorX, 16, 288, 90 }, C_PURPLE);
        snprintf(buf, sizeof(buf), "RIFT LEVEL  %02d", g.stage);
        DrawCentered(buf, 25, 15, C_TEXT);
        snprintf(buf, sizeof(buf), "SECTOR %02d  //  %s", g.backgroundTheme + 1, GetSectorName());
        DrawCentered(buf, 47, 10, C_CYAN);
        DrawSectorProgress(sectorX + 24.0f, 70.0f, 240.0f, 6.0f);
        snprintf(buf, sizeof(buf), "%d%% TO NEXT ENVIRONMENT", (int)(environmentProgress * 100.0f));
        DrawCentered(buf, 82, 9, C_MUTED);

        /* Integrity, with explicit percentage and color changing at danger thresholds. */
        DrawPanel((Rectangle){ (float)w - 242.0f, 16, 224, 90 }, d->color);
        DrawText(d->name, w - 224, 27, 12, d->color);
        DrawText("INTEGRITY", w - 224, 47, 10, C_MUTED);
        DrawRectangleRounded((Rectangle){ (float)w - 224, 63, 188, 10 }, 0.5f, 6,
                             (Color){ 38, 47, 74, 255 });
        if (healthRatio > 0.0f) {
            DrawRectangleRounded((Rectangle){ (float)w - 224, 63, 188.0f * healthRatio, 10 },
                                 0.5f, 6, healthColor);
        }
        snprintf(buf, sizeof(buf), "%d%%", (int)(healthRatio * 100.0f));
        DrawRight(buf, w - 36, 79, 11, C_TEXT);
    }

    /* Context-sensitive event notices: sector shifts take priority over level-ups. */
    if (g.backgroundTransition > 0.02f) {
        float alpha = ClampF(g.backgroundTransition / 0.55f, 0.0f, 1.0f);
        snprintf(buf, sizeof(buf), "SECTOR SHIFT  //  %s", GetSectorName());
        DrawCentered(buf, (int)((compactHud ? 0.245f : 0.205f) * (float)h), compactHud ? 13 : 18,
                     WithOpacity(C_CYAN, alpha));
    } else if (g.stageNotice > 0.0f) {
        float alpha = ClampF(g.stageNotice * 1.7f, 0.0f, 1.0f);
        DrawCentered("RIFT LEVEL INCREASED  //  PRESSURE RISING",
                     (int)((compactHud ? 0.245f : 0.205f) * (float)h), compactHud ? 11 : 15,
                     WithOpacity(C_GOLD, alpha));
    }

    if (g.player.shieldTime > 0.0f) {
        snprintf(buf, sizeof(buf), "SHIELD  %.1fs", g.player.shieldTime);
        DrawText(buf, 24, h - 30, 13, C_GOLD);
    }
}

void DrawButton(Rectangle r, const char *label, Color accent, bool selected)
{
    Vector2 mouse = GetMousePosition();
    bool hover = HitRect(mouse, r);
    Color fill = selected || hover ? WithOpacity(accent, 0.16f) : C_PANEL;
    Color border = selected || hover ? accent : (Color){ 67, 82, 126, 220 };
    DrawRectangleRounded(r, 0.16f, 10, fill);
    DrawRectangleRoundedLinesEx(r, 0.16f, 10, selected || hover ? 2.0f : 1.0f, border);
    int fontSize = 18;
    while (fontSize > 10 && MeasureText(label, fontSize) > (int)r.width - 22) --fontSize;
    DrawText(label, (int)(r.x + (r.width - (float)MeasureText(label, fontSize)) * 0.5f),
             (int)(r.y + (r.height - (float)fontSize) * 0.5f - 1.0f), fontSize,
             selected || hover ? C_TEXT : C_MUTED);
}

Rectangle DifficultyCard(int i)
{
    float gap = (SW() < 600) ? 6.0f : 16.0f;
    float cardW = MinF(218.0f, ((float)SW() - 40.0f - gap * 2.0f) / 3.0f);
    float total = cardW * 3.0f + gap * 2.0f;
    float start = ((float)SW() - total) * 0.5f;
    float h = (float)SH();
    float cardH = h < 400.0f ? MinF(84.0f, h * 0.20f) :
                  (h < 590.0f ? MinF(104.0f, h * 0.22f) : 104.0f);
    float y = h < 400.0f ? h * 0.34f : (h < 590.0f ? h * 0.36f : h * 0.455f);
    return (Rectangle){ start + i * (cardW + gap), y, cardW, cardH };
}

static void OpenScores(GameState returnState)
{
    g.scoresReturnState = returnState;
    g.state = STATE_SCORES;
}

static Rectangle PauseScoresBounds(void)
{
    float h = (float)SH();
    float y = h < 400.0f ? h * 0.62f : (h < 560.0f ? h * 0.655f : h * 0.64f);
    return (Rectangle){ (float)SW() * 0.5f - 110.0f, y, 220.0f, 20.0f };
}

static Rectangle StartRunBounds(void)
{
    float w = MinF(300.0f, (float)SW() - 36.0f);
    float h = (float)SH();
    bool shortView = h < 590.0f;
    float buttonY = h * (shortView ? (h < 400.0f ? 0.57f : 0.585f) : 0.635f);
    float buttonH = shortView ? 45.0f : 57.0f;
    return (Rectangle){ (float)SW() * 0.5f - w * 0.5f, buttonY, w, buttonH };
}

static Rectangle ScoresLinkBounds(void)
{
    float h = (float)SH();
    float y;
    if (h < 590.0f) {
        Rectangle start = StartRunBounds();
        y = start.y + start.height + 2.0f;
    } else {
        y = h * 0.715f;
    }
    return (Rectangle){ (float)SW() * 0.5f - 110.0f, y, 220.0f, 20.0f };
}

static Rectangle ScoresBackBounds(void)
{
    float w = (float)SW(), h = (float)SH();
    float y = h < 400.0f ? h * 0.75f : h * 0.80f;
    return (Rectangle){ w * 0.5f - MinF(220.0f, w - 36.0f) * 0.5f, y,
                        MinF(220.0f, w - 36.0f), 42.0f };
}

static Rectangle PauseResumeBounds(void)
{
    float w = MinF(300.0f, (float)SW() - 36.0f);
    float h = (float)SH();
    float buttonH = h < 400.0f ? 34.0f : (h < 560.0f ? 42.0f : 52.0f);
    float y = h < 400.0f ? h * 0.405f : (h < 560.0f ? h * 0.47f : h * 0.48f);
    return (Rectangle){ (float)SW() * 0.5f - w * 0.5f, y, w, buttonH };
}

static Rectangle PauseTitleBounds(void)
{
    float w = MinF(300.0f, (float)SW() - 36.0f);
    float h = (float)SH();
    float y = h < 400.0f ? h * 0.52f : h * 0.57f;
    float height = h < 400.0f ? 32.0f : (h < 560.0f ? 36.0f : 42.0f);
    return (Rectangle){ (float)SW() * 0.5f - w * 0.5f, y, w, height };
}

static Rectangle GameOverScoresBounds(Rectangle panel)
{
    return (Rectangle){ panel.x + 24.0f, panel.y + panel.height * 0.80f, panel.width - 48.0f, 20.0f };
}

static Rectangle GameOverTitleBounds(Rectangle panel)
{
    return (Rectangle){ panel.x + 40.0f, panel.y + panel.height * 0.90f, panel.width - 80.0f, 20.0f };
}

void DrawTitle(void)
{
    const int h = SH(), w = SW();
    const bool shortView = h < 590;
    const int titleSize = (w < 440) ? 32 : ((w < 720) ? 40 : 57);
    const int eyebrowY = (int)(h * (shortView ? 0.055f : 0.105f));
    const int titleY = (int)(h * (shortView ? 0.095f : 0.165f));
    const int subtitleY = (int)(h * (shortView ? 0.205f : 0.265f));
    const int chooseY = (int)(h * (h < 400 ? 0.285f : (shortView ? 0.305f : 0.395f)));

    /* Small framing rules help the title feel like a designed interface, not plain text. */
    DrawLineEx(V2((float)w * 0.18f, (float)eyebrowY + 8.0f),
               V2((float)w * 0.32f, (float)eyebrowY + 8.0f), 1.0f,
               WithOpacity(C_CYAN, 0.50f));
    DrawLineEx(V2((float)w * 0.68f, (float)eyebrowY + 8.0f),
               V2((float)w * 0.82f, (float)eyebrowY + 8.0f), 1.0f,
               WithOpacity(C_CYAN, 0.50f));
    DrawCentered("RIFT NETWORK  //  FLIGHT SYSTEMS ONLINE", eyebrowY, 11, C_CYAN);
    DrawCentered("RIFT RUNNER", titleY + 3, titleSize, (Color){ 65, 97, 150, 110 });
    DrawCentered("RIFT RUNNER", titleY, titleSize, C_TEXT);
    if (w < 440) {
        DrawCentered("SIGNAL ZERO  •  SURVIVE THE VOID", subtitleY, 10, C_MUTED);
    } else {
        DrawCentered("SIGNAL ZERO  •  SURVIVE THE SIGNAL. OUTLAST THE VOID.", subtitleY, 12, C_MUTED);
    }
    DrawCentered("SELECT YOUR PRESSURE LEVEL", chooseY, 12, C_CYAN);

    for (int i = 0; i < 3; ++i) {
        Rectangle r = DifficultyCard(i);
        bool selected = g.menuSelection == i;
        const DifficultyInfo *d = &DIFFICULTIES[i];
        DrawRectangleRounded(r, 0.12f, 10, selected ? (Color){ 25, 34, 69, 245 } : C_PANEL);
        DrawRectangleRoundedLinesEx(r, 0.12f, 10, selected ? 2.0f : 1.0f,
                                    selected ? d->color : (Color){ 60, 73, 112, 220 });
        DrawLineEx(V2(r.x + 12.0f, r.y + 1.0f),
                   V2(r.x + r.width - 12.0f, r.y + 1.0f), 1.0f,
                   WithOpacity(C_TEXT, selected ? 0.12f : 0.04f));
        int nameSize = r.width < 120.0f ? 15 : 19;
        DrawText(d->name, (int)(r.x + (r.width - (float)MeasureText(d->name, nameSize)) * 0.5f),
                 (int)r.y + (shortView ? 10 : 18), nameSize, d->color);
        if (r.width > 150.0f) {
            DrawText(d->tagline, (int)(r.x + 12), (int)r.y + (shortView ? 37 : 54), 11, C_MUTED);
        }
        DrawText(i == 0 ? "1" : (i == 1 ? "2" : "3"),
                 (int)(r.x + r.width - 25), (int)r.y + 10, 11, selected ? C_TEXT : C_MUTED);
        if (selected) {
            DrawRectangleRounded((Rectangle){ r.x + 16, r.y + r.height - 12,
                                               r.width - 32, 3 }, 0.5f, 5, d->color);
        }
    }

    Rectangle startBtn = StartRunBounds();
    DrawButton(startBtn, "ENTER THE RIFT  [ENTER]", C_CYAN, true);
    Rectangle scoresLink = ScoresLinkBounds();
    bool scoresHover = HitRect(GetMousePosition(), scoresLink);
    DrawCentered("VIEW PERSONAL BESTS  [B]", (int)scoresLink.y + 3, 10,
                 scoresHover ? C_CYAN : C_MUTED);

    if (!shortView) {
        DrawCentered("WASD / ARROWS MOVE     •     MOUSE TO AIM     •     AUTO-FIRE", (int)(h * 0.78f), 12, C_TEXT);
        DrawCentered("P / ESC  PAUSE DURING A RUN     •     HOLD MOUSE TO AIM", (int)(h * 0.825f), 11, C_MUTED);
#if defined(__EMSCRIPTEN__)
        DrawCentered("NEW SPACE ENVIRONMENT EVERY 1,000 SCORE", (int)(h * 0.865f), 10,
                     (Color){ 111, 128, 176, 255 });
#else
        DrawCentered("NEW SPACE ENVIRONMENT EVERY 1,000 SCORE     •     ESC QUIT", (int)(h * 0.865f), 10,
                     (Color){ 111, 128, 176, 255 });
#endif
        DrawCentered("PERSONAL BESTS ARE SAVED ON THIS DEVICE", (int)(h * 0.91f), 9,
                     (Color){ 85, 103, 149, 255 });
    } else if (h >= 400) {
        DrawCentered("MOVE: WASD / ARROWS     •     AIM: MOUSE",
                     (int)(h * (h < 480 ? 0.765f : 0.75f)), 10, C_TEXT);
        DrawCentered("NEW SECTOR EVERY 1,000 SCORE", (int)(h * 0.80f), 9, C_MUTED);
    } else {
        DrawCentered("NEW SECTOR EVERY 1,000 SCORE", (int)(h * 0.84f), 9, C_MUTED);
    }
}

void DrawScores(void)
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
    Rectangle back = ScoresBackBounds();
    const char *backLabel = g.scoresReturnState == STATE_PAUSED ? "RETURN TO RUN" :
                            (g.scoresReturnState == STATE_GAME_OVER ? "RETURN TO RUN SUMMARY" : "BACK TO FLIGHT DECK");
    DrawButton(back, backLabel, C_CYAN, true);
    DrawCentered("B / ESC  BACK", (int)(h * 0.92f), 10, C_MUTED);
}

void DrawGameOver(void)
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
    snprintf(buf, sizeof(buf), "%s    /    RIFT LEVEL %02d", GetDifficulty()->name, g.stage);
    DrawCentered(buf, (int)p.y + (int)(167 * scaleY), (w < 440) ? 9 : 12, GetDifficulty()->color);
    if (g.newPersonalBest) DrawCentered("NEW PERSONAL BEST FOR THIS MODE!", (int)p.y + (int)(207 * scaleY), (w < 440) ? 11 : 15, C_GOLD);
    else DrawCentered("MODE BEST: SCORE NOT BEATEN", (int)p.y + (int)(207 * scaleY), (w < 440) ? 9 : 11, C_MUTED);
    Rectangle again = { p.x + 24, p.y + panelH * 0.64f, p.width - 48, 50 };
    DrawButton(again, "RUN AGAIN  [ENTER]", C_CYAN, true);
    Rectangle scoresLink = GameOverScoresBounds(p);
    bool scoresHover = HitRect(GetMousePosition(), scoresLink);
    DrawCentered("VIEW PERSONAL BESTS  [B]", (int)scoresLink.y + 2, 10,
                 scoresHover ? C_CYAN : C_MUTED);
    Rectangle titleLink = GameOverTitleBounds(p);
    bool titleHover = HitRect(GetMousePosition(), titleLink);
    DrawCentered("RETURN TO FLIGHT DECK  [ESC]", (int)titleLink.y + 2, 9,
                 titleHover ? C_CYAN : C_MUTED);
}

void DrawPause(void)
{
    int w = SW(), h = SH();
    DrawRectangle(0, 0, w, h, (Color){ 3, 4, 13, 195 });
    float panelW = MinF(420.0f, (float)w - 32.0f);
    bool veryShort = h < 400;
    float panelY = veryShort ? h * 0.24f : h * 0.32f;
    float panelH = veryShort ? h * 0.54f : h * 0.42f;
    Rectangle panel = { (float)w * 0.5f - panelW * 0.5f, panelY, panelW, panelH };
    DrawPanel(panel, C_CYAN);
    DrawCentered("PAUSED", (int)(h * (veryShort ? 0.275f : 0.365f)), veryShort ? 27 : 36, C_TEXT);
    DrawCentered("YOUR RUN IS ON HOLD", (int)(h * (veryShort ? 0.355f : 0.43f)), 10, C_MUTED);
    DrawButton(PauseResumeBounds(), "RESUME RUN  [P / ENTER]", C_CYAN, true);
    DrawButton(PauseTitleBounds(), "RETURN TO FLIGHT DECK", C_PURPLE, false);
    Rectangle scoresLink = PauseScoresBounds();
    bool scoresHover = HitRect(GetMousePosition(), scoresLink);
    DrawCentered("VIEW PERSONAL BESTS  [B]", (int)scoresLink.y + 3, 10,
                 scoresHover ? C_CYAN : C_MUTED);
    DrawCentered("ESC ALSO RESUMES", (int)(h * (veryShort ? 0.70f : 0.715f)), 9, C_MUTED);
}

void DrawTouchControls(void)
{
    if (g.state != STATE_PLAYING) return;
    if (SW() > 820 && GetTouchPointCount() <= 0) return;
    Vector2 anchor = V2(105.0f, (float)SH() - 126.0f);
    DrawCircleLines((int)anchor.x, (int)anchor.y, 55.0f, (Color){ 126, 206, 255, 95 });
    DrawCircleV(anchor, 22.0f, (Color){ 77, 123, 195, 50 });
    DrawCircleLines(SW() - 84, SH() - 92, 39.0f, (Color){ 126, 206, 255, 95 });
    DrawText("AIM", SW() - 111, SH() - 98, 11, C_CYAN);
}

static void DrawAimingReticle(void)
{
    if (!IsMouseButtonDown(MOUSE_BUTTON_LEFT) && !IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) return;
    if (GetTouchPointCount() > 0) return;
    Vector2 p = GetMousePosition();
    Color faint = WithOpacity(C_CYAN, 0.78f);
    DrawCircleLines((int)p.x, (int)p.y, 7.0f, faint);
    DrawLineEx(V2(p.x - 15.0f, p.y), V2(p.x - 9.0f, p.y), 1.2f, faint);
    DrawLineEx(V2(p.x + 9.0f, p.y), V2(p.x + 15.0f, p.y), 1.2f, faint);
    DrawLineEx(V2(p.x, p.y - 15.0f), V2(p.x, p.y - 9.0f), 1.2f, faint);
    DrawLineEx(V2(p.x, p.y + 9.0f), V2(p.x, p.y + 15.0f), 1.2f, faint);
    DrawCircleV(p, 1.5f, C_TEXT);
}

void DrawFrame(void)
{
    int w = SW(), h = SH();
    Camera2D camera = { 0 };
    float sx = 0.0f, sy = 0.0f;
    if (g.screenShake > 0.0f) {
        float amount = g.screenShake * 19.0f;
        /* Deterministic oscillation feels controlled and does not consume gameplay RNG. */
        sx = (sinf(g.worldTime * 61.0f) + 0.32f * sinf(g.worldTime * 103.0f)) * amount * 0.62f;
        sy = (cosf(g.worldTime * 67.0f) + 0.28f * sinf(g.worldTime * 89.0f)) * amount * 0.62f;
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
    if (g.state == STATE_PLAYING) DrawAimingReticle();
    if (g.flashAlpha > 0.0f) DrawRectangle(0, 0, w, h, (Color){ 188, 226, 255, (unsigned char)(ClampF(g.flashAlpha, 0, 0.4f) * 255.0f) });
    EndDrawing();
}





void UpdateTitle(void)
{
    if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A)) SetDifficulty(g.menuSelection - 1);
    if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D)) SetDifficulty(g.menuSelection + 1);
    if (IsKeyPressed(KEY_ONE) || IsKeyPressed(KEY_KP_1)) SetDifficulty(DIFF_EASY);
    if (IsKeyPressed(KEY_TWO) || IsKeyPressed(KEY_KP_2)) SetDifficulty(DIFF_MEDIUM);
    if (IsKeyPressed(KEY_THREE) || IsKeyPressed(KEY_KP_3)) SetDifficulty(DIFF_HARD);
    if (IsKeyPressed(KEY_B)) { OpenScores(STATE_TITLE); return; }
    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) { StartRun(); return; }

    Vector2 m;
    if (GetUIPointerPress(&m)) {
        for (int i = 0; i < 3; ++i) if (HitRect(m, DifficultyCard(i))) SetDifficulty(i);
        if (HitRect(m, ScoresLinkBounds())) { OpenScores(STATE_TITLE); return; }
        if (HitRect(m, StartRunBounds())) { StartRun(); return; }
    }
    if (IsKeyPressed(KEY_ESCAPE)) {
#if !defined(__EMSCRIPTEN__)
        g.quitRequested = true;
#endif
    }
}

void UpdatePaused(void)
{
    if (IsKeyPressed(KEY_P) || IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_ESCAPE)) { g.state = STATE_PLAYING; return; }
    if (IsKeyPressed(KEY_B)) { OpenScores(STATE_PAUSED); return; }
    Vector2 m;
    if (GetUIPointerPress(&m)) {
        if (HitRect(m, PauseResumeBounds())) { g.state = STATE_PLAYING; return; }
        if (HitRect(m, PauseTitleBounds())) { g.state = STATE_TITLE; return; }
        if (HitRect(m, PauseScoresBounds())) { OpenScores(STATE_PAUSED); return; }
    }
}

void UpdateGameOver(void)
{
    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) StartRun();
    if (IsKeyPressed(KEY_B)) { OpenScores(STATE_GAME_OVER); return; }
    if (IsKeyPressed(KEY_ESCAPE)) { g.state = STATE_TITLE; }
    Vector2 m;
    if (GetUIPointerPress(&m)) {
        float panelW = MinF(476.0f, (float)SW() - 24.0f);
        float panelH = MinF(410.0f, (float)SH() - 24.0f);
        Rectangle again = { (float)SW() * 0.5f - panelW * 0.5f + 24.0f, (float)SH() * 0.5f - panelH * 0.5f + panelH * 0.64f, panelW - 48.0f, 50.0f };
        if (HitRect(m, again)) { StartRun(); return; }
        float panelW2 = MinF(476.0f, (float)SW() - 24.0f);
        float panelH2 = MinF(410.0f, (float)SH() - 24.0f);
        Rectangle panel = { (float)SW() * 0.5f - panelW2 * 0.5f,
                            (float)SH() * 0.5f - panelH2 * 0.5f, panelW2, panelH2 };
        if (HitRect(m, GameOverScoresBounds(panel))) { OpenScores(STATE_GAME_OVER); return; }
        if (HitRect(m, GameOverTitleBounds(panel))) { g.state = STATE_TITLE; return; }
    }
}

void UpdateScores(void)
{
    if (IsKeyPressed(KEY_B) || IsKeyPressed(KEY_ESCAPE)) {
        g.state = g.scoresReturnState;
        return;
    }
    Vector2 m;
    if (GetUIPointerPress(&m) && HitRect(m, ScoresBackBounds())) {
        g.state = g.scoresReturnState;
    }
}


/* ======================================================================
 * SECTION: main.c — merged into main.c from the v0.26.0 modular source.
 * ====================================================================== */

/* RIFT RUNNER: SIGNAL ZERO — application entry point. */
#if defined(__EMSCRIPTEN__)
#endif

/* The shared live state is owned here; other systems access it through game.h. */
Game g;

/*
 * MAIN GAME PROCESS
 * This function is intentionally an outline: detailed logic lives in each system file.
 * The order matters: move and fire first, then resolve shots, enemies and pickups.
 */
void UpdatePlaying(float dt)
{
    /* 1. Pause input: do not update gameplay while the pause screen is open. */
    if (IsKeyPressed(KEY_P) || IsKeyPressed(KEY_ESCAPE)) {
        g.state = STATE_PAUSED;
        return;
    }

    /* 2. Player: read keyboard/touch/mouse input, move and aim. */
    UpdatePlayer(dt);

    /* 3. Blaster shots: move projectiles and check hits against enemies. */
    UpdateBullets(dt);
    if (g.state != STATE_PLAYING) return;

    /* 4. Enemies: update their movement/AI and check player contact damage. */
    UpdateEnemies(dt);
    if (g.state != STATE_PLAYING) return;

    /* 5. Pickups: the banana restores health and grants an 8-second shield. */
    UpdatePickups(dt);

    /* 6. Temporary effects: hit particles, screen shake and transition flashes. */
    UpdateEffects(dt);

    /* 7. Change to a new space environment every 1,000 score. */
    if (g.score / 1000 != g.backgroundTheme) {
        g.backgroundTheme = g.score / 1000;
        g.backgroundTransition = 0.90f;
    }

    /* 8. Occasionally create a banana pickup somewhere inside the arena. */
    g.bananaClock -= dt;
    if (g.bananaClock <= 0.0f) {
        AddPickup(V2(RandF(55.0f, (float)SW() - 55.0f),
                     RandF(75.0f, (float)SH() - 75.0f)));
        g.bananaClock = RandF(22.0f, 34.0f);
    }

    /* 9. Spawn the next enemy wave based on difficulty and current stage. */
    g.spawnClock -= dt;
    if (g.spawnClock <= 0.0f) {
        SpawnEnemy();
        float scoreRate = 1.0f + (float)(g.stage - 1) * 0.15f;
        float interval = GetDifficulty()->spawnInterval / scoreRate;
        g.spawnClock = MaxF(0.27f, interval) * RandF(0.84f, 1.16f);
        /* Hard mode adds pressure waves earlier and more often. At later
           stages, a rare third spawn creates a short survival spike. */
        if (g.difficulty == DIFF_HARD) {
            if (g.stage >= 3 && (rand() % 100) < 34) SpawnEnemy();
            if (g.stage >= 7 && (rand() % 100) < 12) SpawnEnemy();
        } else if (g.stage >= 4 && (rand() % 100) < 18) {
            SpawnEnemy();
        }
    }
}

/* Run one update-and-draw cycle. This is called once per rendered frame. */
void GameFrame(void)
{
    /* A stable delta time keeps movement consistent across different machines. */
    float dt = GetFrameTime();
    if (!isfinite(dt) || dt < 0.0f) dt = 0.0f;
    if (dt > 1.0f / 25.0f) dt = 1.0f / 25.0f;
    g.worldTime += dt;

    /* Poll mouse/touch once per frame so every menu uses the same input edge. */
    PollUIPointer();

    /* Route input and time to the active screen/game state. */
    switch (g.state) {
        case STATE_TITLE:    UpdateTitle();    UpdateEffects(dt); break;
        case STATE_PLAYING:  UpdatePlaying(dt);                break;
        case STATE_PAUSED:   UpdatePaused();   UpdateEffects(dt); break;
        case STATE_GAME_OVER:UpdateGameOver(); UpdateEffects(dt); break;
        case STATE_SCORES:   UpdateScores();   UpdateEffects(dt); break;
    }

    /* DrawFrame calls the background, game world, HUD and active screen. */
    DrawFrame();
}

/* Program setup starts here; platform-specific loop selection is at the bottom. */
int main(void)
{
    /* 1. Configure a resizable desktop window (fixed browser canvas on Web). */
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

    /* 2. Load starting state, background stars and the three stored best scores. */
    InitGame();

    /* 3. Hand control to the browser loop or run the desktop loop ourselves. */
#if defined(__EMSCRIPTEN__)
    emscripten_set_main_loop(GameFrame, 0, 1);
#else
    while (!WindowShouldClose() && !g.quitRequested) {
        GameFrame();
    }
    ShutdownAudio();
    CloseWindow();
#endif
    return 0;
}
