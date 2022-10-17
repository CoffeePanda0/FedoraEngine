#ifndef _H_PAUSE
#define _H_PAUSE

#include "../world/include/camera.h"
#include "../entity/include/player.h"

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