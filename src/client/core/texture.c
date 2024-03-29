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
        SDL_Texture* text = SDL_CreateTextureFromSurface(PresentGame->Client->Renderer, s);
        SDL_FreeSurface(s);
        return text;
    } else {
        free(s);
        warn("Texture %s not found", path);
        return FE_TextureFromRGBA(COLOR_PINK);
    }
}

SDL_Texture *FE_TextureFromAtlas(FE_TextureAtlas *atlas, size_t index)
{
    // Generates an SDL texture from a texture atlas
    if (!atlas) {
        warn("TextureManager: Passing NULL to FE_TextureFromAtlas");
        return FE_TextureFromRGBA(COLOR_PINK);
    }
    if (index >= atlas->max_index) {
        warn("TextureManager: Index %zu is out of bounds", index);
        return FE_TextureFromRGBA(COLOR_PINK);
    }

    // Calculate the position of the texture in the atlas
    vec2 pos = FE_GetTexturePosition(atlas, index);
    SDL_Rect rect = {pos.x, pos.y, atlas->texturesize, atlas->texturesize};

    // create new texture to draw to
    SDL_Texture *t = FE_CreateRenderTexture(atlas->texturesize, atlas->texturesize);
    SDL_SetRenderTarget(PresentGame->Client->Renderer, t);
    SDL_RenderCopy(PresentGame->Client->Renderer, atlas->atlas, &rect, NULL);
    SDL_SetRenderTarget(PresentGame->Client->Renderer, NULL);

    return t;
}

SDL_Texture* FE_TextureFromRGBA(SDL_Color color) // Returns a plain texture from a color
{
    SDL_Surface* s = SDL_CreateRGBSurface(0,1,1,32,0,0,0,0);
    SDL_FillRect(s, NULL, SDL_MapRGBA(s->format, color.r, color.g, color.b, color.a));
    SDL_Texture* text = SDL_CreateTextureFromSurface(PresentGame->Client->Renderer, s);
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
    if (!t || !t->Texture || !t->path) return -1;

    return SDL_QueryTexture(t->Texture, NULL, NULL, w, h);
}

void FE_FillTexture(SDL_Texture *texture, int r, int g, int b, int a)
{
    SDL_SetRenderTarget(PresentGame->Client->Renderer, texture);
    SDL_SetRenderDrawBlendMode(PresentGame->Client->Renderer, SDL_BLENDMODE_NONE);
    SDL_SetRenderDrawColor(PresentGame->Client->Renderer, r, g, b, a);
    SDL_RenderFillRect(PresentGame->Client->Renderer, NULL);
}

SDL_Texture *FE_CreateRenderTexture(int w, int h)
{
    SDL_Texture *t = SDL_CreateTexture(PresentGame->Client->Renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, w, h);
    FE_FillTexture(t, 0, 0, 0, 0);
    SDL_SetRenderTarget(PresentGame->Client->Renderer, NULL);
    return t;
}

vec2 FE_GetTexturePosition(FE_TextureAtlas *atlas, size_t index)
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

    return vec(col * atlas->texturesize, row * atlas->texturesize);
}

FE_TextureAtlas *FE_LoadTextureAtlas(const char *name)
{
	if (!name || mstrlen(name) == 0) {
		warn("Editor_LoadAtlas: Passing NULL to path");
		return 0;
	}

	FE_TextureAtlas *atlas = xmalloc(sizeof(FE_TextureAtlas));

	char *path = mstradd(ATLAS_PATH, name);
	atlas->atlas = FE_TextureFromFile(path);
    atlas->path = mstrdup(name);
	free(path);

	SDL_QueryTexture(atlas->atlas, NULL, NULL, &atlas->width, &atlas->height);
	atlas->texturesize = 64;
    
    // calculate max number of textures
    int atlas_cols = atlas->width / atlas->texturesize;
    int atlas_rows = atlas->height / atlas->texturesize;
    atlas->max_index = atlas_cols * atlas_rows;

	return atlas;
}

void FE_RenderAtlasTexture(FE_TextureAtlas *atlas, size_t index, SDL_Rect *dst)
{
    if (!atlas) {
        warn("NULL atlas being passed (FE_RenderAtlasTexture)");
        return;
    }

    if (atlas->height < atlas->texturesize || atlas->width < atlas->texturesize || atlas->texturesize == 0)
        return;
    
    vec2 pos = FE_GetTexturePosition(atlas, index);
    SDL_RenderCopy(PresentGame->Client->Renderer, atlas->atlas, &(SDL_Rect){pos.x, pos.y, atlas->texturesize, atlas->texturesize}, dst);
}

void FE_DestroyTextureAtlas(FE_TextureAtlas *atlas)
{
    if (!atlas) {
        warn("NULL atlas being passed (FE_DestroyTextureAtlas)");
        return;
    }

    if (atlas->path)
        free(atlas->path);
    if (atlas->atlas)
        SDL_DestroyTexture(atlas->atlas);
    free(atlas);
}