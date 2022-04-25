#include "include/game.h"

/* Contains functions for initializing and exiting the game */

SDL_Window* window;
SDL_Renderer* renderer;
bool FE_GameActive;

void FE_init(const char* window_title, int xpos, int ypos, int window_width, int window_height)
{
	if (SDL_Init(SDL_INIT_EVERYTHING) == 0)
	{	
		FE_ConsoleInit();

		// Check that everything has loaded in correctly
		window = SDL_CreateWindow(window_title, xpos, ypos, window_width, window_height, SDL_WINDOW_ALLOW_HIGHDPI);
		
		if (window) {
			info("Window Created");
		} else 
			error("Window could not be created! SDL_Error: %s", SDL_GetError());

		renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC);

		if (renderer)
			info("Renderer Created");
		else
			error("Renderer could not be created! SDL_Error: %s", SDL_GetError());

		if (IMG_Init(IMG_INIT_PNG) == 0)
    		error("IMG Failed to initialize. SDL_Error: %s", TTF_GetError());

		Sans = TTF_OpenFont("game/fonts/baloo.ttf", 20);
	 	if (!Sans)
		 	warn("Failed to load font baloo.ttf");

		FE_Menu_MainMenu();

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
		FE_CloseTextureManager();
		SDL_DestroyWindow(window);
		TTF_CloseFont(Sans);
		SDL_Quit();
		exit(0);
	}
}