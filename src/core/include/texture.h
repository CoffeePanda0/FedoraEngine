#ifndef H_TEXTURE
#define H_TEXTURE

#include <SDL.h>

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


/* Cleanly exits the texture manager */
void FE_CloseTextureManager();

#endif