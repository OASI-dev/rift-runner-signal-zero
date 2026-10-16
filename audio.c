#include "game.h"

void FirstAudioGesture(void)
{
    if (g.firstAudioGesture) return;
    /* Initialize only after a real key/mouse gesture so browser audio is not autoplay-blocked. */
    if (!g.audioReady) {
        InitAudioDevice();
        g.audioReady = IsAudioDeviceReady();
        if (g.audioReady) {
            g.sounds[SFX_SHOT] = LoadSound("assets/shot.wav");
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

void PlaySfx(SoundId id)
{
    if (g.audioReady && id >= 0 && id < SFX_COUNT && IsSoundValid(g.sounds[id])) PlaySound(g.sounds[id]);
}

void ShutdownAudio(void)
{
#if !defined(__EMSCRIPTEN__)
    if (g.audioReady) {
        if (g.musicLoaded) { StopMusicStream(g.ambient); UnloadMusicStream(g.ambient); }
        for (int i = 0; i < SFX_COUNT; ++i) {
            if (IsSoundValid(g.sounds[i])) UnloadSound(g.sounds[i]);
        }
        CloseAudioDevice();
    }
#endif
}
