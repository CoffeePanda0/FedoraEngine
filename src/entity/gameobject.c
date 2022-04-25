// basic gameobject functions
#include "../include/game.h"

#define AssetPath "game/assets/"

static FE_List *FE_GameObjects = 0; // list of all gameobjects

FE_GameObject *FE_CreateGameObject(SDL_Rect r, const char *texture_path, char *name, enum FE_ObjectType type, int mass)
{
	/* fill data from passed parameters */
	struct FE_GameObject *obj;
	obj = xmalloc(sizeof(*obj));
	obj->name = name;

	// combine asset path and texture path
	char *path = xmalloc(strlen(AssetPath) + strlen(texture_path) + 1);
	strcpy(path, AssetPath);
	strcat(path, texture_path);
	obj->texture = FE_LoadTexture(path);
	free(path);
	
	obj->type = type;

	/* create physics object */
	FE_PhysObj *p = xmalloc(sizeof(FE_PhysObj));
	p->body = r;
	p->velocity = VEC_EMPTY;
	p->mass = mass;

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
		SDL_RenderCopy(renderer, obj->texture, NULL, &renderrect);
	}
}

void FE_CleanGameObjects() // Destroys all game objects
{
	// destroy all game objects inside every node
	for (FE_List *o = FE_GameObjects; o; o = o->next) {
		FE_GameObject *obj = o->data;
		FE_RemovePhysInteractable(obj->phys);
		FE_FreeTexture(obj->texture);
		free(obj);
	}

	FE_List_Destroy(&FE_GameObjects);
}

int FE_DestroyGameObject(FE_GameObject *obj)
{
	// free gameobject data
	FE_RemovePhysInteractable(obj->phys);
	free(obj->phys);
	FE_FreeTexture(obj->texture);
	
	if (FE_List_Remove(&FE_GameObjects, obj) == -1) // remove from list
		return -1;
	
	free(obj); // free gameobject
	return 1;
}