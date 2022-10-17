#ifndef H_GAMEOBJECT
#define H_GAMEOBJECT

#include "../../../common/physics/include/physics.h"
#include "../../world/include/camera.h"
#include "../../core/include/texture.h"
#include "../../core/include/resourcemanager.h"

typedef struct FE_GameObject {
	FE_Phys_Rigidbody *phys;
	struct FE_Texture* texture;
} FE_GameObject;


/* Destroys and frees memory used by all gameobjects */
void FE_GameObject_Clean();


/** Renders all game objects
 *\param c - The camera currently in use
*/
void FE_GameObject_Render(FE_Camera *c);


/** Frees and destroys resources from a gameobject
 *\param obj - The gameobject to destroy
 *\return -1 on error, 1 on success
*/
int FE_GameObject_Destroy(FE_GameObject *obj);


/** Creates and initializes a new gameobject
 *\param body - The rectangle to draw of the object
 *\param texture_path - The name of the texture in the assets directory
 *\param name - The name to be displayed on-screen for the object
 *\param mass - The mass to be applied to the physics of the object (or 0 to not fall)
 *\returns A pointer to the new object
*/
FE_GameObject *FE_GameObject_Create(SDL_Rect body, const char *texture_path, int mass);

#endif