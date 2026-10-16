/* Non-musical sound design: combat effects plus a quiet space ambience bed. */
#include "game.h"
#include <stdlib.h>

void FirstAudioGesture(void)
{
    if (g.firstAudioGesture) return;

    /* Initialize after a user gesture so browser autoplay restrictions are respected. */
    if (!g.audioReady) {
        InitAudioDevice();
        g.audioReady = IsAudioDeviceReady();
        if (g.audioReady) {
            g.sounds[SFX_SHOT] = LoadSound("assets/shot.wav");
            g.sounds[SFX_IMPACT] = LoadSound("assets/impact.wav");
            g.sounds[SFX_PICKUP] = LoadSound("assets/pickup.wav");
            g.sounds[SFX_ENEMY_DOWN] = LoadSound("assets/enemy_down.wav");
            g.sounds[SFX_SURGE] = LoadSound("assets/surge.wav");
            g.sounds[SFX_SPACE_AMBIENCE] = LoadSound("assets/space_ambience.wav");
            g.sounds[SFX_RIFT_SHIFT] = LoadSound("assets/rift_shift.wav");
            g.sounds[SFX_VOID_WHISPER] = LoadSound("assets/void_whisper.wav");
            g.sounds[SFX_DEEP_RUMBLE] = LoadSound("assets/deep_rumble.wav");

            if (IsSoundValid(g.sounds[SFX_SPACE_AMBIENCE])) SetSoundVolume(g.sounds[SFX_SPACE_AMBIENCE], 0.22f);
            if (IsSoundValid(g.sounds[SFX_VOID_WHISPER])) SetSoundVolume(g.sounds[SFX_VOID_WHISPER], 0.18f);
            if (IsSoundValid(g.sounds[SFX_DEEP_RUMBLE])) SetSoundVolume(g.sounds[SFX_DEEP_RUMBLE], 0.24f);
            if (IsSoundValid(g.sounds[SFX_RIFT_SHIFT])) SetSoundVolume(g.sounds[SFX_RIFT_SHIFT], 0.42f);
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

void UpdateAmbientAudio(float dt)
{
    if (!g.audioReady || !g.firstAudioGesture) return;

    /* The ambience is a textured sound bed, not music; replay it when its WAV ends. */
    if (IsSoundValid(g.sounds[SFX_SPACE_AMBIENCE]) &&
        !IsSoundPlaying(g.sounds[SFX_SPACE_AMBIENCE])) {
        PlaySound(g.sounds[SFX_SPACE_AMBIENCE]);
    }

    /* Keep rare environmental accents during active play, never rhythmic or melodic. */
    if (g.state != STATE_PLAYING) return;
    g.ambientEventClock -= dt;
    if (g.ambientEventClock <= 0.0f) {
        if ((rand() % 3) == 0) PlaySfx(SFX_DEEP_RUMBLE);
        else PlaySfx(SFX_VOID_WHISPER);
        g.ambientEventClock = RandF(19.0f, 34.0f);
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
