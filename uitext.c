#include "game.h"

SDL_Color Black = {0,0,0};

// Next few declerations are for the Text debugging overlay 
SDL_Rect fpsRect; SDL_Rect yposRect; SDL_Rect xposRect; SDL_Rect accposRect; SDL_Rect velocityRect;
char fpsbuffer[20]; char xBuffer[20]; char yBuffer[20]; char accBuffer[20]; char velocityBuffer[20];


SDL_Texture* CreateText(char *text, SDL_Color color, TTF_Font *font) // Generates texture for text
{
	SDL_Surface* textsurface = TTF_RenderText_Solid(font, text, color); 
	SDL_Texture* tmpTexttext = SDL_CreateTextureFromSurface(renderer, textsurface);
	SDL_FreeSurface(textsurface);
	return tmpTexttext;
}

void RenderText(SDL_Texture* textTexture, SDL_Rect txtRect) // Renders *ONE* given texture. I'll make it do all text later
{
	SDL_RenderCopy(renderer, textTexture, NULL, &txtRect);
	SDL_DestroyTexture(textTexture);
	SDL_DestroyTexture(tmpTexttext);
}

void TextDebugOverlay() // Renders the text overlay
{
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
}

void InitDebugOverlay() // Initializes the rects in memory if overlay is enabled
{
	fpsRect.x = 0; fpsRect.y = 0; fpsRect.w = 150; fpsRect.h = 35;
	yposRect.x = 0; yposRect.y = 20; yposRect.w = 45; yposRect.h = 35;
	xposRect.x = 0; xposRect.y = 40; xposRect.w = 45; xposRect.h = 35;
	accposRect.x = 0; accposRect.y = 60; accposRect.w = 100; accposRect.h = 35;
	velocityRect.x = 0; velocityRect.y = 80; velocityRect.w = 100; velocityRect.h = 35;
}