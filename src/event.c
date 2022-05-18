#include "include/game.h"

/* Input priority is:
    1- Console
    2- MessageBox
    3- Textbox input
    4- User input
*/

static bool fullscreen = false;

void FE_GameEventHandler(FE_Camera *camera, FE_Player *player)
{
    const Uint8* keyboard_state = SDL_GetKeyboardState(NULL);
	SDL_PumpEvents();
	SDL_Event event;

    while (SDL_PollEvent(&event)) {
		if (PresentGame->ConsoleVisible) {
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
                    if (PresentGame->DialogueActive) {
                        FE_DialogueInteract();
                        return;
                    }

                    if (keyboard_state[SDL_SCANCODE_ESCAPE] && event.key.repeat == 0) {
                        PresentGame->GameState = GAME_STATE_PAUSE;
                        break;
                    }
                    if (keyboard_state[SDL_SCANCODE_GRAVE] && event.key.repeat == 0) {
                        SDL_StartTextInput();
                        PresentGame->StartedInput = true;
                        FE_ConsoleShow();
                        break;
                    }
                    if (FE_GetActiveTextBox() != NULL) {
                        FE_UpdateTextBox(event.key.keysym.sym);
                        break;
                    }
                    
                    if (keyboard_state[SDL_SCANCODE_Z] && event.key.repeat == 0)
                        FE_Camera_SmoothZoom(camera, 0.5, 250);
                    if (keyboard_state[SDL_SCANCODE_X] && event.key.repeat == 0)
                        FE_Camera_SmoothZoom(camera, -0.5, 250);
                    

                    if (keyboard_state[SDL_SCANCODE_M]) {
                        if (PresentGame->MapConfig.AmbientLight < 255) {
                            PresentGame->MapConfig.AmbientLight += 3;
                        }
                    }
                    if (keyboard_state[SDL_SCANCODE_N]) {
                        if (PresentGame->MapConfig.AmbientLight > 0) {
                            PresentGame->MapConfig.AmbientLight -= 3;
                        }
                    }
                    if (keyboard_state[SDL_SCANCODE_I] && event.key.repeat == 0)
                        PresentGame->DebugConfig.ShowTiming = !PresentGame->DebugConfig.ShowTiming;

                    if (keyboard_state[SDL_SCANCODE_L] && event.key.repeat == 0)
                        FE_ToggleLight(player->Light);

                    if (keyboard_state[SDL_SCANCODE_F] && event.key.repeat == 0) {
                        fullscreen = !fullscreen;
                        SDL_SetWindowFullscreen(PresentGame->Window, fullscreen ? SDL_WINDOW_FULLSCREEN : 0);
                        SDL_GetWindowSize(PresentGame->Window, &PresentGame->Window_width, &PresentGame->Window_height); // recalculate window size
                    }
                break;
            }
        }
    }

    // Handle essential inputs here to prevent first click issue
    if (!PresentGame->ConsoleVisible && !PresentGame->DialogueActive) {
        if (keyboard_state[SDL_SCANCODE_A])
            FE_MovePlayer(player, FE_NewVector(-player->movespeed, 0));
        if (keyboard_state[SDL_SCANCODE_D])
            FE_MovePlayer(player, FE_NewVector(player->movespeed, 0));
        if (keyboard_state[SDL_SCANCODE_SPACE]) {
            if (!player->jump_started) {
                FE_StartPlayerJump(player);
            } else {
                FE_UpdatePlayerJump(player);
            }
        }
    }
}