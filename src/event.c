#include "include/game.h"

/* Input priority is:
    1- Console
    2- MessageBox
    3- Textbox input
    4- User input
*/

void FE_GameEventHandler(FE_Camera *camera)
{
    const Uint8* keyboard_state = SDL_GetKeyboardState(NULL);
	SDL_PumpEvents();
	SDL_Event event;

    while (SDL_PollEvent(&event)) {
		if (FE_ConsoleVisible) {
			FE_HandleConsoleInput(&event, keyboard_state);
		} else {
            switch (event.type) {
                case SDL_QUIT:
                    FE_Clean();
                break;

                case SDL_MOUSEBUTTONDOWN:
                    if (event.button.button == SDL_BUTTON_LEFT) {
                        FE_UI_HandleClick(&event);
                    }
                break;

                case SDL_KEYDOWN:
                    if (keyboard_state[SDL_SCANCODE_GRAVE] && event.key.repeat == 0) {
                        SDL_StartTextInput();
                        FE_StartedInput = true;
                        FE_ConsoleShow();
                        break;
                    }
                    if (FE_GetActiveTextBox() != NULL) {
                        FE_UpdateTextBox(event.key.keysym.sym);
                        break;
                    }
                    
                    if (keyboard_state[SDL_SCANCODE_W])
						FE_MoveCamera(0, -10, camera);
					else if (keyboard_state[SDL_SCANCODE_A])
						FE_MoveCamera(-10, 0, camera);
					else if (keyboard_state[SDL_SCANCODE_S])
						FE_MoveCamera(0, 10, camera);
					else if (keyboard_state[SDL_SCANCODE_D])
						FE_MoveCamera(10, 0, camera);

                break;

            }
        }
    }
}