//Main file to load other functions
#include "game.h"

/* TODO:
game objects and text objects memory management
smooth movement
multiple inputs at once
multithreading
audio
everything lol
*/

// CHANGE THESE IF YOU WANT
bool vsync = true;
bool overlay = true;

Uint32 fps_current; 

int main(int argc, char* argv[])
{
	Uint32 fps_lasttime = SDL_GetTicks();
	
	Uint32 fps_frames = 0;
	char buffer[20];
	Uint32 frame_start;

	log_init();

	init("FedoraEngine", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, screen_width, screen_height);
	
	info("Application started");

	while (GameActive) {
		frame_start = SDL_GetTicks();
		Render();
		Update();
		event_handler();

		fps_frames++;
   		if (fps_lasttime < SDL_GetTicks() - 1000)
   		{
      		fps_lasttime = SDL_GetTicks();
      		fps_current = fps_frames;
      		fps_frames = 0;
		}
	}
	Clean();
	return 0;

}