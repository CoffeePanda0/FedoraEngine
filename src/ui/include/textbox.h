#ifndef _TEXTBOX_H
#define _TEXTBOX_H

#include "label.h"
#include "../../core/include/texture.h"

typedef struct {
    FE_Texture *text;
    FE_Texture *active_text;
    
    GPU_Rect r;
    char *content;
    FE_UI_Label *label;
    bool active;

    void (*onenter)();
    void *data;
} FE_UI_Textbox;

/** Creates a new textbox for user input on screen
 *\param x The x coordinate of the textbox
 *\param y The y coordinate of the textbox
 *\param w The width of the textbox
 *\param value The default value inside the textbox (can be NULL)
 *\returns A pointer to the created textbox
*/
FE_UI_Textbox *FE_UI_CreateTextbox(int x, int y, int w, char *value);


/** Renders a textbox on screen
 *\param tb The textbox to render
*/
void FE_UI_RenderTextbox(FE_UI_Textbox *tb);


/** Destroys and frees resources used by a textbox
 *\param l A pointer to the textbox to destroy
 */
void FE_UI_DestroyTextbox(FE_UI_Textbox *tb, bool global);


/* Adds one character to the end of the selected textbox */
void FE_UI_UpdateTextbox(char c);


/* Returns a string of the user-inputted text */
char *FE_UI_GetTextboxContent(FE_UI_Textbox *tb);


/** Sets the value of a textbox
*\param t A pointer to the textbox to update
*\param value The value for the textbox input to display
*/
void FE_UI_SetTextboxContent(FE_UI_Textbox *tb, char *value);


/* Returns a pointer to the currently active (selected) textbox, or NULL otherwise */
FE_UI_Textbox *FE_UI_GetActiveTextbox();


/* Forces the textbox to be active (where user input will go to) */
void FE_UI_ForceActiveTextbox(FE_UI_Textbox *tb);


/* Handles clicking on a textbox to make it active. Returns true if textbox was clicked, false otherwise */
bool FE_UI_TextboxClick(int x, int y);

/* Moves the textbox including the label to the given coordinates */
void FE_UI_MoveTextbox(FE_UI_Textbox *tb, int x, int y);


/** Adds a callback when the enter key is pressed in a textbox
 * \param tb A pointer to the textbox to add the callback to
 * \param callback The callback to add
 * \param data A pointer to data to pass to the callback
 */
void FE_UI_AddTextboxCallback(FE_UI_Textbox *tb, void (*callback)(), void *data);


#endif