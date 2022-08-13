/*
	*********************************************
	FedoraEngine - credit CoffeePanda
	https://github.com/CoffeePanda0/FedoraEngine
	BSD3 license
	*********************************************

	FedoraEngine.c - Primary Game file
*/
#include "include/game.h"

FE_Game *PresentGame;

static void LoadArgs(int argc, char *argv[])
{
	if (argc > 1) {
		if (mstrcmp(argv[1], "--editor") == 0 || mstrcmp(argv[1], "-e") == 0) {
			/* If command --editor or -e, skip the menu and load directly into editor */
			PresentGame->GameState = GAME_STATE_EDITOR;
			FE_StartEditor(NULL);
		} else if (mstrcmp(argv[1], "--map") == 0 || mstrcmp(argv[1], "-m") == 0) {
			/* If command --map or -m, skip the menu and load directly into given map */
			if (argc > 2)
				FE_StartGame(argv[2]);
			else
				error("No map specified");
		} else {
			FE_Menu_LoadMenu("Main");
		}
	} else FE_Menu_LoadMenu("Main");
}

int main(int argc, char* argv[])
{
	/* Initialise FedoraEngine systems first */
	
	FE_InitConfig *IC = FE_NewInitConfig();
	IC->vsync = false;
	FE_Init(IC);

	/* Check launch args */
	LoadArgs(argc, argv);
		
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
				FE_RenderEditor();
				FE_EventEditorHandler();
			break;

		}
	}
	
	FE_Clean();

	return 0;

}
