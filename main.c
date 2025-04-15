#include "raylib.h"
#include <string.h>
#include <stdio.h>

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
#define OBSTACLE_SPEED 10.0f
#define BIRD_ANIM_DELAY 0.2f

#define COLLISION_OFFSET 3

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

} GameState;

// parametrii ferestrei

typedef struct {
    int width;
    int height;
    bool isFullscreen;
    float scaleFactor;
    ResolutionButton resolutions[NUM_RESOLUTIONS];
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

// resetare joc in caz de gameover

void ResetGame(GameState* game) {
    if (game->score > game->highScore) {
        game->highScore = game->score;
        SaveHighScore(game->highScore);
    }
    game->basePosition.y = BASE_RESOLUTION.y - GROUND_HEIGHT - game->runFrameHeight;
    game->isJumping = false;
    game->score = 0;
    game->gameOver = false;

    // dezactivează toate obstacolele
    for (int i = 0; i < MAX_OBSTACLES; i++) {
        game->obstacles.obstacles[i].active = false;
    }
}

// initializare obstacole

void InitObstacles(GameState* state) {
    for (int i = 0; i < MAX_OBSTACLES; i++) {
        state->obstacles.obstacles[i].active = false;
    }
    state->obstacles.spawnTimer = 0.0f;
    state->obstacles.nextSpawnTime = GetRandomValue(MIN_SPAWN_INTERVAL, MAX_SPAWN_INTERVAL);
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

// actualizare obstacole

void UpdateObstacles(GameState* state, const WindowState* window, float deltaTime) {
    if (state->gameOver) return;

    state->obstacles.spawnTimer += deltaTime;
    if (state->obstacles.spawnTimer >= state->obstacles.nextSpawnTime) {
        SpawnObstacle(state, window);
        state->obstacles.spawnTimer = 0.0f;
        state->obstacles.nextSpawnTime = GetRandomValue(MIN_SPAWN_INTERVAL, MAX_SPAWN_INTERVAL);
    }

    for (int i = 0; i < MAX_OBSTACLES; i++) {
        Obstacle* obs = &state->obstacles.obstacles[i];
        if (obs->active) {
            obs->rect.x -= OBSTACLE_SPEED * window->scaleFactor;
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

// initializarea parametrilor de joc

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
    UpdateButtonPositions(state);
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

// functia ce contine toate manipularile cu jocul (sarit, furis, redimensionare etc.)

void HandleInput(WindowState* window, GameState* game) {
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

// creare butoane

void DrawButtons(const WindowState* window) {
    for (int i = 0; i < NUM_RESOLUTIONS; i++) {
        Color btnColor = CheckCollisionPointRec(GetMousePosition(), window->resolutions[i].rect) ? GRAY : LIGHTGRAY;
        DrawRectangleRec(window->resolutions[i].rect, btnColor);
        DrawRectangleLinesEx(window->resolutions[i].rect, 2, DARKGRAY);
        int textX = window->resolutions[i].rect.x + (window->resolutions[i].rect.width - window->resolutions[i].textWidth) / 2;
        int textY = window->resolutions[i].rect.y + 5;
        DrawText(window->resolutions[i].text, textX, textY, 20, BLACK);
    }
}

// initializarea elemntelor de joc pe ecran

void DrawGame(const WindowState* window, const GameState* game) {
    BeginDrawing();
    ClearBackground(WHITE);

    // pamant
    float groundY = (BASE_RESOLUTION.y - GROUND_HEIGHT) * window->scaleFactor;
    float groundHeight = GROUND_HEIGHT * window->scaleFactor;
    DrawRectangle(0, groundY, window->width, groundHeight, DARKGRAY);

    // caracterul
    const Rectangle* frame = game->isCrouching ? &game->crouchFrames[game->currentFrame] : &game->runFrames[game->currentFrame];
    Rectangle destRect = {
        game->screenPosition.x,
        game->screenPosition.y,
        frame->width * window->scaleFactor,
        frame->height * window->scaleFactor
    };
    DrawTexturePro(game->spriteSheet, *frame, destRect, (Vector2){0}, 0, WHITE);

    // obstacole
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
            DrawTexturePro(game->spriteSheet, source, obs->rect, (Vector2){0}, 0, WHITE);
        }
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

    DrawButtons(window);
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
        if (!game.gameOver) {
            UpdatePhysics(&game, &window, deltaTime);
            UpdateScore(&game, deltaTime);
            UpdateObstacles(&game, &window, deltaTime);
            
            if (!game.isJumping) {
                UpdateAnimation(&game, &window, deltaTime);
            }
        }
        
        DrawGame(&window, &game);
    }

    UnloadTexture(game.spriteSheet);
    CloseWindow();
    return 0;
}
