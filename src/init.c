#include <FE_Client.h>
#include <SDL_image.h>

/* Contains functions for initializing and exiting the game */

bool FE_GameInitialised;

FE_InitConfig *FE_NewInitConfig()
{
	/* Fills default values */
	FE_InitConfig *config = xmalloc(sizeof(FE_InitConfig));
	config->WindowTitle = "FedoraEngine";
	config->WindowWidth = 1280;
	config->WindowHeight = 720;
	config->Headless = false;
	config->Vsync = true;
	config->ShowFPS = true;
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
		.DisconnectInfo = {false, 0, 0},
		.MapConfig = {false, ic->WindowWidth, ic->WindowHeight, 0, VEC_EMPTY, 0.0f, 20},
		.AudioConfig = {50, false},
		.Timing = {0,0,0},
		.DebugConfig = {false, true, false, false},
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
	FE_Time_Init();
	FE_Log_Init();
	FE_ResourceManager_Init();

	info("Initialised FedoraEngine subsystems");
}

void FE_CleanAll() // Cleans all resources possible without exiting
{
	FE_Map_CloseLoaded();
	FE_Prefab_Clean();
	FE_Trigger_Clean();
	FE_Light_Clean();
	FE_Parallax_Clean();
	FE_UI_ClearElements(PresentGame->UIConfig.ActiveElements);
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
		free(PresentGame->UIConfig.ActiveElements);
		FE_Console_Destroy();
		SDL_DestroyRenderer(PresentGame->Renderer);
		SDL_DestroyWindow(PresentGame->Window);
		FE_Font_Clean();
		free(PresentGame->config);
		free(PresentGame);
	}
	FE_GameInitialised = false;
	SDL_Quit();
	exit(0);

}