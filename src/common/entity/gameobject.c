// basic gameobject functions
#include <FE_Common.h>
#include "include/gameobject.h"
#include "../../common/physics/include/physics.h"

FE_List *FE_GameObjects = 0; // list of all gameobjects
size_t FE_GameObjectIDCounter = 0;

FE_GameObject *FE_GameObject_Create_Basic(SDL_Rect body, int mass, char *texture_path, char *name)
{
	/* fill data from passed parameters */
	struct FE_GameObject *obj;
	obj = xmalloc(sizeof(FE_GameObject));
	
	obj->texture_path = mstrdup(texture_path);

	/* set texture to null for a basic object */
	obj->texture = 0;

	/* no callback as we have no texture to destroy */
	obj->destroy_cb = NULL;
	obj->destroy_data = NULL;

	/* create physics object */
	FE_Phys_Rigidbody *p = FE_Physics_CreateBody(mass, body);

	FE_Physics_AddBody(p);

	obj->phys = p;
	obj->name = mstrdup(name);
	obj->id = FE_GameObjectIDCounter++;
	obj->last_position = vec(-1, -1);
	
	FE_List_Add(&FE_GameObjects, obj);

	return obj;
}

void FE_GameObject_Clean() // Destroys all game objects
{
	// destroy all game objects inside every node
	for (FE_List *o = FE_GameObjects; o; o = o->next) {
		FE_GameObject *obj = o->data;
		FE_Physics_Remove(obj->phys);
		if (obj->destroy_cb)
			obj->destroy_cb(obj->destroy_data);
		if (obj->name)
			free(obj->name);
		if (obj->texture_path)
			free(obj->texture_path);

		free(obj);
	}

	FE_List_Destroy(&FE_GameObjects);
}

int FE_GameObject_Destroy(FE_GameObject *obj)
{
	// free gameobject data
	FE_Physics_Remove(obj->phys);
	if (obj->destroy_cb)
		obj->destroy_cb(obj->destroy_data);
	
	if (FE_List_Remove(&FE_GameObjects, obj) == -1) // remove from list
		return -1;
	
	free(obj->name); // free name (if it exists)
	if (obj->texture_path) free(obj->texture_path); // free texture path (if it exists)
	
	free(obj); // free gameobject
	return 1;
}