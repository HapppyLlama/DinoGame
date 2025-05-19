#include "raylib.h"

typedef struct Obstacle {
    Rectangle rect;
    Vector2 position;
    Vector2 size;
    bool active;
    bool hasPassedPlayer;
} Obstacle;

void InitObstacle(Obstacle *obstacle, float x, float y, float width, float height) {
    obstacle->position = (Vector2){ x, y };
    obstacle->size = (Vector2){ width, height };
    obstacle->rect = (Rectangle){ x, y, width, height };
    obstacle->active = true;
    obstacle->hasPassedPlayer = false;
}

void UpdateObstacle(Obstacle *obstacle, float deltaTime) {
    if (obstacle->active) {
        obstacle->position.y += 300.0f * deltaTime;
        obstacle->rect.y = obstacle->position.y;

        if (obstacle->position.y > 0) {
            obstacle->hasPassedPlayer = true;
        }
    }
}

void DrawObstacle(Obstacle *obstacle) {
    if (obstacle->active) {
        DrawRectangleRec(obstacle->rect, RED);
    }
}