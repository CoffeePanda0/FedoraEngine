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

extern bool FE_intext;
extern bool FE_DialogueActive;

// Functions

/* The render function used for the menus */
void FE_RenderMenu();

/* Cleans all UI element types */
void FE_FreeUI();

/* The event handler used for the menus */
void FE_MenuEventHandle();

/* Renders all UI elements */
void FE_RenderUI();

#endif
