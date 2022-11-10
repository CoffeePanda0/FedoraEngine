#ifndef H_FEDORAENGINE
#define H_FEDORAENGINE

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "linkedlist.h"

#include "../../../common/net/include/net.h"
#include "vector.h"

typedef enum FE_GAMESTATE {
    GAME_STATE_MENU,
    GAME_STATE_PLAY,
    GAME_STATE_PAUSE,
    GAME_STATE_EDITOR,
    GAME_STATE_CLIENT,
    GAME_STATE_SERVER
} FE_GAMESTATE;

typedef struct FE_DebugConfig {
    bool ShowTiming;
    bool LightingEnabled;
    bool ShowTriggers;
    bool NoClip;
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
    uint8_t AmbientLight;
} FE_MapConfig;

typedef struct FE_AudioConfig {
    uint8_t Volume;
    bool Muted;
} FE_AudioConfig;

typedef struct FE_InitConfig {
    const char *WindowTitle;
    uint16_t WindowWidth, WindowHeight;
    bool Vsync, ShowFPS;
    bool Headless;
    char *default_font;
} FE_InitConfig;

typedef struct FE_Game {
    FE_InitConfig *config;
    struct FE_Client *Client;

    int WindowHeight, WindowWidth;

    bool GameActive;
    FE_GAMESTATE GameState;

    FE_DisconnectInfo DisconnectInfo;

    FE_MapConfig MapConfig;
    FE_AudioConfig AudioConfig;
    FE_Timing Timing;
    FE_DebugConfig DebugConfig;
    struct FE_UIConfig *UIConfig;
} FE_Game;

extern bool FE_GameInitialised;
extern FE_Game *PresentGame;

/* Cleans all resources possible without exiting */
void FE_CleanAll();

/* Returns a FedoraEngine init configuration with default values */
FE_InitConfig *FE_NewInitConfig();

/* Initializes the engine with the given configuration */
void FE_Init(FE_InitConfig *InitConfig);

/* Cleans all resources and exits cleanly */
void FE_Clean();


#endif