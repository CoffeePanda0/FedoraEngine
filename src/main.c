//Main file to load other functions
#include "include/game.h"

int screen_height = 512;
int screen_width = 512;
float dT = 0;

int main (int argc, char* argv[])
{
	log_init();
	
	FE_init("FedoraEngine", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, screen_width, screen_height);
	info("Application started");

	Uint32 lastupdate = SDL_GetTicks();
	FE_GameState = GAME_STATE_MENU;

	// allow loading maps or starting editor from arguments
	if (argc > 1) {
		if (strcmp(argv[1], "--editor") == 0 || strcmp(argv[1], "-e") == 0) {
			FE_GameState = GAME_STATE_EDITOR;
			FE_StartEditor();
		} else if (strcmp(argv[1], "--map") == 0 || strcmp(argv[1], "-m") == 0) {
			if (argc > 2)
				FE_StartGame(argv[2]);
			else
				error("No map specified");
		}
	}

	while (FE_GameActive) {
		Uint32 current = SDL_GetTicks();
		dT = (current - lastupdate) / 1000.0f;


		switch (FE_GameState) {
			case GAME_STATE_MENU:
				FE_RenderMenu();
				FE_MenuEventHandle();
			break;
			case GAME_STATE_PLAY:
				FE_GameLoop();
			break;
			case GAME_STATE_EDITOR:
				FE_RenderEditor();
				FE_EventEditorHandler();
			break;
			case GAME_STATE_PAUSE:
			break;
		}
		lastupdate = current;
	}
	
	FE_Clean();
	return 0;

}

