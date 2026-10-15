#ifndef RIFT_RUNNER_RENDER_H
#define RIFT_RUNNER_RENDER_H

#include "game.h"

void Renderer_Init(void);
void Renderer_Update(float deltaTime);
void Renderer_Draw(const Game *game);
void Renderer_Shutdown(void);

#endif /* RIFT_RUNNER_RENDER_H */
