#ifndef SOUND_H
#define SOUND_H

#include "raylib.h"
#include "types.h"

void InitSounds(GameState* game);
void UnloadSounds(GameState* game);
void PlayJumpSound(GameState* game);
void PlayGameOverSound(GameState* game);
void PlayWinSound(GameState* game);
void PlayMeteorImpactSound(GameState* game);
void StopAllSounds(GameState* game);

#endif
