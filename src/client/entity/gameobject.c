// basic gameobject functions
#include "../core/include/include.h"
#include "../../common/physics/include/physics.h"
#include "include/gameobject.h"

#define AssetPath "game/sprites/"

void FE_GameObject_Render(FE_Camera *c)
{
	for (FE_List *o = FE_GameObjects; o; o = o->next) {
		FE_GameObject *obj = o->data;
        if (obj->texture)
		    FE_RenderCopy(c, false, obj->texture, NULL, &obj->phys->body);
	}
}

FE_GameObject *FE_GameObject_Create(SDL_Rect body, const char *texture_path, int mass)
{
	if (!texture_path) {
		warn("NULL texture_path passed to FE_GameObject_Create");
		return 0;
	}
	
	/* fill data from passed parameters */
	struct FE_GameObject *obj;
	obj = xmalloc(sizeof(FE_GameObject));


	// combine asset path and texture path
	char *path = xmalloc(mstrlen(AssetPath) + mstrlen(texture_path) + 1);
	mstrcpy(path, AssetPath);
	mstrcat(path, texture_path);
	obj->texture = FE_LoadResource(FE_RESOURCE_TYPE_TEXTURE, path);
	free(path);

	/* Set the destroy callback to free the texture */
	obj->destroy_cb = &FE_DestroyResource;
	obj->destroy_data = obj->texture->path;
	
	/* create physics object */
	FE_Phys_Rigidbody *p = FE_Physics_CreateBody(mass, body);

	FE_Physics_AddBody(p);

	obj->phys = p;
	FE_List_Add(&FE_GameObjects, obj);
	return obj;
}
