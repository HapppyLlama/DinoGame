#include "raylib.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "game.h"
#include "window.h"
#include "menu.h"
#include "draw.h"
#include "utils.h"

void InitWindowState(WindowState* state) {
    state->width = BASE_RESOLUTION.x;
    state->height = BASE_RESOLUTION.y;
    state->isFullscreen = false;
    ResolutionButton resolutions[NUM_RESOLUTIONS] = {
        {.text = "1280x720", .width = 1280, .height = 720},
        {.text = "1600x900", .width = 1600, .height = 900},
        {.text = "1920x1080", .width = 1920, .height = 1080},
        {.text = "Fullscreen"}
    };
    for (int i = 0; i < NUM_RESOLUTIONS; i++) {
        state->resolutions[i] = resolutions[i];
        state->resolutions[i].textWidth = MeasureText(resolutions[i].text, 20);
        state->resolutions[i].rect = (Rectangle){0};
    }
    UpdateResolutionButtonPositions(state);
    state->gameState = GAME_STATE_MENU;
    InitMenuButtons(state);
}

void UpdateScaleFactor(WindowState* window) {
    window->scaleFactor = (float)window->height / BASE_RESOLUTION.y;
}

void RescaleGame(GameState* game, WindowState* window) {
    UpdateScaleFactor(window);
    game->screenPosition.x = game->basePosition.x * window->scaleFactor;
    game->screenPosition.y = game->basePosition.y * window->scaleFactor;
    if (game->lightMask.id != 0) {
        UnloadRenderTexture(game->lightMask);
    }
    game->lightMask = LoadRenderTexture(window->width, window->height);
    const Rectangle* frame = game->isCrouching ? &game->crouchFrames[game->currentFrame] : &game->runFrames[game->currentFrame];
    game->rect.width = frame->width * window->scaleFactor;
    game->rect.height = frame->height * window->scaleFactor;
    game->rect.x = game->screenPosition.x;
    game->rect.y = game->screenPosition.y;
    for (int i = 0; i < MAX_OBSTACLES; i++) {
        Obstacle* obs = &game->obstacles.obstacles[i];
        if (obs->active) {
            const ObstacleDimensions dims = CACTUS_DIMENSIONS[obs->type];
            float groundY = window->height - GROUND_HEIGHT * window->scaleFactor;
            float yPos = groundY - dims.height * window->scaleFactor + dims.yOffset * window->scaleFactor;
            obs->rect.width = dims.width * window->scaleFactor;
            obs->rect.height = dims.height * window->scaleFactor;
            obs->rect.y = yPos;
            obs->collisionRect = (Rectangle){
                obs->rect.x + COLLISION_OFFSET * window->scaleFactor,
                obs->rect.y + COLLISION_OFFSET * window->scaleFactor,
                obs->rect.width - 2 * COLLISION_OFFSET * window->scaleFactor,
                obs->rect.height - 2 * COLLISION_OFFSET * window->scaleFactor
            };
        }
    }
    UpdateButtonPositions(window);
    UpdateResolutionButtonPositions(window);
    InitMenuButtons(window);
}

void ChangeResolution(WindowState* window, GameState* game, int width, int height, bool fullscreen) {
    if (fullscreen) {
        if (!window->isFullscreen) HandleFullscreenToggle(window);
    } else {
        if (window->isFullscreen) HandleFullscreenToggle(window);
        window->width = width;
        window->height = height;
        SetWindowSize(window->width, window->height);
    }
    RescaleGame(game, window);
}

void HandleFullscreenToggle(WindowState* window) {
    if (window->isFullscreen) {
        ToggleFullscreen();
        SetWindowSize(window->width, window->height);
    } else {
        const int monitor = GetCurrentMonitor();
        int monitorWidth = GetMonitorWidth(monitor);
        int monitorHeight = GetMonitorHeight(monitor);
        SetWindowSize(monitorWidth, monitorHeight);
        ToggleFullscreen();
        window->width = monitorWidth;
        window->height = monitorHeight;
    }
    window->isFullscreen = !window->isFullscreen;
}

void UpdateButtonPositions(WindowState* window) {
    const float buttonWidth = 120;
    const float buttonHeight = 30;
    const float startX = window->width - buttonWidth - 20;
    const float startY = 20;
    for (int i = 0; i < NUM_RESOLUTIONS; i++) {
        window->resolutions[i].rect = (Rectangle){
            startX,
            startY + i * (buttonHeight + 10),
            buttonWidth,
            buttonHeight
        };
    }
}

void UpdateResolutionButtonPositions(WindowState* window) {
    const float buttonWidth = 200;
    const float buttonHeight = 50;
    const float spacing = 20;
    const float totalHeight = NUM_RESOLUTIONS * buttonHeight + (NUM_RESOLUTIONS - 1) * spacing;
    const float startX = window->width / 2 - buttonWidth / 2;
    const float startY = window->height / 2 - totalHeight / 2;
    for (int i = 0; i < NUM_RESOLUTIONS; i++) {
        window->resolutions[i].rect = (Rectangle){
            startX,
            startY + i * (buttonHeight + spacing),
            buttonWidth,
            buttonHeight
        };
    }
}
