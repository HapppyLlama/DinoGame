#include "raylib.h"
#include "src/types.h"
#include "src/game.h"
#include "src/window.h"
#include "src/menu.h"
#include "src/draw.h"
#include "src/utils.h"

#include <stdbool.h>

int main(void) {
    InitWindow(BASE_RESOLUTION.x, BASE_RESOLUTION.y, "DinoGame");
    SetTargetFPS(60);

    WindowState window = {0};
    GameState game = {0};
    
    InitWindowState(&window);
    InitGameState(&game);
    RescaleGame(&game, &window);

    while (!WindowShouldClose()) {
        float deltaTime = GetFrameTime();
        
        HandleInput(&window, &game);
        
        switch (window.gameState) {
            case GAME_STATE_MENU:
                DrawMenu(&window);
                break;

            case GAME_STATE_RESOLUTION:
                DrawResolutionMenu(&window);
                break;
                
            case GAME_STATE_PLAYING:
                if (!game.gameOver && !game.pauseMenu.isPaused) {
                    UpdatePhysics(&game, &window, deltaTime);
                    UpdateScore(&game, deltaTime);
                    UpdateObstacles(&game, &window, deltaTime);
                    UpdateBossFight(&game, &window, deltaTime);

                    if (!game.isNight) {
                        game.dayCycleTimer += deltaTime;
                        if (game.score >= 200 && game.dayCycleTimer >= DAY_DURATION) {
                            game.nightModeActive = true;
                            game.nightCycleTimer = 0.0f;
                            game.nightAlpha = 0.0f;
                            game.isNight = true;
                        }
                    }
                    if (game.nightModeActive) {
                        game.nightCycleTimer += deltaTime;
                        if (game.nightCycleTimer < FADE_DURATION) {
                            game.nightAlpha = game.nightCycleTimer / FADE_DURATION;
                        } else if (game.nightCycleTimer < (FADE_DURATION + NIGHT_DURATION)) {
                            game.nightAlpha = 1.0f;
                        } else if (game.nightCycleTimer < (FADE_DURATION + NIGHT_DURATION + FADE_DURATION)) {
                            game.nightAlpha = 1.0f - (game.nightCycleTimer - FADE_DURATION - NIGHT_DURATION) / FADE_DURATION;
                        } else {
                            game.nightModeActive = false;
                            game.nightCycleTimer = 0.0f;
                            game.nightAlpha = 0.0f;
                            game.isNight = false;
                            game.dayCycleTimer = 0.0f;
                        }
                    }
                }
                UpdateAnimation(&game, &window, deltaTime);
                DrawGame(&window, &game);
                break;
                
            case GAME_STATE_GAME_OVER:
                break;
        }
    }

    CloseWindow();
    return 0;
}

void UpdateObstacles(GameState *game, WindowState *window, float deltaTime) {
    for (int i = 0; i < game->obstacleCount; i++) {
        Obstacle *obs = &game->obstacles[i];
        if (obs->type == OBSTACLE_METEORITE) {
            if (!obs->hasPassedPlayer && obs->y > game->player.y) {
                obs->hasPassedPlayer = true;
                if (game->hp > 0) game->hp -= 1;
            }
        }
    }
}