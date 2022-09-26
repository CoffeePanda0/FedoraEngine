#ifndef H_MAPBG
#define H_MAPBG

#include "map.h"
#include "../../core/include/strarr.h"

typedef struct FE_Map_Parallax {
    float scale;
    FE_Texture *texture;
    
    SDL_Rect r1; // two rects to give infinite scrolling
    SDL_Rect r2;
} FE_Map_Parallax;

/** Renders the parallax layers (if added) or static bg image
 * \param camera The camera to render the map from
 * \param map The map to render
 */
void FE_Map_RenderBG(FE_Camera *camera, FE_LoadedMap *map);


/** Adds a parallax layer
 * \param layer_name The name of the texture layer
 * \param scale The scale to move at relative to the camera
 */
void FE_Parallax_Add(const char *layer_name, float scale);


/** Changes the ratio of the speed of all layers
 * \param speed The new ratio to move the layers at
*/
void FE_Parallax_SetSpeed(float speed);


/** Loads an entire parallax background from a config
 * \param name The name of the parallax to load
*/
void FE_Parallax_Load(const char *name);


/* Removes all loaded parallax layers */
void FE_Parallax_Clean();


/* Returns the amount of parallax backgrounds that exist and their names */
FE_StrArr *FE_Parallax_Count();

char *FE_Parallax_GetName();

#endif