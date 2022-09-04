#include "../include/game.h"
#include "include/internal.h"

/* todo:
    sending chats
    sending config and map and other connected clients
    rcon
    pause menu
    configure player animations / actual sprites
    handle DISC_NOCON
*/


typedef enum  {
    MODE_SERVER,
    MODE_CLIENT,
} MULT_MODE;

MULT_MODE mode = MODE_SERVER;

void FE_Multiplayer_InitServer()
{
    mode = MODE_SERVER;
    PresentGame->GameState = GAME_STATE_MULTIPLAYER;

    info("Initialising server");

    if (InitServer() < 0) {
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
    if (mode == MODE_SERVER) {
        UpdateServer();
    } else if (mode == MODE_CLIENT) {
       ClientUpdate();
    }
}

void FE_Multiplayer_Render()
{
    if (mode == MODE_SERVER) {
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
    if (mode == MODE_CLIENT) {
        ClientEventHandle();
    }
}