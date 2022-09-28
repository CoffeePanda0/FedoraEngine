// basic gameobject functions
#include "include/gameobject.h"
#include "../core/include/include.h"
#include "../physics/include/physics.h"

#define AssetPath "game/sprites/"

static FE_List *FE_GameObjects = 0; // list of all gameobjects

FE_GameObject *FE_GameObject_Create(GPU_Rect r, const char *texture_path, int mass)
{
	if (!texture_path) {
		warn("NULL texture_path passed to FE_GameObject_Create");
		return 0;
	}
	
	/* fill data from passed parameters */
	struct FE_GameObject *obj;
	obj = xmalloc(sizeof(*obj));

	// combine asset path and texture path
	char *path = xmalloc(mstrlen(AssetPath) + mstrlen(texture_path) + 1);
	mstrcpy(path, AssetPath);
	mstrcat(path, texture_path);
	obj->texture = FE_LoadResource(FE_RESOURCE_TYPE_TEXTURE, path);
	free(path);
	
	/* create physics object */
	FE_Phys_Rigidbody *p = FE_Physics_CreateBody(mass, r);

	FE_Physics_AddBody(p);

	obj->phys = p;
	FE_List_Add(&FE_GameObjects, obj);
	return obj;
}


void FE_GameObject_Render(FE_Camera *c)
{
	for (FE_List *o = FE_GameObjects; o; o = o->next) {
		FE_GameObject *obj = o->data;
		FE_RenderCopy(0, c, false, obj->texture, NULL, &obj->phys->body);
	}
}

void FE_GameObject_Clean() // Destroys all game objects
{
	// destroy all game objects inside every node
	for (FE_List *o = FE_GameObjects; o; o = o->next) {
		FE_GameObject *obj = o->data;
		FE_Physics_Remove(obj->phys);
		FE_DestroyResource(obj->texture->path);
		free(obj);
	}

	FE_List_Destroy(&FE_GameObjects);
}

int FE_GameObject_Destroy(FE_GameObject *obj)
{
	// free gameobject data
	FE_Physics_Remove(obj->phys);
	FE_DestroyResource(obj->texture->path);
	
	if (FE_List_Remove(&FE_GameObjects, obj) == -1) // remove from list
		return -1;
	
	free(obj); // free gameobject
	return 1;
}