#ifndef _GAME_H
#define _GAME_H

#include <FE_Client.h>

/** Loads a map and initalises a game
 * \param mapname The name of the map to load
 */
void FE_StartGame(const char *mapname);


/* The main game loop. Calls rendering, game event handling and update */
void FE_GameLoop();


/* Initialises FedoraEngine to run as a regular client. NOTE: Must be called after subsystem initialisation */
void FE_Client_Init(FE_InitConfig *InitConfig);


#endif