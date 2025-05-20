#include "sound.h"
#include "raylib.h"
#include "types.h"

void InitSounds(GameState* game) {
    InitAudioDevice();
    
    game->jumpSound = LoadSound("resources/jump.wav");
    game->gameOverSound = LoadSound("resources/fail.wav");
    game->winSound = LoadSound("resources/win.wav");
    game->meteorImpactSound = LoadSound("resources/meteor.wav");
    
    game->soundPlayed = false;
}

void UnloadSounds(GameState* game) {
    UnloadSound(game->jumpSound);
    UnloadSound(game->gameOverSound);
    UnloadSound(game->winSound);
    UnloadSound(game->meteorImpactSound);
    
    CloseAudioDevice();
}

void PlayJumpSound(GameState* game) {
    PlaySound(game->jumpSound);
}

void PlayGameOverSound(GameState* game) {
    if (!game->soundPlayed) {
        PlaySound(game->gameOverSound);
        game->soundPlayed = true;
    }
}

void PlayWinSound(GameState* game) {
    if (!game->soundPlayed) {
        PlaySound(game->winSound);
        game->soundPlayed = true;
    }
}

void PlayMeteorImpactSound(GameState* game) {
    PlaySound(game->meteorImpactSound);
}

void StopAllSounds(GameState* game) {
    StopSound(game->jumpSound);
    StopSound(game->gameOverSound);
    StopSound(game->winSound);
    StopSound(game->meteorImpactSound);
}
