#include "../include/game.h"

SDL_Texture *FE_TextureFromFile(const char *path) // Returns a texture from a file
{
    if (!path) {
        warn("TextureManager: Passing NULL to FE_TextureFromFile");
        return FE_TextureFromRGBA(COLOR_PINK);
    }
    
    SDL_Surface* s = IMG_Load(path); // we have this to check the image is valid
    if (s) {
        SDL_Surface* tmpSurface = IMG_Load(path); 
        SDL_Texture* text = SDL_CreateTextureFromSurface(renderer, tmpSurface);
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
    SDL_Texture* text = SDL_CreateTextureFromSurface(renderer, s);
    SDL_FreeSurface(s);

    if (!text) {
        warn("TextureFromRGB failed");
        return NULL;
    }

    return text;
}

int FE_RenderCopy(FE_Texture *texture, SDL_Rect *src, SDL_Rect *dst) // Renders a texture to the screen if in camera bounds
{
    if (!texture || !dst || !texture->Texture) {
        error("FE_RenderCopy: NULL texture or dst");
        return -1;
    }

    // Check if rect is in screen bounds
    if (FE_Camera_Inbounds(dst, &(SDL_Rect){0,0,screen_width,screen_height}))
        return SDL_RenderCopy(renderer, texture->Texture, src, dst);
    else
        return 0;
}

int FE_RenderCopyEx(FE_Texture *texture, SDL_Rect *src, SDL_Rect *dst, double angle, SDL_RendererFlip flip)
{
    if (!texture || !dst || !texture->Texture) {
        error("FE_RenderCopyEx: NULL texture or dst");
        return -1;
    }

    if (FE_Camera_Inbounds(dst, &(SDL_Rect){0,0,screen_width,screen_height})) {
        const SDL_Point center = (SDL_Point){dst->w/2, dst->h/2};
        return SDL_RenderCopyEx(renderer, texture->Texture, src, dst, angle, &center, flip);
    } else
        return 0;
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