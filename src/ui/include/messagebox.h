#ifndef H_MESSAGEBOX
#define H_MESSAGEBOX

#include "font.h"
#include "textbox.h"
#include "button.h"
#include "../../core/include/texture.h"

enum FE_MBTYPE {
    MESSAGEBOX_TEXT,
    MESSAGEBOX_TEXTBOX
};


typedef struct FE_MessageBox {
    enum FE_MBTYPE type;

    SDL_Rect displayrect;

    FE_Texture *texture;

    SDL_Texture *title_texture;
    SDL_Rect title_rect;

    SDL_Texture *dialogue_texture;
    SDL_Rect dialogue_rect;

    FE_TextBox *textbox;

    FE_Button *button;
    
} FE_MessageBox;


/** Shows a messagebox with an input text prompt
*\param title The title of the messagebox
*\param dialogue The dialogue to be displayed
*\param onclick The function to be called when the button is clicked
*\param onclick_data The data to be passed to the onclick function
*/
int FE_ShowInputMessageBox(char *title, char *dialogue, void (*onclick), void *onclick_data);


/** Shows a simple messagebox
 * \param title The title of the messagebox
 * \param body The dialogue to be displayed
*/
int FE_ShowMessageBox(char *title, char *body);


/** Renders the current messagebox being displayed, if any */
void FE_RenderMessageBox();


/* Destroys the current messagebox being shown */
void FE_DestroyMessageBox();


/* Returns the text that the user has entered into the messagebox input prompt */
char *FE_GetMessageBoxInput();

#endif