#ifndef _H_GAMEOBJECT
#define _H_GAMEOBJECT

#include <stdint.h>

#include "../../physics/include/physics.h"

typedef struct FE_GameObject {
	FE_Phys_Rigidbody *phys;
	struct FE_Texture* texture;
	char *texture_path;

	int (*destroy_cb)(char*);
	void *destroy_data;

	char *name;
	size_t id;

	vec2 last_position; // can be ignored, for networking
} FE_GameObject;

extern FE_List *FE_GameObjects;
extern size_t FE_GameObjectIDCounter;

/** Creates a basic gameobject (only consiting of a physics body, no rendering)
 *\param body - The rectangle to draw of the object
 *\param mass - The mass to be applied to the physics of the object (or 0 to not fall)
 *\param name - The name of the object (does not have to be unique)
 *\returns A pointer to the new object
*/
FE_GameObject *FE_GameObject_Create_Basic(SDL_Rect body, int mass, char *texture_path, char *name);


/* Destroys and frees memory used by all gameobjects */
void FE_GameObject_Clean();


/** Frees and destroys resources from a gameobject
 *\param obj - The gameobject to destroy
 *\return -1 on error, 1 on success
*/
int FE_GameObject_Destroy(FE_GameObject *obj);


#endif