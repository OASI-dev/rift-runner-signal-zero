#include "audio.h"

#include "raylib.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

static bool audioReady = false;
static Music ambientMusic = {0};
static Sound soundDash = {0};
static Sound soundPickup = {0};
static Sound soundImpact = {0};
static Sound soundEnemyDown = {0};
static Sound soundSurge = {0};

static float previousDashTimer = 0.0f;
static int previousShardCount = 0;
static int previousEnemiesDefeated = 0;
static int previousLives = 3;
static bool previousSurgeWarning = false;
static bool previousSurgeActive = false;
static GameScreen previousScreen = GAME_SCREEN_TITLE;

static void BuildAssetPath(char *destination, size_t capacity, const char *filename)
{
    const char *applicationDirectory = GetApplicationDirectory();
    if (applicationDirectory == NULL) applicationDirectory = "";
    (void)snprintf(destination, capacity, "%sassets/%s", applicationDirectory, filename);
}

static void PlayIfReady(Sound sound, float volume)
{
    if (audioReady && sound.stream.buffer != NULL && volume > 0.0f) {
        SetSoundVolume(sound, volume);
        PlaySound(sound);
    }
}

void Audio_Init(void)
{
    if (audioReady) return;
    InitAudioDevice();
    if (!IsAudioDeviceReady()) return;

    audioReady = true;
    /* Resolve assets beside the executable, independent of the launch directory. */
    char assetPath[512];
    BuildAssetPath(assetPath, sizeof(assetPath), "ambient_loop.wav");
    ambientMusic = LoadMusicStream(assetPath);
    BuildAssetPath(assetPath, sizeof(assetPath), "dash.wav");
    soundDash = LoadSound(assetPath);
    BuildAssetPath(assetPath, sizeof(assetPath), "pickup.wav");
    soundPickup = LoadSound(assetPath);
    BuildAssetPath(assetPath, sizeof(assetPath), "impact.wav");
    soundImpact = LoadSound(assetPath);
    BuildAssetPath(assetPath, sizeof(assetPath), "enemy_down.wav");
    soundEnemyDown = LoadSound(assetPath);
    BuildAssetPath(assetPath, sizeof(assetPath), "surge.wav");
    soundSurge = LoadSound(assetPath);
    ambientMusic.looping = true;
    if (ambientMusic.stream.buffer != NULL) PlayMusicStream(ambientMusic);
}

void Audio_Update(const Game *game, float deltaTime)
{
    (void)deltaTime;
    if (game == NULL || !audioReady) return;

    bool muted = game->audioMuted;
    if (ambientMusic.stream.buffer != NULL) {
        SetMusicVolume(ambientMusic, muted ? 0.0f : game->musicVolume);
        UpdateMusicStream(ambientMusic);
        if (!IsMusicStreamPlaying(ambientMusic)) PlayMusicStream(ambientMusic);
    }
    if (!muted) {
        if (game->player.dashTimer > 0.0f && previousDashTimer <= 0.0f)
            PlayIfReady(soundDash, game->sfxVolume);
        if (game->shardsCollected > previousShardCount)
            PlayIfReady(soundPickup, game->sfxVolume);
        if (game->enemiesDefeated > previousEnemiesDefeated)
            PlayIfReady(soundEnemyDown, game->sfxVolume);
        if (game->lives < previousLives)
            PlayIfReady(soundImpact, game->sfxVolume);
        if ((game->coreSurgeWarning && !previousSurgeWarning) ||
            (game->coreSurgeActive && !previousSurgeActive)) {
            PlayIfReady(soundSurge, game->sfxVolume);
        }
        if (game->screen == GAME_SCREEN_GAME_OVER && previousScreen != GAME_SCREEN_GAME_OVER)
            PlayIfReady(soundImpact, game->sfxVolume);
    }

    previousDashTimer = game->player.dashTimer;
    previousShardCount = game->shardsCollected;
    previousEnemiesDefeated = game->enemiesDefeated;
    previousLives = game->lives;
    previousSurgeWarning = game->coreSurgeWarning;
    previousSurgeActive = game->coreSurgeActive;
    previousScreen = game->screen;
}

void Audio_Shutdown(void)
{
    if (!audioReady) return;
    if (ambientMusic.stream.buffer != NULL) {
        StopMusicStream(ambientMusic);
        UnloadMusicStream(ambientMusic);
    }
    if (soundDash.stream.buffer != NULL) UnloadSound(soundDash);
    if (soundPickup.stream.buffer != NULL) UnloadSound(soundPickup);
    if (soundImpact.stream.buffer != NULL) UnloadSound(soundImpact);
    if (soundEnemyDown.stream.buffer != NULL) UnloadSound(soundEnemyDown);
    if (soundSurge.stream.buffer != NULL) UnloadSound(soundSurge);
    CloseAudioDevice();
    audioReady = false;
}
