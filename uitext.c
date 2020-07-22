#include "game.h"

SDL_Color Black = {0,0,0};

// Next few declerations are for the Text debugging overlay 
SDL_Rect fpsRect; SDL_Rect yposRect; SDL_Rect xposRect; SDL_Rect accposRect; SDL_Rect velocityRect;
char fpsbuffer[20]; char xBuffer[20]; char yBuffer[20]; char accBuffer[20]; char velocityBuffer[20];

int txtw, txth;

struct TextList {
	struct TextObject *obj;
	struct TextList *next;
}; 

struct TextList *objlist;

void tPush (struct TextObject *obj)
{
	struct TextList *newobj;
	newobj = malloc(sizeof(*newobj));
	newobj->obj = obj;
	newobj->next = objlist;
	objlist = newobj;
}

void NewText(struct TextObject *obj, char *text, SDL_Color color, int xPos, int yPos) // Responsible for creating textures and the rect for them in the struct
{
	SDL_Surface* tmpSurface = TTF_RenderText_Solid(Sans, text, color);
	obj->texture = SDL_CreateTextureFromSurface(renderer, tmpSurface);

	SDL_QueryTexture(obj->texture, NULL, NULL, &txtw, &txth); // query texture size so rect doesnt look odd 
	
	obj->rect.x = xPos; obj->rect.y = yPos; obj->rect.h = txth; obj->rect.w = txtw;
	
	SDL_FreeSurface(tmpSurface);
	tPush(obj);
}

SDL_Texture* CreateText(char *text, SDL_Color color, TTF_Font *font) 
{
	SDL_Surface* textsurface = TTF_RenderText_Solid(font, text, color); 
	
	SDL_FreeSurface(textsurface);
	if (tmpTexttext)
		return tmpTexttext;
	else
		warn("Error making text texture! SDL Error: %s", TTF_GetError());
}

void RenderText() {
	for (struct TextList *o = objlist; o; o = o->next)
		SDL_RenderCopy(renderer, o->obj->texture, NULL, &o->obj->rect);
}

void TextDebugOverlay() // Renders the text overlay
{
	/*
	// RENDER FPS
	if (vsync)
		snprintf(fpsbuffer, sizeof(fpsbuffer), "FPS (VSYNC ON): %d", fps_current);
	else
		snprintf(fpsbuffer, sizeof(fpsbuffer), "FPS (VSYNC OFF): %d", fps_current);
	SDL_Texture* fps = CreateText(fpsbuffer, Black, Sans);
	RenderText(fps, fpsRect);

	// RENDER XPOS, YPOS
	snprintf(xBuffer, sizeof(xBuffer), "X: %d", playerRect.x);
	snprintf(yBuffer, sizeof(yBuffer), "Y: %d", playerRect.y);
	SDL_Texture* xpos = CreateText(xBuffer, Black, Sans);
	RenderText(xpos, xposRect);
	SDL_Texture* ypos = CreateText(yBuffer, Black, Sans);
	RenderText(ypos, yposRect);

	// RENDER ACCELERATION AND VELOCITY
	snprintf(accBuffer, sizeof(accBuffer), "Acceleration: %.2f", acceleration);
	snprintf(velocityBuffer, sizeof(velocityBuffer), "Velocity: %.2f", (-1 * velocity));
	SDL_Texture* accel = CreateText(accBuffer, Black, Sans);
	RenderText(accel, accposRect);
	SDL_Texture* vel = CreateText(velocityBuffer, Black, Sans);
	RenderText(vel, velocityRect);
	*/
}

void InitDebugOverlay() // Initializes the rects in memory if overlay is enabled
{
	/*
	fpsRect.x = 0; fpsRect.y = 0; fpsRect.w = 150; fpsRect.h = 35;
	yposRect.x = 0; yposRect.y = 20; yposRect.w = 45; yposRect.h = 35;
	xposRect.x = 0; xposRect.y = 40; xposRect.w = 45; xposRect.h = 35;
	accposRect.x = 0; accposRect.y = 60; accposRect.w = 100; accposRect.h = 35;
	velocityRect.x = 0; velocityRect.y = 80; velocityRect.w = 100; velocityRect.h = 35;
	*/
}