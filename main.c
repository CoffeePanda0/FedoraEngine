//Main file to load other functions
#include "game.h"

/* TODO:
smooth movement
multiple inputs at once
multithreading
better audio
gravity on gameobjects?
everything lol
*/

// CHANGE THESE IF YOU WANT
bool vsync = true;
bool overlay = false;
bool paused;

Uint32 fps_current; 

int main(int argc, char* argv[])
{
	Uint32 fps_lasttime = SDL_GetTicks();
	Uint32 fps_frames = 0;
	
	log_init();

	init("FedoraEngine", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, screen_width, screen_height);
	
	info("Application started");


	while (GameActive) {
		if (!paused) {
			event_handler();
			Render();
			Update();

			fps_frames++;
			if (fps_lasttime < SDL_GetTicks() - 1000)
			{
				fps_lasttime = SDL_GetTicks();
				fps_current = fps_frames;
				fps_frames = 0;
			}

		} else {
			MenuEventHandle();
			RenderMenu();
		}
	}

	Clean();
	return 0;

}

