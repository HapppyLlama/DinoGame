#ifndef TYPES_H
#define TYPES_H
#include "raylib.h"
#include <stdbool.h>
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
#define PRE_BOSS_THRESHOLD (BOSS_THRESHOLD_SCORE - 20)
#define MAX_METEORS 15
#define METEOR_SPAWN_INTERVAL_MIN 0.5f
#define METEOR_SPAWN_INTERVAL_MAX 1.5f
#define METEOR_FALL_SPEED_X 700.0f
#define METEOR_FALL_SPEED_Y 1000.0f
#define METEOR_ANIM_DELAY 0.08f
#define METEOR_GROUND_LIFETIME 10.0f
#define METEOR_IMPACT_FRAMES 3
#define MAX_CLOUDS 4
#define CLOUD_MIN_SPEED 50.0f
#define CLOUD_MAX_SPEED 150.0f
#define CLOUD_MIN_ALPHA 0.5f
#define CLOUD_MAX_ALPHA 0.9f
#define CLOUD_SPAWN_INTERVAL_MIN 2.0f
#define CLOUD_SPAWN_INTERVAL_MAX 5.0f
static const Vector2 BASE_RESOLUTION = { 1600, 900 };
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
static const ObstacleDimensions CACTUS_DIMENSIONS[] = {
    [OBSTACLE_CACTUS_1] = {.width = 34,  .height = 68, .yOffset = 0},
    [OBSTACLE_CACTUS_2] = {.width = 68,  .height = 68, .yOffset = 0},
    [OBSTACLE_CACTUS_3] = {.width = 102, .height = 68, .yOffset = 0},
    [OBSTACLE_CACTUS_4] = {.width = 50,  .height = 94, .yOffset = 0},
    [OBSTACLE_CACTUS_5] = {.width = 100, .height = 94, .yOffset = 0},
    [OBSTACLE_CACTUS_6] = {.width = 150, .height = 95, .yOffset = 0},
    [OBSTACLE_BIRD]     = {.width = 93,  .height = 80,  .yOffset = -61}
};
typedef struct {
    Rectangle rect;
    Rectangle collisionRect;
    bool active;
    ObstacleType type;
    int currentFrame;
    float frameTime;
    bool hasPassedPlayer;
} Obstacle;
typedef struct {
    Obstacle obstacles[7];
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
    bool hasDealtDamage;
} Meteor;
typedef struct {
    Vector2 position;
    float speed;
    float scale;
    float alpha;
    bool active;
} Cloud;
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
    Meteor meteors[15];
    float meteorSpawnTimer;
    float nextMeteorSpawnTime;
    int hp;
    bool gameWon;
    Sound jumpSound;
    Sound gameOverSound;
    Sound winSound;
    Sound meteorImpactSound;
    bool soundPlayed;

    Texture2D cloudTexture;
    Cloud clouds[MAX_CLOUDS];
    float cloudSpawnTimer;
    float nextCloudSpawnTime;
} GameState;
typedef struct {
    int width;
    int height;
    bool isFullscreen;
    float scaleFactor;
    ResolutionButton resolutions[4];
    MenuState menu;
    GameStates gameState;
} WindowState;
#endif
