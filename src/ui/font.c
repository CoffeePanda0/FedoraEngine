#include "../core/include/include.h"
#include "include/font.h"
#define FONT_DIR "game/fonts/"
#define FONT_EXT ".ttf"

static FE_List *fonts = 0;

FE_Font *FE_LoadFont(char *name, uint8_t size)
{
    // Check if a font with the same name and size already exists
    for (FE_List *l = fonts; l; l = l->next) {
        FE_Font *f = l->data;
        if (strcmp(f->name, name) == 0 && f->size == size) {
            f->refs++;
            return f;
        }
    }

    char *font_path = xmalloc(mstrlen(name) + mstrlen(FONT_DIR) + mstrlen(FONT_EXT) + 1);
    sprintf(font_path, "%s%s%s", FONT_DIR, name, FONT_EXT);
    
    // Open the font
    TTF_Font *font = TTF_OpenFont(font_path, size);
    if (!font) {
        warn("Could not load font '%s'", name);
        free(font_path);
        return NULL;
    }
    free(font_path);

    // Create the font struct
    FE_Font *f = xmalloc(sizeof(FE_Font));
    f->font = font;
    f->size = size;
    f->name = mstrdup(name);
    f->refs = 1;

    // Add the font to the list
    FE_List_Add(&fonts, f);
    return f;
}

void FE_DestroyFont(FE_Font *font)
{
    if (!font)
        return;

    if (font->refs > 1) {
        font->refs--;
        return;
    }

    // Remove the font from the list
    FE_List_Remove(&fonts, font);

    TTF_CloseFont(font->font);
    free(font->name);
    free(font);
}

void FE_CleanFonts()
{
    for (struct FE_List *l = fonts; l; l = l->next) {
        FE_Font *f = l->data;
        if (!f->font) continue;
        TTF_CloseFont(f->font);
        free(f->name);
        free(f);
    }
    FE_List_Destroy(&fonts);
    fonts = 0;
}

int FE_FontStringSize(FE_Font *font, char *str)
{
    int w, h;
    TTF_SizeText(font->font, str, &w, &h);
    return w;
}

SDL_Surface *FE_RenderText(FE_Font *font, const char *str, SDL_Color color)
{
    if (!font) {
        warn("Passing font as NULL (FE_RenderText)");
        return NULL;
    }

    if (!str || mstrlen(str) == 0)
        return NULL;

    SDL_Surface *text = TTF_RenderText_Blended(font->font, str, color);
    if (!text) {
        warn("Could not render text");
        return NULL;
    }
    return text;
}