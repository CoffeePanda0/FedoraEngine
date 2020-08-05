#pragma once
#include "SDL.h"


struct GameObject {
	SDL_Rect objRect;
	SDL_Texture* texture;
};

enum CollDir {
	DIR_LEFT,
	DIR_RIGHT,
	DIR_ABOVE,
	DIR_BELOW,
	DIR_NONE,
};

extern enum CollDir dir;

void RenderObject();
void CollisionDetection();
void DestroyObject(struct GameObject *obj);
void CreateObject(int xPos, int yPos, int height, int width, const char* textPath, struct GameObject *obj);