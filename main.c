#include "raylib.h"
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>

// definirea constantelor

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

#define DAY_DURATION 10.0f     // 10 secunde zi
#define NIGHT_DURATION 10.0f    // 10 secunde noapte
#define FADE_DURATION 0.2f     // 0.5 secunde fade
#define LIGHT_RADIUS 400.0f   // Raza mare pentru lumina
#define NIGHT_ALPHA 255
#define FADE_DISTANCE 100.0f 

// Add new boss fight related constants
#define BOSS_THRESHOLD_SCORE 100
#define BOSS_HP_MAX 3
#define SCREEN_SHAKE_DURATION 1.0f
#define SCREEN_SHAKE_INTENSITY 10.0f

// Add pre-boss threshold
#define PRE_BOSS_THRESHOLD (BOSS_THRESHOLD_SCORE - 50)

// Update meteor-related constants for faster and more varied falling
#define MAX_METEORS 15               // Allow more meteors on screen
#define METEOR_SPAWN_INTERVAL_MIN 0.5f  // Even faster spawning
#define METEOR_SPAWN_INTERVAL_MAX 1.5f  // Even faster spawning
#define METEOR_FALL_SPEED_X 300.0f      // Faster horizontal speed
#define METEOR_FALL_SPEED_Y 1000.0f      // Much faster vertical speed
#define METEOR_ANIM_DELAY 0.08f         // Faster animation
#define METEOR_GROUND_LIFETIME 10.0f    // Meteors stay on ground much longer

#define METEOR_IMPACT_FRAMES 3

typedef enum {
    GAME_STATE_MENU,
    GAME_STATE_PLAYING,
    GAME_STATE_GAME_OVER,
    GAME_STATE_RESOLUTION // Added resolution state
} GameStates;

typedef struct {
    Rectangle rect;
    const char* text;
    int textWidth;
} MenuButton;

typedef struct {
    MenuButton playButton;
    MenuButton storyButton; // Add story mode button
    MenuButton quitButton;
    MenuButton resolutionButton;
    bool playHovered;
    bool storyHovered; // Add hover state for story mode
    bool quitHovered;
    bool resolutionHovered;
} MenuState;

// strucutura cu info privind butoane

typedef struct {
    Rectangle rect;
    const char* text;
    int textWidth;
    int width;
    int height;
} ResolutionButton;

// enum cu tipurile de obstacole

typedef enum {
    OBSTACLE_CACTUS_1,
    OBSTACLE_CACTUS_2,
    OBSTACLE_CACTUS_3,
    OBSTACLE_CACTUS_4,
    OBSTACLE_CACTUS_5,
    OBSTACLE_CACTUS_6,
    OBSTACLE_BIRD
} ObstacleType;

// dimensiuni obstacole

typedef struct {
    float width;
    float height;
    float yOffset;
} ObstacleDimensions;

// structura cu info despre obstacole

typedef struct {
    Rectangle rect;
    Rectangle collisionRect;
    bool active;
    ObstacleType type;
    int currentFrame;
    float frameTime;
} Obstacle;

// structura folosita in viitor la spawnul de obstacole

typedef struct {
    Obstacle obstacles[MAX_OBSTACLES];
    float spawnTimer;
    float nextSpawnTime;
} ObstaclePool;

// Add PauseMenuState structure
typedef struct {
    Rectangle continueButton;
    Rectangle mainMenuButton;
    bool continueHovered;
    bool mainMenuHovered;
    bool isPaused;
} PauseMenuState;

// Add meteor states
typedef enum {
    METEOR_STATE_FALLING,
    METEOR_STATE_IMPACT,
    METEOR_STATE_INACTIVE
} MeteorState;

// Add meteor structure
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

// structura ce contine parametrii de joc

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
    RenderTexture2D lightMask;
    float nightAlpha;

    PauseMenuState pauseMenu; // Added pause menu state

    // Story mode and boss fight
    bool isStoryMode;
    bool bossActive;
    int bossHP;
    float screenShakeTimer;
    float screenShakeIntensity;

    // Meteor system
    Meteor meteors[MAX_METEORS];
    float meteorSpawnTimer;
    float nextMeteorSpawnTime;
} GameState;

// parametrii ferestrei

typedef struct {
    int width;
    int height;
    bool isFullscreen;
    float scaleFactor;
    ResolutionButton resolutions[NUM_RESOLUTIONS];
    MenuState menu;
    GameStates gameState;
} WindowState;

static const Vector2 BASE_RESOLUTION = { 1600, 900 }; // rezolutia de baza a frerestrei

// dimensiunile obstacolelor

static const ObstacleDimensions CACTUS_DIMENSIONS[] = {
    [OBSTACLE_CACTUS_1] = {.width = 34,  .height = 68, .yOffset = 0},
    [OBSTACLE_CACTUS_2] = {.width = 68,  .height = 68, .yOffset = 0},
    [OBSTACLE_CACTUS_3] = {.width = 102, .height = 68, .yOffset = 0},
    [OBSTACLE_CACTUS_4] = {.width = 50,  .height = 94, .yOffset = 0},
    [OBSTACLE_CACTUS_5] = {.width = 100, .height = 94, .yOffset = 0},
    [OBSTACLE_CACTUS_6] = {.width = 150, .height = 95, .yOffset = 0},
    [OBSTACLE_BIRD]     = {.width = 93,  .height = 80,  .yOffset = -61}
};

// Forward declarations
void InitMeteors(GameState* state);

// salveaza scorul maxim obstinut in fisier

void SaveHighScore(int highScore) {
    FILE* file = fopen("highscore.bin", "wb");
    if (file) {
        fwrite(&highScore, sizeof(int), 1, file);
        fclose(file);
    }
}

// scoate din fisier info cu scorul maxim

int LoadHighScore() {
    int highScore = 0;
    FILE* file = fopen("highscore.bin", "rb");
    if (file) {
        fread(&highScore, sizeof(int), 1, file);
        fclose(file);
    }
    return highScore;
}

// Consolidate button hover checks into a utility function
bool IsButtonHovered(const Rectangle* button) {
    return CheckCollisionPointRec(GetMousePosition(), *button);
}

// Optimize ResetGame by removing redundant resets
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

    // Reset boss fight variables
    game->bossActive = false;
    game->bossHP = BOSS_HP_MAX;
    game->screenShakeTimer = 0.0f;
    game->screenShakeIntensity = 0.0f;

    // Reset meteors
    InitMeteors(game);
}

// initializare obstacole

void InitObstacles(GameState* state) {
    for (int i = 0; i < MAX_OBSTACLES; i++) {
        state->obstacles.obstacles[i].active = false;
    }
    state->obstacles.spawnTimer = 0.0f;
    state->obstacles.nextSpawnTime = GetRandomValue(MIN_SPAWN_INTERVAL, MAX_SPAWN_INTERVAL);
}

// Initialize meteors
void InitMeteors(GameState* state) {
    for (int i = 0; i < MAX_METEORS; i++) {
        state->meteors[i].active = false;
        state->meteors[i].state = METEOR_STATE_INACTIVE;
    }
    state->meteorSpawnTimer = 0.0f;
    state->nextMeteorSpawnTime = GetRandomValue(METEOR_SPAWN_INTERVAL_MIN * 100, 
                                            METEOR_SPAWN_INTERVAL_MAX * 100) / 100.0f;
}

// functia ce raspunde de spawnul de obstacole

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

// Update SpawnMeteor to create meteors from varied positions
void SpawnMeteor(GameState* game, const WindowState* window) {
    for (int i = 0; i < MAX_METEORS; i++) {
        if (!game->meteors[i].active) {
            Meteor* meteor = &game->meteors[i];
            meteor->active = true;
            meteor->state = METEOR_STATE_FALLING;
            meteor->currentFrame = 0;
            meteor->frameTime = 0;
            meteor->impactTime = 0;
            
            // Much more varied spawn positions
            // Choose from 3 different spawn patterns
            int spawnPattern = GetRandomValue(0, 2);
            switch (spawnPattern) {
                case 0: // Far right, high up
                    meteor->position.x = window->width + GetRandomValue(50, 250);
                    meteor->position.y = -GetRandomValue(100, 300);
                    break;
                    
                case 1: // Middle-right, very high
                    meteor->position.x = window->width * 0.7f + GetRandomValue(-100, 100);
                    meteor->position.y = -GetRandomValue(200, 400);
                    break;
                    
                case 2: // Far right, medium height
                    meteor->position.x = window->width + GetRandomValue(50, 150);
                    meteor->position.y = -GetRandomValue(50, 150);
                    break;
            }
            
            // More varied sizes
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

// Modify UpdateMeteors so grounded meteors move like obstacles
void UpdateMeteors(GameState* game, const WindowState* window, float deltaTime) {
    if (!game->isStoryMode || !game->bossActive || game->gameOver) {
        return;
    }
    
    // Spawn new meteors
    game->meteorSpawnTimer += deltaTime;
    if (game->meteorSpawnTimer >= game->nextMeteorSpawnTime) {
        SpawnMeteor(game, window);
        game->meteorSpawnTimer = 0.0f;
        game->nextMeteorSpawnTime = METEOR_SPAWN_INTERVAL_MIN + 
            (METEOR_SPAWN_INTERVAL_MAX - METEOR_SPAWN_INTERVAL_MIN) * GetRandomValue(0, 100) / 100.0f;
    }
    
    // Ground level for impact detection
    float groundY = window->height - GROUND_HEIGHT * window->scaleFactor;
    
    // Update existing meteors
    for (int i = 0; i < MAX_METEORS; i++) {
        Meteor* meteor = &game->meteors[i];
        if (!meteor->active) continue;
        
        // Update animation frames faster
        meteor->frameTime += deltaTime;
        
        if (meteor->state == METEOR_STATE_FALLING) {
            // Faster animation for falling
            if (meteor->frameTime >= METEOR_ANIM_DELAY) {
                meteor->frameTime = 0;
                meteor->currentFrame = (meteor->currentFrame + 1) % 2;
            }
            
            // Much faster diagonal movement
            meteor->position.x -= METEOR_FALL_SPEED_X * deltaTime;
            meteor->position.y += METEOR_FALL_SPEED_Y * deltaTime;
            
            // Update meteor position
            meteor->rect.x = meteor->position.x;
            meteor->rect.y = meteor->position.y;
            
            // Check if meteor hit the ground or is off-screen
            if (meteor->position.x < -meteor->rect.width * 2) {
                // If meteor goes off-screen to the left, deactivate it
                meteor->active = false;
                continue;
            }
            
            if (meteor->position.y >= groundY - meteor->rect.height) {
                meteor->state = METEOR_STATE_IMPACT;
                meteor->position.y = groundY - meteor->rect.height;
                meteor->rect.y = meteor->position.y;
                meteor->currentFrame = 0;
                meteor->frameTime = 0;
                
                // Make the screen shake more noticeable on impact
                game->screenShakeTimer = 0.3f;
                game->screenShakeIntensity = 8.0f;
                
                // Set up collision rectangle for impact state
                meteor->collisionRect = (Rectangle){
                    meteor->position.x + 10,
                    meteor->position.y + meteor->rect.height - 25,
                    meteor->rect.width - 20,
                    25
                };
            }
        } else if (meteor->state == METEOR_STATE_IMPACT) {
            // move toward player at obstacle speed
            meteor->position.x -= OBSTACLE_SPEED * window->scaleFactor * deltaTime;
            meteor->rect.x = meteor->position.x;
            meteor->collisionRect.x = meteor->position.x + 10;

            // Handle impact state - meteors remain on the ground longer
            
            // Only animate during the initial impact
            if (meteor->impactTime < 1.0f) {
                if (meteor->frameTime >= METEOR_ANIM_DELAY * 1.5f) {
                    meteor->frameTime = 0;
                    if (meteor->currentFrame < METEOR_IMPACT_FRAMES - 1) {
                        meteor->currentFrame++;
                    }
                }
            }
            
            meteor->impactTime += deltaTime;
            
            // Keep meteors on the ground much longer
            if (meteor->impactTime > METEOR_GROUND_LIFETIME) {
                // Make meteors fade out very gradually rather than disappear suddenly
                if (meteor->impactTime > METEOR_GROUND_LIFETIME + 3.0f) {
                    meteor->active = false;
                }
            }
            
            // Check for collision with player as long as the meteor is active
            if (CheckCollisionRecs(game->rect, meteor->collisionRect)) {
                game->gameOver = true;
            }
        }
    }
}

// Modify UpdateObstacles to stop spawning near boss fight
void UpdateObstacles(GameState* state, const WindowState* window, float deltaTime) {
    if (state->gameOver) return;

    // Only spawn if not within 50 points of boss fight
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
            }
        }
    }
}

// Update UpdateBossFight to spawn more meteors at the start
void UpdateBossFight(GameState* game, WindowState* window, float deltaTime) {
    if (!game->isStoryMode || game->gameOver) return;
    
    // Check for boss trigger
    if (!game->bossActive && game->score >= BOSS_THRESHOLD_SCORE) {
        game->bossActive = true;
        game->screenShakeTimer = SCREEN_SHAKE_DURATION;
        game->screenShakeIntensity = SCREEN_SHAKE_INTENSITY;
        
        // Immediately spawn several meteors when boss activates
        for (int i = 0; i < 3; i++) {
            SpawnMeteor(game, window);
        }
        
        // Reset meteor spawn timer for quicker first spawn
        game->meteorSpawnTimer = 0.0f;
        game->nextMeteorSpawnTime = 0.3f;
    }
    
    // Update screen shake effect
    if (game->screenShakeTimer > 0) {
        game->screenShakeTimer -= deltaTime;
        if (game->screenShakeTimer <= 0) {
            game->screenShakeIntensity = 0.0f;
        }
    }
    
    // Update meteors
    if (game->bossActive) {
        UpdateMeteors(game, window, deltaTime);
    }
}

// Draw HP bar for boss fight
void DrawBossHP(const WindowState* window, const GameState* game) {
    if (!game->isStoryMode || !game->bossActive) return;
    
    const float barWidth = 500 * window->scaleFactor; // Increased from 300 to 500
    const float barHeight = 30 * window->scaleFactor;
    const float spacing = 5 * window->scaleFactor;
    const float segmentWidth = (barWidth - (spacing * 2)) / BOSS_HP_MAX;
    const float startX = (window->width - barWidth) / 2;
    const float startY = window->height - barHeight - 10 * window->scaleFactor;
    
    // Draw background bar
    DrawRectangle(startX, startY, barWidth, barHeight, DARKGRAY);
    
    // Draw each HP segment
    for (int i = 0; i < game->bossHP; i++) {
        float segX = startX + (i * (segmentWidth + spacing));
        DrawRectangle(segX, startY, segmentWidth, barHeight, RED);
    }
    
    // Draw border
    DrawRectangleLinesEx((Rectangle){ startX, startY, barWidth, barHeight }, 2 * window->scaleFactor, BLACK);
}

// Apply screen shake to drawing
Vector2 ApplyScreenShake(const GameState* game) {
    if (game->screenShakeTimer <= 0 || game->screenShakeIntensity <= 0) {
        return (Vector2){ 0, 0 };
    }
    
    return (Vector2){
        GetRandomValue(-game->screenShakeIntensity, game->screenShakeIntensity),
        GetRandomValue(-game->screenShakeIntensity, game->screenShakeIntensity)
    };
}

// initializarea parametrilor de joc

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
    state->lightMask = LoadRenderTexture(BASE_RESOLUTION.x, BASE_RESOLUTION.y);
    state->nightAlpha = 0.0f;

    // Initialize boss fight variables
    state->isStoryMode = false;
    state->bossActive = false;
    state->bossHP = BOSS_HP_MAX;
    state->screenShakeTimer = 0.0f;
    state->screenShakeIntensity = 0.0f;

    // Initialize meteors
    InitMeteors(state);

    InitPauseMenu(state, &(WindowState){ .width = BASE_RESOLUTION.x, .height = BASE_RESOLUTION.y });
}

// actualizare scor (1 pct la 0.1 sec)

void UpdateScore(GameState* game, float deltaTime) {
    game->scoreTimer += deltaTime;
    if (game->scoreTimer >= 0.1f) {
        game->score++;
        game->scoreTimer = 0.0f;
        if (game->score > game->highScore) game->highScore = game->score;
    }
}

// mintine pozitia butoanelor relativ cu fereastra 

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

    // Title
    const char* title = "DINO GAME";
    int titleWidth = MeasureText(title, 60);
    DrawText(title, window->width/2 - titleWidth/2, 150, 60, DARKGRAY);

    // Play button
    DrawRectangleRec(window->menu.playButton.rect, window->menu.playHovered ? SKYBLUE : LIGHTGRAY);
    DrawText(window->menu.playButton.text, 
        window->menu.playButton.rect.x + (window->menu.playButton.rect.width - window->menu.playButton.textWidth)/2,
        window->menu.playButton.rect.y + 10,
        30, DARKBLUE);

    // Story Mode button
    DrawRectangleRec(window->menu.storyButton.rect, window->menu.storyHovered ? SKYBLUE : LIGHTGRAY);
    DrawText(window->menu.storyButton.text, 
        window->menu.storyButton.rect.x + (window->menu.storyButton.rect.width - window->menu.storyButton.textWidth)/2,
        window->menu.storyButton.rect.y + 10,
        30, DARKBLUE);

    // Resolution button
    DrawRectangleRec(window->menu.resolutionButton.rect, window->menu.resolutionHovered ? SKYBLUE : LIGHTGRAY);
    DrawText(window->menu.resolutionButton.text,
        window->menu.resolutionButton.rect.x + (window->menu.resolutionButton.rect.width - window->menu.resolutionButton.textWidth)/2,
        window->menu.resolutionButton.rect.y + 10,
        30, DARKBLUE);

    // Quit button
    DrawRectangleRec(window->menu.quitButton.rect, window->menu.quitHovered ? SKYBLUE : LIGHTGRAY);
    DrawText(window->menu.quitButton.text,
        window->menu.quitButton.rect.x + (window->menu.quitButton.rect.width - window->menu.quitButton.textWidth)/2,
        window->menu.quitButton.rect.y + 10,
        30, DARKBLUE);

    EndDrawing();
}

// Optimize DrawResolutionMenu by reducing redundancy
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

// Update InitMenuButtons to properly position all buttons with consistent spacing
void InitMenuButtons(WindowState* window) {
    const float buttonWidth = 200;
    const float buttonHeight = 50;
    const float buttonSpacing = 20; // Define consistent spacing between buttons
    Vector2 center = { window->width/2 - buttonWidth/2, window->height/2 - buttonHeight/2 };

    // Start from a higher position to accommodate all buttons
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

// intializare parametrilor de fereastra

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

// formula de calcul a factorului de scalare la diferite dimensiuni a ferestrei

void UpdateScaleFactor(WindowState* window) {
    window->scaleFactor = (float)window->height / BASE_RESOLUTION.y;
}

// in cazul selectarii unei dimensiuni se activeaza functia data care modifica parametrii elementelor de joc ca acestea sa se scaleze conform dimensiunii

void RescaleGame(GameState* game, WindowState* window) {
    UpdateScaleFactor(window); // primeste factorul de scalare
    game->screenPosition.x = game->basePosition.x * window->scaleFactor;
    game->screenPosition.y = game->basePosition.y * window->scaleFactor;

    // Recrează lightMask cu noile dimensiuni
    if (game->lightMask.id != 0) {
        UnloadRenderTexture(game->lightMask);
    }
    game->lightMask = LoadRenderTexture(window->width, window->height);
    
    // actualizează dreptunghiul personajului
    const Rectangle* frame = game->isCrouching ? &game->crouchFrames[game->currentFrame] : &game->runFrames[game->currentFrame];
    game->rect.width = frame->width * window->scaleFactor;
    game->rect.height = frame->height * window->scaleFactor;
    game->rect.x = game->screenPosition.x;
    game->rect.y = game->screenPosition.y;

    // actualizează obstacolele active
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
    InitMenuButtons(window); // Add this to recenter menu buttons
}

// modul fullscreen

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

// actualizare animatii

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

// Optimize HandlePauseMenuInput by using the utility function
void HandlePauseMenuInput(WindowState* window, GameState* game) {
    game->pauseMenu.continueHovered = IsButtonHovered(&game->pauseMenu.continueButton);
    game->pauseMenu.mainMenuHovered = IsButtonHovered(&game->pauseMenu.mainMenuButton);

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        if (game->pauseMenu.continueHovered) {
            game->pauseMenu.isPaused = false; // Resume the game
        } else if (game->pauseMenu.mainMenuHovered) {
            game->pauseMenu.isPaused = false; // Unpause the game
            ResetGame(game); // Reset the game state
            window->gameState = GAME_STATE_MENU; // Go to main menu
            window->width = GetScreenWidth();
            window->height = GetScreenHeight();
            UpdateScaleFactor(window); // Update scale factor without changing resolution
        }
    }
}

// Optimize HandleInput by reducing redundant checks
void HandleInput(WindowState* window, GameState* game) {
    Vector2 mousePos = GetMousePosition();

    if (window->gameState == GAME_STATE_MENU) {
        window->menu.playHovered = IsButtonHovered(&window->menu.playButton.rect);
        window->menu.storyHovered = IsButtonHovered(&window->menu.storyButton.rect); // Add story button hover check
        window->menu.quitHovered = IsButtonHovered(&window->menu.quitButton.rect);
        window->menu.resolutionHovered = IsButtonHovered(&window->menu.resolutionButton.rect);

        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            if (window->menu.playHovered) {
                window->gameState = GAME_STATE_PLAYING;
                game->isStoryMode = false; // Regular mode
                ResetGame(game);
            } else if (window->menu.storyHovered) {
                window->gameState = GAME_STATE_PLAYING;
                game->isStoryMode = true; // Story mode
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

    // procesare input pentru butoanele de rezoluție
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

    // daca jocul s-a terminat, permite restartarea prin tasta SPACE
    if (game->gameOver) {
        if (IsKeyPressed(KEY_SPACE)) {
            ResetGame(game);
        }
        return;
    }
    
    // actualizare stari personaj (sarit/furis)
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
        if (IsKeyPressed(KEY_O)) { // Pause the game when "O" is pressed
            game->pauseMenu.isPaused = !game->pauseMenu.isPaused;
        }

        if (game->pauseMenu.isPaused) {
            HandlePauseMenuInput(window, game);
            return;
        }
    }
}

// actualizarea parametrilor ce raspund de fizica jocului

void UpdatePhysics(GameState* game, const WindowState* window, float deltaTime) {
    // aici se configureaza saritura
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

// initializarea elemntelor de joc pe ecran

// Optimize DrawPauseMenu by reducing redundant calls
void DrawPauseMenu(const WindowState* window, const GameState* game) {
    DrawRectangle(0, 0, window->width, window->height, (Color){ 0, 0, 0, 150 });

    const Rectangle* buttons[] = { &game->pauseMenu.continueButton, &game->pauseMenu.mainMenuButton }; // Use const Rectangle*
    const char* labels[] = { "Continue", "Main Menu" };
    bool hovered[] = { game->pauseMenu.continueHovered, game->pauseMenu.mainMenuHovered };

    for (int i = 0; i < 2; i++) {
        DrawRectangleRec(*buttons[i], hovered[i] ? SKYBLUE : LIGHTGRAY);
        DrawText(labels[i], buttons[i]->x + 20, buttons[i]->y + 10, 30, DARKBLUE);
    }
}

// Update DrawMeteors to handle the permanent ground state
void DrawMeteors(const WindowState* window, const GameState* game, Vector2 shakeOffset) {
    // Debug output to see if any meteors are active
    int activeCount = 0;
    for (int i = 0; i < MAX_METEORS; i++) {
        if (game->meteors[i].active) activeCount++;
    }
    
    if (game->isStoryMode && game->bossActive) {
        DrawText(TextFormat("Active Meteors: %d", activeCount), 10, 150, 20, RED);
    }
    
    for (int i = 0; i < MAX_METEORS; i++) {
        const Meteor* meteor = &game->meteors[i];
        if (!meteor->active) continue;
        
        Rectangle source;
        Color tint = WHITE;
        
        if (meteor->state == METEOR_STATE_FALLING) {
            // Falling meteor sprite
            source = (Rectangle){ 260, 0, 60, 60 };
        } else if (meteor->state == METEOR_STATE_IMPACT) {
            // Impact crater sprite
            source = (Rectangle){ 320, 0, 60, 60 };
            
            // If the meteor has been on the ground for a while, start fading it
            if (meteor->impactTime > METEOR_GROUND_LIFETIME) {
                float alpha = 1.0f - (meteor->impactTime - METEOR_GROUND_LIFETIME) / 3.0f;
                tint = (Color){ 255, 255, 255, (unsigned char)(255 * alpha) };
            }
        } else {
            continue;
        }
        
        // Draw the meteor with shake offset
        Rectangle destRect = {
            meteor->rect.x + shakeOffset.x,
            meteor->rect.y + shakeOffset.y,
            meteor->rect.width,
            meteor->rect.height
        };
        
        // Use orange color rectangle to make it visible for debugging
        DrawRectangleRec(destRect, ORANGE);
        DrawTexturePro(game->spriteSheet, source, destRect, (Vector2){0}, 0, tint);
    }
}

// Update DrawGame to include meteors
void DrawGame(const WindowState* window, const GameState* game) {
    BeginDrawing();
    ClearBackground(WHITE);
    
    // Apply screen shake offset
    Vector2 shakeOffset = ApplyScreenShake(game);
    
    // Ground
    float groundY = (BASE_RESOLUTION.y - GROUND_HEIGHT) * window->scaleFactor;
    float groundHeight = GROUND_HEIGHT * window->scaleFactor;
    DrawRectangle(0, groundY, window->width, groundHeight, DARKGRAY);

    // Draw character with screen shake offset
    const Rectangle* frame = game->isCrouching ? &game->crouchFrames[game->currentFrame] : &game->runFrames[game->currentFrame];
    Rectangle destRect = {
        game->screenPosition.x + shakeOffset.x,
        game->screenPosition.y + shakeOffset.y,
        frame->width * window->scaleFactor,
        frame->height * window->scaleFactor
    };
    DrawTexturePro(game->spriteSheet, *frame, destRect, (Vector2){0}, 0, WHITE);

    // Draw obstacles with screen shake offset
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

    // Draw meteors (after obstacles, before night mode)
    if (game->isStoryMode && game->bossActive) {
        DrawMeteors(window, game, shakeOffset);
    }

    // Night mode with proper scaling
    if (game->nightModeActive && game->nightAlpha > 0) {
        BeginTextureMode(game->lightMask);
        ClearBackground(BLANK);

        // Full dark overlay
        DrawRectangle(0, 0, window->width, window->height, 
                      (Color){0, 0, 0, (unsigned char)(game->nightAlpha * NIGHT_ALPHA)});

        // Visibility circle around the player
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

        // Draw the light mask
        DrawTextureRec(game->lightMask.texture, 
                       (Rectangle){ 0, 0, window->width, -window->height }, 
                       (Vector2){ 0, 0 }, 
                       WHITE);
    }

    // Draw boss HP bar if in story mode and boss is active
    if (game->isStoryMode && game->bossActive) {
        DrawBossHP(window, game);
    }

    // gameover
    if (game->gameOver) {
        const char* text = "GAME OVER - Press SPACE to restart";
        int textWidth = MeasureText(text, 40);
        DrawText(text, (window->width - textWidth) / 2, window->height / 2, 40, RED);
    }

    // scorul și scorul maxim
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

            case GAME_STATE_RESOLUTION: // Added resolution menu state
                DrawResolutionMenu(&window);
                break;
                
            case GAME_STATE_PLAYING:
                if (!game.gameOver && !game.pauseMenu.isPaused) {
                    UpdatePhysics(&game, &window, deltaTime);
                    UpdateScore(&game, deltaTime);
                    UpdateObstacles(&game, &window, deltaTime);
                    UpdateBossFight(&game, &window, deltaTime); // This now includes meteor updates
                    
                    if (game.score >= 10 && !game.nightModeActive) {
                        game.nightModeActive = true;
                        game.nightCycleTimer = 0.0f;
                    }

                    if (game.nightModeActive) {
                        game.nightCycleTimer += deltaTime;
                        float totalCycleTime = DAY_DURATION + NIGHT_DURATION + 2 * FADE_DURATION;
                        float cycleProgress = fmod(game.nightCycleTimer, totalCycleTime);

                        if (cycleProgress < DAY_DURATION) {
                            game.nightAlpha = 0.0f;
                        } else if (cycleProgress < DAY_DURATION + FADE_DURATION) {
                            float fadeProgress = (cycleProgress - DAY_DURATION) / FADE_DURATION;
                            game.nightAlpha = fadeProgress;
                        } else if (cycleProgress < DAY_DURATION + FADE_DURATION + NIGHT_DURATION) {
                            game.nightAlpha = 1.0f;
                        } else {
                            float fadeProgress = (cycleProgress - (DAY_DURATION + FADE_DURATION + NIGHT_DURATION)) / FADE_DURATION;
                            game.nightAlpha = 1.0f - fadeProgress;
                        }
                    }

                    if (!game.isJumping) {
                        UpdateAnimation(&game, &window, deltaTime);
                    }
                }
                DrawGame(&window, &game);
                break;

            case GAME_STATE_GAME_OVER:
                DrawGame(&window, &game);
                break;
        }
    }

    UnloadRenderTexture(game.lightMask);
    UnloadTexture(game.spriteSheet);
    CloseWindow();
    return 0;
}
