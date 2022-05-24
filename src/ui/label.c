#include "../core/include/include.h"
#include "include/label.h"

static FE_List *FE_Labels = 0;

void FE_RenderLabels()
{
	for (struct FE_List *o = FE_Labels; o; o = o->next) {
        FE_Label *l = o->data;
        if (l->showbackground)
            FE_RenderRect(&l->r, l->backcolor);
        SDL_RenderCopy(PresentGame->Renderer, l->texture, NULL, &l->r);
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

/* generates one multiline/wrapped text texture for rendering */
static void GenerateTexture(FE_Label *l)
{
    SDL_Surface **surfaces = 0;
    size_t surface_count = 0;
    SDL_Texture **layer_textures = 0;
    
    // Check if the text is too long to fit on one line
    size_t max_chars = l->linewidth / l->font->size;
    // Split newtext into lines based on max_chars
    char **lines = StrWrap(l->text, max_chars, &surface_count);

    // Load each line as a surface and then export to texture
    surfaces = xmalloc(sizeof(SDL_Surface*) * surface_count);
    layer_textures = xmalloc(sizeof(SDL_Texture*) * surface_count);

    int largest_w = 0;

    // here we go
    for (size_t i = 0; i < surface_count; i++) {
        surfaces[i] = FE_RenderText(l->font, lines[i], l->color);
        int w, h;
        TTF_SizeText(l->font->font, lines[i], &w, &h);
        largest_w = w > largest_w ? w : largest_w;
        layer_textures[i] = SDL_CreateTextureFromSurface(PresentGame->Renderer, surfaces[i]);
        SDL_FreeSurface(surfaces[i]);
        free(lines[i]);
    }
    free(lines);
    free(surfaces);

    int h = TTF_FontHeight(l->font->font);
    l->texture = FE_CreateRenderTexture(largest_w, h * surface_count);
    
    // Create buffer texture to render each line of text to
    SDL_SetRenderTarget(PresentGame->Renderer, l->texture);
    SDL_SetTextureBlendMode(l->texture, SDL_BLENDMODE_BLEND);

    // Render each line of text to the buffer texture
    for (size_t i = 0; i < surface_count; i++) {
        SDL_Rect r = {0,l->font->size * i,0,0};
        SDL_QueryTexture(layer_textures[i], NULL, NULL, &r.w, &r.h); // Get w and h for rect
        SDL_RenderCopy(PresentGame->Renderer, layer_textures[i], NULL, &r);
        SDL_DestroyTexture(layer_textures[i]);
    }
    free(layer_textures);
    SDL_SetRenderTarget(PresentGame->Renderer, NULL);

    l->r = (SDL_Rect){l->r.x, l->r.y, largest_w, h * surface_count};
}

FE_Label *FE_CreateLabel(FE_Font *font, char *text, uint16_t linewidth, Vector2D position, SDL_Color color)
{
    FE_Label *newlabel = xmalloc(sizeof(FE_Label));
    newlabel->text = text ? strdup(text) : strdup(" ");
    newlabel->linewidth = linewidth;
    newlabel->color = color;
    newlabel->r = (SDL_Rect){position.x, position.y, 0, 0};

    if (!font) {
        info("Passing NULL font to FE_CreateLabel. Using default font");
        newlabel->font = PresentGame->font;
    } else
        newlabel->font = font;

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

    FE_List_Add(&FE_Labels, newlabel);
    
    return newlabel;
}

int FE_UpdateLabel(FE_Label *l, char *text) // Updates a pre-existing label text
{
    if (!l) {
        warn("Passed NULL label to FE_UpdateLabel");
        return -1;
    }

    // Don't bother updating label if new text is the same
    if (strcmp(l->text, text) == 0)
        return 0;

    // Remove old texture and text
    SDL_DestroyTexture(l->texture);
    free(l->text);

    // Update label text
    l->text = strdup(text);
    GenerateTexture(l);

    return 1;
}

int FE_DestroyLabel(FE_Label *l)
{
    if (!l) {
        warn("Passing NULL (DestroyLabel)");
        return -1;
    }

    SDL_DestroyTexture(l->texture);
    if (l->text)
        free(l->text);
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
        SDL_DestroyTexture(tmp->texture);
        free(tmp->text);
        free(tmp);
    }

    FE_List_Destroy(&FE_Labels);

    FE_Labels = 0;
    return 1;
}