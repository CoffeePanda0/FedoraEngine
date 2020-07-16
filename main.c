//Main file to load other functions
#include "game.h"
#include <string.h>
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
char buffer[20];
Uint32 frame_start;
int frame_time;

int main(int argc, char* argv[])
{
	Uint32 fps_lasttime = SDL_GetTicks(); //the last recorded time.
	Uint32 fps_current; //the current FPS.
	Uint32 fps_frames = 0; //frames passed since the last recorded fps.

	log_init();

	init("FedoraEngine", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, screen_width, screen_height, false);
	
	info("Application started\n");
	
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
			printf("FPS: %s \n", itoa(fps_current, buffer, 10));
   		}
		
		/*frame_start = SDL_GetTicks() - frame_start;

		if (frame_delay > frame_time)
			SDL_Delay(frame_delay - frame_time); */
	}
	Clean();

	return 0;

}

