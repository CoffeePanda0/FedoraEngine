#ifndef H_LIGHT
#define H_LIGHT

#include <SDL.h>
#include "../../core/include/circle.h"

typedef struct FE_LightObject {
    SDL_Rect obj_rect; // rectangle for rendering the source of the light
    FE_Texture *obj_texture; // texture for rendering the source of the light (e.g a sprite)
    
    SDL_Rect effect_rect;
    FE_Texture *effect_texture;
} FE_LightObject;


/** Creates and renders a new light object
 *\param obj_image The texture for the light source
 *\param obj_rect The rectangle for the light source
 *\param effect_name The name of the light effect (e.g "torch", "glow", "fire")
 *\param effect_offset_y The offset of the effect relative to the light source
 *\param size The size of the effect
 *\return The light object
*/
FE_LightObject *FE_CreateLightObject(char *obj_image, SDL_Rect obj_rect, char *effect_name, int effect_offset_x, int effect_offset_y, int size);


/** Destroys a light object
 *\param o The light object
*/
void FE_DestroyLightObject(FE_LightObject *o);


/** Renders all light objects
 *\param camera - The camera currently in use
*/
void FE_RenderLightObjects(FE_Camera *camera);


/* Destroys all light objects */
void FE_CleanLightObjects();

#endif