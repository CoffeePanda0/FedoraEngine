#include <FE_Client.h>
#include <SDL_image.h>

void FE_Client_Init(FE_InitConfig *InitConfig)
{
	/* Create a new client struct */
	PresentGame->Client = xmalloc(sizeof(FE_Client));
	PresentGame->Client->FullScreen = false;
	PresentGame->Client->Font = 0;
	PresentGame->Client->ConsoleVisible = false;
	PresentGame->Client->DialogueSpeed = 85;
	PresentGame->Client->StartedInput = false;

	PresentGame->UIConfig = xmalloc(sizeof(FE_UIConfig));
	PresentGame->UIConfig->ActiveElements = 0;
	PresentGame->UIConfig->UIFont = 0;
	PresentGame->UIConfig->UIFontColor= (SDL_Color){255, 255, 255, 255};
	PresentGame->UIConfig->InText = false;
	PresentGame->UIConfig->DialogueActive = false;
	PresentGame->UIConfig->MBShown = 0;

	if (SDL_Init(SDL_INIT_EVERYTHING) == 0)
	{
		// Check that everything has loaded in correctly
		PresentGame->Client->Window = SDL_CreateWindow(
			InitConfig->WindowTitle,
			SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
			InitConfig->WindowWidth, InitConfig->WindowHeight,
			SDL_WINDOW_ALLOW_HIGHDPI
		);
		
		PresentGame->WindowHeight = InitConfig->WindowHeight;
		PresentGame->WindowWidth = InitConfig->WindowWidth;

		if (PresentGame->Client->Window)
			info("Window Created");
		else 
			error("Window could not be created! SDL_Error: %s", SDL_GetError());
		
		if (InitConfig->Vsync)
			PresentGame->Client->Renderer = SDL_CreateRenderer(PresentGame->Client->Window, -1, SDL_RENDERER_PRESENTVSYNC);
		else
			PresentGame->Client->Renderer = SDL_CreateRenderer(PresentGame->Client->Window, -1, SDL_RENDERER_ACCELERATED);
		

		if (PresentGame->Client->Renderer)
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

		FE_ResourceManager_Init();

		PresentGame->Client->Font = FE_Font_Load(InitConfig->default_font, 24);
		
		FE_Console_Init();
		FE_UI_InitUI();
		FE_Key_Init();

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
	FE_Map_CloseLoaded();
	FE_Prefab_Clean();
	FE_Trigger_Clean();
	FE_Light_Clean();
	FE_Parallax_Clean();
	FE_UI_ClearElements(PresentGame->UIConfig->ActiveElements);
	FE_Dialogue_Free();
	FE_Editor_Destroy();
	FE_CleanCameras();
	FE_GameObject_Clean();
	FE_CleanAudio();
	FE_Timers_Clean();
	FE_Animations_Clean();
	FE_Particles_Clean();
}

void FE_Clean() // Exits the game cleanly, freeing all resources
{
	info("Exiting FedoraEngine \n");
	log_close();

	if (FE_GameInitialised) {
		PresentGame->GameActive = false;
		IMG_Quit();
		FE_CleanAll();
		FE_Key_Clean();
		free(PresentGame->UIConfig->ActiveElements);
		FE_Console_Destroy();
		SDL_DestroyRenderer(PresentGame->Client->Renderer);
		SDL_DestroyWindow(PresentGame->Client->Window);
		FE_Font_Clean();
		free(PresentGame->config);
		free(PresentGame->Client);
		free(PresentGame->UIConfig);
		free(PresentGame);
	}
	FE_GameInitialised = false;
	SDL_Quit();
	exit(0);

}