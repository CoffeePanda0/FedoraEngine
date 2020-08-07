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

			int RightDifference = out.x - playerRect.x;
			int LeftDifference = out.x - playerRect.x - playerRect.w;

			int TopDifference = o->obj->objRect.y - playerRect.y - o->obj->objRect.h;
			int Bottom = o->obj->objRect.y + o->obj->objRect.h;

			if (TopDifference > o->obj->objRect.h) {
				dir = DIR_ABOVE;
			} else if (playerRect.y + playerRect.h > Bottom) {
				dir = DIR_BELOW;
			} else if (RightDifference > 0) {
				dir = DIR_RIGHT;
			} else if (LeftDifference < 0 && RightDifference == 0)
				dir = DIR_LEFT;
		} else
			dir = DIR_NONE;
	}
}

void DestroyObject(struct GameObject *obj) {
	error("DestroyObject: Object not found!");
}