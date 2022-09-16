//Core game loops and functions for game operation
#include "include/game.h"

static FE_Camera *GameCamera;
static FE_Player *GamePlayer;

static FE_ParticleSystem *SnowParticles;
SDL_Texture *world;

/* Starts the main game */
void FE_StartGame(const char *mapname)
{
	char *map = mstrdup(mapname);
	FE_CleanAll();

	// Map setup
	FE_LoadedMap *m = 0;
	if (!(m = FE_LoadMap(map))) {
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
	GamePlayer = FE_CreatePlayer(40, 18, (SDL_Rect){PresentGame->MapConfig.PlayerSpawn.x, PresentGame->MapConfig.PlayerSpawn.y, 120, 100});
	GameCamera->follow = &GamePlayer->render_rect;

	// test particle system
	SnowParticles = FE_CreateParticleSystem(
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

	PresentGame->GameState = GAME_STATE_PLAY;
}


void FE_RenderGame()
{
	if (PresentGame->DebugConfig.LightingEnabled)
		SDL_SetRenderTarget(PresentGame->Renderer, world);
	SDL_RenderClear(PresentGame->Renderer);
	FE_RenderMapBackground(GameCamera);
	FE_RenderParticles(GameCamera);
	FE_RenderLoadedMap(GameCamera);
	FE_Trigger_Render(GameCamera);
	FE_GameObject_Render(GameCamera);
	FE_Trigger_Render(GameCamera);
	FE_RenderPlayer(GamePlayer, GameCamera);

	if (PresentGame->DebugConfig.LightingEnabled)
		FE_Light_Render(GameCamera, world);
	FE_UI_Render();
	SDL_RenderPresent(PresentGame->Renderer);
}

void FE_GameLoop()
{
	if (!PresentGame->MapConfig.Loaded || PresentGame->GameState != GAME_STATE_PLAY)
		error("Gamestate Error: Game not configured correctly");

	if (FE_FPS == 0) {
		return;
	}

	FE_DebugUI_Update(GamePlayer);
	FE_Dialogue_Update();

	PresentGame->Timing.EventTime = SDL_GetPerformanceCounter();
	PresentGame->Timing.EventTime = ((SDL_GetPerformanceCounter() - PresentGame->Timing.EventTime) / SDL_GetPerformanceFrequency()) * 1000;
	FE_GameEventHandler(GameCamera, GamePlayer);
	PresentGame->Timing.UpdateTime = SDL_GetPerformanceCounter();
	FE_UpdateTimers();
	FE_RunPhysics();
	FE_UpdateParticles(); // todo can we multithread this?
	FE_UpdatePlayer(GamePlayer);
	FE_UpdateAnimations();
	FE_Prefab_Update();
	FE_UpdateCamera(GameCamera);
	PresentGame->Timing.UpdateTime = ((SDL_GetPerformanceCounter() - PresentGame->Timing.UpdateTime) / SDL_GetPerformanceFrequency()) * 1000;

	PresentGame->Timing.RenderTime = SDL_GetPerformanceCounter();
	FE_RenderGame();
	PresentGame->Timing.RenderTime = ((SDL_GetPerformanceCounter() - PresentGame->Timing.RenderTime) / SDL_GetPerformanceFrequency()) * 1000;
}