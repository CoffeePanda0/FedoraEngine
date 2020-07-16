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

char buffer[20];
Uint32 frame_start;
int frame_time;

// CHANGE THESE IF YOU WANT
bool showfps = true;
bool vsync = false;

int main(int argc, char* argv[])
{
	Uint32 fps_lasttime = SDL_GetTicks();
	Uint32 fps_current; 
	Uint32 fps_frames = 0;

	log_init();

	init("FedoraEngine", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, screen_width, screen_height);
	
	info("Application started");
	
	while (GameActive) {
		frame_start = SDL_GetTicks();
		
		Render();
		Update();
		event_handler();

		if (showfps) {
			fps_frames++;
   			if (fps_lasttime < SDL_GetTicks() - 1000)
   			{
      			fps_lasttime = SDL_GetTicks();
      			fps_current = fps_frames;
      			fps_frames = 0;
				if (fps_current != 1)
					printf("FPS: %s \n", itoa(fps_current, buffer, 10));
   			}
		}
	}
	Clean();

	return 0;

}

