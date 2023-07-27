// basic gameobject functions
#include "../core/include/include.h"
#include "../../common/physics/include/physics.h"
#include "../../common/physics/include/collision.h"
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

int FE_GameObject_CollisionAbove(SDL_Rect *o)
{
	for (FE_List *f = FE_GameObjects; f; f = f->next) {
		FE_GameObject *obj = f->data;

		if (obj->phys->mass == 0) /* Skip if no mass */
			continue;

		SDL_Rect b = (SDL_Rect){obj->phys->position.x, obj->phys->position.y, obj->phys->body.w, obj->phys->body.h};
		
		if (FE_AABB_Collision(o, &b))
			return b.y;
	}

	return -1;
}

FE_GameObject *FE_GameObject_Create(SDL_Rect body, const char *texture_path, int mass, char *name)
{
	if (!texture_path) {
		warn("NULL texture_path passed to FE_GameObject_Create");
		return 0;
	}
	
	/* fill data from passed parameters */
	struct FE_GameObject *obj;
	obj = xmalloc(sizeof(FE_GameObject));
	
	obj->texture_path = mstrdup(texture_path);
	obj->id = FE_GameObjectIDCounter++;
	obj->name = mstrdup(name);

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
	obj->last_position = vec(-1, -1);

	FE_List_Add(&FE_GameObjects, obj);
	return obj;
}
