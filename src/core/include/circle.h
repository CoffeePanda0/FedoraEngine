#ifndef H_CIRCLE
#define H_CIRCLE

#include <stdlib.h>
#include <SDL.h>

typedef struct FE_Circle {
    int32_t x, y; // x and y centre of circle
    int32_t radius; // radius of circle

    SDL_Color color; // color of circle (inside and out)
    bool filled; // whether or not to fill the circle
} FE_Circle;


/** Renders a circle to the screen.
 * \param circle The circle to render.
 */
void FE_RenderCircle(FE_Circle *circle);

#endif