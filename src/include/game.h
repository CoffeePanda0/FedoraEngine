#ifndef H_GAME
#define H_GAME

#include "include.h"

#include "../entity/include/gameobject.h"
#include "../entity/include/timer.h"
#include "../entity/include/light.h"
#include "../entity/include/animation.h"

#include "../world/include/camera.h"
#include "../world/include/physics.h"
#include "../world/include/map.h"

#include "../editor/editor.h"

#include "../core/include/utils.h"
#include "../core/include/texture.h"
#include "../core/include/circle.h"
#include "../core/include/vector2d.h"

#include "../ui/include/ui.h"
#include "audio.h"
#include "linkedlist.h"

#define LEN(x) (sizeof(x)/sizeof(x[0]))

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

extern unsigned long int FE_LastUpdate;
extern float dT;

void FE_init(const char* window_title, int xpos, int ypos, int window_width, int window_height);
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

void FE_GameEventHandler(FE_Camera *camera);

#endif