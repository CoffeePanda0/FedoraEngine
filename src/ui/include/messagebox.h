#ifndef _MESSAGEBOX_H
#define _MESSAGEBOX_H

#include "font.h"
#include "textbox.h"
#include "button.h"
#include "../../core/include/texture.h"

typedef enum {
    MESSAGEBOX_TEXT,
    MESSAGEBOX_TEXTBOX
} FE_UI_MBType;

typedef struct {
    FE_UI_MBType type;

    GPU_Rect displayrect;

    FE_Texture *texture;

    FE_UI_Label *title;
    FE_UI_Label *content;

    FE_UI_Textbox *textbox;

    FE_UI_Button *button;
    
} FE_UI_Messagebox;

/** Shows a messagebox
 * \param title The title of the messagebox
 * \param body The dialogue to be displayed
 * \param type The type of messagebox
*/
void FE_Messagebox_Show(char *title, char *body, FE_UI_MBType type);


/** Renders the current messagebox being displayed, if any */
void FE_Messagebox_Render();


/* Destroys the current messagebox being shown */
void FE_Messagebox_Destroy();


/* Returns the text that the user has entered into the messagebox input prompt */
char *FE_Messagebox_GetText();


/* Triggers the callback on messagebox closed */
void FE_Messagebox_HandleCallBack();


/** Adds a callback to the confirm button of the messagebox
 * \param func The function to call
 * \param data The params to call func with
*/
void FE_Messagebox_AddCallback(void (*func)(), void *data);

/* Checks if the button or textbox the messagebox is being clicked on */
bool FE_Messagebox_Click(int x, int y);


#endif