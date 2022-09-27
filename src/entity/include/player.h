#ifndef H_PLAYER
#define H_PLAYER

#include <SDL_gpu.h>
#include <stdbool.h>
#include "../../core/include/vector.h"
#include "animation.h"
#include "../../world/include/camera.h"
#include "../../physics/include/physics.h"
#include "../../world/include/lighting.h"

typedef struct FE_Player {
    int id;

    float moveforce;
    float maxspeed;
    
    float jumpforce;
    float jump_elapsed;
    int last_y_check;
    bool jump_started;
    
    bool on_ground;
    bool moving;
    bool facing_right;

    FE_Phys_Rigidbody *PhysObj;

    GPU_Rect render_rect; // the location displayed on the screen
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
FE_Player *FE_Player_Create(float acceleration, float maxspeed, float jumpforce, GPU_Rect body);


/** Moves the player by a small amount (e.g walking, user input)
 *\param player - The player to move
 *\param camera - The current camera in use
 *\param position - The force to be applied in each direction to the player
*/
void FE_Player_Move(FE_Player *player, vec2 movement);


/** The main update loop needed to update the player's properties and for the camera to follow
 *\param player - The player to update
 *\param camera - The current camera in use
*/
void FE_Player_Update(FE_Player *player);


/** Makes the player jump if they are on the ground
 *\param jump_duration - How long the jump button was held for (longer means higher jump)
 *\param player - The player to jump
 *\param camera - The current camera in use
 */
void FE_PlayerJump(size_t jump_duration, FE_Player *player, FE_Camera *camera);


/** Continues to jump higher when called
 *\param player - The player to jump
 */
void FE_Player_UpdateJump(FE_Player *player);


/** Begins the player jumping sequence
 * \param player - The player to jump
 */
void FE_Player_StartJump(FE_Player *player);


/** Destroys and frees all resources used by a player
 * \param player - The player to destroy
 */
void FE_Player_Destroy(FE_Player *player);


#endif