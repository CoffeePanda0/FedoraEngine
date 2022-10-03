/*
	*********************************************
	FedoraEngine - credit CoffeePanda
	https://github.com/CoffeePanda0/FedoraEngine
	BSD3 license
	*********************************************

	FedoraEngine.c - Primary Game file
*/
#include "include/game.h"
#include "web/net.h"

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
	PresentGame->DebugConfig.NoClip = true;
		
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
			case GAME_STATE_MULTIPLAYER:
				FE_Multiplayer_Update();
				FE_Multiplayer_Render();
				FE_Multiplayer_EventHandle();
			break;

		}
	}
	
	FE_Clean();

	return 0;

}
