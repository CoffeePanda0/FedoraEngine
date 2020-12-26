// Handles UI (Rendering text on the screen along with text creation and destruction)
#include "../game.h"

static int txtw, txth;

struct GameObject TextBox;
struct TextObject TextName;
struct TextObject TextText;

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

void UIText(char *text, char *speaker)
{
	CreateObject(0, screen_height - screen_height / 3, screen_height / 3, screen_width, "game/ui/dialogue.png", &TextBox);
	NewText(&TextName, speaker, White, 20, TextBox.objRect.y + 20);
	NewText(&TextText, text, White, 20, TextBox.objRect.y + 50);
	TextPaused = true;
}

void UITextUpdate(int option, char* text) { // used to update UItext (the speech on screen)
	if (option == 0)
		UpdateText(&TextName, text, Black);
	else if (option == 1)
		UpdateText(&TextText, text, Black);
	else
		warn("Unkown Option for UI Text update. (Choose 0-1)");
}

void UITextInteract(int option) // add options for how to interact with text. By default 0 is any key and will close dialogue, but you can add more such
{
	switch(option) {
		case 0: // i think there is a memory leak here somewhere - too bad
			DestroyObject(&TextBox);
			FreeText(&TextText);
			FreeText(&TextName);
			TextPaused = false;
	}
}