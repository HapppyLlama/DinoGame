#ifndef MENU_H
#define MENU_H
#include "raylib.h"
#include "window.h"
#include "game.h"
#include "types.h"
#include <stdbool.h>

void InitMenuButtons(WindowState* window);
void DrawMenu(const WindowState* window);
void DrawResolutionMenu(const WindowState* window);
void HandlePauseMenuInput(WindowState* window, GameState* game);
void InitPauseMenu(GameState* game, const WindowState* window);

#endif
