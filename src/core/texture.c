#include "../include/game.h"
// todo nice texture manager, return previously loaded textures


/* array of the paths of files that FE has tried to load but
failed, as there is no point in loading a texture in the first
place if it wasn't loaded correctly before */
static char **failed_textures = 0;
static size_t failed_textures_count = 0;

static void FailTexture(const char *path) // adds a texture to the failed paths so we dont try to load it again
{
    failed_textures_count++;
    if (!failed_textures)
        failed_textures = xmalloc(sizeof(char*));
    else
        failed_textures = xrealloc(failed_textures, sizeof(char*) * failed_textures_count);
    
    failed_textures[failed_textures_count -1] = strdup(path);
}

void FE_CloseTextureManager()
{
    if (failed_textures_count != 0) {
        for (size_t i = 0; i < failed_textures_count; i++)
            xfree(failed_textures[i]);
        xfree(failed_textures);
        failed_textures = 0;
        failed_textures_count = 0;
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

SDL_Texture *FE_TextureFromFile(const char *path) // Returns a texture from a file
{
    if (StrInArr(failed_textures, failed_textures_count, (char*)path))
        return FE_TextureFromRGBA(COLOR_PINK);
    
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
        FailTexture(path);
        return FE_TextureFromRGBA(COLOR_PINK);
    }
}

int FE_RenderCopy(SDL_Texture *texture, SDL_Rect *src, SDL_Rect *dst) // Renders a texture to the screen if in camera bounds
{
    // Check if rect is in screen bounds
    if (FE_Camera_Inbounds(dst, &(SDL_Rect){0,0,screen_width,screen_height}))
        return SDL_RenderCopy(renderer, texture, src, dst);
    else
        return 0;
}

int FE_RenderCopyEx(SDL_Texture *texture, SDL_Rect *src, SDL_Rect *dst, double angle, SDL_RendererFlip flip)
{
    if (FE_Camera_Inbounds(dst, &(SDL_Rect){0,0,screen_width,screen_height})) {
        const SDL_Point center = (SDL_Point){dst->w/2, dst->h/2};
        return SDL_RenderCopyEx(renderer, texture, src, dst, angle, &center, flip);
    } else
        return 0;
}