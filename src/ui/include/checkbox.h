#ifndef H_CHECKBOX
#define H_CHECKBOX

typedef struct FE_Checkbox{ 
    SDL_Rect r; // rect containing the checkbox
    bool checked; // is the checkbox checked?
    void (*onclick)(void *data); // callback function on checkbox click
    void *onclick_data; // data passed to callback func

    SDL_Texture *label;
    SDL_Rect label_rect;
} FE_CheckBox;


/** Creates and renders a checkbox
*\param label The text to display on the checkbox
*\param x The x position of the checkbox
*\param y The y position of the checkbox
*\param checked Whether the checkbox is checked or not
*\param onclick The callback function to call when the checkbox is clicked
*\param onclick_data The data to pass to the callback function
*/
FE_CheckBox *FE_CreateCheckbox(const char *label, int x, int y, bool checked, void (*onclick)(), void *onclick_data);


/* Destroys all checkboxes on screen */
int FE_CleanCheckBoxes();


/* Renders all created checkboxes */
void FE_RenderCheckBoxes();


/* Handles click event if the mouse clicks on an existing checkbox */
bool FE_CheckboxClick(int x, int y);

#endif
