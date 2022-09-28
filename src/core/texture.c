#include <SDL_image.h>
#include "include/include.h"

#ifndef _MATH_H
    #include <math.h>
#endif

#define ATLAS_PATH "game/map/textures/"

GPU_Image *FE_TextureFromFile(const char *path) // Returns a texture from a file
{
    if (!path) {
        warn("TextureManager: Passing NULL to FE_TextureFromFile");
        return FE_TextureFromRGBA(COLOR_PINK);
    }

    GPU_Image *i = GPU_LoadImage(path);
    // enable texture filtering
    GPU_SetImageFilter(i, GPU_FILTER_NEAREST);

    if (!i) {
        warn("Texture %s not found", path);
        return FE_TextureFromRGBA(COLOR_PINK);
    }
    return i;
}

GPU_Image *FE_TextureFromAtlas(FE_TextureAtlas *atlas, size_t index)
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
    GPU_Rect rect = {pos.x, pos.y, atlas->texturesize, atlas->texturesize};

    // create new texture to draw to
    GPU_Image *t = GPU_CreateImage(atlas->texturesize, atlas->texturesize, GPU_FORMAT_RGBA);
    
    GPU_Target *target = GPU_LoadTarget(t);
    GPU_BlitRect(atlas->atlas, &rect, target, 0);
    GPU_FreeTarget(target);

    return t;
}

GPU_Image* FE_TextureFromRGBA(SDL_Color color) // Returns a plain texture from a color
{
    SDL_Surface* s = SDL_CreateRGBSurface(0,1,1,32,0,0,0,0);
    SDL_FillRect(s, NULL, SDL_MapRGBA(s->format, color.r, color.g, color.b, color.a));
    GPU_Image* text = GPU_CopyImageFromSurface(s);
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
        GPU_FreeImage(texture->Texture);
    if (texture->path)
        free(texture->path);
    
    free(texture);

    return 1;
}

int FE_QueryTexture(FE_Texture *t, int *w, int *h)
{
    if (!t || !t->Texture || !t->path) return -1;

    *w = t->Texture->w;
    *h = t->Texture->h;

    return 0;
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

    atlas->width = atlas->atlas->w;
    atlas->height = atlas->atlas->h;

	atlas->texturesize = 64;
    
    // calculate max number of textures
    int atlas_cols = atlas->width / atlas->texturesize;
    int atlas_rows = atlas->height / atlas->texturesize;
    atlas->max_index = atlas_cols * atlas_rows;

	return atlas;
}

void FE_RenderAtlasTexture(FE_TextureAtlas *atlas, size_t index, GPU_Rect *dst)
{
    if (!atlas) {
        warn("NULL atlas being passed (FE_RenderAtlasTexture)");
        return;
    }

    if (atlas->height < atlas->texturesize || atlas->width < atlas->texturesize || atlas->texturesize == 0)
        return;
    
    vec2 pos = FE_GetTexturePosition(atlas, index);
    GPU_BlitRect(atlas->atlas,  &(GPU_Rect){pos.x, pos.y, atlas->texturesize, atlas->texturesize}, PresentGame->Screen, dst);
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
        GPU_FreeImage(atlas->atlas);
    free(atlas);
}