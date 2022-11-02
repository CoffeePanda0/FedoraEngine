#include <fedoraengine.h>
#include <utils.h>
#include <../lib/string.h>
#include "include/include.h"
#include "../ui/include/menu.h"

bool FE_Multiplayer_InitClient(char *addr, int port, char *username)
{
    PresentGame->GameState = GAME_STATE_CLIENT;
    
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
        free(_addr);
        return false;
    }
    free(_uname);
    free(_addr);
    return true;
}

void FE_RunClient()
{
    ClientEventHandle();
    ClientUpdate();
    ClientRender();
}
