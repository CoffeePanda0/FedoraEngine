#include "../core/include/include.h"
#include "include/label.h"

static FE_List *FE_Labels = 0;

void FE_RenderLabels()
{
	for (struct FE_List *o = FE_Labels; o; o = o->next) {
        FE_Label *l = o->data;
        if (l->showbackground)
            FE_RenderRect(&l->r, l->backcolor);
        SDL_RenderCopy(PresentGame->Renderer, l->text, NULL, &l->r);
    }
}

SDL_Texture *FE_TextureFromText(char *text, SDL_Color color)
{
    SDL_Surface *s = FE_RenderText(PresentGame->font, text, color);
    SDL_Texture *t = SDL_CreateTextureFromSurface(PresentGame->Renderer, s);
    SDL_FreeSurface(s);
    if (!t)
        warn("Could not create texture from text");
    return t;
}

FE_Label *FE_CreateLabel(FE_Font *font, char *text, Vector2D position, SDL_Color color)
{
    char *ntext = 0;
    if (!text) {
        ntext = strdup(" ");
    } else {
        ntext = strdup(text);
    }

    FE_Label *newlabel = xmalloc(sizeof(FE_Label));

    if (!font)
        newlabel->font = PresentGame->font;
    else
        newlabel->font = font;

    if (!newlabel->font) {
        warn("Unable to create label - No font loaded");
        return NULL;
    }

    // create texture from surface
    SDL_Surface *text_surface = FE_RenderText(newlabel->font, ntext, color); 
    free(ntext);

    newlabel->text = SDL_CreateTextureFromSurface(PresentGame->Renderer, text_surface);
    SDL_FreeSurface(text_surface);

    SDL_QueryTexture(newlabel->text, NULL, NULL, &newlabel->r.w, &newlabel->r.h); // Get w and h for rect
    newlabel->r.x = position.x;
    newlabel->r.y = position.y;
    newlabel->color = color;
    
    /* Set opposite colors for the background to provide contrast */
    if (color.r > 128 && color.g > 128 && color.b > 128)
        newlabel->backcolor = COLOR_BLACK;
    else
        newlabel->backcolor = COLOR_WHITE;
    newlabel->showbackground = false;

    FE_List_Add(&FE_Labels, newlabel);
    
    return newlabel;
}

int FE_UpdateLabel(FE_Label *l, char *text) // Updates a pre-existing label text
{
    for (FE_List *f = FE_Labels; f; f = f->next) {
        FE_Label *tmp = f->data;
        if (tmp == l) { // when we reach the given one, free old textures first
            SDL_DestroyTexture(tmp->text);
            // regenerate textures, then query to change rect size
            SDL_Surface *text_surface = FE_RenderText(PresentGame->font, text, tmp->color); 
            tmp->text = SDL_CreateTextureFromSurface(PresentGame->Renderer, text_surface);
            SDL_FreeSurface(text_surface);


            SDL_QueryTexture(tmp->text, NULL, NULL, &tmp->r.w, &tmp->r.h); // Get w and h for rect
            return 0;
        }
    }
    warn("Unable to update label");
    return -1;
}

int FE_DestroyLabel(FE_Label *l)
{
    if (!l) {
        warn("Passing NULL (DestroyLabel)");
        return -1;
    }

    SDL_DestroyTexture(l->text);
    FE_List_Remove(&FE_Labels, l);
    free(l);
    
    return -1;
}

int FE_CleanLabels() // Destroys all labels and free'd
{
    if (!FE_Labels) {
        return 0;
    }

    for (struct FE_List *l = FE_Labels; l; l = l->next) {
        struct FE_Label *tmp = l->data;
        SDL_DestroyTexture(tmp->text);
        free(tmp);
    }

    FE_List_Destroy(&FE_Labels);

    FE_Labels = 0;
    return 1;
}