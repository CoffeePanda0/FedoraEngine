#ifndef _GAME_H
#define _GAME_H

#include "include.h"

#include "../core/include/include.h"
#include "../core/include/keymap.h"

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
#include "../ui/include/include.h"
#include "../ui/include/menu.h"

/** Loads a map and initalises a game
 * \param mapname The name of the map to load
 */
void FE_StartGame(const char *mapname);

/* The main game loop. Calls rendering, game event handling and update */
void FE_GameLoop();

/* Pauses the game */
void FE_PauseGame();

/* Unpauses the game */
void FE_ResumeGame();

/* Renders the pause menu */
void FE_Pause_Render();

/** Handles game events
 * \param camera The camera to be used for event handling
 * \param player The player to be used for event handling
*/
void FE_GameEventHandler(FE_Camera *camera, FE_Player *player);

#endif