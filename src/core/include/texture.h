#ifndef H_TEXTURE
#define H_TEXTURE

#include <SDL.h>

#define COLOR_BLACK (SDL_Color){0, 0, 0, 255}
#define COLOR_WHITE (SDL_Color){255, 255, 255, 255}
#define COLOR_RED (SDL_Color){255, 0, 0, 255}
#define COLOR_GREEN (SDL_Color){0, 255, 0, 255}
#define COLOR_BLUE (SDL_Color){0, 0, 255, 255}
#define COLOR_PINK (SDL_Color){255,20,147, 255}


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


/** A wrapper for SDL_RenderCopy - Renders a texture to the screen if in screen bounds only
 * 
 * \param texture The texture to render
 * \param src The source rectangle of the texture
 * \param dst The destination rectangle of the texture
 * \return 0 on success, -1 on failure
 */
int FE_RenderCopy(SDL_Texture *texture, SDL_Rect *src, SDL_Rect *dst);

#endif