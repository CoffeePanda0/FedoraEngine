/*
	*********************************************
	FedoraEngine - credit CoffeePanda
	https://github.com/CoffeePanda0/FedoraEngine
	BSD3 license
	*********************************************

	FedoraEngine.c - Primary Game file
*/

#include <FE_Common.h>
#include "common/net/include/net.h"
#include "server/include/include.h"

/* Uses ifdefs to decide which int main() should be used (if we are compiling server only or not) */

/******************************************************/

/* Uncomment this to compile server only */
// #define SERVER_ONLY 1

/*****************************************************/

#ifdef __has_include
    #if __has_include(<SDL.h>)
	#else
		// If SDL is not installed, automatically assume server install
		#ifndef SERVER_ONLY
			#define SERVER_ONLY 1
		#endif
    #endif
#endif

#ifndef SERVER_ONLY
	#include <FE_Client.h>
	#include "client/ui/include/menu.h"
#endif

FE_Game *PresentGame;


#ifdef SERVER_ONLY
int main(int argc, char* arv[])
{
	(void)argc, (void)arv;

	FE_InitConfig *IC = FE_NewInitConfig();

	/* Initialise FedoraEngine subsystems */
	FE_Init(IC);

	/* Initialise the server */
	FE_Server_Init();
	FE_Multiplayer_InitServer();

	while (PresentGame->GameActive) {
		FE_CalculateDT();
		FE_RunServer();
	}

	return 0;
}

#else

int main(int argc, char* argv[])
{
	FE_InitConfig *IC = FE_NewInitConfig();

	/* Initialise FedoraEngine subsystems */
	FE_Init(IC);

	/* Initialises FedoraEngine to run as either a client or a server */
	if (FE_ArgExists(argc, argv, "--server", "-s")) {
		IC->Headless = true;
		FE_Server_Init();
		FE_Multiplayer_InitServer();
	} else {
		FE_Client_Init(IC);

		// Check if map is already specified
		if (FE_ArgExists(argc, argv, "--map", "-m") && argc == 3)
			FE_StartGame(argv[2]);
		else
			FE_Menu_LoadMenu("Main");
	}

	
	/* main game loop - calls functions based on game state */
	while (PresentGame->GameActive) {
		FE_CalculateDT();
		FE_CalculateFPS();
		
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

#endif