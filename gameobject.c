#include "game.h"

void CreateObject(int xPos, int yPos, int height, int width, const char* textPath, struct GameObject *obj)
{
	// does this cause a memory leak.. yes it does somewhere i think
	SDL_Rect objRect;
	SDL_Texture* texture;

	obj->objRect.x = xPos;
	obj->objRect.y = yPos;
	obj->objRect.w = width;
	obj->objRect.h = height;
	obj->texture = TextureManager(textPath, renderer);
}

void RenderObject(struct GameObject obj)
{
	SDL_RenderCopy(renderer, obj.texture, NULL, &obj.objRect);
}

void DestroyObject(struct GameObject *obj)
{
	SDL_DestroyTexture(obj->texture);
}