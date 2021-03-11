#pragma once
#include "SDL.h"


struct GameObject {
	bool interactable;
	char *name;
	SDL_Rect objRect;
	SDL_Rect render_rect;
	SDL_Texture* texture;
};

enum CollDir {
	DIR_NONE,
	DIR_LEFT,
	DIR_RIGHT,
	DIR_ABOVE,
	DIR_BELOW,
};

extern enum CollDir dir;

void CleanObjects();
void RenderObject();
void CollisionDetection();
void DestroyObject(struct GameObject *obj);
void CreateObject(int xPos, int yPos, int height, int width, const char* textPath, struct GameObject *obj, char *name);