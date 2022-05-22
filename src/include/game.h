#ifndef H_GAME
#define H_GAME

#include "include.h"
#include "../core/include/include.h"

#include "audio.h"
#include "init.h"

#include "../entity/include/gameobject.h"
#include "../entity/include/timer.h"
#include "../entity/include/animation.h"
#include "../entity/include/player.h"
#include "../entity/include/particle.h"
#include "../entity/include/trigger.h"

#include "../world/include/include.h"
#include "../editor/editor.h"
#include "../ui/include/ui.h"

extern void (*MenuPage)(); // pointer to current menu page function for redrawing


/* Loads a map and initalises a game */
void FE_StartGame(const char *mapname);

/* The main game loop. Calls rendering, game event handling and update */
void FE_GameLoop();


void FE_GameEventHandler(FE_Camera *camera, FE_Player *player);

#endif