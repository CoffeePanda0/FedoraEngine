#pragma once
#include "SDL.h"

#define tile_size 64

void RenderMap();
void InitMap(const char* map);

extern bool gBelow(SDL_Rect r);
extern bool gAbove(SDL_Rect r);
extern bool gLeft(SDL_Rect r);
extern bool gRight(SDL_Rect r);

extern int map_height;
extern int map_width;

void MoveCamera(int x, int y);
void InitCamera();

void DestroyMap();
int MapLoaded();

