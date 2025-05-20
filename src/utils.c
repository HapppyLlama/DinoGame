#include "utils.h"
#include <stdio.h>
#include <stdbool.h>
#include "raylib.h"
#include <stdlib.h>
#include <math.h>
#include "game.h"
#include "window.h"
#include "menu.h"
#include "draw.h"
#include "sound.h"

void SaveHighScore(int highScore) {
    FILE* file = fopen("highscore.bin", "wb");
    if (file) {
        fwrite(&highScore, sizeof(int), 1, file);
        fclose(file);
    }
}

int LoadHighScore() {
    int highScore = 0;
    FILE* file = fopen("highscore.bin", "rb");
    if (file) {
        fread(&highScore, sizeof(int), 1, file);
        fclose(file);
    }
    return highScore;
}

bool IsButtonHovered(const Rectangle* button) {
    return CheckCollisionPointRec(GetMousePosition(), *button);
}

bool IsButtonHoveredScaled(const Rectangle* button, float scale) {
    Vector2 mouse = GetMousePosition();
    Rectangle scaled = {
        button->x * scale,
        button->y * scale,
        button->width * scale,
        button->height * scale
    };
    return CheckCollisionPointRec(mouse, scaled);
}

void HandleInput(WindowState* window, GameState* game) {
    if (game->pauseMenu.isPaused) {
        HandlePauseMenuInput(window, game);
        return;
    }

    Vector2 mousePos = GetMousePosition();
    if (window->gameState == GAME_STATE_MENU) {
        window->menu.playHovered = IsButtonHovered(&window->menu.playButton.rect);
        window->menu.storyHovered = IsButtonHovered(&window->menu.storyButton.rect);
        window->menu.quitHovered = IsButtonHovered(&window->menu.quitButton.rect);
        window->menu.resolutionHovered = IsButtonHovered(&window->menu.resolutionButton.rect);
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            if (window->menu.playHovered) {
                window->gameState = GAME_STATE_PLAYING;
                game->isStoryMode = false;
                ResetGame(game);
            } else if (window->menu.storyHovered) {
                window->gameState = GAME_STATE_PLAYING;
                game->isStoryMode = true;
                ResetGame(game);
            } else if (window->menu.quitHovered) {
                CloseWindow();
                exit(0);
            } else if (window->menu.resolutionHovered) {
                window->gameState = GAME_STATE_RESOLUTION;
            }
        }
        return;
    }
    if (window->gameState == GAME_STATE_RESOLUTION) {
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            for (int i = 0; i < NUM_RESOLUTIONS; i++) {
                if (IsButtonHovered(&window->resolutions[i].rect)) {
                    if (i == NUM_RESOLUTIONS - 1) {
                        ChangeResolution(window, game, 0, 0, true);
                    } else {
                        ChangeResolution(window, game, window->resolutions[i].width, window->resolutions[i].height, false);
                    }
                    break;
                }
            }
            Rectangle backButton = { window->width / 2 - 100, window->height - 100, 200, 50 };
            if (IsButtonHovered(&backButton)) {
                window->gameState = GAME_STATE_MENU;
            }
        }
        return;
    }
    static bool wasWPressed = false;
    bool isWPressed = IsKeyDown(KEY_W);
    bool wasCrouching = game->isCrouching;
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        Vector2 mousePos = GetMousePosition();
        for (int i = 0; i < NUM_RESOLUTIONS; i++) {
            if (CheckCollisionPointRec(mousePos, window->resolutions[i].rect)) {
                if (i == NUM_RESOLUTIONS - 1) {
                    HandleFullscreenToggle(window);
                } else {
                    if (window->isFullscreen) HandleFullscreenToggle(window);
                    window->width = window->resolutions[i].width;
                    window->height = window->resolutions[i].height;
                    SetWindowSize(window->width, window->height);
                }
                RescaleGame(game, window);
                break;
            }
        }
    }
    if (IsKeyPressed(KEY_F11)) HandleFullscreenToggle(window);
    if (game->gameWon && IsKeyPressed(KEY_SPACE)) {
        StopAllSounds(game);
        window->gameState = GAME_STATE_MENU;
        ResetGame(game);
        return;
    }
    if (game->gameOver && !game->gameWon && IsKeyPressed(KEY_SPACE)) {
        StopAllSounds(game);
        ResetGame(game);
        window->gameState = GAME_STATE_PLAYING;
        return;
    }
    game->isCrouching = IsKeyDown(KEY_S);
    if (!game->isJumping && (game->isCrouching != wasCrouching)) {
        float newHeight = game->isCrouching ? game->crouchFrameHeight : game->runFrameHeight;
        game->basePosition.y = BASE_RESOLUTION.y - GROUND_HEIGHT - newHeight;
        game->currentFrame = 0;
        game->frameTime = 0;
        const Rectangle* newFrame = game->isCrouching ? &game->crouchFrames[0] : &game->runFrames[0];
        game->baseSize.x = newFrame->width;
        game->baseSize.y = game->isCrouching ? game->crouchFrameHeight : game->runFrameHeight;
    }
    if (!game->isJumping && !game->isCrouching) {
        if (!wasWPressed && isWPressed) {
            game->isJumping = true;
            game->isJumpCharging = true;
            game->jumpChargeTime = 0.0f;
            game->baseJumpVelocity = JUMP_FORCE;
            
            PlayJumpSound(game);
        }
    }
    if (wasWPressed && !isWPressed) {
        game->isJumpCharging = false;
    }
    wasWPressed = isWPressed;
    if (game->isCrouching && game->isJumping) {
        game->baseJumpVelocity = FAST_FALL_VELOCITY;
    }
    if (window->gameState == GAME_STATE_PLAYING) {
        if (IsKeyPressed(KEY_O)) {
            game->pauseMenu.isPaused = !game->pauseMenu.isPaused;
        }
        if (game->pauseMenu.isPaused) {
            HandlePauseMenuInput(window, game);
            return;
        }
    }
}
