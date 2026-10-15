#include "game.h"
#include "persistence.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int fail(const char *message)
{
    fprintf(stderr, "FAIL: %s\n", message);
    return 1;
}

int main(void)
{
    Game source;
    memset(&source, 0, sizeof(source));
    source.bestScore = 9876;
    source.bestSector = 12;
    source.runsPlayed = 45;
    source.lifetimeShards = 678;
    source.musicVolume = 0.35f;
    source.sfxVolume = 0.80f;
    source.audioMuted = true;
    source.screenShakeEnabled = false;

    if (!Profile_Save(&source)) return fail("profile save failed");

    Game loaded;
    memset(&loaded, 0, sizeof(loaded));
    loaded.musicVolume = 0.32f;
    loaded.sfxVolume = 0.65f;
    loaded.bestSector = 1;
    Profile_Load(&loaded);
    if (loaded.bestScore != 9876 || loaded.bestSector != 12 ||
        loaded.runsPlayed != 45 || loaded.lifetimeShards != 678 ||
        fabsf(loaded.musicVolume - 0.35f) > 0.002f ||
        fabsf(loaded.sfxVolume - 0.80f) > 0.002f || !loaded.audioMuted ||
        loaded.screenShakeEnabled) {
        return fail("profile round-trip mismatch");
    }

    /* A non-finite value in the file must not poison audio controls. */
    FILE *file = fopen("rift_runner_profile.dat", "w");
    if (file == NULL) return fail("unable to write corrupt-profile fixture");
    fputs("RIFT_RUNNER_PROFILE 1\nmusic_volume nan\nsfx_volume 4.0\n", file);
    fclose(file);
    loaded.musicVolume = 0.32f;
    loaded.sfxVolume = 0.65f;
    loaded.screenShakeEnabled = true;
    Profile_Load(&loaded);
    if (fabsf(loaded.musicVolume - 0.32f) > 0.002f ||
        fabsf(loaded.sfxVolume - 1.0f) > 0.002f) {
        return fail("corrupt volume values not safely bounded");
    }

    /* Short invalid headers should be rejected without reading past initialized text. */
    file = fopen("rift_runner_profile.dat", "w");
    if (file == NULL) return fail("unable to write short-header fixture");
    fputs("x\n", file);
    fclose(file);
    loaded.bestScore = 1234;
    Profile_Load(&loaded);
    if (loaded.bestScore != 1234) return fail("invalid profile changed defaults");

    /* Old version-1 profiles remain valid when optional screen_shake is absent. */
    file = fopen("rift_runner_profile.dat", "w");
    if (file == NULL) return fail("unable to write legacy profile fixture");
    fputs("RIFT_RUNNER_PROFILE 1\nbest_score 42\n", file);
    fclose(file);
    loaded.screenShakeEnabled = true;
    Profile_Load(&loaded);
    if (loaded.bestScore != 42 || !loaded.screenShakeEnabled)
        return fail("legacy profile did not preserve the default screen-shake setting");

    /* Oversized numeric values must be rejected without scanf integer overflow. */
    file = fopen("rift_runner_profile.dat", "w");
    if (file == NULL) return fail("unable to write oversized-number fixture");
    fputs("RIFT_RUNNER_PROFILE 1\nbest_score 999999999999999999999999\nruns_played -1\n", file);
    fclose(file);
    loaded.bestScore = 321;
    loaded.runsPlayed = 9;
    Profile_Load(&loaded);
    if (loaded.bestScore != 321 || loaded.runsPlayed != 9)
        return fail("oversized/negative profile counters were not safely ignored");

    remove("rift_runner_profile.dat");
    remove("rift_runner_profile.dat.tmp");
    puts("profile persistence tests passed");
    return 0;
}
