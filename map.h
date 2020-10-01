#pragma once
#include "SDL.h"

void RenderMap();
void InitMap(char* map);

extern bool gBelow;
extern bool gAbove;
extern bool gLeft;
extern bool gRight;

void ListObjects();