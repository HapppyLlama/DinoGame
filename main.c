#include "raylib.h"
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>

#define NUM_RESOLUTIONS 4
#define GROUND_HEIGHT 100
#define FRAME_DELAY 0.15f
#define GRAVITY 1.8f
#define JUMP_FORCE -5.0f 
#define MAX_JUMP_CHARGE_TIME 0.3f 
#define JUMP_CHARGE_FORCE -4.5f
#define FAST_FALL_VELOCITY 20.0f

#define MAX_OBSTACLES 7
#define MIN_SPAWN_INTERVAL 1.5f
#define MAX_SPAWN_INTERVAL 2.0f
#define OBSTACLE_SPEED 700.0f
#define BIRD_ANIM_DELAY 0.2f

#define COLLISION_OFFSET 3

#define DAY_DURATION 20.0f
#define NIGHT_DURATION 10.0f
#define FADE_DURATION 0.2f
#define LIGHT_RADIUS 400.0f
#define NIGHT_ALPHA 255
#define FADE_DISTANCE 100.0f 

#define BOSS_THRESHOLD_SCORE 100
#define BOSS_HP_MAX 3
#define SCREEN_SHAKE_DURATION 1.0f
#define SCREEN_SHAKE_INTENSITY 10.0f

#define PRE_BOSS_THRESHOLD (BOSS_THRESHOLD_SCORE - 50)

#define MAX_METEORS 15
#define METEOR_SPAWN_INTERVAL_MIN 0.5f
#define METEOR_SPAWN_INTERVAL_MAX 1.5f
#define METEOR_FALL_SPEED_X 700.0f
#define METEOR_FALL_SPEED_Y 1000.0f
#define METEOR_ANIM_DELAY 0.08f
#define METEOR_GROUND_LIFETIME 10.0f

#define METEOR_IMPACT_FRAMES 3

typedef enum {
    GAME_STATE_MENU,
    GAME_STATE_PLAYING,
    GAME_STATE_GAME_OVER,
    GAME_STATE_RESOLUTION
} GameStates;

typedef struct {
    Rectangle rect;
    const char* text;
    int textWidth;
} MenuButton;

typedef struct {
    MenuButton playButton;
    MenuButton storyButton;
    MenuButton quitButton;
    MenuButton resolutionButton;
    bool playHovered;
    bool storyHovered;
    bool quitHovered;
    bool resolutionHovered;
} MenuState;

typedef struct {
    Rectangle rect;
    const char* text;
    int textWidth;
    int width;
    int height;
} ResolutionButton;

typedef enum {
    OBSTACLE_CACTUS_1,
    OBSTACLE_CACTUS_2,
    OBSTACLE_CACTUS_3,
    OBSTACLE_CACTUS_4,
    OBSTACLE_CACTUS_5,
    OBSTACLE_CACTUS_6,
    OBSTACLE_BIRD
} ObstacleType;

typedef struct {
    float width;
    float height;
    float yOffset;
} ObstacleDimensions;

typedef struct {
    Rectangle rect;
    Rectangle collisionRect;
    bool active;
    ObstacleType type;
    int currentFrame;
    float frameTime;
} Obstacle;

typedef struct {
    Obstacle obstacles[MAX_OBSTACLES];
    float spawnTimer;
    float nextSpawnTime;
} ObstaclePool;

typedef struct {
    Rectangle continueButton;
    Rectangle mainMenuButton;
    bool continueHovered;
    bool mainMenuHovered;
    bool isPaused;
} PauseMenuState;

typedef enum {
    METEOR_STATE_FALLING,
    METEOR_STATE_IMPACT,
    METEOR_STATE_INACTIVE
} MeteorState;

typedef struct {
    Vector2 position;
    Rectangle rect;
    Rectangle collisionRect;
    MeteorState state;
    int currentFrame;
    float frameTime;
    float impactTime;
    bool active;
} Meteor;

typedef struct {
    Rectangle rect;
    Vector2 screenPosition;
    Vector2 basePosition;
    Vector2 baseSize;
    float baseJumpVelocity;
    bool isJumping;
    bool isCrouching;
    bool isJumpCharging;
    float jumpChargeTime;
    
    Texture2D spriteSheet;
    Rectangle runFrames[2];
    Rectangle crouchFrames[2];
    int currentFrame;
    float frameTime;
    float runFrameHeight;
    float crouchFrameHeight;
    int score;
    int highScore;
    float scoreTimer;

    ObstaclePool obstacles;
    bool gameOver;

    bool nightModeActive;
    float nightCycleTimer;
    bool isNight;
    float dayCycleTimer;
    RenderTexture2D lightMask;
    float nightAlpha;

    PauseMenuState pauseMenu;

    bool isStoryMode;
    bool bossActive;
    int bossHP;
    float screenShakeTimer;
    float screenShakeIntensity;

    Meteor meteors[MAX_METEORS];
    float meteorSpawnTimer;
    float nextMeteorSpawnTime;
} GameState;

typedef struct {
    int width;
    int height;
    bool isFullscreen;
    float scaleFactor;
    ResolutionButton resolutions[NUM_RESOLUTIONS];
    MenuState menu;
    GameStates gameState;
} WindowState;

static const Vector2 BASE_RESOLUTION = { 1600, 900 };

static const ObstacleDimensions CACTUS_DIMENSIONS[] = {
    [OBSTACLE_CACTUS_1] = {.width = 34,  .height = 68, .yOffset = 0},
    [OBSTACLE_CACTUS_2] = {.width = 68,  .height = 68, .yOffset = 0},
    [OBSTACLE_CACTUS_3] = {.width = 102, .height = 68, .yOffset = 0},
    [OBSTACLE_CACTUS_4] = {.width = 50,  .height = 94, .yOffset = 0},
    [OBSTACLE_CACTUS_5] = {.width = 100, .height = 94, .yOffset = 0},
    [OBSTACLE_CACTUS_6] = {.width = 150, .height = 95, .yOffset = 0},
    [OBSTACLE_BIRD]     = {.width = 93,  .height = 80,  .yOffset = -61}
};

void InitMeteors(GameState* state);
void ChangeResolution(WindowState* window, GameState* game, int width, int height, bool fullscreen);

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

void ResetGame(GameState* game) {
    if (game->score > game->highScore) {
        game->highScore = game->score;
        SaveHighScore(game->highScore);
    }
    game->basePosition.y = BASE_RESOLUTION.y - GROUND_HEIGHT - game->runFrameHeight;
    game->isJumping = false;
    game->score = 0;
    game->gameOver = false;

    for (int i = 0; i < MAX_OBSTACLES; i++) {
        game->obstacles.obstacles[i].active = false;
    }
    game->nightModeActive = false;

    game->bossActive = false;
    game->bossHP = BOSS_HP_MAX;
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
    }
    state->meteorSpawnTimer = 0.0f;
    state->nextMeteorSpawnTime = GetRandomValue(METEOR_SPAWN_INTERVAL_MIN * 100, 
                                            METEOR_SPAWN_INTERVAL_MAX * 100) / 100.0f;
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
            
            float sizeVariation = GetRandomValue(70, 110);
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
    if (!game->isStoryMode || !game->bossActive || game->gameOver) {
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
    
    for (int i = 0; i < MAX_METEORS; i++) {
        Meteor* meteor = &game->meteors[i];
        if (!meteor->active) continue;
        
        meteor->frameTime += deltaTime;
        
        if (meteor->state == METEOR_STATE_FALLING) {
            if (meteor->frameTime >= METEOR_ANIM_DELAY) {
                meteor->frameTime = 0;
                meteor->currentFrame = (meteor->currentFrame + 1) % 2;
            }
            
            meteor->position.x -= METEOR_FALL_SPEED_X * deltaTime;
            meteor->position.y += METEOR_FALL_SPEED_Y * deltaTime;
            
            meteor->rect.x = meteor->position.x;
            meteor->rect.y = meteor->position.y;
            
            if (meteor->position.x < -meteor->rect.width * 2) {
                meteor->active = false;
                continue;
            }
            
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
            }
        } else if (meteor->state == METEOR_STATE_IMPACT) {
            meteor->position.x -= OBSTACLE_SPEED * window->scaleFactor * deltaTime;
            meteor->rect.x = meteor->position.x;
            meteor->collisionRect.x = meteor->position.x + 10;

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
                game->gameOver = true;
                break;
            }
        }
    }
}

void UpdateObstacles(GameState* state, const WindowState* window, float deltaTime) {
    if (state->gameOver) return;

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

void UpdateBossFight(GameState* game, WindowState* window, float deltaTime) {
    if (!game->isStoryMode || game->gameOver) return;
    
    if (!game->bossActive && game->score >= BOSS_THRESHOLD_SCORE) {
        game->bossActive = true;
        game->screenShakeTimer = SCREEN_SHAKE_DURATION;
        game->screenShakeIntensity = SCREEN_SHAKE_INTENSITY;
        
        for (int i = 0; i < 3; i++) {
            SpawnMeteor(game, window);
        }
        
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

void DrawBossHP(const WindowState* window, const GameState* game) {
    if (!game->isStoryMode || !game->bossActive) return;
    
    const float barWidth = 500 * window->scaleFactor;
    const float barHeight = 30 * window->scaleFactor;
    const float spacing = 5 * window->scaleFactor;
    const float segmentWidth = (barWidth - (spacing * 2)) / BOSS_HP_MAX;
    const float startX = (window->width - barWidth) / 2;
    const float startY = window->height - barHeight - 10 * window->scaleFactor;
    
    DrawRectangle(startX, startY, barWidth, barHeight, DARKGRAY);
    
    for (int i = 0; i < game->bossHP; i++) {
        float segX = startX + (i * (segmentWidth + spacing));
        DrawRectangle(segX, startY, segmentWidth, barHeight, RED);
    }
    
    DrawRectangleLinesEx((Rectangle){ startX, startY, barWidth, barHeight }, 2 * window->scaleFactor, BLACK);
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
    state->bossHP = BOSS_HP_MAX;
    state->screenShakeTimer = 0.0f;
    state->screenShakeIntensity = 0.0f;

    InitMeteors(state);

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

void HandleInput(WindowState* window, GameState* game) {
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

    if (game->gameOver) {
        if (IsKeyPressed(KEY_SPACE)) {
            ResetGame(game);
        }
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
        }
    }
    
    if (wasWPressed && !isWPressed) {
        game->isJumpCharging = false;
    }
    wasWPressed = isWPressed;

    if (game->isCrouching && game->isJumping) {
        game->baseJumpVelocity = FAST_FALL_VELOCITY;
    }

    if (game->gameOver && IsKeyPressed(KEY_SPACE)) {
        window->gameState = GAME_STATE_MENU;
        ResetGame(game);
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

    EndDrawing();
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
