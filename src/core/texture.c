#include "../include/game.h"
// Contains functions for creating and managing textures

// todo nice texture manager, return previously loaded textures

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

SDL_Texture *FE_TextureFromFile(const char *path) // Returns a texture from a file
{
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
        return(FE_TextureFromRGBA(COLOR_PINK));
    }
}