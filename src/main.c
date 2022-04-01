//Main file to load other functions
#include "include/game.h"

int screen_height = 512;
int screen_width = 512;
float dT = 0;

int main (int argc, char* argv[])
{
	(void)argc; (void)argv;
	log_init();
	
	FE_init("FedoraEngine", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, screen_width, screen_height);
	info("Application started");

	Uint32 lastupdate = SDL_GetTicks();
	FE_GameState = GAME_STATE_MENU;

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

