#include <fedoraengine.h>
#include <utils.h>
#include <../lib/string.h>

#define ENET_IMPLEMENTATION
#include "../common/ext/enet.h"
#include "include/include.h"


void FE_Multiplayer_InitServer()
{
    PresentGame->GameState = GAME_STATE_SERVER;

    info("Initialising server");

    if (InitServer() < 0) {
        warn("Failed to initialsze server");
        exit(0);
    }
}

void FE_RunServer()
{
    UpdateServer();

    // apply delay so that server won't use 100% cpu
    static const float target = (1.0f / 60.0f) * 1000;
    int del = target - PresentGame->Timing.UpdateTime;
    if (del > 0)
        SDL_Delay(del);
}