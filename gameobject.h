#pragma once
#include "SDL.h"

struct GameObject {
	SDL_Rect objRect;
	SDL_Texture* texture;
};

void RenderObject(struct GameObject obj);
void DestroyObject(struct GameObject *obj);
void CreateObject(int xPos, int yPos, int height, int width, const char* textPath, struct GameObject *obj);