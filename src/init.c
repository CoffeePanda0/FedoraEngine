#include "include/game.h"
#include <SDL_image.h>

/* Contains functions for initializing and exiting the game */

bool FE_GameInitialised;

FE_InitConfig *FE_NewInitConfig()
{
	/* Fills default values */
	FE_InitConfig *config = xmalloc(sizeof(FE_InitConfig));
	config->window_title = "FedoraEngine";
	config->window_width = 1920;
	config->window_height = 1080;
	config->vsync = true;
	config->show_fps = true;
	config->default_font = "OpenSans";
	return config;
}


static FE_Game *NewGame(FE_InitConfig *ic)
{
	/* Sets default values */
	FE_Game *Game = xmalloc(sizeof(FE_Game));
	*Game = (FE_Game){
		0, ic,
		0, 0,
		false, false, false, false, false,
		0, 0,
		false,
		GAME_STATE_MENU,
		(FE_MapConfig){0, 0, 0, VEC_EMPTY, 0.0f, 150},
		(FE_AudioConfig){50, false}
	};
	return Game;
}

void FE_Init(FE_InitConfig *InitConfig)
{
	if (!InitConfig) {
		printf("FE_Init: InitConfig is null\n");
		exit(-1);
	}

	if (FE_GameInitialised) {
		printf("FE_Init: Game already initialised\n");
		return;
	}

	PresentGame = NewGame(InitConfig);

	FE_Log_Init();

	if (SDL_Init(SDL_INIT_EVERYTHING) == 0)
	{	
		FE_ResourceManager_Init();
		
		// Check that everything has loaded in correctly
		PresentGame->window = SDL_CreateWindow(
			InitConfig->window_title,
			SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
			InitConfig->window_width, InitConfig->window_height,
			SDL_WINDOW_ALLOW_HIGHDPI
		);
		
		PresentGame->window_height = InitConfig->window_height;
		PresentGame->window_width = InitConfig->window_width;

		if (PresentGame->window) {
			info("Window Created");
		} else 
			error("Window could not be created! SDL_Error: %s", SDL_GetError());
		
		if (InitConfig->vsync)
			PresentGame->renderer = SDL_CreateRenderer(PresentGame->window, -1, SDL_RENDERER_PRESENTVSYNC);
		else
			PresentGame->renderer = SDL_CreateRenderer(PresentGame->window, -1, SDL_RENDERER_ACCELERATED);
		

		if (PresentGame->renderer)
			info("Renderer Created");
		else
			error("Renderer could not be created! SDL_Error: %s", SDL_GetError());

		if (!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0"))
			warn("Warning: Nearest texture filtering not enabled! SDL_Error: %s", SDL_GetError());

		if (IMG_Init(IMG_INIT_PNG) == 0)
    		error("IMG Failed to initialize. SDL_Error: %s", IMG_GetError());

		if (TTF_Init() != 0)
			error("TTF Failed to initialize. SDL_Error: %s", TTF_GetError());

		PresentGame->font = FE_LoadFont(InitConfig->default_font, 24);
		
		FE_ConsoleInit();
		FE_InitLighting();

		info("FedoraEngine started successfully");

		FE_GameInitialised = true;
		PresentGame->GameActive = true;

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
	FE_CleanCameras();
	FE_CleanGameObjects();
	FE_DestroyMessageBox();
	FE_CleanAudio();
	FE_CleanTimers();
	FE_CleanAnimations();
	FE_CleanParticles();
}

void FE_Clean() // Exits the game cleanly, freeing all resources
{
	info("Exiting FedoraEngine \n");
	log_close();

	if (FE_GameInitialised) {
		PresentGame->GameActive = false;
		IMG_Quit();
		FE_CleanAll();
		FE_DestroyConsole();
		FE_CleanLighting();
		SDL_DestroyRenderer(PresentGame->renderer);
		SDL_DestroyWindow(PresentGame->window);
		FE_CleanFonts();
		free(PresentGame->config);
		free(PresentGame);
	}
	FE_GameInitialised = false;
	SDL_Quit();
	exit(0);

}