#ifndef H_UIOBJECT
#define H_UIOBJECT

#include "../../core/include/texture.h"

typedef struct {
    SDL_Rect r;
    FE_Texture *texture;
} FE_UI_Object;


/** Renders a UI object to the screen
 * \param o The UI object to be rendered
 */
void FE_UI_RenderObject(FE_UI_Object *o);


/** Creates a new UI object
 *\param x The x position of the object
 *\param y The y position of the object
 *\param w The width of the object
 *\param h The height of the object
 *\param texture The texture to use for the object
 *\return The new UI object
 */
FE_UI_Object *FE_UI_CreateObject(int x, int y, int w, int h, char *texture_path);


/** Destroys a UI object, freeing resources
 * \param o The UI object to destroy
 * \param global Whether to free the object from the global list of objects
 */
void FE_UI_DestroyObject(FE_UI_Object *o, bool global);


#endif
