#include "include/game.h"

/* Input priority is:
    1- Console
    2- MessageBox
    3- Textbox input
    4- User input
*/

void FE_GameEventHandler(FE_Camera *camera, FE_Player *player)
{
	SDL_PumpEvents();
	SDL_Event event;
    const Uint8* keyboard_state = SDL_GetKeyboardState(NULL);

    bool ui_handled = false;

    while (SDL_PollEvent(&event)) {
        
        if (FE_UI_HandleEvent(&event, keyboard_state)) {
            ui_handled = true;
            break;
        }

        switch (event.type) {
            case SDL_KEYDOWN:
                if (keyboard_state[SDL_SCANCODE_ESCAPE] && event.key.repeat == 0) {
                    PresentGame->GameState = GAME_STATE_PAUSE;
                    break;
                }
                if (keyboard_state[SDL_SCANCODE_GRAVE] && event.key.repeat == 0) {
                    SDL_StartTextInput();
                    PresentGame->StartedInput = true;
                    FE_Console_Show();
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

            break;
        }
    }

    // Handle essential inputs here to prevent first click issue
    if (!ui_handled) {
        if (keyboard_state[SDL_SCANCODE_A])
            FE_MovePlayer(player, vec2(-player->movespeed, 0));
        if (keyboard_state[SDL_SCANCODE_D])
            FE_MovePlayer(player, vec2(player->movespeed, 0));
        if (keyboard_state[SDL_SCANCODE_SPACE]) {
            if (!player->jump_started) {
                FE_StartPlayerJump(player);
            } else {
                FE_UpdatePlayerJump(player);
            }
        }
    }
}