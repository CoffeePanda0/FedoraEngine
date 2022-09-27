#ifndef H_RENDER
#define H_RENDER

#include <SDL_gpu.h>
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
int FE_RenderCopy(FE_Camera *camera, bool locked, FE_Texture *texture, GPU_Rect *src, GPU_Rect *dst);


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
int FE_RenderCopyEx(FE_Camera *camera, bool locked, FE_Texture *texture, GPU_Rect *src, GPU_Rect *dst, double angle, GPU_FlipEnum flip);


/** Applies camera zoom and position to a rect
 * \param rect The rect to apply the zoom to
 * \param camera The camera to apply the zoom from
 * \param locked Whether or not the camera is locked
 * \returns The zoomed rect
*/
GPU_Rect FE_ApplyZoom(GPU_Rect *r, FE_Camera *camera, bool locked);


/** Renders a border around a rect
 * \param thickness The thickness of the border
 * \param rect The rect to create a border around
 * \param color The color of the border
*/
void FE_RenderBorder(int thickness, GPU_Rect r, SDL_Color color);


#endif