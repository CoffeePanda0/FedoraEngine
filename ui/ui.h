// HOLDS UI VALUES (COLOURS, FONTS and functions for UI.c)
#pragma once
#include <SDL_ttf.h>
#include <SDL.h>

#define blue (SDL_Color){0, 102, 255}
#define Black (SDL_Color){0, 0, 0}
#define White (SDL_Color){255, 255, 255}

struct TextObject {
	SDL_Rect rect;
	SDL_Texture *texture;
};

extern TTF_Font* Sans;

SDL_Texture *CreateText(char *text, SDL_Color color, TTF_Font *font);

void DialogueInteract(int option);
void FreeText(struct TextObject *obj);

void NewText(struct TextObject *obj, char *text, SDL_Color color, int xPos, int yPos);
void PlayDialogue(int startindex, int max, char *fp);

void UpdateUI();
void InitPlayerUI();

void RenderText();
void UITextInteract(int option);
void UpdateText(struct TextObject *obj, char *text, SDL_Color color);
