#pragma once
#include <SDL_ttf.h>
#include <SDL.h>

extern TTF_Font* Sans;
SDL_Texture* CreateText(char *text, SDL_Color color, TTF_Font *font);
SDL_Texture * tmpTexttext;

void InitDebugOverlay();
void TextDebugOverlay();

extern SDL_Color Black;