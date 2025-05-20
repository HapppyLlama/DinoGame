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
    const float barHeight = 36 * window->scaleFactor;
    const float borderRadius = 18 * window->scaleFactor;
    const float startX = (window->width - barWidth) / 2;
    const float startY = window->height - barHeight - 30 * window->scaleFactor;
    DrawRectangleRounded((Rectangle){ startX, startY, barWidth, barHeight }, 0.5f, 32, (Color){40, 40, 40, 220});
    float hpPercent = (float)game->bossHP / 10.0f;
    float fillWidth = barWidth * hpPercent;
    Color fillColor = (hpPercent > 0.5f) ? (Color){ 0, 220, 40, 255 } : (hpPercent > 0.2f ? ORANGE : RED);
    DrawRectangleRounded((Rectangle){ startX, startY, fillWidth, barHeight }, 0.5f, 32, fillColor);
    DrawRectangleRoundedLines((Rectangle){ startX, startY, barWidth, barHeight }, 0.5f, 32, BLACK);
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

void DrawClouds(const WindowState* window, const GameState* game) {
    for (int i = 0; i < MAX_CLOUDS; i++) {
        if (!game->clouds[i].active) continue;
        
        const Cloud* cloud = &game->clouds[i];
        
        Vector2 scaledPos = {
            cloud->position.x * window->scaleFactor,
            cloud->position.y * window->scaleFactor
        };
        
        DrawTextureEx(
            game->cloudTexture, 
            scaledPos,
            0,
            cloud->scale * window->scaleFactor,
            Fade(WHITE, cloud->alpha)
        );
    }
}

void DrawGame(const WindowState* window, const GameState* game) {
    BeginDrawing();
    ClearBackground(WHITE);
    
    DrawClouds(window, game);
    
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
    if (game->gameOver && !game->gameWon) {
        const char* text = "GAME OVER - Press SPACE to restart";
        int textWidth = MeasureText(text, 40);
        DrawText(text, (window->width - textWidth) / 2, window->height / 2, 40, RED);
    } else if (game->gameWon) {
        ClearBackground(BLACK);
        const char* winText = "YOU WON!";
        int fontSize = 100 * window->scaleFactor;
        int textWidth = MeasureText(winText, fontSize);
        DrawText(winText, (window->width - textWidth) / 2, window->height / 2 - fontSize / 2, fontSize, YELLOW);
        const char* info = "Press SPACE to return to menu";
        int infoWidth = MeasureText(info, 30);
        DrawText(info, (window->width - infoWidth) / 2, window->height / 2 + fontSize / 2 + 20, 30, LIGHTGRAY);
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

    if (game->isStoryMode && game->bossActive && game->bossHP <= 0) {
        ClearBackground(BLACK);
        const char* winText = "YOU WON!";
        int fontSize = 100 * window->scaleFactor;
        int textWidth = MeasureText(winText, fontSize);
        DrawText(winText, (window->width - textWidth) / 2, window->height / 2 - fontSize / 2, fontSize, YELLOW);
        const char* info = "Press SPACE to return to menu";
        int infoWidth = MeasureText(info, 30);
        DrawText(info, (window->width - infoWidth) / 2, window->height / 2 + fontSize / 2 + 20, 30, LIGHTGRAY);
    } else if (game->gameWon) {
        ClearBackground(BLACK);
        const char* winText = "YOU WON!";
        int fontSize = 100 * window->scaleFactor;
        int textWidth = MeasureText(winText, fontSize);
        DrawText(winText, (window->width - textWidth) / 2, window->height / 2 - fontSize / 2, fontSize, YELLOW);
        const char* info = "Press SPACE to return to menu";
        int infoWidth = MeasureText(info, 30);
        DrawText(info, (window->width - infoWidth) / 2, window->height / 2 + fontSize / 2 + 20, 30, LIGHTGRAY);
    }

    EndDrawing();
}
