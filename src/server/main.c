#include <fedoraengine.h>
#include <utils.h>
#include <systime.h>
#include <../lib/string.h>

#define ENET_IMPLEMENTATION
#include "../common/ext/enet.h"

#include "include/server.h"
#include "include/include.h"

void FE_Server_Init()
{
	PresentGame->config->Vsync = false; // disable Vsync so we can cap at constant 60hz/ups
	PresentGame->GameActive = true;
}

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
    float target = (1000 / server_config.tickrate);
    int del = target - PresentGame->Timing.UpdateTime;
    if (del > 0)
        FE_Delay(del);
}