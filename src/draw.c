#include "draw.h"
#include "window.h"
#include "game.h"
#include "menu.h"
#include "utils.h"
#include "raylib.h"
#include "types.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

void DrawPauseMenu(const WindowState* window, const GameState* game) {
    DrawRectangle(0, 0, window->width, window->height, (Color){ 0, 0, 0, 150 });
    const Rectangle* buttons[] = { &game->pauseMenu.continueButton, &game->pauseMenu.mainMenuButton };
    const char* labels[] = { "Continue", "Main Menu" };
    bool hovered[] = { game->pauseMenu.continueHovered, game->pauseMenu.mainMenuHovered };
    for (int i = 0; i < 2; i++) {
        DrawRectangleRec(*buttons[i], hovered[i] ? SKYBLUE : LIGHTGRAY);
        DrawText(labels[i], buttons[i]->x + 20, buttons[i]->y + 10, 30, DARKBLUE);
    }
}

void DrawMeteors(const WindowState* window, const GameState* game, Vector2 shakeOffset) {
#if DEBUG_METEOR_COUNT
    int activeCount = 0;
    for (int i = 0; i < MAX_METEORS; i++) {
        if (game->meteors[i].active) activeCount++;
    }
    if (game->isStoryMode && game->bossActive) {
        DrawText(TextFormat("Active Meteors: %d", activeCount), 10, 150, 20, RED);
    }
#endif
    for (int i = 0; i < MAX_METEORS; i++) {
        const Meteor* meteor = &game->meteors[i];
        if (!meteor->active) continue;
        Rectangle source = {0};
        Color tint = WHITE;
        if (meteor->state == METEOR_STATE_FALLING) {
            if (meteor->currentFrame == 0) {
                source = (Rectangle){ 2158, 6, 110, 120 };
            } else {
                source = (Rectangle){ 2275, 6, 110, 120 };
            }
        } else if (meteor->state == METEOR_STATE_IMPACT) {
            if (meteor->currentFrame == 0) {
                source = (Rectangle){ 2392, 34, 103, 68 };
            } else if (meteor->currentFrame == 1) {
                source = (Rectangle){ 2497, 34, 129, 74 };
            } else {
                source = (Rectangle){ 2643, 34, 90, 51 };
            }
            if (meteor->impactTime > METEOR_GROUND_LIFETIME) {
                float alpha = 1.0f - (meteor->impactTime - METEOR_GROUND_LIFETIME) / 3.0f;
                tint = (Color){ 255, 255, 255, (unsigned char)(255 * alpha) };
            }
        } else {
            continue;
        }
        Rectangle destRect = {
            meteor->rect.x + shakeOffset.x,
            meteor->rect.y + shakeOffset.y,
            meteor->rect.width,
            meteor->rect.height
        };
        DrawTexturePro(game->spriteSheet, source, destRect, (Vector2){0}, 0, tint);
    }
}

void DrawBossHP(const WindowState* window, const GameState* game) {
    if (!game->isStoryMode || !game->bossActive) return;
    const float barWidth = 500 * window->scaleFactor;
    const float barHeight = 30 * window->scaleFactor;
    const float spacing = 5 * window->scaleFactor;
    const float segmentWidth = (barWidth - (spacing * 2)) / 100;
    const float startX = (window->width - barWidth) / 2;
    const float startY = window->height - barHeight - 10 * window->scaleFactor;
    DrawRectangle(startX, startY, barWidth, barHeight, DARKGRAY);
    for (int i = 0; i < 100; i++) {
        float segX = startX + (i * (segmentWidth + spacing));
        Color segColor;
        if (i < game->bossHP) {
            if (game->bossHP < BOSS_HP_MAX && i == game->bossHP) {
                segColor = (Color){255, 255, 0, 255};
            } else {
                segColor = RED;
            }
        } else {
            segColor = (Color){80, 80, 80, 255};
        }
        DrawRectangle(segX, startY, segmentWidth, barHeight, segColor);
    }
    DrawRectangleLinesEx((Rectangle){ startX, startY, barWidth, barHeight }, 2 * window->scaleFactor, BLACK);
    char hpText[32];
    snprintf(hpText, sizeof(hpText), "Boss HP: %d/%d", game->bossHP, BOSS_HP_MAX);
    int textWidth = MeasureText(hpText, 24);
    DrawText(hpText, startX + (barWidth - textWidth) / 2, startY - 32 * window->scaleFactor, 24 * window->scaleFactor, YELLOW);
}

Vector2 ApplyScreenShake(const GameState* game) {
    if (game->screenShakeTimer <= 0 || game->screenShakeIntensity <= 0) {
        return (Vector2){ 0, 0 };
    }
    return (Vector2){
        GetRandomValue(-game->screenShakeIntensity, game->screenShakeIntensity),
        GetRandomValue(-game->screenShakeIntensity, game->screenShakeIntensity)
    };
}

void UpdateAnimation(GameState* game, const WindowState* window, float deltaTime) {
    game->frameTime += deltaTime;
    if (game->frameTime >= FRAME_DELAY) {
        game->frameTime = 0;
        game->currentFrame ^= 1;
        const Rectangle* frame = game->isCrouching ? &game->crouchFrames[game->currentFrame] : &game->runFrames[game->currentFrame];
        game->baseSize.x = frame->width;
        game->baseSize.y = frame->height;
    }
}

void DrawGame(const WindowState* window, const GameState* game) {
    BeginDrawing();
    ClearBackground(WHITE);
    Vector2 shakeOffset = ApplyScreenShake(game);
    float groundY = (BASE_RESOLUTION.y - GROUND_HEIGHT) * window->scaleFactor;
    float groundHeight = GROUND_HEIGHT * window->scaleFactor;
    DrawRectangle(0, groundY, window->width, groundHeight, DARKGRAY);
    const Rectangle* frame = game->isCrouching ? &game->crouchFrames[game->currentFrame] : &game->runFrames[game->currentFrame];
    Rectangle destRect = {
        game->screenPosition.x + shakeOffset.x,
        game->screenPosition.y + shakeOffset.y,
        frame->width * window->scaleFactor,
        frame->height * window->scaleFactor
    };
    DrawTexturePro(game->spriteSheet, *frame, destRect, (Vector2){0}, 0, WHITE);
    for (int i = 0; i < MAX_OBSTACLES; i++) {
        const Obstacle* obs = &game->obstacles.obstacles[i];
        if (obs->active) {
            Rectangle source = {0};
            switch (obs->type) {
                case OBSTACLE_CACTUS_1: source = (Rectangle){446, 0, 34, 68}; break;
                case OBSTACLE_CACTUS_2: source = (Rectangle){480, 0, 68, 68}; break;
                case OBSTACLE_CACTUS_3: source = (Rectangle){548, 0, 102, 68}; break;
                case OBSTACLE_CACTUS_4: source = (Rectangle){650, 0, 50, 94}; break;
                case OBSTACLE_CACTUS_5: source = (Rectangle){700, 0, 100, 94}; break;
                case OBSTACLE_CACTUS_6: source = (Rectangle){800, 0, 150, 95}; break;
                case OBSTACLE_BIRD: source = (Rectangle){260 + obs->currentFrame * 93, 0, 93, 80}; break;
            }
            Rectangle destRect = {
                obs->rect.x + shakeOffset.x,
                obs->rect.y + shakeOffset.y,
                obs->rect.width,
                obs->rect.height
            };
            DrawTexturePro(game->spriteSheet, source, destRect, (Vector2){0}, 0, WHITE);
        }
    }
    if (game->isStoryMode && game->bossActive) {
        DrawMeteors(window, game, shakeOffset);
    }
    if (game->nightModeActive && game->nightAlpha > 0) {
        BeginTextureMode(game->lightMask);
        ClearBackground(BLANK);
        DrawRectangle(0, 0, window->width, window->height, 
                      (Color){0, 0, 0, (unsigned char)(game->nightAlpha * NIGHT_ALPHA)});
        Vector2 playerCenter = {
            game->screenPosition.x + game->rect.width / 2,
            game->screenPosition.y + game->rect.height / 2
        };
        DrawCircleGradient(
            (int)playerCenter.x,
            (int)playerCenter.y,
            (LIGHT_RADIUS + FADE_DISTANCE) * window->scaleFactor,
            (Color){0, 0, 0, 0},
            (Color){0, 0, 0, (unsigned char)(game->nightAlpha * NIGHT_ALPHA)}
        );
        EndTextureMode();
        DrawTextureRec(game->lightMask.texture, 
                       (Rectangle){ 0, 0, window->width, -window->height }, 
                       (Vector2){ 0, 0 }, 
                       WHITE);
    }
    if (game->isStoryMode && game->bossActive) {
        DrawBossHP(window, game);
    }
    if (game->gameOver) {
        const char* text = "GAME OVER - Press SPACE to restart";
        int textWidth = MeasureText(text, 40);
        DrawText(text, (window->width - textWidth) / 2, window->height / 2, 40, RED);
    }
    const char* scoreText = TextFormat("SCORE: %d", game->score);
    const char* highScoreText = TextFormat("HIGH SCORE: %d", game->highScore);
    int scoreWidth = MeasureText(scoreText, 40);
    int highScoreWidth = MeasureText(highScoreText, 30);
    DrawText(scoreText, (window->width - scoreWidth) / 2, 20, 40, BLACK);
    DrawText(highScoreText, (window->width - highScoreWidth) / 2, 70, 30, DARKGRAY);
    if (game->score == game->highScore && game->score > 0) {
        DrawText("NEW HIGH SCORE!", (window->width - MeasureText("NEW HIGH SCORE!", 30)) / 2, 110, 30, GREEN);
    }
    if (game->pauseMenu.isPaused) {
        DrawPauseMenu(window, game);
    }

    char hpText[32];
    snprintf(hpText, sizeof(hpText), "HP: %d", game->hp);
    DrawText(hpText, 20, 20, 24, RED);

    EndDrawing();
}
