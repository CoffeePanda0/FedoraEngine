// basic gameobject functions
#include "include/gameobject.h"
#include "../core/include/linkedlist.h"
#include "../core/include/fedoraengine.h"
#include "../core/include/utils.h"
#include "../world/include/physics.h"

#define AssetPath "game/sprites/"

static FE_List *FE_GameObjects = 0; // list of all gameobjects

FE_GameObject *FE_CreateGameObject(SDL_Rect r, const char *texture_path, char *name, enum FE_ObjectType type, int mass, bool moveable)
{
	/* fill data from passed parameters */
	struct FE_GameObject *obj;
	obj = xmalloc(sizeof(*obj));
	obj->name = name;

	// combine asset path and texture path
	char *path = xmalloc(strlen(AssetPath) + strlen(texture_path) + 1);
	strcpy(path, AssetPath);
	strcat(path, texture_path);
	obj->texture = FE_LoadResource(FE_RESOURCE_TYPE_TEXTURE, path);
	free(path);
	
	obj->type = type;

	/* create physics object */
	FE_PhysObj *p = FE_CreatePhysObj(mass, 128, r, moveable);

	FE_AddPhysInteractable(p);

	obj->phys = p;
	FE_List_Add(&FE_GameObjects, obj);
	return obj;
}


void FE_RenderGameObjects(FE_Camera *c)
{
	for (FE_List *o = FE_GameObjects; o; o = o->next) {
		FE_GameObject *obj = o->data;
		SDL_Rect renderrect = (SDL_Rect){obj->phys->body.x - c->x, obj->phys->body.y - c->y, obj->phys->body.w, obj->phys->body.h};
		SDL_RenderCopy(PresentGame->renderer, obj->texture->Texture, NULL, &renderrect);
	}
}

void FE_CleanGameObjects() // Destroys all game objects
{
	// destroy all game objects inside every node
	for (FE_List *o = FE_GameObjects; o; o = o->next) {
		FE_GameObject *obj = o->data;
		FE_RemovePhysInteractable(obj->phys);
		FE_DestroyResource(obj->texture->path);
		free(obj);
	}

	FE_List_Destroy(&FE_GameObjects);
}

int FE_DestroyGameObject(FE_GameObject *obj)
{
	// free gameobject data
	FE_RemovePhysInteractable(obj->phys);
	free(obj->phys);
	FE_DestroyResource(obj->texture->path);
	
	if (FE_List_Remove(&FE_GameObjects, obj) == -1) // remove from list
		return -1;
	
	free(obj); // free gameobject
	return 1;
}