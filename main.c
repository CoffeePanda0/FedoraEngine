//Main file to load other functions
#include "game.h"

bool paused;

int main(int argc, char* argv[])
{
	log_init();

	init("FedoraEngine", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, screen_width, screen_height);
	
	info("Application started");

	while (GameActive) {
		if (!paused) {
			event_handler();
			Render();
			Update();

		} else {
			MenuEventHandle();
			RenderMenu();
		}
	}
	Clean();
	return 0;

}

