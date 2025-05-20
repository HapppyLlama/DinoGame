#ifndef UTILS_H
#define UTILS_H
#include <stdbool.h>
#include "raylib.h"
#include "types.h"

int LoadHighScore();
void SaveHighScore(int highScore);
bool IsButtonHovered(const Rectangle* button);
bool IsButtonHoveredScaled(const Rectangle* button, float scale);
void HandleInput(WindowState* window, GameState* game);

#endif
