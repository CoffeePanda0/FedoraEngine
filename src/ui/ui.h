// HOLDS UI VALUES (COLOURS, FONTS and functions for UI.c)
#pragma once
#include <SDL_ttf.h>
#include <SDL.h>

#define blue (SDL_Color){0, 102, 255, 0}
#define Black (SDL_Color){0, 0, 0, 0}
#define White (SDL_Color){255, 255, 255, 0}

struct TextObject {
	SDL_Rect rect;
	SDL_Texture *texture;
	TTF_Font *font;
};

struct TextBox { // Used for panels etc
	struct TextObject *text;
	char *contents;
	SDL_Rect rect;
	SDL_Texture *texture;
};

struct UIObject {
	SDL_Rect rect;
	SDL_Texture *texture;
};

extern TTF_Font* Sans;
extern TTF_Font* ConsoleFont;

SDL_Texture *CreateText(char *text, SDL_Color color);

void DialogueInteract(int option);
void FreeText(struct TextObject *obj);

void NewText(struct TextObject *obj, TTF_Font *font, char *text, SDL_Color color, int xPos, int yPos);
void PlayDialogue(int startindex, int max, char *fp);

void CreateTextBox(int w, int h, int x, int y);
void TextBoxUpdate(char *new);
char *GetTextBoxText();
void TextBoxInteract();
void FreeTextBox();

void CreateUIObject(struct UIObject *o, int x, int y, int w, int h, char *tx);
void FreeUIObject(struct UIObject *o);

void UpdateUI();
void InitPlayerUI();

void CleanUI();

void RenderUI();
void UITextInteract(int option);
void UpdateText(struct TextObject *obj, char *text, SDL_Color color);
