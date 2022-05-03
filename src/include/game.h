#ifndef H_GAME
#define H_GAME

#include "include.h"
#include "audio.h"

#include "../entity/include/gameobject.h"
#include "../entity/include/timer.h"
#include "../entity/include/light.h"
#include "../entity/include/animation.h"
#include "../entity/include/player.h"
#include "../entity/include/particle.h"

#include "../world/include/camera.h"
#include "../world/include/physics.h"
#include "../world/include/map.h"

#include "../editor/editor.h"

#include "../core/include/utils.h"
#include "../core/include/texture.h"
#include "../core/include/circle.h"
#include "../core/include/vector2d.h"
#include "../core/include/linkedlist.h"
#include "../core/include/timing.h"

#include "../ui/include/ui.h"

typedef struct FE_InitConfig {
    const char *window_title;
    Uint16 window_width, window_height;
    bool vsync;
} FE_InitConfig;

enum FE_GAMESTATE {
    GAME_STATE_MENU,
    GAME_STATE_PLAY,
    GAME_STATE_EDITOR,
    GAME_STATE_PAUSE
};

extern enum FE_GAMESTATE FE_GameState;
extern void (*MenuPage)(); // pointer to current menu page function for redrawing

extern int screen_height;
extern int screen_width;

void FE_Init(FE_InitConfig InitConfig);
void FE_Clean();

extern bool FE_GameActive;

extern TTF_Font* Sans;

extern SDL_Window* window;
extern SDL_Renderer* renderer;


/* Loads a map and initalises a game */
void FE_StartGame(const char *mapname);

/* The main game loop. Calls rendering, game event handling and update */
void FE_GameLoop();

/* Cleans all resources possible without exiting */
void FE_CleanAll();

void FE_GameEventHandler(FE_Camera *camera, FE_Player *player);

#endif