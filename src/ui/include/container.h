#ifndef _CONTAINER_H
#define _CONTAINER_H

#include <SDL_gpu.h>
#include "label.h"
#include "ui.h"
#include "button.h"
#include "../../core/include/texture.h"

typedef struct {
    GPU_Rect body; // For rendering
    GPU_Rect inner_rect; // For elements to go into

    FE_Texture *texture;

    FE_UI_Label *title;
    
    size_t children_count;
    FE_UI_Element *children;
    int last_child_bottom; // Used for calculating Y position

} FE_UI_Container;


/** Destroys a container and removes from render
 * \param c The container to destroy
 * \param free_children Whether to free the children
 * \param global Whether to free the container from the global list of containers
 */
void FE_UI_DestroyContainer(FE_UI_Container *c, bool free_children, bool global);


/** Adds a child to a container
 * \param container The container to add the child to
 * \param type The type of child to add
 * \param element The element to add
 * \param location The location of the child
*/
void FE_UI_AddChild(FE_UI_Container *container, FE_UI_Type type, void *element, FE_UI_LOCATION location);


/** Creates a new UI container
 * \param x The x position of the container
 * \param y The y position of the container
 * \param w The width of the container
 * \param h The minimum height of the container
 * \param title The title of the container
 * \param lockcontrols Whether to lock buttons and input outside of the container
 * \return The new container
 */
FE_UI_Container *FE_UI_CreateContainer(int x, int y, int w, int h, char *title, bool lockcontrols);


/** Renders a container and its children
 * \param c The container to render
 */
void FE_UI_RenderContainer(FE_UI_Container *c);


/** Calculates the position for r to be centred inside the container
 * \param r The rect to centre
 * \param container The container to centre within
 * \return A vector containing the position for r to be centred
*/
vec2 FE_GetCentre(GPU_Rect r, GPU_Rect container);


/** Adds an invisible height spacer for a container
 * \param container The container to add the spacer to
 * \param height The height of the spacer
 */
void FE_UI_AddContainerSpacer(FE_UI_Container *container, int h);


/** Adds a close button to a container
 * \param container The container to add the close button to
 */
void FE_UI_AddContainerClose(FE_UI_Container *c);


/* Closes the current open container for ESCAPE key */
bool FE_UI_CloseOpenContainer();


#endif