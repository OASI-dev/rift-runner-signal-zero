#include "persistence.h"

#include <errno.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define RR_PROFILE_PATH "rift_runner_profile.dat"
#define RR_PROFILE_HEADER "RIFT_RUNNER_PROFILE 1"


static bool ParseIntToken(const char *text, int *result)
{
    if (text == NULL || result == NULL || *text == '\0') return false;

    errno = 0;
    char *end = NULL;
    long parsed = strtol(text, &end, 10);
    if (errno == ERANGE || end == text || end == NULL || *end != '\0' ||
        parsed < INT_MIN || parsed > INT_MAX) return false;

    *result = (int)parsed;
    return true;
}

static float ClampVolume(float value)
{
    /* Guard against NaN in corrupted profile files as well as out-of-range values. */
    if (!isfinite(value)) return 0.0f;
    if (value < 0.0f) return 0.0f;
    if (value > 1.0f) return 1.0f;
    return value;
}

void Profile_Load(Game *game)
{
    if (game == NULL) return;

    FILE *file = fopen(RR_PROFILE_PATH, "r");
    if (file == NULL) return; /* First launch: keep defaults. */

    char line[160];
    const size_t headerLength = strlen(RR_PROFILE_HEADER);
    if (fgets(line, sizeof(line), file) == NULL) {
        fclose(file);
        return;
    }
    size_t lineLength = strlen(line);
    if (lineLength < headerLength ||
        strncmp(line, RR_PROFILE_HEADER, headerLength) != 0 ||
        (lineLength > headerLength && line[headerLength] != '\n' &&
         line[headerLength] != '\r' && line[headerLength] != '\0')) {
        fclose(file);
        return;
    }

    while (fgets(line, sizeof(line), file) != NULL) {
        char key[48] = {0};
        char value[96] = {0};
        if (sscanf(line, "%47s %95s", key, value) != 2) continue;
        if (strcmp(key, "best_score") == 0) {
            int n = 0; if (ParseIntToken(value, &n) && n >= 0) game->bestScore = n;
        } else if (strcmp(key, "best_sector") == 0) {
            int n = 0; if (ParseIntToken(value, &n) && n >= 1) game->bestSector = n;
        } else if (strcmp(key, "runs_played") == 0) {
            int n = 0; if (ParseIntToken(value, &n) && n >= 0) game->runsPlayed = n;
        } else if (strcmp(key, "lifetime_shards") == 0) {
            int n = 0; if (ParseIntToken(value, &n) && n >= 0) game->lifetimeShards = n;
        } else if (strcmp(key, "music_volume") == 0) {
            float n = 0.32f; if (sscanf(value, "%f", &n) == 1 && isfinite(n)) game->musicVolume = ClampVolume(n);
        } else if (strcmp(key, "sfx_volume") == 0) {
            float n = 0.65f; if (sscanf(value, "%f", &n) == 1 && isfinite(n)) game->sfxVolume = ClampVolume(n);
        } else if (strcmp(key, "audio_muted") == 0) {
            int n = 0; if (ParseIntToken(value, &n)) game->audioMuted = (n != 0);
        } else if (strcmp(key, "screen_shake") == 0) {
            int n = 1; if (ParseIntToken(value, &n)) game->screenShakeEnabled = (n != 0);
        }
    }
    fclose(file);
}

bool Profile_Save(const Game *game)
{
    if (game == NULL) return false;

    /* Write a complete temporary profile first so a partial write doesn't clobber a good save. */
    const char *temporaryPath = RR_PROFILE_PATH ".tmp";
    FILE *file = fopen(temporaryPath, "w");
    if (file == NULL) return false;

    int failed = fprintf(file,
        RR_PROFILE_HEADER "\n"
        "best_score %d\n"
        "best_sector %d\n"
        "runs_played %d\n"
        "lifetime_shards %d\n"
        "music_volume %.3f\n"
        "sfx_volume %.3f\n"
        "audio_muted %d\n"
        "screen_shake %d\n",
        game->bestScore < 0 ? 0 : game->bestScore,
        game->bestSector < 1 ? 1 : game->bestSector,
        game->runsPlayed < 0 ? 0 : game->runsPlayed,
        game->lifetimeShards < 0 ? 0 : game->lifetimeShards,
        ClampVolume(game->musicVolume), ClampVolume(game->sfxVolume),
        game->audioMuted ? 1 : 0, game->screenShakeEnabled ? 1 : 0) < 0;
    if (fflush(file) != 0) failed = 1;
    if (fclose(file) != 0) failed = 1;
    if (failed) {
        (void)remove(temporaryPath);
        return false;
    }

    if (rename(temporaryPath, RR_PROFILE_PATH) == 0) return true;

#if defined(_WIN32)
    /* Windows C rename() does not replace an existing destination. */
    if (remove(RR_PROFILE_PATH) == 0 && rename(temporaryPath, RR_PROFILE_PATH) == 0) return true;
#endif

    (void)remove(temporaryPath);
    return false;
}
