/*
	*********************************************
	FedoraEngine - credit CoffeePanda
	https://github.com/CoffeePanda0/FedoraEngine
	BSD3 license
	*********************************************

	FedoraEngine.c - Primary Game file
*/
#include "include/game.h"

static void LoadArgs(int argc, char *argv[])
{
	if (argc > 1) {
		if (strcmp(argv[1], "--editor") == 0 || strcmp(argv[1], "-e") == 0) {
			/* If command --editor or -e, skip the menu and load directly into editor */
			FE_GameState = GAME_STATE_EDITOR;
			FE_StartEditor();
		} else if (strcmp(argv[1], "--map") == 0 || strcmp(argv[1], "-m") == 0) {
			/* If command --map or -m, skip the menu and load directly into given map */
			if (argc > 2)
				FE_StartGame(argv[2]);
			else
				error("No map specified");
		} else {
			FE_Menu_MainMenu();
		}
	} else {
		/* If no launch args specified, load into default main menu */
		FE_Menu_MainMenu();
	}
}

int main(int argc, char* argv[])
{
	/* Initialise FedoraEngine systems first */
	FE_InitConfig i = (FE_InitConfig){
		"FedoraEngine",
		512, 512,
		true
	};
	FE_Init(i);	

	/* Check launch args */
	LoadArgs(argc, argv);
	
	while (FE_GameActive) {
		// main game loop - calls functions based on game state
		switch (FE_GameState) {
			case GAME_STATE_MENU:
				FE_RenderMenu();
				FE_MenuEventHandle();
			break;
			case GAME_STATE_PLAY:
				FE_GameLoop();
			break;
			case GAME_STATE_PAUSE:
				FE_GameLoop();
			break;
			case GAME_STATE_EDITOR:
				FE_RenderEditor();
				FE_EventEditorHandler();
			break;

		}
		FE_CalculateDT();
	}
	
	FE_Clean();
	return 0;

}
