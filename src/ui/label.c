#include "../core/include/include.h"
#include "include/label.h"

void FE_UI_RenderLabel(FE_UI_Label *l)
{
    if (l->showbackground)
        GPU_RectangleFilled2(PresentGame->Screen, l->r, l->backcolor);
    GPU_BlitRect(l->texture, NULL, PresentGame->Screen, &l->r);
}

GPU_Image *FE_TextureFromText(char *text, SDL_Color color)
{
    SDL_Surface *s = FE_RenderText(PresentGame->font, text, color);
    GPU_Image *t = GPU_CopyImageFromSurface(s);
    SDL_FreeSurface(s);
    if (!t)
        warn("Could not create texture from text");
    return t;
}

/* generates one multiline/wrapped text texture for rendering */
static void GenerateTexture(FE_UI_Label *l)
{
    SDL_Surface **surfaces = 0;
    size_t surface_count = 0;
    GPU_Image **layer_textures = 0;
    
    // Check if the text is too long to fit on one line
    int char_width = 0;
    TTF_SizeText(l->font->font, "o", &char_width, NULL);
    size_t max_chars = l->linewidth / char_width;

    if (max_chars < 1) {
        warn("Label is too long to fit on one line");
        return;
    }

    // Split newtext into lines based on max_chars
    char **lines = mstrwrap(l->text, max_chars, &surface_count);

    // Load each line as a surface and then export to texture
    surfaces = xmalloc(sizeof(SDL_Surface*) * surface_count);
    layer_textures = xmalloc(sizeof(GPU_Image*) * surface_count);

    int largest_w = 0;

    // here we go
    for (size_t i = 0; i < surface_count; i++) {
        surfaces[i] = FE_RenderText(l->font, lines[i], l->color);
        if (!surfaces[i]) {
            warn("Could not render text surface");
            return;
        }
        
        int w, h;
        TTF_SizeText(l->font->font, lines[i], &w, &h);
        largest_w = w > largest_w ? w : largest_w;
        layer_textures[i] = GPU_CopyImageFromSurface(surfaces[i]);
        SDL_FreeSurface(surfaces[i]);
        free(lines[i]);
    }
    free(lines);
    free(surfaces);

    int h = TTF_FontHeight(l->font->font);
    if (largest_w == 0) largest_w = 1; // prevent divide by zero during texture creation

    l->texture = GPU_CreateImage(largest_w, h * surface_count, GPU_FORMAT_RGBA);
    
    // Create buffer texture to render each line of text to
    GPU_Target *target = GPU_LoadTarget(l->texture);

    // Render each line of text to the buffer texture
    for (size_t i = 0; i < surface_count; i++) {
        GPU_Rect r = {0,l->font->size * i,0,0};
        r.w = layer_textures[i]->w;
        r.h = layer_textures[i]->h;
        GPU_BlitRect(layer_textures[i], NULL, target, &r);
        GPU_FreeImage(layer_textures[i]);
    }
    free(layer_textures);
    GPU_FreeTarget(target);

    l->r = (GPU_Rect){l->r.x, l->r.y, largest_w, h * surface_count};
}

FE_UI_Label *FE_UI_CreateLabel(FE_Font *font, char *text, uint16_t linewidth, int x, int y, SDL_Color color)
{
    FE_UI_Label *newlabel = xmalloc(sizeof(FE_UI_Label));
    newlabel->text = text ? mstrdup(text) : mstrdup(" ");
    newlabel->linewidth = linewidth;
    newlabel->color = color;
    newlabel->r = (GPU_Rect){x, y, 0, 0};

    newlabel->font = font ? font : PresentGame->font;

    if (!newlabel->font) {
        warn("Unable to create label - No font loaded");
        return NULL;
    }

    // Generate label texture
    GenerateTexture(newlabel);

    /* Set opposite colors for the background to provide contrast */
    if (color.r > 128 && color.g > 128 && color.b > 128)
        newlabel->backcolor = COLOR_BLACK;
    else
        newlabel->backcolor = COLOR_WHITE;
    newlabel->showbackground = false;
    
    return newlabel;
}

void FE_UI_UpdateLabel(FE_UI_Label *l, char *text) // Updates a pre-existing label text
{
    if (!l) {
        warn("Passed NULL label to FE_UpdateLabel");
        return;
    }

    // Don't bother updating label if new text is the same
    if (mstrcmp(l->text, text) == 0)
        return;

    // Remove old texture and text
    GPU_FreeImage(l->texture);
    free(l->text);

    // Update label text
    l->text = mstrdup(text);
    GenerateTexture(l);
}

void FE_UI_DestroyLabel(FE_UI_Label *l, bool global)
{
    if (!l) {
        warn("Passing NULL (DestroyLabel)");
        return;
    }

    GPU_FreeImage(l->texture);
    if (l->text)
        free(l->text);

    // Check if this label exists in the global list, if so remove it
    if (global) {
        int r = FE_List_Remove(&PresentGame->UIConfig.ActiveElements->Labels, l);
        if (r == 1) PresentGame->UIConfig.ActiveElements->Count--;
    }
    
    free(l);
    
}