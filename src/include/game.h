#ifndef H_GAME
#define H_GAME

#include "include.h"
#include "audio.h"

#include "../core/include/resourcemanager.h"
#include "../core/include/fedoraengine.h"

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
#include "../core/include/render.h"

#include "../ui/include/ui.h"

extern void (*MenuPage)(); // pointer to current menu page function for redrawing

void FE_Init(FE_InitConfig *InitConfig);
void FE_Clean();

/* Loads a map and initalises a game */
void FE_StartGame(const char *mapname);

/* The main game loop. Calls rendering, game event handling and update */
void FE_GameLoop();

/* Cleans all resources possible without exiting */
void FE_CleanAll();

/* Returns a FedoraEngine init configuration with default values */
FE_InitConfig *FE_NewInitConfig();

void FE_GameEventHandler(FE_Camera *camera, FE_Player *player);

#endif