// basic gameobject functions
#include "game.h"

struct ObjList {
	struct GameObject *obj;
	struct ObjList *next;
}; 

struct ObjList *objlist;

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
	// does this cause a memory leak.. yes it does somewhere i think
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

void DestroyObject(struct GameObject* obj) {
	struct ObjList* tmp;
	if (objlist->obj == obj) {
		tmp = objlist;
		objlist = objlist->next;
		SDL_DestroyTexture(tmp->obj->texture);
		SDL_free(tmp->obj)
			SDL_free(tmp);
		return;
	}
	for (struct ObjList* o = objlist; o; o = o->next) {
		if (o->next == obj) {
			tmp = o->next;
			o->next = o->next->next;
			SDL_DestroyTexture(tmp->obj->texture);
			SDL_free(tmp->obj)
				SDL_free(tmp);
			return;
		}
	}
	error("DestroyObject: Object not found!");
}