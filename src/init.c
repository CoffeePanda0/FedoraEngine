#include "include/game.h"
#include <SDL_image.h>

/* Contains functions for initializing and exiting the game */

bool FE_GameInitialised;

FE_InitConfig *FE_NewInitConfig()
{
	/* Fills default values */
	FE_InitConfig *config = xmalloc(sizeof(FE_InitConfig));
	config->window_title = "FedoraEngine";
	config->WindowWidth = 1280;
	config->WindowHeight = 720;
	config->vsync = true;
	config->show_fps = true;
	config->default_font = "OpenSans";
	return config;
}

static FE_Game *NewGame(FE_InitConfig *ic)
{
	if (!ic)
		error("Could not start FedoraEngine - NULL config");

	/* Sets default values */
	FE_Game *Game = xmalloc(sizeof(FE_Game));
	*Game = (FE_Game){
		.config = ic,
		.FullScreen = false,
		.font = NULL,
		.ConsoleVisible = false,
		.StartedInput = false,
		.DialogueSpeed = 85,
		.Window = NULL,
		.Renderer = NULL,
		.GameActive = false,
		.GameState = GAME_STATE_MENU,
		.MapConfig = {false, ic->WindowWidth, ic->WindowHeight, 0, VEC_EMPTY, 0.0f, 50},
		.AudioConfig = {50, false},
		.Timing = {0,0,0},
		.DebugConfig = {false, true, false},
		.UIConfig = {0, 0, COLOR_WHITE, false, false, false}
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
		PresentGame->Window = SDL_CreateWindow(
			InitConfig->window_title,
			SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
			InitConfig->WindowWidth, InitConfig->WindowHeight,
			SDL_WINDOW_ALLOW_HIGHDPI
		);
		
		PresentGame->WindowHeight = InitConfig->WindowHeight;
		PresentGame->WindowWidth = InitConfig->WindowWidth;

		if (PresentGame->Window) {
			info("Window Created");
		} else 
			error("Window could not be created! SDL_Error: %s", SDL_GetError());
		
		if (InitConfig->vsync)
			PresentGame->Renderer = SDL_CreateRenderer(PresentGame->Window, -1, SDL_RENDERER_PRESENTVSYNC);
		else
			PresentGame->Renderer = SDL_CreateRenderer(PresentGame->Window, -1, SDL_RENDERER_ACCELERATED);
		

		if (PresentGame->Renderer)
			info("Renderer Created");
		else
			error("Renderer could not be created! SDL_Error: %s", SDL_GetError());

		if (!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0"))
			warn("Nearest texture filtering not enabled! SDL_Error: %s", SDL_GetError());

		if (!SDL_SetHint(SDL_HINT_RENDER_BATCHING, "1"))
			warn("Texture batching could not be enabled! SDL_Error: %s", SDL_GetError());

		if (IMG_Init(IMG_INIT_PNG) == 0)
    		error("IMG Failed to initialize. SDL_Error: %s", IMG_GetError());

		if (TTF_Init() != 0)
			error("TTF Failed to initialize. SDL_Error: %s", TTF_GetError());

		PresentGame->font = FE_LoadFont(InitConfig->default_font, 24);
		
		FE_Console_Init();
		FE_UI_InitUI();

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
	FE_Trigger_Clean();
	FE_CleanLighting();
	FE_Parallax_Clean();
	FE_UI_ClearElements(PresentGame->UIConfig.ActiveElements);
	FE_Dialogue_Free();
	FE_CleanEditor();
	FE_CleanCameras();
	FE_CleanGameObjects();
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
		free(PresentGame->UIConfig.ActiveElements);
		FE_Console_Destroy();
		SDL_DestroyRenderer(PresentGame->Renderer);
		SDL_DestroyWindow(PresentGame->Window);
		FE_CleanFonts();
		free(PresentGame->config);
		free(PresentGame);
	}
	FE_GameInitialised = false;
	SDL_Quit();
	exit(0);

}