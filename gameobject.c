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
	if (objlist != NULL) {
		for (struct ObjList *o = objlist; o; o = o->next) {

			if (SDL_HasIntersection(&playerRect, &o->obj->objRect)) {
				// If collided check which direction
				SDL_Rect out;
				SDL_IntersectRect(&playerRect, &o->obj->objRect, &out);

				int RightDifference = out.x - playerRect.x;
				int LeftDifference = out.x - playerRect.x - playerRect.w;

				int TopDifference = playerRect.y - o->obj->objRect.y + o->obj->objRect.h;
				
				if (TopDifference < 0)
					dir = DIR_ABOVE;
				else if (TopDifference > playerRect.h)
					dir = DIR_BELOW;
				else if (RightDifference > 0)
					dir = DIR_RIGHT;
				else if (LeftDifference < 0 && RightDifference == 0)
					dir = DIR_LEFT;
				
			} else
				dir = DIR_NONE;
		}
	} else 
		dir = DIR_NONE;
}

void DestroyObject (struct GameObject *obj) {

	struct ObjList *tmp;
	// cheers to gibson for help writing this, pointer and linked lists go aee
	if (objlist->obj == obj) {
		SDL_DestroyTexture(obj->texture);
		tmp = objlist;
		objlist = objlist->next;
		free(tmp);
	} else {
		for (struct ObjList *t = objlist; t && t->next; t = t->next) {
			if (t->next->obj == obj) {
				SDL_DestroyTexture(t->next->obj->texture);
				tmp = t->next;
				t->next = t->next->next;
				free(tmp);
			}
		}
	}
}