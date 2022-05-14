#ifndef H_FEDORAENGINE
#define H_FEDORAENGINE

#include <stdint.h>
#include <stdbool.h>
#include "../../ui/include/font.h"
#include "vector2d.h"

typedef enum FE_GAMESTATE {
    GAME_STATE_MENU,
    GAME_STATE_PLAY,
    GAME_STATE_PAUSE,
    GAME_STATE_EDITOR
} FE_GAMESTATE;

typedef struct FE_MapConfig {
    uint16_t MapWidth, MapHeight, MinimumX;
    Vector2D PlayerSpawn;
    float Gravity;
    Uint8 AmbientLight;
} FE_MapConfig;

typedef struct FE_AudioConfig {
    uint8_t Volume;
    bool Muted;
} FE_AudioConfig;

typedef struct FE_InitConfig {
    const char *window_title;
    uint16_t window_width, window_height;
    bool vsync, show_fps;
    char *default_font;
} FE_InitConfig;

typedef struct FE_Game {
    FE_Font *font;
    FE_InitConfig *config;
    int window_height, window_width;

    bool ConsoleVisible;
    bool DialogueActive;
    bool InText;
    bool StartedInput;
    bool MBShown;
    
    SDL_Window *window;
    SDL_Renderer *renderer;
    
    bool GameActive;
    FE_GAMESTATE GameState;

    FE_MapConfig MapConfig;
    FE_AudioConfig AudioConfig;

} FE_Game;

extern bool FE_GameInitialised;
extern FE_Game *PresentGame;

#endif