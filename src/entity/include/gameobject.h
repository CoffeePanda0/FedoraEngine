#ifndef H_GAMEOBJECT
#define H_GAMEOBJECT

#include <SDL.h>
#include "../../world/include/physics.h"
#include "../../world/include/camera.h"
#include "../../core/include/texture.h"
#include "../../core/include/resourcemanager.h"

enum FE_ObjectType {
	ENEMY,
};

typedef struct FE_GameObject {
	char *name;
	enum FE_ObjectType type;
	FE_PhysObj *phys;
	FE_Texture* texture;
} FE_GameObject;


/* Destroys and frees memory used by all gameobjects */
void FE_CleanGameObjects();


/** Renders all game objects
 *\param c - The camera currently in use
*/
void FE_RenderGameObjects(FE_Camera *c);


/** Frees and destroys resources from a gameobject
 *\param obj - The gameobject to destroy
 *\return -1 on error, 1 on success
*/
int FE_DestroyGameObject(FE_GameObject *obj);


/** Creates and initializes a new gameobject
 *\param r - The rectangle to draw of the object
 *\param texture_path - The name of the texture in the assets directory
 *\param name - The name to be displayed on-screen for the object
 *\param type - The type of object (e.g entity, enemy)
 *\param mass - The mass to be applied to the physics of the object (or 0 to not fall)
 *\param moveable - Whether the object can be moved by another object (or player)
 *\returns A pointer to the new object
*/
FE_GameObject *FE_CreateGameObject(SDL_Rect r, const char *texture_path, char *name, enum FE_ObjectType type, int mass, bool moveable);

#endif