#include "include/game.h"

/* Contains functions for initializing and exiting the game */

int screen_height;
int screen_width;

SDL_Window* window;
SDL_Renderer* renderer;
bool FE_GameActive;

void FE_Init(FE_InitConfig InitConfig)
{
	FE_Log_Init();

	if (SDL_Init(SDL_INIT_EVERYTHING) == 0)
	{	
		FE_ConsoleInit();
		FE_ResourceManager_Init();
		
		// Check that everything has loaded in correctly
		window = SDL_CreateWindow(
			InitConfig.window_title,
			SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
			InitConfig.window_width, InitConfig.window_height,
			SDL_WINDOW_ALLOW_HIGHDPI
		);
		
		screen_height = InitConfig.window_height;
		screen_width = InitConfig.window_width;

		if (window) {
			info("Window Created");
		} else 
			error("Window could not be created! SDL_Error: %s", SDL_GetError());
		
		if (InitConfig.vsync)
			renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC);
		else
			renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

		if (renderer)
			info("Renderer Created");
		else
			error("Renderer could not be created! SDL_Error: %s", SDL_GetError());

		if (IMG_Init(IMG_INIT_PNG) == 0)
    		error("IMG Failed to initialize. SDL_Error: %s", TTF_GetError());

		Sans = TTF_OpenFont("game/fonts/baloo.ttf", 20);
	 	if (!Sans)
		 	warn("Failed to load font baloo.ttf");

		info("FedoraEngine started successfully");
		FE_GameActive = true;

	} else {
		error("Could not start SDL. SDL_Error: %s", SDL_GetError());
		FE_Clean();
	}
}

void FE_CleanAll() // Cleans all resources possible without exiting
{
	FE_CloseMap();
	FE_FreeUI();
	FE_FreeDialogue();
	FE_CleanEditor();
	FE_CleanGameObjects();
	FE_DestroyMessageBox();
	FE_CleanAudio();
	FE_CleanTimers();
	FE_CleanLightObjects();
	FE_CleanAnimations();
	FE_CleanParticles();
}

void FE_Clean() // Exits the game cleanly, freeing all resources
{
	if (FE_GameActive) {
		info("Exiting FedoraEngine \n");
		log_close();
		
		FE_GameActive = false;
		IMG_Quit();
		FE_CleanAll();
		FE_DestroyConsole();
		SDL_DestroyWindow(window);
		TTF_CloseFont(Sans);
		SDL_Quit();
		exit(0);
	}
}