#ifndef H_UI
#define H_UI

#include <SDL.h>

#include "event.h"
#include "dialogue.h"
#include "uiobject.h"
#include "console.h"
#include "button.h"
#include "label.h"
#include "textbox.h"
#include "checkbox.h"
#include "menu.h"
#include "messagebox.h"
#include "font.h"

typedef enum FE_UI_CENTRE_TYPE {
    FE_UI_CENTRE_NONE,
    FE_UI_CENTRE_VERTICAL,
    FE_UI_CENTRE_HORIZONTAL,
    FE_UI_CENTRE_ALL
} FE_UI_CENTRE_TYPE;

/* The render function used for the menus */
void FE_RenderMenu();

/* Cleans all UI element types */
void FE_FreeUI();

/* The event handler used for the menus */
void FE_MenuEventHandle();

/* Renders all UI elements */
void FE_RenderUI();


/** Centres a rect inside the screen 
 * \param type The type of centering to perform
 * \param r The rect to centre
*/
void FE_CentreRect(FE_UI_CENTRE_TYPE type, SDL_Rect *r);

#endif
