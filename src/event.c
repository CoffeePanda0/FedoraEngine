#include "include/game.h"

/* Input priority is:
    1- Console
    2- MessageBox
    3- Textbox input
    4- User input
*/

static bool positionlog = false; // used for testing, probably removed later
static size_t lastlogtime = 0;

static bool fullscreen = false;

void FE_GameEventHandler(FE_Camera *camera, FE_Player *player)
{
    if (positionlog) {
        lastlogtime += 1;
        if (lastlogtime >= (10 / FE_DT_MULTIPLIER)) { // only log every 10 frames to save cpu use
            lastlogtime = 0;
            printf("PLAYER X: %i | PLAYER Y: %i| VELOCITY: %f,%f | DT: %f\n", player->PhysObj->body.x, player->PhysObj->body.y + camera->y, player->PhysObj->velocity.x, player->PhysObj->velocity.y, FE_DT);
        }
    }

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
                    if (keyboard_state[SDL_SCANCODE_ESCAPE] && event.key.repeat == 0) {
                        FE_GameState = GAME_STATE_PAUSE;
                        break;
                    }
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
                    
                    if (keyboard_state[SDL_SCANCODE_P] && event.key.repeat == 0)
                        positionlog = !positionlog;
                    
                    if (keyboard_state[SDL_SCANCODE_F] && event.key.repeat == 0) {
                        fullscreen = !fullscreen;
                        SDL_SetWindowFullscreen(window, fullscreen ? SDL_WINDOW_FULLSCREEN : 0);
                        SDL_GetWindowSize(window, &screen_width, &screen_height); // recalculate window size
                    }
                break;
            }
        }
    }

    // Handle essential inputs here to prevent first click issue
    if (!FE_ConsoleVisible) {
        if (keyboard_state[SDL_SCANCODE_A])
            FE_MovePlayer(player, camera, FE_NewVector(-player->movespeed, 0));
        if (keyboard_state[SDL_SCANCODE_D])
            FE_MovePlayer(player, camera, FE_NewVector(player->movespeed, 0));
        if (keyboard_state[SDL_SCANCODE_SPACE]) {
            if (!player->jump_started) {
                FE_StartPlayerJump(player);
            } else {
                FE_UpdatePlayerJump(player);
            }
        }
    }
}