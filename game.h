#ifndef RR_GAME_H
#define RR_GAME_H
#include "raylib.h"
#include <stdbool.h>
#include "player.h"
#include "enemy.h"
#include "bullets.h"
#include "effects.h"
#include "pickups.h"
#include "background.h"
#include "audio.h"
#include "colors.h"

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
    Floater floaters[MAX_FLOATERS];
    Star stars[MAX_STARS];
    int bestScores[3];
    int score, kills, stage, combo;
    bool newPersonalBest;
    bool audioReady, musicLoaded, firstAudioGesture, quitRequested;
    float worldTime, spawnClock, screenShake, flashAlpha, stageNotice;
    float bananaClock, backgroundTransition, comboClock;
    int backgroundTheme, menuSelection;
    Music ambient;
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
Vector2 Normalize(Vector2 v);
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
void AddPickup(Vector2 pos, PickupKind kind);
void UpdatePickups(float dt);
void AddParticle(Vector2 pos, Vector2 vel, float radius, float life, Color color);
void Burst(Vector2 pos, Color color, int count, float speed, float radius);
void AddFloater(Vector2 pos, int points, Color color);
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
#endif
