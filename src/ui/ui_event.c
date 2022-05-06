#include "../include/game.h"

bool FE_StartedInput = false;

static char InputBuffer[256];

/* Calls the appropriate function depending on which UI element is clicked */
bool FE_UI_HandleClick(SDL_Event *e)
{
    if (e->button.button == SDL_BUTTON_LEFT) {
        if (PresentGame->ConsoleVisible)
            return true;
        
        int mouse_x, mouse_y;
        SDL_GetMouseState(&mouse_x, &mouse_y);

        // todo - known issue - user can click elements under a messagebox

        // Check for which element was clicked
        if (FE_ButtonClick(mouse_x, mouse_y))
            return true;
        if (PresentGame->MBShown) // prevent clicking under box
            return false;
        if (FE_CheckboxClick(mouse_x, mouse_y))
            return true;
        if (FE_TextBoxClick(mouse_x, mouse_y))
            return true;
    } else {
        warn("FE_UI_HandleClick: Innapropriate event type");
    }
    return false;
}

/* Takes input once the console is open */
void FE_HandleConsoleInput(SDL_Event *e, const Uint8* keyboard_state)
{
    if (!e || !keyboard_state)
        return;

    // close the console
    if (keyboard_state[SDL_SCANCODE_GRAVE] && e->key.repeat == 0) {
        if (e->type == SDL_KEYDOWN) { // only close the console if the key is pressed
            SDL_StopTextInput();
            FE_ConsoleHide();
            FE_StartedInput = false;
        }
        return;
	}

    // handle enter key
    if (keyboard_state[SDL_SCANCODE_RETURN] && e->key.repeat == 0) {
        info("FE_HandleConsoleInput: Enter key not implemented");
        return;
    }
    
    // handle backspace
    if (keyboard_state[SDL_SCANCODE_BACKSPACE]) {
        if (strlen(InputBuffer) > 0) {
            InputBuffer[strlen(InputBuffer) - 1] = '\0';
            FE_ConsoleUpdateInput(InputBuffer);
        }
    }

    switch (e->type) {
        // handle text input
        case SDL_TEXTINPUT:
            if (strlen(InputBuffer) < 256) {
                strcat(InputBuffer, e->text.text);
                FE_ConsoleUpdateInput(InputBuffer);
            }
        break;
        // window controls
        case SDL_QUIT:
			FE_Clean();
		break;
    }
}