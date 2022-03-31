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
void FE_RenderMenu();
void FE_FreeUI();
void FE_MenuEventHandle();
void FE_RenderUI();

#endif
