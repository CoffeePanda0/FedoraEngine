#ifndef H_LIGHTING
#define H_LIGHTING

typedef struct FE_Light {
    SDL_Rect Rect;
    FE_Texture *Texture;
} FE_Light;


/** Renders lights to screen (and ambient brightness)
 * \param camera The camera to render from
 * \param world The world to render
*/
void FE_RenderLighting(FE_Camera *camera, SDL_Texture *world);


/** Creates a new light effect and adds it to the world
 * \param rect The rectangle to render the light effect in (location and size)
 * \param texture The texture to render the light effect with
 * \returns The new light effect
*/
FE_Light *FE_CreateLight(SDL_Rect rect, const char *texture);


/** Destroys a light and frees memory
 * \param light The light to destroy
*/
void FE_DestroyLight(FE_Light *light);


/* Initialises the lighting system. */
void FE_InitLighting();


/* Destroys the lighting system */
void FE_CleanLighting();


/** Moves a light to a new location. Use this instead of modifiying the light directly.
 * \param light The light to move
 * \param x The new x location
 * \param y The new y location
*/
void FE_MoveLight(FE_Light *light, int x, int y);

#endif