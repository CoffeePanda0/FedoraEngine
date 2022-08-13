#ifndef H_TEXTURE
#define H_TEXTURE

#include <SDL.h>
#include "../../physics/include/vector.h"

#define COLOR_BLACK (SDL_Color){0, 0, 0, 255}
#define COLOR_WHITE (SDL_Color){255, 255, 255, 255}
#define COLOR_RED (SDL_Color){255, 0, 0, 255}
#define COLOR_GREEN (SDL_Color){0, 255, 0, 255}
#define COLOR_BLUE (SDL_Color){0, 0, 255, 255}
#define COLOR_PINK (SDL_Color){255,20,147, 255}

typedef struct FE_Texture {
    SDL_Texture *Texture;
    char *path;
} FE_Texture;

/** Returns a pointer to a solid texture from a color
 *\param color The color to make the texture
 *\return A pointer to the texture
*/
SDL_Texture *FE_TextureFromRGBA(SDL_Color color);


/** Returns a pointer to a texture from an image file
 *\param path The path to the image file
 *\returns A pointer to the texture
 */
SDL_Texture *FE_TextureFromFile(const char *path);


/** Destroys a FedoraEngine texture (freeing all memory used
 * \param texture The texture to destroy
*/
int FE_DestroyTexture(FE_Texture *texture);


/** Wrapper for SDL_QueryTexture. Used to find the width and height of a texture
* \param t The texture to query
* \param w The int to set the width to
* \param h The int to set the height to
*/
int FE_QueryTexture(FE_Texture *t, int *w, int *h);


/** Fills a texture with a color (NOTE: TextureAccess must be set to TARGET)
 * \param texture The texture to fill
 * \param r The red value to fill with
 * \param g The green value to fill with
 * \param b The blue value to fill with
 * \param a The alpha value to fill with
*/
void FE_FillTexture(SDL_Texture *texture, int r, int g, int b, int a);


/* Returns an accessable texture with the screen dimensions */
SDL_Texture *FE_CreateRenderTexture(int w, int h);


/* Cleanly exits the texture manager */
void FE_CloseTextureManager();

typedef struct FE_TextureAtlas {
    SDL_Texture *atlas;
    char *path;

    int width, height;
    uint16_t texturesize;
} FE_TextureAtlas;


/** Returns the SDL_Rect of a texture in the atlas
 * \param atlas The atlas to search
 * \param index The index of the texture to search for
 * \returns The location of the texture
*/
vec2 FE_GetTexturePosition(FE_TextureAtlas *atlas, size_t index);


/** Returns a texture atlas
 * \param name The name of the atlas
 * \returns A pointer to the atlas
*/
FE_TextureAtlas *FE_LoadTextureAtlas(const char *name);


/** Destroys a texture atlas
 * \param atlas The atlas to destroy
*/
void FE_DestroyTextureAtlas(FE_TextureAtlas *atlas);


/** Renders a texture from an atlas
 * \param atlas The atlas to render from
 * \param index The index of the texture to render
 * \param dst The destination to render to
*/
void FE_RenderAtlasTexture(FE_TextureAtlas *atlas, size_t index, SDL_Rect *dst);


#endif