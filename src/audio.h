#ifndef RIFT_RUNNER_AUDIO_H
#define RIFT_RUNNER_AUDIO_H

#include "game.h"

void Audio_Init(void);
void Audio_Update(const Game *game, float deltaTime);
void Audio_Shutdown(void);

#endif /* RIFT_RUNNER_AUDIO_H */
