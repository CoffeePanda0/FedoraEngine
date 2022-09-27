#ifndef _BUTTON_H
#define _BUTTON_H

typedef struct {
    GPU_Image *text;
    GPU_Image *hover_text;

    GPU_Image *label;
    GPU_Rect label_rect;

    GPU_Rect r;
    void (*onclick)();
    void *onclick_data;

    bool hover;
} FE_UI_Button;

typedef enum FE_BUTTON_TYPE {
    BUTTON_CLOSE,
    BUTTON_TINY,
    BUTTON_SMALL,
    BUTTON_MEDIUM,
    BUTTON_LARGE
} FE_BUTTON_TYPE;

/** Creates a new button on the UI
*\param text The text to display on the button
*\param int x The x position of the button
*\param int y The y position of the button
*\param enum FE_BUTTON_TYPE type The type of button to create (tiny, small, medium, large)
*\param onclick The function to call when the button is clicked
*\param onclick_data The data to pass to the onclick function
*\return The new button
*/
FE_UI_Button *FE_UI_CreateButton(const char *text, int x, int y, FE_BUTTON_TYPE t, void (*onclick)(), void *onclick_data);


/** Destroys a button, freeing resources
 * \param b The button to destroy
 * \param global Whether to free the button from the global list of buttons
*/
void FE_UI_DestroyButton(FE_UI_Button *b, bool global);


/** Renders a button to the screen
 * \param b The button to render
*/
void FE_UI_RenderButton(FE_UI_Button *b);


/* Handles callback when a button is clicked
* \param x The x position of the mouse click
* \param y The y position of the mouse click
* \return true if button was clicked, false otherwise
*/
bool FE_UI_ButtonClick(int x, int y);


/* Checks if the mouse is hovering over a button on screen */
void FE_UI_CheckButtonHover();


/** Moves a button to a new position
 * \param b The button to move
 * \param x The new x position
 * \param y The new y position
*/
void FE_UI_MoveButton(FE_UI_Button *b, int x, int y);

#endif
