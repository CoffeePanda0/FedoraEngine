#include "game.h"

SDL_Color Black = {0,0,0};

// Next few declerations are for the Text debugging overlay 
char fpsbuffer[30]; char xBuffer[20]; char yBuffer[20]; char accBuffer[20]; char velocityBuffer[20];
struct TextObject ui_fps; struct TextObject ui_ypos; struct TextObject ui_xpos; struct TextObject ui_accel; struct TextObject ui_vel;

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

void FreeText(struct TextObject *obj)
{
	struct TextList *tmp;

	for (struct TextList *o = objlist; o; o = o->next) {
		if (o->obj == obj)
			SDL_DestroyTexture(o->obj->texture);
	}
}

void NewText(struct TextObject *obj, char *text, SDL_Color color, int xPos, int yPos) // Responsible for creating textures and the rect for them in the struct
{
	SDL_Surface* tmpSurface = TTF_RenderText_Blended(Sans, text, color);
	obj->texture = SDL_CreateTextureFromSurface(renderer, tmpSurface);

	SDL_QueryTexture(obj->texture, NULL, NULL, &txtw, &txth); // query texture size so rect doesnt look odd 
	
	obj->rect.x = xPos; obj->rect.y = yPos; obj->rect.h = txth; obj->rect.w = txtw;
	
	SDL_FreeSurface(tmpSurface);
	tPush(obj);
}

void RenderText() {
	for (struct TextList *o = objlist; o; o = o->next)
		SDL_RenderCopy(renderer, o->obj->texture, NULL, &o->obj->rect);
}

void UpdateText(struct TextObject *obj, char *text, SDL_Color color) // used to update text every frame 
{
	SDL_DestroyTexture(obj->texture);
	SDL_Surface* tmpSurface = TTF_RenderText_Blended(Sans, text, color);
	obj->texture = SDL_CreateTextureFromSurface(renderer, tmpSurface);
	SDL_QueryTexture(obj->texture, NULL, NULL, &txtw, &txth); // query texture size so rect doesnt look odd 
	obj->rect.h = txth; obj->rect.w = txtw;
	SDL_FreeSurface(tmpSurface);
}

// Code for debug overlay below 
void TextDebugOverlay() // Updates the text on the overlay, as we can't use NewText in case something changes
{
	char *oldfps, *oldy, *oldx, *oldaccel, *oldvel; // we compare against old as rendering new textures each frame uses high cpu
	
	// RENDER FPS
	oldfps = malloc(strlen(fpsbuffer) + 1);
	strcpy(oldfps,fpsbuffer);
	if (vsync)
		snprintf(fpsbuffer, sizeof(fpsbuffer), "FPS (VSYNC ON): %d", fps_current);
	else
		snprintf(fpsbuffer, sizeof(fpsbuffer), "FPS (VSYNC OFF): %d", fps_current);
	if (strcmp(fpsbuffer, oldfps)) 
		UpdateText(&ui_fps, fpsbuffer, Black);

	// RENDER XPOS, YPOS
	oldy = malloc(strlen(yBuffer) + 1);
	strcpy(oldy,yBuffer);
	oldx = malloc(strlen(xBuffer) + 1);
	strcpy(oldx,xBuffer);
	snprintf(xBuffer, sizeof(xBuffer), "X: %d", playerRect.x);
	snprintf(yBuffer, sizeof(yBuffer), "Y: %d", playerRect.y);
	if (strcmp(oldy, yBuffer))
		UpdateText(&ui_xpos, yBuffer, Black);
	if (strcmp(oldx, xBuffer)) 
		UpdateText(&ui_ypos, xBuffer, Black);


	// RENDER ACCELERATION AND VELOCITY
	oldaccel = malloc(strlen(accBuffer) + 1);
	strcpy(oldaccel,accBuffer);
	oldvel = malloc(strlen(velocityBuffer) + 1);
	strcpy(oldvel,velocityBuffer);
	snprintf(accBuffer, sizeof(accBuffer), "Acceleration: %.2f", acceleration);
	snprintf(velocityBuffer, sizeof(velocityBuffer), "Velocity: %.2f", (-1 * velocity));
	if (strcmp(accBuffer, oldaccel)) 
		UpdateText(&ui_accel, accBuffer, Black);
	if (strcmp(velocityBuffer, oldvel)) 
		UpdateText(&ui_vel, velocityBuffer, Black);
	
	free(oldfps); free(oldy); free(oldx); free(oldaccel); free(oldvel);

}

void InitDebugOverlay() // creates text objects and adds them to the render
{
	// RENDER FPS
	if (vsync)
		snprintf(fpsbuffer, sizeof(fpsbuffer), "FPS (VSYNC ON): %d", fps_current);
	else
		snprintf(fpsbuffer, sizeof(fpsbuffer), "FPS (VSYNC OFF): %d", fps_current);
	NewText(&ui_fps, fpsbuffer, Black, 0, 0);

	// RENDER XPOS, YPOS
	snprintf(xBuffer, sizeof(xBuffer), "X: %d", playerRect.x);
	snprintf(yBuffer, sizeof(yBuffer), "Y: %d", playerRect.y);
	NewText(&ui_xpos, xBuffer, Black, 0, 20); NewText(&ui_ypos, yBuffer, Black, 0, 40);

	// RENDER ACCELERATION AND VELOCITY
	snprintf(accBuffer, sizeof(accBuffer), "Acceleration: %.2f", acceleration);
	snprintf(velocityBuffer, sizeof(velocityBuffer), "Velocity: %.2f", (-1 * velocity));
	NewText(&ui_accel, accBuffer, Black, 0, 60); NewText(&ui_vel, velocityBuffer, Black, 0, 80);
}

void FreeOverlay() // frees up memory and removes overlay from screen
{
	FreeText(&ui_fps); FreeText(&ui_xpos); FreeText(&ui_ypos); FreeText(&ui_accel); FreeText(&ui_vel);
}