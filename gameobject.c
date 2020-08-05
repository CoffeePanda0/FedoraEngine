// basic gameobject functions
#include "game.h"

struct ObjList {
	struct GameObject *obj;
	struct ObjList *next;
}; 

struct ObjList *objlist;
enum CollDir dir;

void push (struct GameObject *obj)
{
	struct ObjList *newobj;
	newobj = malloc(sizeof(*newobj));
	newobj->obj = obj;
	newobj->next = objlist;
	objlist = newobj;
}

void CreateObject(int xPos, int yPos, int height, int width, const char* textPath, struct GameObject *obj)
{
	struct objList *objList;
	SDL_Rect objRect;
	SDL_Texture* texture;

	obj->objRect.x = xPos;
	obj->objRect.y = yPos;
	obj->objRect.w = width;
	obj->objRect.h = height;
	obj->texture = TextureManager(textPath, renderer);
	push(obj);
}

void RenderObject() {
	for (struct ObjList *o = objlist; o; o = o->next)
		SDL_RenderCopy(renderer, o->obj->texture, NULL, &o->obj->objRect);
}

void CollisionDetection()
{
	for (struct ObjList *o = objlist; o; o = o->next) {
		if (SDL_HasIntersection(&playerRect, &o->obj->objRect)) {
			// If collided check which direction
			SDL_Rect out;
			SDL_IntersectRect(&playerRect, &o->obj->objRect, &out);

			SDL_free(&out);
		} else
			dir = DIR_NONE;
	}
}

void DestroyObject(struct GameObject *obj) {
	error("DestroyObject: Object not found!");
}