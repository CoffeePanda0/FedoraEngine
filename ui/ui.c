// Handles UI (Rendering text on the screen along with text creation and destruction and dialogue system)
#include "../game.h"
#include <string.h>

static int txtw, txth;

static int HealthText; // store current text here so we can compare as updating constantly is not worth it

struct TextObject hp;

struct TextList {
	struct TextObject *obj;
	struct TextList *next;
}; 

struct TextList *txtlist;

void tPush (struct TextObject *obj)
{
	struct TextList *newobj;
	newobj = malloc(sizeof(*newobj));
	newobj->obj = obj;
	newobj->next = txtlist;
	txtlist = newobj;
}

void FreeText (struct TextObject *obj) {

	struct TextList *tmp;
	// cheers to gibson for help writing this, pointer and linked lists go aee
	if (txtlist->obj == obj) {
		SDL_DestroyTexture(obj->texture);
		tmp = txtlist;
		txtlist = txtlist->next;
		free(tmp);
	} else {
		for (struct TextList *t = txtlist; t && t->next; t = t->next) {
			if (t->next->obj == obj) {
				SDL_DestroyTexture(t->next->obj->texture);
				tmp = t->next;
				t->next = t->next->next;
				free(tmp);
			}
		}
	}
}

void UpdateUI()
{
	int curhealth = (int)HealthText;
	if (curhealth != Health) {
		char healthstr[100];
		sprintf(healthstr, "HP: %i", Health); // only update label if text changed as it is intensive
		UpdateText(&hp, healthstr, Black);
		HealthText = Health;
	}
}

void InitPlayerUI() // init ui for HUD etc health
{
	char healthstr[100];
	sprintf(healthstr, "HP: %i", Health);
	NewText(&hp, healthstr, Black, 0, 0);
	HealthText = Health;
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
	for (struct TextList *o = txtlist; o; o = o->next)
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
