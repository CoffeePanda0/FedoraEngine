#ifndef H_BUTTON
#define H_BUTTON

typedef struct FE_Button {
    SDL_Texture *text;
    SDL_Texture *hover_text;

    SDL_Texture *label;
    SDL_Rect label_rect;

    SDL_Rect r;
    void (*onclick)();
    void *onclick_data;

    bool hover;
} FE_Button;

enum FE_BUTTON_TYPE {
    BUTTON_TINY,
    BUTTON_SMALL,
    BUTTON_MEDIUM,
    BUTTON_LARGE
};

/** Creates a new button on the UI
*\param text The text to display on the button
*\param int x The x position of the button
*\param int y The y position of the button
*\param enum FE_BUTTON_TYPE type The type of button to create (tiny, small, medium, large)
*\param onclick The function to call when the button is clicked
*\param onclick_data The data to pass to the onclick function
*\return The new button
*/
FE_Button *FE_CreateButton(const char *text, int x, int y, enum FE_BUTTON_TYPE t, void (*onclick), void *onclick_data);


/** Destroys a button, freeing resources used and removed from render
 * \param b The button to destroy
 * \return -1 on error, 1 on success
*/
int FE_DestroyButton(FE_Button *b);


/* Renders all buttons on screen */
void FE_RenderButtons();


/** Destroys all buttons
 * \return -1 on error, 1 on success
 */
int FE_CleanButtons();


/* Handles callback when a button is clicked
    * \param x The x position of the mouse click
    * \param y The y position of the mouse click
    * \return true if button was clicked, false otherwise
    */
bool FE_ButtonClick(int x, int y);

#endif
