#include "../include/game.h"
// todo add missing texture to textureresource

typedef struct TextureResource {
    char *path;
    SDL_Texture *texture;
    size_t uses; // how many times is the texture pointer being used?
} TextureResource;

/* array of the paths of files that FE has tried to load but
failed, as there is no point in loading a texture in the first
place if it wasn't loaded correctly before */
static char **failed_textures = 0;
static size_t failed_textures_count = 0;


/* We have a linked list of loaded textures. This means that instead
of loading a new texture each time, we can return a pointer to the
existing one. This is not very efficient - todo - replace with hashtable */
static FE_List *TextureList = 0;
static size_t texturecount = 0;

static void FailTexture(const char *path) // adds a texture to the failed paths so we dont try to load it again
{
    failed_textures_count++;
    if (!failed_textures)
        failed_textures = xmalloc(sizeof(char*));
    else
        failed_textures = xrealloc(failed_textures, sizeof(char*) * failed_textures_count);
    
    failed_textures[failed_textures_count -1] = strdup(path);
}

SDL_Texture *FE_LoadTexture(const char *path) // Returns pointer to a texture
{
    // Check if image is in failed first
    if (StrInArr(failed_textures, failed_textures_count, (char*)path))
        return FE_TextureFromRGBA(COLOR_PINK);
    
    // if texture has previously been loaded, return a pointer to this instead
    SDL_Texture *t = 0;
    for (FE_List *l = TextureList; l; l = l->next) {
        TextureResource *tr = l->data;
        if (strcmp(tr->path, path) == 0) {
            tr->uses++;
            return tr->texture;
        }
    }
    if (t) return t;

    SDL_Surface *s = IMG_Load(path);
    if (s) {
        SDL_Texture *text = SDL_CreateTextureFromSurface(renderer, s);
        SDL_FreeSurface(s);

        // add new texture to list
        TextureResource *tr = xmalloc(sizeof(TextureResource));
        tr->path = strdup(path);
        tr->texture = text;
        tr->uses = 1;

        FE_List_Add(&TextureList, tr);
        texturecount++;

        return text;
    } else {
        SDL_FreeSurface(s);
        warn("TextureManager: Texture %s not found", path);
        FailTexture(path);
        return FE_TextureFromRGBA(COLOR_PINK);
    }
}

bool FE_FreeTexture(SDL_Texture *t)
{
    if (texturecount == 0)
        return false;
    if (!t) {
        warn("TextureManager: Passing NULL to free texture");
        return false;
    }
    

    // find the node in the list with our texture in
    FE_List *data = 0;
    TextureResource *tr = 0;
    for (FE_List *l = TextureList; l; l = l->next) {
        tr = l->data;
        if (tr->texture == t) {
            data = l->data;
            break;
        }
    }
    if (!data) {
        warn("TextureManager: Texture not found in list");
        return false;
    }

    // check if our texture is used anywhere else, if it is then we don't want to destroy it
    if (tr->uses == 1) {
        if (tr->path) xfree(tr->path);
        if (tr->texture) SDL_DestroyTexture(tr->texture);
        tr->uses = 0;
        FE_List_Remove(&TextureList, data);
        xfree(tr);
        texturecount--;
        return true;
    }
    
    tr->uses--;
    return true;
}

void FE_CloseTextureManager()
{
    // Clear failed textures
    if (failed_textures_count != 0) {
        for (size_t i = 0; i < failed_textures_count; i++)
            xfree(failed_textures[i]);
        xfree(failed_textures);
        failed_textures = 0;
        failed_textures_count = 0;
    }

    if (texturecount != 0) {

        // clear loaded textures if any remain
        for (FE_List *l = TextureList; l; l = l->next) {
            TextureResource *tr = l->data;
            if (tr->path) xfree(tr->path);
            if (tr->texture) SDL_DestroyTexture(tr->texture);
            tr->uses = 0;
            xfree(tr);
        }
        FE_List_Destroy(&TextureList);
        TextureList = 0;
    }
}

SDL_Texture *FE_TextureFromFile(const char *path) // Returns a texture from a file
{
    if (!path) {
        warn("TextureManager: Passing NULL to FE_TextureFromFile");
        return FE_TextureFromRGBA(COLOR_PINK);
    }

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

int FE_RenderCopy(SDL_Texture *texture, SDL_Rect *src, SDL_Rect *dst) // Renders a texture to the screen if in camera bounds
{
    if (!texture || !dst) {
        error("FE_RenderCopy: NULL texture or dst");
        return -1;
    }

    // Check if rect is in screen bounds
    if (FE_Camera_Inbounds(dst, &(SDL_Rect){0,0,screen_width,screen_height}))
        return SDL_RenderCopy(renderer, texture, src, dst);
    else
        return 0;
}

int FE_RenderCopyEx(SDL_Texture *texture, SDL_Rect *src, SDL_Rect *dst, double angle, SDL_RendererFlip flip)
{
    if (!texture || !dst) {
        error("FE_RenderCopyEx: NULL texture or dst");
        return -1;
    }

    if (FE_Camera_Inbounds(dst, &(SDL_Rect){0,0,screen_width,screen_height})) {
        const SDL_Point center = (SDL_Point){dst->w/2, dst->h/2};
        return SDL_RenderCopyEx(renderer, texture, src, dst, angle, &center, flip);
    } else
        return 0;
}