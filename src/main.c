//Main file to load other functions
#include "game.h"

bool paused;

int screen_height = 512;
int screen_width = 512;
Uint64 now = 0;

bool multiplayer = false;

int main (int argc, char* argv[])
{
	Uint64 start_time = SDL_GetPerformanceCounter();
	log_init();
	
	init("FedoraEngine", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, screen_width, screen_height);
	info("Application started");

	GameActive = true;
	while (GameActive) {
   		now = (SDL_GetPerformanceCounter() - start_time) / 1000000; // Time in MS since game started

		if (!paused) {
			event_handler();
			Update();
			Render();	

		} else {
			MenuEventHandle();
			RenderMenu();
		}
	}
	Clean();
	return 0;

}

