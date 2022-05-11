#ifndef H_RENDER
#define H_RENDER

#include <SDL.h>
#include "../../world/include/camera.h"
#include "texture.h"


/** A wrapper for SDL_RenderCopy - Renders a texture to the screen if in screen bounds only, applying camera offset
 * \param camera The camera to render the texture to 
 * \param locked Whether or not the camera is locked
 * \param texture The texture to render
 * \param src The source rectangle of the texture
 * \param dst The destination rectangle of the texture
 * \return SDL_RenderCopy code
 */
int FE_RenderCopy(FE_Camera *camera, bool locked, FE_Texture *texture, SDL_Rect *src, SDL_Rect *dst);


/** A wrapper for SDL_RenderCopyEx - Renders a texture to the screen with rotation if in screen bounds only, applying camera offset
 * \param camera The camera to render the texture in
 * \param locked Whether or not the camera is locked
 *\param texture The texture to render
 *\param src The source rectangle of the texture
 *\param dst The destination rectangle of the texture
 *\param angle The angle to rotate the texture by
 *\param flip The flip to apply to the texture
 *\return SDL_RenderCopy code
 */
int FE_RenderCopyEx(FE_Camera *camera, bool locked, FE_Texture *texture, SDL_Rect *src, SDL_Rect *dst, double angle, SDL_RendererFlip flip);


/** Renders an SDL_Rect to the screen with a filled colour
 * \param r The rectangle to render
 * \param color The color to fill the rectangle with
 * \returns 1 on success, -1 on failure
*/
int FE_RenderRect(SDL_Rect *rect, SDL_Color color);


/** Renders a line to the screen. Wrapper for SDL_RenderDrawLine but applies camera
 * \param x1 The x coordinate of the first point
 * \param y1 The y coordinate of the first point
 * \param x2 The x coordinate of the second point
 * \param y2 The y coordinate of the second point
 * \param color The color of the line to draw
 * \returns 1 on success, -1 on failure
 */
int FE_RenderDrawLine(FE_Camera *camera, int x1, int y1, int x2, int y2, SDL_Color color);


#endif