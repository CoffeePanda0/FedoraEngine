#ifndef _H_CLIENT_GAMEOBJECT
#define _H_CLIENT_GAMEOBJECT

#include "../../../common/entity/include/gameobject.h"
#include "../../world/include/camera.h"

/** Renders all game objects
 *\param c - The camera currently in use
*/
void FE_GameObject_Render(FE_Camera *c);


/** Creates and initializes a new gameobject
 *\param body - The rectangle to draw of the object
 *\param texture_path - The name of the texture in the assets directory
 *\param mass - The mass to be applied to the physics of the object (or 0 to not fall)
 *\returns A pointer to the new object
*/
FE_GameObject *FE_GameObject_Create(SDL_Rect body, const char *texture_path, int mass);


#endif