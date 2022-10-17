/*
	*********************************************
	FedoraEngine - credit CoffeePanda
	https://github.com/CoffeePanda0/FedoraEngine
	BSD3 license
	*********************************************

	FedoraEngine.c - Primary Game file
*/
#include <FE_Common.h>
#include "client/ui/include/menu.h"
#include "include/game.h"
#include "common/net/include/net.h"

FE_Game *PresentGame;

static void LoadArgs(int argc, char *argv[], FE_InitConfig *IC)
{
	if (argc > 1) {
		if (mstrcmp(argv[1], "--server") == 0 || mstrcmp(argv[1], "-s") == 0) {
			/* Load headless FedoraEngine */
			IC->headless = true;
		}
	}
}

int main(int argc, char* argv[])
{
	FE_InitConfig *IC = FE_NewInitConfig();
	IC->vsync = true;

	/* Check launch args */
	LoadArgs(argc, argv, IC);

	/* Initialise FedoraEngine systems */
	FE_Init(IC);

	if (!IC->headless)
		FE_Menu_LoadMenu("Main");
	else
		FE_Multiplayer_InitServer();
		
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
