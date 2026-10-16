/* Original one-shot game sound effects. No music or ambience loop. */
#include "game.h"

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
