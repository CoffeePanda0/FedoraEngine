#ifndef _GAME_H
#define _GAME_H

#include <FE_Client.h>

#include "../common/physics/include/physics.h"
#include "../world/include/include.h"


/** Loads a map and initalises a game
 * \param mapname The name of the map to load
 */
void FE_StartGame(const char *mapname);


/* The main game loop. Calls rendering, game event handling and update */
void FE_GameLoop();


#endif