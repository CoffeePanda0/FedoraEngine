#include <SDL_image.h>
#include "include/include.h"

#ifndef _MATH_H
    #include <math.h>
#endif

#define ATLAS_PATH "game/map/textures/"

SDL_Texture *FE_TextureFromFile(const char *path) // Returns a texture from a file
{
    if (!path) {
        warn("TextureManager: Passing NULL to FE_TextureFromFile");
        return FE_TextureFromRGBA(COLOR_PINK);
    }
    
    SDL_Surface* s = IMG_Load(path); // we have this to check the image is valid
    if (s) {
        SDL_Surface* tmpSurface = IMG_Load(path); 
        SDL_Texture* text = SDL_CreateTextureFromSurface(PresentGame->Renderer, tmpSurface);
        SDL_FreeSurface(tmpSurface);
        SDL_FreeSurface(s);
        return text;
    } else {
        free(s);
        warn("Texture %s not found", path);
        return FE_TextureFromRGBA(COLOR_PINK);
    }
}

SDL_Texture* FE_TextureFromRGBA(SDL_Color color) // Returns a plain texture from a color
{
    SDL_Surface* s = SDL_CreateRGBSurface(0,1,1,32,0,0,0,0);
    SDL_FillRect(s, NULL, SDL_MapRGBA(s->format, color.r, color.g, color.b, color.a));
    SDL_Texture* text = SDL_CreateTextureFromSurface(PresentGame->Renderer, s);
    SDL_FreeSurface(s);

    if (!text) {
        warn("TextureFromRGB failed");
        return NULL;
    }

    return text;
}

int FE_DestroyTexture(FE_Texture *texture)
{
    if (!texture) {
        warn("Passing NULL texture (DestroyTexture)");
        return -1;
    }

    if (texture->Texture)
        SDL_DestroyTexture(texture->Texture);
    if (texture->path)
        free(texture->path);
    
    free(texture);

    return 1;
}

int FE_QueryTexture(FE_Texture *t, int *w, int *h)
{
    if (!t) return -1;

    return SDL_QueryTexture(t->Texture, NULL, NULL, w, h);
}

void FE_FillTexture(SDL_Texture *texture, int r, int g, int b, int a)
{
    SDL_SetRenderTarget(PresentGame->Renderer, texture);
    SDL_SetRenderDrawBlendMode(PresentGame->Renderer, SDL_BLENDMODE_NONE);
    SDL_SetRenderDrawColor(PresentGame->Renderer, r, g, b, a);
    SDL_RenderFillRect(PresentGame->Renderer, NULL);
}

SDL_Texture *FE_CreateRenderTexture(int w, int h)
{
    SDL_Texture *t = SDL_CreateTexture(PresentGame->Renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, w, h);
    FE_FillTexture(t, 0, 0, 0, 0);
    SDL_SetRenderTarget(PresentGame->Renderer, NULL);
    return t;
}

Vector2D FE_GetTexturePosition(FE_TextureAtlas *atlas, size_t index)
{
    if (!atlas) {
        warn("NULL atlas being passed (FE_GetTexturePoisition)");
        return VEC_EMPTY;
    }

    if (atlas->height < atlas->texturesize || atlas->width < atlas->texturesize || atlas->texturesize == 0)
        return VEC_EMPTY;

    if (index == 0) {
        return VEC_EMPTY;
    }

    size_t atlas_cols = atlas->width / atlas->texturesize;

    // Calculate the X and Y coordinates on the atlas
    int row = ceil(index / atlas_cols);
    int col = (index % atlas_cols);
    if (col == 0)
        col = atlas_cols;

    return vec2(col * atlas->texturesize, row * atlas->texturesize);
}

FE_TextureAtlas *FE_LoadTextureAtlas(const char *name)
{
	if (!name || mstrlen(name) == 0) {
		warn("Editor_LoadAtlas: Passing NULL to path");
		return 0;
	}

	FE_TextureAtlas *atlas = xmalloc(sizeof(FE_TextureAtlas));

	char *path = mstradd(ATLAS_PATH, name);
	atlas->atlas = FE_LoadResource(FE_RESOURCE_TYPE_TEXTURE, path);
	free(path);

	FE_QueryTexture(atlas->atlas, &atlas->width, &atlas->height);
	atlas->texturesize = 64;

	return atlas;
}