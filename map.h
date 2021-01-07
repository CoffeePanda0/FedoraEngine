#pragma once
#include "SDL.h"

#define tile_size 26

void RenderMap();
void InitMap(char* map);

extern bool gBelow();
extern bool gAbove();
extern bool gLeft();
extern bool gRight();

extern int map_height;
extern int map_width;

void MoveCamera(int x, int y);
void InitCamera();

int MapLoaded();

