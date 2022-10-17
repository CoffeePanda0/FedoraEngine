#ifndef H_PLAYER
#define H_PLAYER

#include <stdbool.h>
#include "../../common/physics/include/rigidbody.h"

typedef struct FE_PlayerProperties {
    int id;

    float moveforce;
    float maxspeed;

    float jumpforce;
    float jump_elapsed;
    bool jump_started;

    FE_Phys_Rigidbody *PhysObj;

} FE_PlayerProperties;


/** Moves the player by a small amount (e.g walking, user input)
 *\param player - The player to move
 *\param position - The force to be applied in each direction to the player
*/
void FE_Player_Move(FE_PlayerProperties *player, vec2 movement);


/** Continues to jump higher when called
 *\param player - The player to jump
 */
void FE_Player_UpdateJump(FE_PlayerProperties *player);


/** Begins the player jumping sequence
 * \param player - The player to jump
 */
void FE_Player_StartJump(FE_PlayerProperties *player);


#endif