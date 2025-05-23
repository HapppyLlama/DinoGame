#include "game.h"
#include "utils.h"
#include "raylib.h"
#include "menu.h"
#include "sound.h"
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>

void ResetGame(GameState* game) {
    if (game->score > game->highScore) {
        game->highScore = game->score;
        SaveHighScore(game->highScore);
    }
    game->basePosition.y = BASE_RESOLUTION.y - GROUND_HEIGHT - game->runFrameHeight;
    game->isJumping = false;
    game->score = 0;
    game->gameOver = false;
    game->gameWon = false;
    game->soundPlayed = false;

    for (int i = 0; i < MAX_CLOUDS; i++) {
        game->clouds[i].position.x = (BASE_RESOLUTION.x / MAX_CLOUDS) * i + GetRandomValue(-50, 50);
        game->clouds[i].position.y = GetRandomValue(20, 120);
        game->clouds[i].speed = CLOUD_MIN_SPEED + GetRandomValue(0, 100) / 100.0f * (CLOUD_MAX_SPEED - CLOUD_MIN_SPEED);
        game->clouds[i].scale = 0.5f + GetRandomValue(0, 100) / 100.0f * 0.3f;
        game->clouds[i].alpha = CLOUD_MIN_ALPHA + GetRandomValue(0, 100) / 100.0f * (CLOUD_MAX_ALPHA - CLOUD_MIN_ALPHA);
        game->clouds[i].active = true;
    }

    for (int i = 0; i < MAX_OBSTACLES; i++) {
        game->obstacles.obstacles[i].active = false;
    }
    game->nightModeActive = false;

    game->bossActive = false;
    game->bossHP = 10;
    game->screenShakeTimer = 0.0f;
    game->screenShakeIntensity = 0.0f;

    InitMeteors(game);
}

void InitObstacles(GameState* state) {
    for (int i = 0; i < MAX_OBSTACLES; i++) {
        state->obstacles.obstacles[i].active = false;
    }
    state->obstacles.spawnTimer = 0.0f;
    state->obstacles.nextSpawnTime = GetRandomValue(MIN_SPAWN_INTERVAL, MAX_SPAWN_INTERVAL);
}

void InitMeteors(GameState* state) {
    for (int i = 0; i < MAX_METEORS; i++) {
        state->meteors[i].active = false;
        state->meteors[i].state = METEOR_STATE_INACTIVE;
        state->meteors[i].hasDealtDamage = false;
    }
    state->meteorSpawnTimer = 0.0f;
    state->nextMeteorSpawnTime = GetRandomValue(METEOR_SPAWN_INTERVAL_MIN * 100, 
                                            METEOR_SPAWN_INTERVAL_MAX * 100) / 100.0f;
}

void InitClouds(GameState* game) {
    game->cloudTexture = LoadTexture("resources/clouds.png");

    for (int i = 0; i < MAX_CLOUDS; i++) {
        game->clouds[i].active = false;
    }

    game->cloudSpawnTimer = 0.0f;
    game->nextCloudSpawnTime = 3.0f;

    for (int i = 0; i < MAX_CLOUDS; i++) {
        SpawnCloudAt(game, i, GetRandomValue(0, BASE_RESOLUTION.x));
    }
}

void SpawnCloudAt(GameState* game, int index, float xPosition) {
    if (index >= 0 && index < MAX_CLOUDS) {
        game->clouds[index].position.x = xPosition;
        game->clouds[index].position.y = GetRandomValue(20, 120);
        game->clouds[index].speed = CLOUD_MIN_SPEED + GetRandomValue(0, 100) / 100.0f * (CLOUD_MAX_SPEED - CLOUD_MIN_SPEED);
        game->clouds[index].scale = 0.5f + GetRandomValue(0, 100) / 100.0f * 0.3f;
        game->clouds[index].alpha = CLOUD_MIN_ALPHA + GetRandomValue(0, 100) / 100.0f * (CLOUD_MAX_ALPHA - CLOUD_MIN_ALPHA);
        game->clouds[index].active = true;
    }
}

void SpawnObstacle(GameState* state, const WindowState* window) {
    for (int i = 0; i < MAX_OBSTACLES; i++) {
        if (!state->obstacles.obstacles[i].active) {
            Obstacle* obs = &state->obstacles.obstacles[i];
            obs->active = true;
            
            ObstacleType randomType = GetRandomValue(0, OBSTACLE_BIRD);
            obs->type = randomType;
            
            const ObstacleDimensions dims = CACTUS_DIMENSIONS[randomType];
            
            float groundY = window->height - GROUND_HEIGHT * window->scaleFactor;
            float yPos = groundY - dims.height * window->scaleFactor + dims.yOffset * window->scaleFactor;

            obs->rect = (Rectangle){
                window->width,
                yPos,
                dims.width * window->scaleFactor,
                dims.height * window->scaleFactor
            };

            obs->collisionRect = (Rectangle){
                obs->rect.x + COLLISION_OFFSET * window->scaleFactor,
                obs->rect.y + COLLISION_OFFSET * window->scaleFactor,
                obs->rect.width - 2 * COLLISION_OFFSET * window->scaleFactor,
                obs->rect.height - 2 * COLLISION_OFFSET * window->scaleFactor
            };

            if (obs->type == OBSTACLE_BIRD) {
                obs->currentFrame = 0;
                obs->frameTime = 0;
            }
            obs->hasPassedPlayer = false;
            break;
        }
    }
}

void SpawnMeteor(GameState* game, const WindowState* window) {
    for (int i = 0; i < MAX_METEORS; i++) {
        if (!game->meteors[i].active) {
            Meteor* meteor = &game->meteors[i];
            meteor->active = true;
            meteor->state = METEOR_STATE_FALLING;
            meteor->currentFrame = 0;
            meteor->frameTime = 0;
            meteor->impactTime = 0;
            meteor->hasDealtDamage = false; 
            
            int spawnPattern = GetRandomValue(0, 2);
            switch (spawnPattern) {
                case 0:
                    meteor->position.x = window->width + GetRandomValue(50, 250);
                    meteor->position.y = -GetRandomValue(100, 300);
                    break;
                case 1:
                    meteor->position.x = window->width * 0.7f + GetRandomValue(-100, 100);
                    meteor->position.y = -GetRandomValue(200, 400);
                    break;
                case 2:
                    meteor->position.x = window->width + GetRandomValue(50, 150);
                    meteor->position.y = -GetRandomValue(50, 150);
                    break;
            }
            float sizeVariation = GetRandomValue(100, 160);
            meteor->rect = (Rectangle){
                meteor->position.x,
                meteor->position.y,
                sizeVariation,
                sizeVariation
            };
            meteor->collisionRect = (Rectangle){ 0, 0, 0, 0 };
            return;
        }
    }
}

void UpdateMeteors(GameState* game, const WindowState* window, float deltaTime) {
    if (!game->isStoryMode || !game->bossActive || game->gameOver || game->gameWon) {
        return;
    }
    game->meteorSpawnTimer += deltaTime;
    if (game->meteorSpawnTimer >= game->nextMeteorSpawnTime) {
        SpawnMeteor(game, window);
        game->meteorSpawnTimer = 0.0f;
        game->nextMeteorSpawnTime = METEOR_SPAWN_INTERVAL_MIN + 
            (METEOR_SPAWN_INTERVAL_MAX - METEOR_SPAWN_INTERVAL_MIN) * GetRandomValue(0, 100) / 100.0f;
    }
    float groundY = window->height - GROUND_HEIGHT * window->scaleFactor;
    bool bossJustDefeated = false;
    for (int i = 0; i < MAX_METEORS; i++) {
        Meteor* meteor = &game->meteors[i];
        if (!meteor->active) continue;
        meteor->frameTime += deltaTime;
        if (meteor->state == METEOR_STATE_FALLING) {
            meteor->position.x -= METEOR_FALL_SPEED_X * deltaTime;
            meteor->position.y += METEOR_FALL_SPEED_Y * deltaTime;
            meteor->rect.x = meteor->position.x;
            meteor->rect.y = meteor->position.y;
            if (meteor->position.y >= groundY - meteor->rect.height) {
                meteor->state = METEOR_STATE_IMPACT;
                meteor->position.y = groundY - meteor->rect.height;
                meteor->rect.y = meteor->position.y;
                meteor->currentFrame = 0;
                meteor->frameTime = 0;
                game->screenShakeTimer = 0.3f;
                game->screenShakeIntensity = 8.0f;
                meteor->collisionRect = (Rectangle){
                    meteor->position.x + 10,
                    meteor->position.y + meteor->rect.height - 25,
                    meteor->rect.width - 20,
                    25
                };

                PlayMeteorImpactSound(game);
            }
        } else if (meteor->state == METEOR_STATE_IMPACT) {
            meteor->position.x -= OBSTACLE_SPEED * window->scaleFactor * deltaTime;
            meteor->rect.x = meteor->position.x;
            meteor->collisionRect.x = meteor->position.x + 10;
            if (!meteor->hasDealtDamage && (meteor->position.x + meteor->rect.width) < game->basePosition.x) {
                if (game->bossHP > 0) {
                    game->bossHP--;
                    if (game->bossHP <= 0 && !game->gameWon) {
                        game->bossHP = 0;
                        bossJustDefeated = true;
                    }
                }
                meteor->hasDealtDamage = true;
            }
            if (game->bossHP < 0) game->bossHP = 0;
            if (meteor->impactTime < 1.0f) {
                if (meteor->frameTime >= METEOR_ANIM_DELAY * 1.5f) {
                    meteor->frameTime = 0;
                    if (meteor->currentFrame < METEOR_IMPACT_FRAMES - 1) {
                        meteor->currentFrame++;
                    }
                }
            }
            meteor->impactTime += deltaTime;
            if (meteor->impactTime > METEOR_GROUND_LIFETIME) {
                if (meteor->impactTime > METEOR_GROUND_LIFETIME + 3.0f) {
                    meteor->active = false;
                }
            }
            if (CheckCollisionRecs(game->rect, meteor->collisionRect)) {
                if (game->bossHP > 1) {
                    game->gameOver = true;
                    break;
                } else if (game->bossHP == 1 && !meteor->hasDealtDamage) {
                    game->bossHP = 0;
                    meteor->hasDealtDamage = true;
                    bossJustDefeated = true;
                }
            }
        }
    }
    if (bossJustDefeated && !game->gameWon) {
        game->gameWon = true;
        game->gameOver = true;
    }
}

void UpdateObstacles(GameState* state, const WindowState* window, float deltaTime) {
    if (state->gameOver || state->gameWon) return;
    if (!(state->isStoryMode && state->score >= PRE_BOSS_THRESHOLD)) {
        state->obstacles.spawnTimer += deltaTime;
        if (state->obstacles.spawnTimer >= state->obstacles.nextSpawnTime) {
            SpawnObstacle(state, window);
            state->obstacles.spawnTimer = 0.0f;
            state->obstacles.nextSpawnTime = GetRandomValue(MIN_SPAWN_INTERVAL, MAX_SPAWN_INTERVAL);
        }
    }
    for (int i = 0; i < MAX_OBSTACLES; i++) {
        Obstacle* obs = &state->obstacles.obstacles[i];
        if (obs->active) {
            obs->rect.x -= OBSTACLE_SPEED * window->scaleFactor * deltaTime;
            obs->collisionRect.x = obs->rect.x + COLLISION_OFFSET * window->scaleFactor;
            if (obs->type == OBSTACLE_BIRD) {
                obs->frameTime += deltaTime;
                if (obs->frameTime >= BIRD_ANIM_DELAY) {
                    obs->frameTime = 0;
                    obs->currentFrame ^= 1;
                }
            }
            if (obs->rect.x + obs->rect.width < 0) {
                obs->active = false;
            }
            if (CheckCollisionRecs(state->rect, obs->collisionRect)) {
                state->gameOver = true;
                break;
            }
        }
    }
}

void SpawnCloud(GameState* game, const WindowState* window) {
    for (int i = 0; i < MAX_CLOUDS; i++) {
        if (!game->clouds[i].active) {
            float xPos;
            if (window) {
                xPos = window->width;
            } else {
                xPos = BASE_RESOLUTION.x * (1.0f + GetRandomValue(0, 100) / 100.0f);
            }
            
            game->clouds[i].position.x = xPos;
            game->clouds[i].position.y = GetRandomValue(20, 120);
            game->clouds[i].speed = CLOUD_MIN_SPEED + GetRandomValue(0, 100) / 100.0f * (CLOUD_MAX_SPEED - CLOUD_MIN_SPEED);
            game->clouds[i].scale = 0.5f + GetRandomValue(0, 100) / 100.0f * 0.3f;
            game->clouds[i].alpha = CLOUD_MIN_ALPHA + GetRandomValue(0, 100) / 100.0f * (CLOUD_MAX_ALPHA - CLOUD_MIN_ALPHA);
            game->clouds[i].active = true;
            return;
        }
    }
}

void UpdateClouds(GameState* game, const WindowState* window, float deltaTime) {
    for (int i = 0; i < MAX_CLOUDS; i++) {
        if (game->clouds[i].active) {
            game->clouds[i].position.x -= game->clouds[i].speed * deltaTime;

            if (game->clouds[i].position.x + game->cloudTexture.width * game->clouds[i].scale < -100) {
                game->clouds[i].position.x = BASE_RESOLUTION.x;
                game->clouds[i].position.y = GetRandomValue(20, 120);
                game->clouds[i].speed = CLOUD_MIN_SPEED + GetRandomValue(0, 100) / 100.0f * (CLOUD_MAX_SPEED - CLOUD_MIN_SPEED);
                game->clouds[i].scale = 0.5f + GetRandomValue(0, 100) / 100.0f * 0.3f;
                game->clouds[i].alpha = CLOUD_MIN_ALPHA + GetRandomValue(0, 100) / 100.0f * (CLOUD_MAX_ALPHA - CLOUD_MIN_ALPHA);
            }
        }
    }
}

void UpdateBossFight(GameState* game, WindowState* window, float deltaTime) {
    if (!game->isStoryMode || game->gameOver) return;
    if (!game->bossActive && game->score >= BOSS_THRESHOLD_SCORE) {
        game->bossActive = true;
        game->screenShakeTimer = SCREEN_SHAKE_DURATION;
        game->screenShakeIntensity = SCREEN_SHAKE_INTENSITY;
        game->meteorSpawnTimer = 0.0f;
        game->nextMeteorSpawnTime = 0.3f;
    }
    if (game->screenShakeTimer > 0) {
        game->screenShakeTimer -= deltaTime;
        if (game->screenShakeTimer <= 0) {
            game->screenShakeIntensity = 0.0f;
        }
    }
    if (game->bossActive) {
        UpdateMeteors(game, window, deltaTime);
    }
}

void InitGameState(GameState* state) {
    state->spriteSheet = LoadTexture("resources/sprite.png");
    state->runFrames[0] = (Rectangle){ 1514, -4, 88, 94 };
    state->runFrames[1] = (Rectangle){ 1602, -4, 88, 94 };
    state->crouchFrames[0] = (Rectangle){ 1866, 34, 118, 60 };
    state->crouchFrames[1] = (Rectangle){ 1984, 34, 118, 60 };
    state->runFrameHeight = 94;
    state->crouchFrameHeight = 60;
    state->baseSize = (Vector2){88, 94};
    state->currentFrame = 0;
    state->frameTime = 0;
    state->basePosition = (Vector2){BASE_RESOLUTION.x * 0.1f, BASE_RESOLUTION.y - GROUND_HEIGHT - state->runFrameHeight};
    state->isJumpCharging = false;
    state->jumpChargeTime = 0.0f;
    state->score = 0;
    state->scoreTimer = 0.0f;
    InitObstacles(state);
    state->gameOver = false;
    state->highScore = LoadHighScore();
    state->nightModeActive = false;
    state->nightCycleTimer = 0.0f;
    state->isNight = false;
    state->dayCycleTimer = 0.0f;
    state->lightMask = LoadRenderTexture(BASE_RESOLUTION.x, BASE_RESOLUTION.y);
    state->nightAlpha = 0.0f;
    state->isStoryMode = false;
    state->bossActive = false;
    state->bossHP = 10;
    state->screenShakeTimer = 0.0f;
    state->screenShakeIntensity = 0.0f;
    state->hp = 100;
    state->gameWon = false;
    state->soundPlayed = false;
    InitMeteors(state);
    InitSounds(state);
    InitClouds(state);
    InitPauseMenu(state, &(WindowState){ .width = BASE_RESOLUTION.x, .height = BASE_RESOLUTION.y });
}

void UpdateScore(GameState* game, float deltaTime) {
    game->scoreTimer += deltaTime;
    if (game->scoreTimer >= 0.1f) {
        game->score++;
        game->scoreTimer = 0.0f;
        if (game->score > game->highScore) game->highScore = game->score;
    }
}

void UpdatePhysics(GameState* game, const WindowState* window, float deltaTime) {
    if (game->isJumping) {
        if (game->isJumpCharging && game->jumpChargeTime < MAX_JUMP_CHARGE_TIME) {
            game->jumpChargeTime += deltaTime;
            float chargeProgress = game->jumpChargeTime / MAX_JUMP_CHARGE_TIME;
            float additionalForce = JUMP_CHARGE_FORCE * (1.0f - chargeProgress);
            game->baseJumpVelocity += additionalForce * deltaTime * 60.0f;
        }
        game->baseJumpVelocity += GRAVITY;
        game->basePosition.y += game->baseJumpVelocity;
        float baseGroundLevel = BASE_RESOLUTION.y - GROUND_HEIGHT - 
            (game->isCrouching ? game->crouchFrameHeight : game->runFrameHeight);
        if (game->basePosition.y >= baseGroundLevel) {
            game->basePosition.y = baseGroundLevel;
            game->isJumping = false;
            game->baseJumpVelocity = 0.0f;
        }
    }
    game->screenPosition.x = game->basePosition.x * window->scaleFactor;
    game->screenPosition.y = game->basePosition.y * window->scaleFactor;
    game->rect.x = game->screenPosition.x;
    game->rect.y = game->screenPosition.y;
}

