/* Core game loops and functions for game operation */
#include <FE_Client.h>
#include "ui/include/menu.h"

static FE_Camera *GameCamera;
static FE_Player *GamePlayer;

static FE_ParticleSystem *SnowParticles;
SDL_Texture *world;

/* Starts the main game */
void FE_StartGame(const char *mapname)
{
	char *map = mstrdup(mapname);
	FE_CleanAll();

	/* Map setup */
	FE_LoadedMap *m = 0;
	if (!(m = FE_Map_Load(map))) {
		warn("Failed to start game");
		free(map);
		FE_Menu_LoadMenu("Main");
		return;
	} else {
		FE_Game_SetMap(m);
	}
	free(map);

	// camera setup
	GameCamera = FE_CreateCamera();
	GameCamera->maxzoom = 2.0f;
	GameCamera->minzoom = 1.0f;

	// player setup
	GamePlayer = FE_Player_Create(30, 50, 180, (SDL_Rect){PresentGame->MapConfig.PlayerSpawn.x, PresentGame->MapConfig.PlayerSpawn.y - 200, 120, 100});
	GameCamera->follow = &GamePlayer->render_rect;

	// test particle system
	SnowParticles = FE_ParticleSystem_Create(
		(SDL_Rect){0, -20, PresentGame->MapConfig.MapWidth, 20}, // Position for the whole screen, slightly above the top to create more random
		350, // Emission rate
		3000, // Max particles
		10000, // Max lifetime
		true, // Particles to respawn once they go off screen
		"rain.png", // Texture
		(vec2){15, 15}, // Max size of each particle
		(vec2){3, 3}, // Set initial velocity so particle doesn't float until they accelerate
		false
	);

	world = FE_CreateRenderTexture(PresentGame->WindowWidth, PresentGame->WindowHeight);
	FE_ResetDT();

	FE_GameObject_Create((SDL_Rect){1000, -600, 160, 160}, "cube.png", 50);
	FE_GameObject_Create((SDL_Rect){1000, -300, 160, 160}, "cube.png", 50);

	PresentGame->GameState = GAME_STATE_PLAY;
}


void FE_RenderGame()
{
	if (PresentGame->DebugConfig.LightingEnabled)
		SDL_SetRenderTarget(PresentGame->Client->Renderer, world);
	SDL_RenderClear(PresentGame->Client->Renderer);
	FE_Map_RenderBackground(GameCamera);
	FE_Particles_Render(GameCamera);
	FE_Map_RenderLoaded(GameCamera);
	FE_Trigger_Render(GameCamera);
	FE_GameObject_Render(GameCamera);
	FE_Trigger_Render(GameCamera);
	FE_Player_Render(GamePlayer, GameCamera);

	if (PresentGame->DebugConfig.LightingEnabled)
		FE_Light_Render(GameCamera, world);
	FE_UI_Render();
	SDL_RenderPresent(PresentGame->Client->Renderer);
}

void FE_GameLoop()
{
	if (!PresentGame->MapConfig.Loaded || PresentGame->GameState != GAME_STATE_PLAY)
		error("Gamestate Error: Game not configured correctly");

	if (FE_FPS == 0) {
		return;
	}


	float event_start = FE_QueryPerformanceCounter();
	FE_GameEventHandler(GameCamera, GamePlayer);
	PresentGame->Timing.EventTime = (FE_QueryPerformanceCounter() - event_start) / (FE_QueryPerformanceFrequency() / 1000);

	/* Upate Loop*/
	float update_start = FE_QueryPerformanceCounter();
	FE_DebugUI_Update(GamePlayer);
	FE_Dialogue_Update();
	FE_Timers_Update();
	FE_Particles_Update(); // todo can we multithread this
	FE_Player_Update(GamePlayer);
	FE_Animations_Update();
	FE_Prefab_Update();
	FE_UpdateCamera(GameCamera);
	FE_Physics_Update();
	PresentGame->Timing.UpdateTime = (FE_QueryPerformanceCounter() - update_start) / (FE_QueryPerformanceFrequency() / 1000);

	/* Render Loop */
	float render_start = FE_QueryPerformanceCounter();
	FE_RenderGame();
	PresentGame->Timing.RenderTime = (FE_QueryPerformanceCounter() - render_start) / (FE_QueryPerformanceFrequency() / 1000);
}