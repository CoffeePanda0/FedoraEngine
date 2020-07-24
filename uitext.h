#pragma once
#include <SDL_ttf.h>
#include <SDL.h>

struct TextObject {
	SDL_Rect rect;
	SDL_Texture* texture;
};

struct TextObject ui_fps;

extern TTF_Font* Sans;

SDL_Texture* CreateText(char *text, SDL_Color color, TTF_Font *font);
SDL_Texture * tmpTexttext;

void InitDebugOverlay();
void TextDebugOverlay();
void FreeText(struct TextObject *obj);

void NewText(struct TextObject *obj, char *text, SDL_Color color, int xPos, int yPos);

void FreeOverlay();

void RenderText();

void UpdateText(struct TextObject *obj, char *text, SDL_Color color);

extern SDL_Color Black;