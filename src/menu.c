// menu.c - Menu and pause menu logic implementation
#include "menu.h"
#include "window.h"
#include "game.h"
#include "raylib.h"
#include "utils.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// Move all menu and pause menu related function definitions here from main.c
// Example:
// void InitMenuButtons(WindowState* window) { ... }
// void DrawMenu(const WindowState* window) { ... }
// void DrawResolutionMenu(const WindowState* window) { ... }
// void HandlePauseMenuInput(WindowState* window, GameState* game) { ... }
// void InitPauseMenu(GameState* game, const WindowState* window) { ... }

// Menu and pause menu logic implementations moved from main.c

void InitMenuButtons(WindowState* window) {
    const float buttonWidth = 200;
    const float buttonHeight = 50;
    const float buttonSpacing = 20;
    Vector2 center = { window->width/2 - buttonWidth/2, window->height/2 - buttonHeight/2 };
    float startY = center.y - 180;
    window->menu.playButton = (MenuButton){
        .rect = { center.x, startY, buttonWidth, buttonHeight },
        .text = "Play"
    };
    window->menu.storyButton = (MenuButton){
        .rect = { center.x, startY + buttonHeight + buttonSpacing, buttonWidth, buttonHeight },
        .text = "Story Mode"
    };
    window->menu.resolutionButton = (MenuButton){
        .rect = { center.x, startY + 2 * (buttonHeight + buttonSpacing), buttonWidth, buttonHeight },
        .text = "Resolution"
    };
    window->menu.quitButton = (MenuButton){
        .rect = { center.x, startY + 3 * (buttonHeight + buttonSpacing), buttonWidth, buttonHeight },
        .text = "Quit"
    };
    window->menu.playButton.textWidth = MeasureText(window->menu.playButton.text, 30);
    window->menu.storyButton.textWidth = MeasureText(window->menu.storyButton.text, 30);
    window->menu.quitButton.textWidth = MeasureText(window->menu.quitButton.text, 30);
    window->menu.resolutionButton.textWidth = MeasureText(window->menu.resolutionButton.text, 30);
}

void DrawMenu(const WindowState* window) {
    BeginDrawing();
    ClearBackground(RAYWHITE);
    const char* title = "DINO GAME";
    int titleWidth = MeasureText(title, 60);
    DrawText(title, window->width/2 - titleWidth/2, 150, 60, DARKGRAY);
    DrawRectangleRec(window->menu.playButton.rect, window->menu.playHovered ? SKYBLUE : LIGHTGRAY);
    DrawText(window->menu.playButton.text, 
        window->menu.playButton.rect.x + (window->menu.playButton.rect.width - window->menu.playButton.textWidth)/2,
        window->menu.playButton.rect.y + 10,
        30, DARKBLUE);
    DrawRectangleRec(window->menu.storyButton.rect, window->menu.storyHovered ? SKYBLUE : LIGHTGRAY);
    DrawText(window->menu.storyButton.text, 
        window->menu.storyButton.rect.x + (window->menu.storyButton.rect.width - window->menu.storyButton.textWidth)/2,
        window->menu.storyButton.rect.y + 10,
        30, DARKBLUE);
    DrawRectangleRec(window->menu.resolutionButton.rect, window->menu.resolutionHovered ? SKYBLUE : LIGHTGRAY);
    DrawText(window->menu.resolutionButton.text,
        window->menu.resolutionButton.rect.x + (window->menu.resolutionButton.rect.width - window->menu.resolutionButton.textWidth)/2,
        window->menu.resolutionButton.rect.y + 10,
        30, DARKBLUE);
    DrawRectangleRec(window->menu.quitButton.rect, window->menu.quitHovered ? SKYBLUE : LIGHTGRAY);
    DrawText(window->menu.quitButton.text,
        window->menu.quitButton.rect.x + (window->menu.quitButton.rect.width - window->menu.quitButton.textWidth)/2,
        window->menu.quitButton.rect.y + 10,
        30, DARKBLUE);
    EndDrawing();
}

void DrawResolutionMenu(const WindowState* window) {
    BeginDrawing();
    ClearBackground(RAYWHITE);
    const char* title = "RESOLUTION SETTINGS";
    DrawText(title, window->width / 2 - MeasureText(title, 40) / 2, 100, 40, DARKGRAY);
    for (int i = 0; i < NUM_RESOLUTIONS; i++) {
        Color btnColor = IsButtonHovered(&window->resolutions[i].rect) ? SKYBLUE : LIGHTGRAY;
        DrawRectangleRec(window->resolutions[i].rect, btnColor);
        DrawRectangleLinesEx(window->resolutions[i].rect, 2, DARKGRAY);
        DrawText(window->resolutions[i].text,
                 window->resolutions[i].rect.x + (window->resolutions[i].rect.width - window->resolutions[i].textWidth) / 2,
                 window->resolutions[i].rect.y + 15, 20, BLACK);
    }
    const char* backText = "Back";
    Rectangle backButton = { window->width / 2 - 100, window->height - 100, 200, 50 };
    Color backColor = IsButtonHovered(&backButton) ? SKYBLUE : LIGHTGRAY;
    DrawRectangleRec(backButton, backColor);
    DrawRectangleLinesEx(backButton, 2, DARKGRAY);
    DrawText(backText, backButton.x + (backButton.width - MeasureText(backText, 30)) / 2, backButton.y + 10, 30, DARKBLUE);
    EndDrawing();
}

void InitPauseMenu(GameState* game, const WindowState* window) {
    const float buttonWidth = 300;
    const float buttonHeight = 50;
    const float spacing = 20;
    const float startX = window->width / 2 - buttonWidth / 2;
    const float startY = window->height / 2 - (2 * buttonHeight + spacing) / 2;
    game->pauseMenu.continueButton = (Rectangle){ startX, startY, buttonWidth, buttonHeight };
    game->pauseMenu.mainMenuButton = (Rectangle){ startX, startY + buttonHeight + spacing, buttonWidth, buttonHeight };
    game->pauseMenu.isPaused = false;
}

void HandlePauseMenuInput(WindowState* window, GameState* game) {
    game->pauseMenu.continueHovered = IsButtonHovered(&game->pauseMenu.continueButton);
    game->pauseMenu.mainMenuHovered = IsButtonHovered(&game->pauseMenu.mainMenuButton);
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        if (game->pauseMenu.continueHovered) {
            game->pauseMenu.isPaused = false;
        } else if (game->pauseMenu.mainMenuHovered) {
            game->pauseMenu.isPaused = false;
            ResetGame(game);
            window->gameState = GAME_STATE_MENU;
            window->width = GetScreenWidth();
            window->height = GetScreenHeight();
            UpdateScaleFactor(window);
        }
    }
}
