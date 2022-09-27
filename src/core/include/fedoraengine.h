#ifndef H_FEDORAENGINE
#define H_FEDORAENGINE

#include <stdint.h>
#include <SDL_gpu.h>
#include <stdbool.h>
#include "../../core/include/vector.h"
#include "../../ui/include/font.h"
#include "../../core/include/linkedlist.h"
#include "../../web/net.h"

typedef enum FE_GAMESTATE {
    GAME_STATE_MENU,
    GAME_STATE_PLAY,
    GAME_STATE_PAUSE,
    GAME_STATE_EDITOR,
    GAME_STATE_MULTIPLAYER,
} FE_GAMESTATE;

typedef struct FE_DebugConfig {
    bool ShowTiming;
    bool LightingEnabled;
    bool ShowTriggers;
} FE_DebugConfig;

typedef struct FE_UIList {
    FE_List *Containers;
    FE_List *Labels;
    FE_List *Buttons;
    FE_List *Checkboxes;
    FE_List *Objects;
    FE_List *Textboxes;
    FE_List *Grids;
    size_t Count;
} FE_UIList;

typedef struct FE_UIConfig {
    FE_UIList *ActiveElements;
    FE_Font *UIFont;
    SDL_Color UIFontColor;

    bool DialogueActive;
    bool InText;
    bool MBShown;

} FE_UIConfig;

typedef struct FE_Timing {
    long double EventTime;
    long double UpdateTime;
    long double RenderTime;
} FE_Timing;

typedef struct FE_MapConfig {
    bool Loaded;
    uint16_t MapWidth, MapHeight, MinimumX;
    vec2 PlayerSpawn;
    float Gravity;
    Uint8 AmbientLight;
} FE_MapConfig;

typedef struct FE_AudioConfig {
    uint8_t Volume;
    bool Muted;
} FE_AudioConfig;

typedef struct FE_InitConfig {
    const char *window_title;
    uint16_t WindowWidth, WindowHeight;
    bool vsync, show_fps;
    bool headless;
    char *default_font;
} FE_InitConfig;

typedef struct FE_Game {
    FE_InitConfig *config;
    FE_Font *font;

    int WindowHeight, WindowWidth;
    bool FullScreen;
    
    bool ConsoleVisible;
    bool StartedInput;

    int DialogueSpeed;
    
    SDL_Window *Window;
    GPU_Target *Screen;
    
    bool GameActive;
    FE_GAMESTATE GameState;

    FE_DisconnectInfo DisconnectInfo;

    FE_MapConfig MapConfig;
    FE_AudioConfig AudioConfig;
    FE_Timing Timing;
    FE_DebugConfig DebugConfig;
    FE_UIConfig UIConfig;
} FE_Game;

extern bool FE_GameInitialised;
extern FE_Game *PresentGame;

#endif