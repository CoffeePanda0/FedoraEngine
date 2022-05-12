#include <SDL.h>
#include "include/fedoraengine.h"

float FE_DT = 0; // Time taken for each loop
const float FE_DT_TARGET = 1.0f / 60;
float FE_DT_MULTIPLIER = 1.0f; // The value that we need to multiply by to reach target FPS logic
uint32_t FE_FPS = 60;

float FE_GAME_RUNNING = 0; // How many seconds the game has been running for

static void FE_FPSCounter()
{
    // only update every second
    static float fps_timer = 0;
    fps_timer += FE_DT;
    if (fps_timer > 1) {
        fps_timer = 0;
        char title[64];    
        snprintf(title, 64, "%s - FPS: %i", PresentGame->config->window_title, FE_FPS);
        SDL_SetWindowTitle(PresentGame->window, title);
    }
}

int FE_CalculateDT()
{
    static uint64_t current_time = 0;
	static uint64_t last_time = 0;
	
	last_time = current_time;
	current_time = SDL_GetPerformanceCounter();
	FE_DT = (float)((current_time - last_time) / (float)SDL_GetPerformanceFrequency());

	// First frame will always produce invalid delta time, so disregard it
	FE_DT = (last_time == 0 ? 0 : FE_DT);

	FE_FPS = (1 / FE_DT);
	FE_DT_MULTIPLIER = (FE_DT / FE_DT_TARGET);

    if (PresentGame->config->show_fps)
	    FE_FPSCounter();
	
    return 1;
}