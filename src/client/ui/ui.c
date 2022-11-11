#include "../core/include/include.h"
#include "include/include.h"
#include "include/menu.h"
#include "../../common/net/include/net.h"

bool FE_UI_ControlContainerLocked = false;

void FE_UI_InitUI()
{
    PresentGame->UIConfig->ActiveElements = xmalloc(sizeof(FE_UIList));
    PresentGame->UIConfig->ActiveElements->Count = 0;
    PresentGame->UIConfig->ActiveElements->Containers = 0;
    PresentGame->UIConfig->ActiveElements->Buttons = 0;
    PresentGame->UIConfig->ActiveElements->Objects = 0;
    PresentGame->UIConfig->ActiveElements->Labels = 0;
    PresentGame->UIConfig->ActiveElements->Checkboxes = 0;
    PresentGame->UIConfig->ActiveElements->Textboxes = 0;
    PresentGame->UIConfig->ActiveElements->Grids = 0;
    PresentGame->UIConfig->UIFont = PresentGame->Client->Font;
}

void FE_UI_AddElement(FE_UI_Type type, void *element)
{
    if (!element) {
        warn("FE_UI_AddElement: element is null");
        return;
    }
    if (type == FE_UI_LABEL)
        FE_List_Add(&PresentGame->UIConfig->ActiveElements->Labels, element);
    else if (type == FE_UI_BUTTON)
        FE_List_Add(&PresentGame->UIConfig->ActiveElements->Buttons, element);
    else if (type == FE_UI_OBJECT)
        FE_List_Add(&PresentGame->UIConfig->ActiveElements->Objects, element);
    else if (type == FE_UI_CONTAINER)
        FE_List_Add(&PresentGame->UIConfig->ActiveElements->Containers, element);
    else if (type == FE_UI_CHECKBOX)
        FE_List_Add(&PresentGame->UIConfig->ActiveElements->Checkboxes, element);
    else if (type == FE_UI_TEXTBOX)
        FE_List_Add(&PresentGame->UIConfig->ActiveElements->Textboxes, element);
    else if (type == FE_UI_GRID)
        FE_List_Add(&PresentGame->UIConfig->ActiveElements->Grids, element);
    else {
        warn("FE_UI_AddElement: Unknown UI element type");
        return;
    }

    PresentGame->UIConfig->ActiveElements->Count++;
}

void FE_UI_ClearElements(FE_UIList *Elements)
{
    if (!Elements || Elements->Count == 0)
        return;

    if (PresentGame->DebugConfig.ShowTiming) {
		FE_DebugUI_Destroy();
	}

    FE_Messagebox_Destroy();
    
    for (FE_List *l = Elements->Labels; l; l = l->next)
        FE_UI_DestroyLabel((FE_UI_Label *)l->data, false);
    for (FE_List *l = Elements->Buttons; l; l = l->next)
        FE_UI_DestroyButton((FE_UI_Button *)l->data, false);
    for (FE_List *l = Elements->Containers; l; l = l->next)
        FE_UI_DestroyContainer((FE_UI_Container *)l->data, true, false);
    for (FE_List *l = Elements->Objects; l; l = l->next)
        FE_UI_DestroyObject((FE_UI_Object *)l->data, false);
    for (FE_List *l = Elements->Checkboxes; l; l = l->next)
        FE_UI_DestroyCheckbox((FE_UI_Checkbox *)l->data, false);
    for (FE_List *l = Elements->Textboxes; l; l = l->next)
        FE_UI_DestroyTextbox((FE_UI_Textbox *)l->data, false);
    for (FE_List *l = Elements->Grids; l; l = l->next)
        FE_UI_DestroyGrid((FE_UI_Grid *)l->data, false);

    FE_List_Destroy(&Elements->Labels);
    FE_List_Destroy(&Elements->Buttons);
    FE_List_Destroy(&Elements->Containers);
    FE_List_Destroy(&Elements->Objects);
    FE_List_Destroy(&Elements->Checkboxes);
    FE_List_Destroy(&Elements->Textboxes);
    FE_List_Destroy(&Elements->Grids);
    
    Elements->Labels = 0;
    Elements->Buttons = 0;
    Elements->Containers = 0;
    Elements->Objects = 0;
    Elements->Checkboxes = 0;
    Elements->Textboxes = 0;
    Elements->Grids = 0;
    
    Elements->Count = 0;

    FE_UI_ControlContainerLocked = false;
}

void FE_UI_Render()
{
    if (PresentGame->UIConfig->ActiveElements->Count > 0) {

        for (FE_List *l = PresentGame->UIConfig->ActiveElements->Objects; l; l = l->next)
            FE_UI_RenderObject((FE_UI_Object *)l->data);
        for (FE_List *l = PresentGame->UIConfig->ActiveElements->Textboxes; l; l = l->next)
            FE_UI_RenderTextbox((FE_UI_Textbox *)l->data);
        for (FE_List *l = PresentGame->UIConfig->ActiveElements->Labels; l; l = l->next)
            FE_UI_RenderLabel((FE_UI_Label *)l->data);
        for (FE_List *l = PresentGame->UIConfig->ActiveElements->Buttons; l; l = l->next)
            FE_UI_RenderButton((FE_UI_Button *)l->data);
        for (FE_List *l = PresentGame->UIConfig->ActiveElements->Checkboxes; l; l = l->next)
            FE_UI_RenderCheckbox((FE_UI_Checkbox *)l->data);
        for (FE_List *l = PresentGame->UIConfig->ActiveElements->Grids; l; l = l->next)
            FE_UI_RenderGrid((FE_UI_Grid *)l->data);
        for (FE_List *l = PresentGame->UIConfig->ActiveElements->Containers; l; l = l->next)
            FE_UI_RenderContainer((FE_UI_Container *)l->data);
    }
    
    FE_Messagebox_Render();
    FE_Console_Render();
}

bool FE_UI_HandleClick(SDL_Event *event)
{
    // Get mouse position
    if (event->button.button == SDL_BUTTON_LEFT) {
        int x, y;
        SDL_GetMouseState(&x, &y);
        
        if (!PresentGame->UIConfig->MBShown) {
            if (FE_UI_ButtonClick(x,y))
                return true;
            if (FE_UI_CheckboxClick(x,y))
                return true;
            if (FE_UI_TextboxClick(x,y))
                return true;
            if (FE_UI_GridClick())
                return true;
        } else {
            if (FE_Messagebox_Click(x,y))
                return true;
        }
    }
    if (PresentGame->UIConfig->InText)
        FE_UI_ForceActiveTextbox(NULL); // make textbox inactive if clicked outside

    return false;
}

bool FE_UI_HandleEvent(SDL_Event *event, const Uint8* keyboard_state)
{
    if (PresentGame->Client->ConsoleVisible) {
        static char InputBuffer[256];

        if (keyboard_state[SDL_SCANCODE_GRAVE] && event->key.repeat == 0) {
            if (event->type == SDL_KEYDOWN) { // only close the console if the key is pressed
                SDL_StopTextInput();
                FE_Console_Hide();
                PresentGame->Client->StartedInput = false;
                return true;
            }
	    }

        if (keyboard_state[SDL_SCANCODE_BACKSPACE]) {
            if (mstrlen(InputBuffer) > 0) {
                InputBuffer[mstrlen(InputBuffer) - 1] = '\0';
                FE_Console_UpdateInput(InputBuffer);
                return true;
            }
        }

        if ((mstrlen(InputBuffer) < 256) && event->type == SDL_TEXTINPUT) {
            mstrcat(InputBuffer, event->text.text);
            FE_Console_UpdateInput(InputBuffer);
            return true;
        }

    }

    switch (event->type) {
        case SDL_QUIT:
            FE_Clean();
            return true;
        break;

        case SDL_MOUSEMOTION:
            FE_UI_CheckButtonHover();
            FE_UI_CheckGridHover();
        break;
        
        case SDL_MOUSEBUTTONDOWN:
            if (event->button.button == SDL_BUTTON_LEFT)
                if (FE_UI_HandleClick(event)) return true;
        break;

        case SDL_KEYDOWN:
            if (PresentGame->UIConfig->DialogueActive) {
                FE_Dialogue_Interact();
                return true;
            }

            if (PresentGame->UIConfig->InText) {
                FE_UI_UpdateTextbox(event->key.keysym.sym);
                return true;
            }

            if (PresentGame->UIConfig->MBShown && (event->key.keysym.sym == 32 || event->key.keysym.sym == 13)) {
                FE_Messagebox_Destroy();
                return true;
            }

            /* Handle fullscreen */
            if (keyboard_state[SDL_SCANCODE_F]) {
                PresentGame->Client->FullScreen = !PresentGame->Client->FullScreen;
                SDL_SetWindowFullscreen(PresentGame->Client->Window, PresentGame->Client->FullScreen ? SDL_WINDOW_FULLSCREEN : 0);
                SDL_GetWindowSize(PresentGame->Client->Window, &PresentGame->WindowWidth, &PresentGame->WindowHeight); // recalculate window size
                return true;
            }

            /* Close open containers */
            if (keyboard_state[SDL_SCANCODE_ESCAPE]) {
                if (FE_UI_CloseOpenContainer())
                    return true;
            }

        break;
    }
    return FE_UI_ControlContainerLocked;
}

void FE_UI_Func_NotImplemented()
{
    FE_Messagebox_Show("Message", "Function not implemented", MESSAGEBOX_TEXT);
    info("Non-implemented function called");
}