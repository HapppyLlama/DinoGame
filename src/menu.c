#include "menu.h"
#include "window.h"
#include "game.h"
#include "raylib.h"
#include "utils.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>


void InitMenuButtons(WindowState* window) {
    float scale = window->scaleFactor > 0 ? window->scaleFactor : 1.0f;
    const float buttonWidth = 200 * scale;
    const float buttonHeight = 50 * scale;
    const float buttonSpacing = 20 * scale;
    Vector2 center = { window->width/2 - buttonWidth/2, window->height * 0.40f };
    float startY = center.y;
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
    float textSize = 30 * scale;
    window->menu.playButton.textWidth = MeasureText(window->menu.playButton.text, textSize);
    window->menu.storyButton.textWidth = MeasureText(window->menu.storyButton.text, textSize);
    window->menu.quitButton.textWidth = MeasureText(window->menu.quitButton.text, textSize);
    window->menu.resolutionButton.textWidth = MeasureText(window->menu.resolutionButton.text, textSize);
}

void DrawMenu(const WindowState* window) {
    BeginDrawing();
    ClearBackground(RAYWHITE);
    const char* title = "DINO GAME";
    int titleWidth = MeasureText(title, 60 * window->scaleFactor);
    DrawText(title, window->width/2 - titleWidth/2, 150 * window->scaleFactor, 60 * window->scaleFactor, DARKGRAY);
    float textSize = 30 * window->scaleFactor;
    DrawRectangleRec(window->menu.playButton.rect, window->menu.playHovered ? SKYBLUE : LIGHTGRAY);
    DrawText(window->menu.playButton.text, 
        window->menu.playButton.rect.x + (window->menu.playButton.rect.width - window->menu.playButton.textWidth)/2,
        window->menu.playButton.rect.y + 10 * window->scaleFactor,
        textSize, DARKBLUE);
    DrawRectangleRec(window->menu.storyButton.rect, window->menu.storyHovered ? SKYBLUE : LIGHTGRAY);
    DrawText(window->menu.storyButton.text, 
        window->menu.storyButton.rect.x + (window->menu.storyButton.rect.width - window->menu.storyButton.textWidth)/2,
        window->menu.storyButton.rect.y + 10 * window->scaleFactor,
        textSize, DARKBLUE);
    DrawRectangleRec(window->menu.resolutionButton.rect, window->menu.resolutionHovered ? SKYBLUE : LIGHTGRAY);
    DrawText(window->menu.resolutionButton.text,
        window->menu.resolutionButton.rect.x + (window->menu.resolutionButton.rect.width - window->menu.resolutionButton.textWidth)/2,
        window->menu.resolutionButton.rect.y + 10 * window->scaleFactor,
        textSize, DARKBLUE);
    DrawRectangleRec(window->menu.quitButton.rect, window->menu.quitHovered ? SKYBLUE : LIGHTGRAY);
    DrawText(window->menu.quitButton.text,
        window->menu.quitButton.rect.x + (window->menu.quitButton.rect.width - window->menu.quitButton.textWidth)/2,
        window->menu.quitButton.rect.y + 10 * window->scaleFactor,
        textSize, DARKBLUE);
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
    float scale = window->scaleFactor > 0 ? window->scaleFactor : 1.0f;
    const float buttonWidth = 200 * scale;
    const float buttonHeight = 50 * scale;
    const float spacing = 20 * scale;
    float totalHeight = 2 * buttonHeight + spacing;
    // Center vertically and horizontally
    float startX = (window->width - buttonWidth) / 2;
    float startY = (window->height - totalHeight) / 2;
    game->pauseMenu.continueButton = (Rectangle){ startX, startY, buttonWidth, buttonHeight };
    game->pauseMenu.mainMenuButton = (Rectangle){ startX, startY + buttonHeight + spacing, buttonWidth, buttonHeight };
    game->pauseMenu.isPaused = false;
}

void HandlePauseMenuInput(WindowState* window, GameState* game) {
    float scale = window->scaleFactor > 0 ? window->scaleFactor : 1.0f;
    const float buttonWidth = 200 * scale;
    const float buttonHeight = 50 * scale;
    const float spacing = 20 * scale;
    float totalHeight = 2 * buttonHeight + spacing;
    float startX = (window->width - buttonWidth) / 2;
    float startY = (window->height - totalHeight) / 2;

    Rectangle continueButton = { startX, startY, buttonWidth, buttonHeight };
    Rectangle mainMenuButton = { startX, startY + buttonHeight + spacing, buttonWidth, buttonHeight };

    game->pauseMenu.continueHovered = IsButtonHovered(&continueButton);
    game->pauseMenu.mainMenuHovered = IsButtonHovered(&mainMenuButton);

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

void DrawPauseMenu(const WindowState* window, const GameState* game) {
    float scale = window->scaleFactor > 0 ? window->scaleFactor : 1.0f;
    float textSize = 30 * scale;
    DrawRectangle(0, 0, window->width, window->height, (Color){ 0, 0, 0, 150 });

    const float buttonWidth = 200 * scale;
    const float buttonHeight = 50 * scale;
    const float spacing = 20 * scale;
    float totalHeight = 2 * buttonHeight + spacing;
    float startX = (window->width - buttonWidth) / 2;
    float startY = (window->height - totalHeight) / 2;

    Rectangle continueButton = { startX, startY, buttonWidth, buttonHeight };
    Rectangle mainMenuButton = { startX, startY + buttonHeight + spacing, buttonWidth, buttonHeight };

    const Rectangle* buttons[] = { &continueButton, &mainMenuButton };
    const char* labels[] = { "Continue", "Main Menu" };
    bool hovered[] = { game->pauseMenu.continueHovered, game->pauseMenu.mainMenuHovered };
    for (int i = 0; i < 2; i++) {
        DrawRectangleRec(*buttons[i], hovered[i] ? SKYBLUE : LIGHTGRAY);
        int textWidth = MeasureText(labels[i], textSize);
        DrawText(labels[i], buttons[i]->x + (buttons[i]->width - textWidth) / 2,
            buttons[i]->y + 10 * scale, textSize, DARKBLUE);
    }
}
