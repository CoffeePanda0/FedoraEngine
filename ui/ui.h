// HOLDS UI VALUES (COLOURS, FONTS and functions for UI.c)
#pragma once
#include <SDL_ttf.h>
#include <SDL.h>

#define blue (SDL_Color){0, 102, 255}
#define Black (SDL_Color){0, 0, 0}
#define White (SDL_Color){255, 255, 255}

struct TextObject {
	SDL_Rect rect;
	SDL_Texture* texture;
};

extern TTF_Font* Sans;

SDL_Texture* CreateText(char *text, SDL_Color color, TTF_Font *font);
SDL_Texture * tmpTexttext;

void UIText(char *text, char *speaker);
void FreeText(struct TextObject *obj);

void NewText(struct TextObject *obj, char *text, SDL_Color color, int xPos, int yPos);

void RenderText();
void UITextInteract(int option);
void UpdateText(struct TextObject *obj, char *text, SDL_Color color);
void UITextUpdate(int option, char* text);
