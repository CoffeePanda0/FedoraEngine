#ifndef _H_CLIENT_PLAYER
#define _H_CLIENT_PLAYER

#include <SDL.h>
#include <vector.h>
#include "animation.h"
#include "../../world/include/camera.h"
#include "../../../common/physics/include/physics.h"
#include "../../world/include/lighting.h"

#include "../../../common/include/player.h"

typedef struct FE_Player {
    FE_PlayerProperties *player;

    bool on_ground;
    bool moving;
    bool facing_right;

    int last_y_check;

    SDL_Rect render_rect; // the location displayed on the screen
    FE_Light *Light;
    
    FE_Animation *idle_animation;
    FE_Animation *walk_animation;
    FE_Animation *jump_animation;
    
} FE_Player;


/** Renders a player
* \param player The player to render
* \param camera The camera to render the player in
*/
void FE_Player_Render(FE_Player *player, FE_Camera *camera);


/** Creates a new player
 *\param acceleration - The speed that the player accelerates by when moving
 *\param jumpforce - The force for the player to jump at
 *\param body - The rectangle for the player's size and position in the world
*/
FE_Player *FE_Player_Create(float acceleration, float maxspeed, float jumpforce, SDL_Rect body);


/** The main update loop needed to update the player's properties and for the camera to follow
 *\param player - The player to update
 *\param camera - The current camera in use
*/
void FE_Player_Update(FE_Player *player);


/** Destroys and frees all resources used by a player
 * \param player - The player to destroy
 */
void FE_Player_Destroy(FE_Player *player);


#endif