#ifndef DRAW_H
#define DRAW_H
#include "raylib.h"
#include "window.h"
#include "game.h"
#include "types.h"

void DrawGame(const WindowState* window, const GameState* game);
void DrawPauseMenu(const WindowState* window, const GameState* game);
void DrawMeteors(const WindowState* window, const GameState* game, Vector2 shakeOffset);
void DrawBossHP(const WindowState* window, const GameState* game);
void UpdateAnimation(GameState* game, const WindowState* window, float deltaTime);
Vector2 ApplyScreenShake(const GameState* game);

#endif 
