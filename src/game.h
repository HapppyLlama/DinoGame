#ifndef GAME_H
#define GAME_H
#include "types.h"
#include <stdbool.h>

void InitGameState(GameState* state);
void ResetGame(GameState* game);
void UpdatePhysics(GameState* game, const WindowState* window, float deltaTime);
void UpdateScore(GameState* game, float deltaTime);
void UpdateObstacles(GameState* state, const WindowState* window, float deltaTime);
void UpdateBossFight(GameState* game, WindowState* window, float deltaTime);
void InitObstacles(GameState* state);
void InitMeteors(GameState* state);
void SpawnObstacle(GameState* state, const WindowState* window);
void SpawnMeteor(GameState* game, const WindowState* window);
void UpdateMeteors(GameState* game, const WindowState* window, float deltaTime);
void InitClouds(GameState* game);
void SpawnCloud(GameState* game, const WindowState* window);
void SpawnCloudAt(GameState* game, int index, float xPosition);
void UpdateClouds(GameState* game, const WindowState* window, float deltaTime);

#endif
