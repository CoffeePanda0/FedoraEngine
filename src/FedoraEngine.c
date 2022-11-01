/*
	*********************************************
	FedoraEngine - credit CoffeePanda
	https://github.com/CoffeePanda0/FedoraEngine
	BSD3 license
	*********************************************

	FedoraEngine.c - Primary Game file
*/
#include <FE_Client.h>
#include "client/ui/include/menu.h"
#include "common/net/include/net.h"
#include "server/include/include.h"

FE_Game *PresentGame;

static bool ArgExists(int argc, char *argv[], char *arg, char *s_arg)
{
	if (argc > 1) {
		/* create strings for the args with delim */
		return (mstrcmp(argv[1], arg) == 0 || mstrcmp(argv[1], s_arg) == 0);
	}
	return false;
}

int main(int argc, char* argv[])
{
	FE_InitConfig *IC = FE_NewInitConfig();
	IC->Vsync = true;

	/* Initialise FedoraEngine subsystems */
	FE_Init(IC);

	/* Initialises FedoraEngine to run as either a client or a server */
	if (ArgExists(argc, argv, "--server", "-s")) {
		IC->Headless = true;
		FE_Server_Init(IC);
		FE_Multiplayer_InitServer();
	} else {
		FE_Client_Init(IC);

		// Check if map is already specified
		if (ArgExists(argc, argv, "--map", "-m") && argc == 3)
			FE_StartGame(argv[2]);
		else
			FE_Menu_LoadMenu("Main");
	}

		
	/* main game loop - calls functions based on game state */
	while (PresentGame->GameActive) {
		FE_CalculateDT();

		switch (PresentGame->GameState) {
			case GAME_STATE_MENU:
				FE_Menu_Render();
				FE_Menu_EventHandler();
			break;
			case GAME_STATE_PLAY:
				FE_GameLoop();
			break;
			case GAME_STATE_PAUSE:
				FE_Pause_Render();
				FE_Menu_EventHandler();
			break;
			case GAME_STATE_EDITOR:
				FE_Editor_Render();
				FE_Editor_EventHandler();
			break;
			case GAME_STATE_CLIENT:
				FE_RunClient();
			break;
			case GAME_STATE_SERVER:
				FE_RunServer();
			break;
		}
	}
	
	FE_Clean();

	return 0;

}
