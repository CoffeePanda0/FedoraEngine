#include <FE_Common.h>

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
		.Client = 0,
		.GameActive = false,
		.GameState = GAME_STATE_MENU,
		.DisconnectInfo = {false, 0, 0},
		.MapConfig = {false, ic->WindowWidth, ic->WindowHeight, 0, VEC_EMPTY, 0.0f, 20},
		.AudioConfig = {50, false},
		.Timing = {0,0,0},
		.DebugConfig = {false, true, false, false},
		.UIConfig = 0,
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

	info("Initialised FedoraEngine subsystems");
}
