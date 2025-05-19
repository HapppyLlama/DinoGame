#ifndef WINDOW_H
#define WINDOW_H
#include "raylib.h"
#include <stdbool.h>
#include "types.h"

void InitWindowState(WindowState* state);
void UpdateScaleFactor(WindowState* window);
void RescaleGame(GameState* game, WindowState* window);
void ChangeResolution(WindowState* window, GameState* game, int width, int height, bool fullscreen);
void HandleFullscreenToggle(WindowState* window);
void UpdateButtonPositions(WindowState* window);
void UpdateResolutionButtonPositions(WindowState* window);

#endif // WINDOW_H
