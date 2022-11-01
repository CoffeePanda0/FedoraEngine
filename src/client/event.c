#include <FE_Client.h>

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
                if (event.key.repeat == 0) {
                    if (keyboard_state[SDL_SCANCODE_ESCAPE]) {
                        FE_PauseGame();
                        break;
                    }
                    else if (keyboard_state[SDL_SCANCODE_GRAVE]) {
                        SDL_StartTextInput();
                        PresentGame->StartedInput = true;
                        FE_Console_Show();
                        break;
                    }
                
                    if (keyboard_state[FE_Key_Get("ZOOM IN")])
                        FE_Camera_SmoothZoom(camera, 0.5, 250);
                    else if (keyboard_state[FE_Key_Get("ZOOM OUT")])
                        FE_Camera_SmoothZoom(camera, -0.5, 250);
                    
                    if (keyboard_state[SDL_SCANCODE_I])
                        PresentGame->DebugConfig.ShowTiming = !PresentGame->DebugConfig.ShowTiming;

                    if (keyboard_state[SDL_SCANCODE_L])
                        FE_Light_Toggle(player->Light);
                }

                if (keyboard_state[SDL_SCANCODE_M]) {
                    if (PresentGame->MapConfig.AmbientLight <= 252) {
                        PresentGame->MapConfig.AmbientLight += 3;
                    }
                }
                else if (keyboard_state[SDL_SCANCODE_N]) {
                    if (PresentGame->MapConfig.AmbientLight >= 3) {
                        PresentGame->MapConfig.AmbientLight -= 3;
                    }
                }

            break;
        }
    }

    // Handle essential inputs here to prevent first click issue
    if (!ui_handled) {
        if (keyboard_state[FE_Key_Get("LEFT")])
            FE_Player_Move(player->player, vec(-1, 0));
        if (keyboard_state[FE_Key_Get("RIGHT")])
            FE_Player_Move(player->player, vec(1, 0));
        if (keyboard_state[FE_Key_Get("JUMP")]) {
            if (!player->player->jump_started) {
                FE_Player_StartJump(player->player);
            } else {
                FE_Player_UpdateJump(player->player);
            }
        }

        // noclip controls
        if (PresentGame->DebugConfig.NoClip) {
            if (keyboard_state[SDL_SCANCODE_LEFT])
                FE_Player_Move(player->player, vec(-100, 0));
            if (keyboard_state[SDL_SCANCODE_RIGHT])
                FE_Player_Move(player->player, vec(100, 0));
            if (keyboard_state[SDL_SCANCODE_UP])
                FE_Player_Move(player->player, vec(0, -100));
            if (keyboard_state[SDL_SCANCODE_DOWN])
                FE_Player_Move(player->player, vec(0, 100));
        }

    }
}