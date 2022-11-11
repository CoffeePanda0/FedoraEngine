#ifndef _CHECKBOX_H
#define _CHECKBOX_H

#include <SDL.h>

typedef struct { 
    SDL_Rect r; // rect containing the checkbox
    bool checked; // is the checkbox checked?
    void (*onclick)(); // callback function on checkbox click
    void *onclick_data; // data passed to callback func

    SDL_Texture *label;
    SDL_Rect label_rect;
} FE_UI_Checkbox;


/** Creates and renders a checkbox
*\param label The text to display on the checkbox
*\param x The x position of the checkbox
*\param y The y position of the checkbox
*\param checked Whether the checkbox is checked or not
*\param onclick The callback function to call when the checkbox is clicked
*\param onclick_data The data to pass to the callback function
*/
FE_UI_Checkbox *FE_UI_CreateCheckbox(const char *label, int x, int y, bool checked, void (*onclick)(), void *onclick_data);


/* Renders a checkbox */
void FE_UI_RenderCheckbox(FE_UI_Checkbox *c);


/** Destroys a checkbox
 * \param c The checkbox to destroy
 * \param global Whether to remove the checkbox from the global list or not
 */
void FE_UI_DestroyCheckbox(FE_UI_Checkbox *c, bool global);


/** Handles click event if the mouse clicks on an existing checkbox
*\param x The x position of the mouse
*\param y The y position of the mouse
*\return Whether the mouse click was on an existing checkbox
*/
bool FE_UI_CheckboxClick(int x, int y);

#endif
