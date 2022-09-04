#include "../include/game.h"
#include "include/internal.h"

/* todo:
    sending chats
    sending config and map and other connected clients
    rcon
    pause menu
    configure player animations / actual sprites
*/


typedef enum  {
    MODE_SERVER,
    MODE_CLIENT,
    MODE_SERVER_HEADLESS,
} MULT_MODE;

MULT_MODE mode = MODE_SERVER;

bool FE_Multiplayer_InitServer(int port)
{
    mode = MODE_SERVER;
    PresentGame->GameState = GAME_STATE_MULTIPLAYER;
    FE_CleanAll();

    info("Initialising server on port %d", port);
    
    if (InitServer(port) < 0) {
        warn("Failed to initialize server");
        FE_Menu_LoadMenu("Main");
        return false;
    }
    FE_UI_AddElement(FE_UI_LABEL, FE_UI_CreateLabel(0, "FedoraEngine Server", 300, midx(256), 50, COLOR_WHITE));
    return true;
}

void FE_Multiplayer_InitHeadless(int port)
{
    mode = MODE_SERVER_HEADLESS;
    PresentGame->GameState = GAME_STATE_MULTIPLAYER;
    FE_CleanAll();

    info("Initialising server on port %d", port);

    if (InitServer(port) < 0) {
        warn("Failed to initialize server");
        exit(0);
    }
}

bool FE_Multiplayer_InitClient(char *addr, int port, char *username)
{
    mode = MODE_CLIENT;
    PresentGame->GameState = GAME_STATE_MULTIPLAYER;
    
    // restore vars as cleanall will reset them
    char *_addr = mstrdup(addr);
    char *_uname = mstrdup(username);
    int _port = port;
    
    FE_CleanAll();

    info("Attempting connection to %s:%i with username: %s", _addr, _port, _uname);

    if (InitClient(_addr, _port, _uname) < 0) {
        warn("Failed to initialise client");
        FE_Menu_LoadMenu("Main");
        free(_uname);
        return false;
    }
    free(_uname);
    return true;
}

void FE_Multiplayer_Update()
{
    if (mode == MODE_SERVER || mode == MODE_SERVER_HEADLESS) {
        UpdateServer();
    } else if (mode == MODE_CLIENT) {
       ClientUpdate();
    }
}

void FE_Multiplayer_Render()
{
    if (mode == MODE_SERVER) {
        SDL_RenderClear(PresentGame->Renderer);
        FE_UI_Render();
        SDL_RenderPresent(PresentGame->Renderer);
    }
    
    if (mode == MODE_SERVER_HEADLESS || mode == MODE_SERVER) {
        const float target = (1.0f / 60.0f) * 1000;
        int del = target - PresentGame->Timing.UpdateTime;
        if (del > 0)
            SDL_Delay(del);
    }

    else if (mode == MODE_CLIENT) {
        ClientRender();
    }

}

void FE_Multiplayer_EventHandle()
{
    
    if (mode == MODE_SERVER_HEADLESS) return;

    if (mode == MODE_SERVER) {
        SDL_PumpEvents();
        SDL_Event event;
        const Uint8* keyboard_state = SDL_GetKeyboardState(NULL);

        while (SDL_PollEvent(&event)) {
            if (FE_UI_HandleEvent(&event, keyboard_state))
                break;
        }
    } else if (mode == MODE_CLIENT) {
        ClientEventHandle();
    }
}