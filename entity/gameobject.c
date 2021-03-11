// basic gameobject functions
#include "../game.h"

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

void CreateObject(int xPos, int yPos, int height, int width, const char* textPath, struct GameObject *obj, char *name)
{
	if (obj == NULL)
		error("Trying to create GameObject from NULL pointer");
	
	SDL_Rect objRect;
	obj->name = name;
	objRect.x = xPos;
	objRect.y = yPos;
	objRect.w = width;
	objRect.h = height;
	obj->name = name;
	obj->objRect = objRect;
	obj->render_rect = objRect;
	obj->texture = TextureManager(textPath, renderer);
	push(obj);
}

void RenderObject() {
	for (struct ObjList *o = objlist; o; o = o->next) {
		o->obj->objRect.x = o->obj->render_rect.x -scrollam;
		o->obj->objRect.y = o->obj->render_rect.y -hscrollam;
		if (o->obj->render_rect.x - (scrollam - o->obj->render_rect.w)  >= 0 && o->obj->render_rect.x -scrollam <= screen_width) // only bother rendering stuff in bounds
			SDL_RenderCopy(renderer, o->obj->texture, NULL, &o->obj->objRect);

	}
}

void CollisionDetection()
{
	if (objlist != NULL) {
		for (struct ObjList *o = objlist; o; o = o->next) {
			
			if (SDL_HasIntersection(&playerRect, &o->obj->objRect)) {
				//The sides of the rectangles
				int leftA, leftB;
				int rightA, rightB;
				int topA, topB;
				int bottomA, bottomB;

				//Calculate the sides of player rect
				leftA = playerRect.x;
				rightA = playerRect.x + playerRect.w;
				topA = playerRect.y;
				bottomA = playerRect.y + playerRect.h;

				//Calculate the sides of obj rect
				leftB = o->obj->objRect.x;
				rightB = o->obj->objRect.x + o->obj->objRect.w;
				topB = o->obj->objRect.y;
				bottomB = o->obj->objRect.y + o->obj->objRect.h;

				if (bottomA <= topB + 5)
					dir = DIR_ABOVE;

				if (topA >= bottomB - 5)
					dir = DIR_BELOW;

				if (rightA <= leftB + 5)
					dir = DIR_RIGHT;

				if (leftA >= rightB - 5)
					dir = DIR_LEFT;

			} else dir = DIR_NONE;
		}
	}
}

void CleanObjects() // Destroys all game objects
{
	struct ObjList *t;
	t = objlist;
	if (t) {
		while (t->next != NULL) { // loop through list
			t = t->next;
			DestroyObject(t->obj); // get last object
		}
		DestroyObject(t->obj);
	}
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