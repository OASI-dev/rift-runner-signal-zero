#ifndef RIFT_RUNNER_PERSISTENCE_H
#define RIFT_RUNNER_PERSISTENCE_H

#include "game.h"

/* Profile is a small, versioned plain-text file in the process working directory. */
void Profile_Load(Game *game);
bool Profile_Save(const Game *game);

#endif /* RIFT_RUNNER_PERSISTENCE_H */
