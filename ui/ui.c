// Handles UI (Rendering text on the screen along with text creation and destruction and dialogue system)
#include "../game.h"
#include <string.h>

static int HealthText; // store current text here so we can compare as updating constantly is not worth it

static size_t textboxcount;
static size_t uiobjectcount;

struct TextObject hp;

struct TextBox *textboxlist;

struct TextList *txtlist;
struct UIObject **uiobjlist;

static size_t activebox;

struct TextList {
	struct TextObject *obj;
	struct TextList *next;
}; 

void tPush (struct TextObject *obj)
{
	struct TextList *newobj;
	newobj = malloc(sizeof(*newobj));
	newobj->obj = obj;
	newobj->next = txtlist;
	txtlist = newobj;
}

void CreateTextBox(int w, int h, int x, int y)
{
	if (textboxcount == 0)
		textboxlist = malloc(sizeof(struct TextBox));
	else
		textboxlist = realloc(textboxlist, sizeof(textboxlist) * textboxcount);

	struct TextBox tmpbox;
	SDL_Rect tmp;
	struct TextObject *tex = malloc(sizeof(struct TextObject));
	NewText(tex, ConsoleFont, "", White, x , y);

	tmp.w = w;
	tmp.h = h;
	tmp.x = x;
	tmp.y = y;

	tmpbox.contents = malloc(512); // Limit for 512 chars (no real need for more in one box) as mallocing for each new char could hurt performance
	tmpbox.rect = tmp;
	tmpbox.texture = TextureManager("game/ui/textbox.png",renderer); // TODO: Texture from rgb
	tmpbox.text = tex;

	textboxlist[textboxcount] = tmpbox;
	activebox = textboxcount; // TODO: Allow more than one active box
	textboxcount++;
}

char *GetTextBoxText() // returns the contents of the current active textbox
{
	if (strlen(textboxlist[activebox].contents) > 0)
		return textboxlist[activebox].contents;
	else
		return "";
} 

void TextBoxUpdate(char *new)
{
	textboxlist[activebox].contents = strdup(new);
	UpdateText(textboxlist[activebox].text, textboxlist[activebox].contents, White);
}

void TextBoxInteract() // interacts with the current active box
{
	if (strlen(textboxlist[activebox].contents) > 0)
		return;
	else
		info("Textbox command executed: %s", textboxlist[activebox].contents);
}

void FreeTextBox() // destroys active text box
{
	if (textboxcount == 0)
		return;
	
	SDL_DestroyTexture(textboxlist[activebox].texture);
	free(textboxlist[activebox].contents);
	FreeText(textboxlist[activebox].text);

	// move items in the array
	for (size_t i = 0; i < textboxcount; i++) {
        if (i == activebox)
            for (size_t a = i; a < textboxcount -1; a++) 
                textboxlist[a] = textboxlist[a+1];     
    }

	intext = false;

	textboxlist = realloc(textboxlist, sizeof(textboxlist) * textboxcount);
	textboxcount--;

}


void FreeText(struct TextObject *obj)
{

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

void CleanUI() // Frees all UI elements
{

	// TODO: CLEAN ALL LABELS

	for (size_t i = 0; i < textboxcount; i++)
		FreeTextBox();

	for (size_t i = 0; i < uiobjectcount; i++)
		FreeUIObject(uiobjlist[i]);
}

void UpdateUI()
{
	int curhealth = (int)HealthText;
	if (curhealth != Health) {
		char healthstr[100];
		sprintf(healthstr, "HP: %i", Health); // only update label if text changed as it is intensive
		UpdateText(&hp, healthstr, White);
		HealthText = Health;
	}
}

void InitPlayerUI() // init ui for HUD etc health
{
	char healthstr[100];
	sprintf(healthstr, "HP: %i", Health);
	NewText(&hp, Sans, healthstr, White, 0, 0);
	HealthText = Health;
}

void NewText(struct TextObject *obj, TTF_Font *font, char *text, SDL_Color color, int xPos, int yPos) // Responsible for creating textures and the rect for them in the struct
{
	if (obj == NULL)
		error("Trying to create TextObject from NULL pointer");
	
	SDL_Surface* tmpSurface = TTF_RenderText_Blended(font, text, color);
	obj->texture = SDL_CreateTextureFromSurface(renderer, tmpSurface);
	SDL_QueryTexture(obj->texture, NULL, NULL, &obj->rect.w, &obj->rect.h); // query texture size so rect doesnt look odd 
	obj->rect.x = xPos; obj->rect.y = yPos;
	obj->font = font;
	SDL_FreeSurface(tmpSurface);
	tPush(obj);
}

void CreateUIObject(struct UIObject *o, int x, int y, int w, int h, char *tx) // Used for panels etc so no collision and scrolling
{
	if (o == NULL)
		error("Trying to create UI object from NULL pointer");

	if (uiobjectcount == 0)
		uiobjlist = malloc(sizeof(struct UIObject));
	else
		uiobjlist = malloc(sizeof(struct UIObject) * uiobjectcount + 1);

	SDL_Rect tmp;

	tmp.w = w;
	tmp.h = h;
	tmp.x = x;
	tmp.y = y;
	o->rect = tmp;

	o->texture = TextureManager(tx, renderer);
	uiobjlist[uiobjectcount] = o;
	uiobjectcount++;
}

void FreeUIObject(struct UIObject *o)
{
	for (size_t i = 0; i < uiobjectcount; i++) {

		if (uiobjlist[i] == o) {
			SDL_DestroyTexture(uiobjlist[i]->texture);
			free(uiobjlist[i]);

			for (size_t a = i; a < uiobjectcount -1; a++) 
				uiobjlist[a] = uiobjlist[a+1];     
			
		}
	}
	uiobjectcount--;
	uiobjlist = malloc(sizeof(uiobjlist) * uiobjectcount);
}

void RenderUI()
{
	for (size_t i = 0; i < uiobjectcount; i++) // render ui panels and objects
		SDL_RenderCopy(renderer, uiobjlist[i]->texture, NULL, &uiobjlist[i]->rect);

	for (struct TextList *o = txtlist; o; o = o->next) // render labels
		SDL_RenderCopy(renderer, o->obj->texture, NULL, &o->obj->rect);

	for (size_t i = 0; i < textboxcount; i++) {// render text boxes
		SDL_RenderCopy(renderer, textboxlist[i].texture, NULL, &textboxlist[i].rect);
		SDL_RenderCopy(renderer, textboxlist[i].text->texture, NULL, &textboxlist[i].text->rect); // call this again to make it topmost
	}
	
}

void UpdateText(struct TextObject *obj, char *text, SDL_Color color) // used to update text every frame 
{
	
	if (obj == NULL)
		error("Trying to update TextObject for NULL pointer");

	SDL_DestroyTexture(obj->texture);
	SDL_Surface* tmpSurface = TTF_RenderText_Blended(obj->font, text, color);
	obj->texture = SDL_CreateTextureFromSurface(renderer, tmpSurface);
	SDL_QueryTexture(obj->texture, NULL, NULL, &obj->rect.w, &obj->rect.h); // query texture size so rect doesnt look odd 
	SDL_FreeSurface(tmpSurface);
}
