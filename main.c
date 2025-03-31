#include <stdio.h>
#include "raylib.h"

typedef struct {
    Rectangle dino;
    Rectangle ground;
    float jumpVelocity;
    float gravity;
    float jumpForce;
    float fastFallVelocity;
    bool isJumping;
    bool isCrouching;
    float normalHeight;
    float crouchHeight;
} VariabileJoc;

typedef struct {
    int currentWidth;
    int currentHeight;
    int baseWidth;
    int baseHeight;
    bool isFullscreen;
    float scaleFactor;
    bool needsRescale;
} StareFereastra;

// Functie pentru initializare variabilelor de joc
void variabileInitiale(VariabileJoc* state, int screenWidth, int screenHeight) {
    *state = (VariabileJoc){
        .dino = {
            .x = screenWidth * 0.1f,
            .y = screenHeight - 200,
            .width = screenWidth * 0.1f,
            .height = screenHeight * 0.16f
        },
        .ground = {
            .x = 0,
            .y = screenHeight - 100,
            .width = screenWidth,
            .height = 100
        },
        .jumpVelocity = 0,
        .gravity = 1.8f,
        .jumpForce = -30.0f,
        .fastFallVelocity = 20.0f,
        .isJumping = false,
        .isCrouching = false,
        .normalHeight = screenHeight * 0.25f,
        .crouchHeight = screenHeight * 0.125f
    };
}

// Functie pentru initializare ferestrei de joc
void fereastraInitiala(StareFereastra* state, int width, int height) {
    *state = (StareFereastra){
        .baseWidth = width,
        .baseHeight = height,
        .currentWidth = width,
        .currentHeight = height,
        .isFullscreen = false,
        .scaleFactor = 1.0f,
        .needsRescale = false
    };
}

// Functie pentru manipulari in cazul redimensionarii ferestrei
void manipulareDimensiuneFereastra(StareFereastra* window, VariabileJoc* game) {
    if (window->needsRescale) {
        // Rata cu care se va modifica variabilele ce raspund de fizica
        window->scaleFactor = ((float)window->currentWidth/window->baseWidth + 
                             (float)window->currentHeight/window->baseHeight) / 2.0f;
        
        game->gravity = 1.8f * window->scaleFactor;
        game->jumpForce = -30.0f * window->scaleFactor;
        game->fastFallVelocity = 20.0f * window->scaleFactor;
        
        // Scalare dimensiune Dino
        game->dino.width = window->baseWidth * 0.08f * ((float)window->currentWidth/window->baseWidth);
        game->dino.height = window->baseHeight * 0.25f * ((float)window->currentHeight/window->baseHeight);
        game->normalHeight = game->dino.height;
        game->crouchHeight = game->dino.height * 0.5f;
        
        game->ground.width = window->currentWidth;
        game->ground.height = 100 * ((float)window->currentHeight/window->baseHeight);
        game->ground.y = window->currentHeight - game->ground.height;
        
        // Resetare pozitie Dino
        game->dino.x = window->currentWidth * 0.1f;
        game->dino.y = game->ground.y - game->dino.height;
        
        window->needsRescale = false;
    }
}

// Fullscreen mode
void ToggleFullscreenMode(StareFereastra* window) {
    if (!window->isFullscreen) {
        int monitor = GetCurrentMonitor();
        window->currentWidth = GetMonitorWidth(monitor);
        window->currentHeight = GetMonitorHeight(monitor);
    } else {
        window->currentWidth = window->baseWidth;
        window->currentHeight = window->baseHeight;
    }
    
    SetWindowSize(window->currentWidth, window->currentHeight);
    ToggleFullscreen();
    window->isFullscreen = !window->isFullscreen;
    window->needsRescale = true;
}

// Functia care raspunde de citirea dimensiunii ferestrei + fullscreen 
void inputFereastra(StareFereastra* window) {
    if (IsKeyPressed(KEY_F11)) {
        ToggleFullscreenMode(window);
    }
    
    if (IsWindowResized() && !window->isFullscreen) {
        window->currentWidth = GetScreenWidth();
        window->currentHeight = GetScreenHeight();
        window->needsRescale = true;
    }
}

// Functia ce raspunde de mecanica jocului
void mecanicaJoc(VariabileJoc* game) {
    if (IsKeyPressed(KEY_W) && !game->isJumping && !game->isCrouching) {
        game->jumpVelocity = game->jumpForce;
        game->isJumping = true;
    }

    if (IsKeyDown(KEY_S)) {
        game->isCrouching = true;
        game->dino.height = game->crouchHeight;
        
        if (game->isJumping) {
            game->jumpVelocity = game->fastFallVelocity;
        } else {
            game->dino.y = game->ground.y - game->crouchHeight;
        }
    } else {
        game->isCrouching = false;
        game->dino.height = game->normalHeight;
        if (game->dino.y + game->dino.height > game->ground.y) {
            game->dino.y = game->ground.y - game->dino.height;
        }
    }
}

// Fizica jocului
void fizicaJocului(VariabileJoc* game) {
    if (game->isJumping) {
        game->dino.y += game->jumpVelocity;
        game->jumpVelocity += game->gravity;
        
        if (game->dino.y + game->dino.height >= game->ground.y) {
            game->dino.y = game->ground.y - game->dino.height;
            game->isJumping = false;
            game->jumpVelocity = 0;
        }
    }
}

// Initializare obiecte
void DrawGame(const StareFereastra* window, const VariabileJoc* game) {
    BeginDrawing();
        ClearBackground(WHITE);
        
        DrawRectangleRec(game->ground, DARKGRAY);
        DrawRectangleRec(game->dino, game->isCrouching ? DARKGREEN : GREEN);
        
        // Debug info
        DrawText(TextFormat("Window: %dx%d", window->currentWidth, window->currentHeight), 10, 10, 20, BLACK);
        DrawText(TextFormat("Scale: %.2fx", window->scaleFactor), 10, 40, 20, BLACK);
        DrawText(TextFormat("Gravity: %.1f", game->gravity), 10, 70, 20, BLACK);
        
    EndDrawing();
}

int main(void) {
    const int baseWidth = 800;
    const int baseHeight = 600;
    
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(baseWidth, baseHeight, "DinoGame");
    SetTargetFPS(60);

    StareFereastra window;
    VariabileJoc game;
    
    fereastraInitiala(&window, baseWidth, baseHeight);
    variabileInitiale(&game, baseWidth, baseHeight);

    while (!WindowShouldClose()) {
        inputFereastra(&window);
        manipulareDimensiuneFereastra(&window, &game);
        
        mecanicaJoc(&game);
        fizicaJocului(&game);
        
        DrawGame(&window, &game);
    }

    CloseWindow();
    return 0;
}