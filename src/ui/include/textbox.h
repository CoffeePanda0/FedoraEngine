#ifndef H_TEXBOX
#define H_TEXBOX

typedef struct FE_TextBox {
    SDL_Texture *text;
    SDL_Rect r;
    char *content;
    FE_Label *label;
    bool active;
} FE_TextBox;

/** Creates a new textbox for user input on screen
 *\param x The x coordinate of the textbox
 *\param y The y coordinate of the textbox
 *\param w The width of the textbox
 *\param h The height of the textbox
 *\param value The default value inside the textbox (can be NULL)
 *\returns A pointer to the created textbox
*/
FE_TextBox *FE_CreateTextBox(int x, int y, int w, int h, char *value);


/* Renders all text boxes on screen */
void FE_RenderTextBox();


/** Destroys and frees resources used by a textbox
 *\param l A pointer to the textbox to destroy
 */
int FE_DestroyTextBox(FE_TextBox *l);


/* Destroys all textboxes */
int FE_CleanTextBoxes();


/* Adds one character to the end of the selected textbox */
int FE_UpdateTextBox(char c);


/* Returns a string of the user-inputted text */
char *FE_GetContent(FE_TextBox *t);


/** Sets the value of a textbox
*\param t A pointer to the textbox to update
*\value The value for the textbox input to display
*/
int FE_SetContent(FE_TextBox *t, char *value);


/* Returns a pointer to the currently active (selected) textbox, or NULL otherwise */
FE_TextBox *FE_GetActiveTextBox();


/* Forces the textbox to be active (where user input will go to) */
int FE_ForceActiveTextBox(FE_TextBox *t);


/* Handles clicking on a textbox to make it active. Returns true if textbox was clicked, false otherwise */
bool FE_TextBoxClick(int x, int y);

#endif