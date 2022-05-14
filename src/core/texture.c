#include <SDL_image.h>
#include "include/texture.h"
#include "include/fedoraengine.h"
#include "include/utils.h"

SDL_Texture *FE_TextureFromFile(const char *path) // Returns a texture from a file
{
    if (!path) {
        warn("TextureManager: Passing NULL to FE_TextureFromFile");
        return FE_TextureFromRGBA(COLOR_PINK);
    }
    
    SDL_Surface* s = IMG_Load(path); // we have this to check the image is valid
    if (s) {
        SDL_Surface* tmpSurface = IMG_Load(path); 
        SDL_Texture* text = SDL_CreateTextureFromSurface(PresentGame->renderer, tmpSurface);
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
    SDL_Texture* text = SDL_CreateTextureFromSurface(PresentGame->renderer, s);
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
        xfree(texture->path);
    
    xfree(texture);

    return 1;
}

int FE_QueryTexture(FE_Texture *t, int *w, int *h)
{
    if (!t) return -1;

    return SDL_QueryTexture(t->Texture, NULL, NULL, w, h);
}

void FE_FillTexture(SDL_Texture *texture, int r, int g, int b, int a)
{
    SDL_SetRenderTarget(PresentGame->renderer, texture);
    SDL_SetRenderDrawBlendMode(PresentGame->renderer, SDL_BLENDMODE_NONE);
    SDL_SetRenderDrawColor(PresentGame->renderer, r, g, b, a);
    SDL_RenderFillRect(PresentGame->renderer, NULL);
}

SDL_Texture *FE_CreateRenderTexture(int w, int h)
{
    SDL_Texture *t = SDL_CreateTexture(PresentGame->renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, w, h);
    FE_FillTexture(t, 0, 0, 0, 0);
    SDL_SetRenderTarget(PresentGame->renderer, NULL);
    return t;
}