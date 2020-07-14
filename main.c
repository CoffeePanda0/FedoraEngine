//Main file to load other functions
#include "game.h"

/* TODO:
game objects
fps counter
smooth movement
multiple inputs at once
multithreading
audio
everything lol
*/

const int maxFPS = 60;
const int frame_delay = 1000 / maxFPS;

Uint32 frame_start;
int frame_time;

int main(int argc, char* argv[])
{
	log_init();

	init("FedoraEngine", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, screen_width, screen_height, false);
	
	info("Application started\n");
	
	while (GameActive) {
		frame_start = SDL_GetTicks();
		
		Render();
		Update();
		event_handler();

		frame_start = SDL_GetTicks() - frame_start;

		if (frame_delay > frame_time)
			SDL_Delay(frame_delay - frame_time);
	}
	Clean();

	return 0;

}

