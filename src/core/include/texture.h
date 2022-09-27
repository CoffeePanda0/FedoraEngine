#ifndef H_TEXTURE
#define H_TEXTURE

#include <SDL_gpu.h>
#include "../../core/include/vector.h"

#define COLOR_BLACK (SDL_Color){0, 0, 0, 255}
#define COLOR_WHITE (SDL_Color){255, 255, 255, 255}
#define COLOR_RED (SDL_Color){255, 0, 0, 255}
#define COLOR_GREEN (SDL_Color){0, 255, 0, 255}
#define COLOR_BLUE (SDL_Color){0, 0, 255, 255}
#define COLOR_PINK (SDL_Color){255,20,147, 255}

typedef struct FE_Texture {
    GPU_Image *Texture;
    char *path;
} FE_Texture;

/** Returns a pointer to a solid texture from a color
 *\param color The color to make the texture
 *\return A pointer to the texture
*/
GPU_Image *FE_TextureFromRGBA(SDL_Color color);


/** Returns a pointer to a texture from an image file
 *\param path The path to the image file
 *\returns A pointer to the texture
 */
GPU_Image *FE_TextureFromFile(const char *path);


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


/* Cleanly exits the texture manager */
void FE_CloseTextureManager();

typedef struct FE_TextureAtlas {
    GPU_Image *atlas;
    char *path;

    int width, height;
    uint16_t texturesize;

    size_t max_index;
} FE_TextureAtlas;


/** Returns the GPU_Rect of a texture in the atlas
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
void FE_RenderAtlasTexture(FE_TextureAtlas *atlas, size_t index, GPU_Rect *dst);


/** Creates an GPU_Image from a tile inside an atlas
 * \param atlas The atlas to create the texture from
 * \param index The index of the texture to create
 * \returns The new texture
 */
GPU_Image *FE_TextureFromAtlas(FE_TextureAtlas *atlas, size_t index);


#endif