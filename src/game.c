//Core game loops and functions for game operation
#include "include/game.h"

static FE_Camera *GameCamera;
static FE_Player *GamePlayer;

static FE_ParticleSystem *SnowParticles;
SDL_Texture *world;
FE_GameObject *fire;

/* Starts the main game */
void FE_StartGame(const char *mapname)
{
	char *map = strdup(mapname);
	FE_CleanAll();

	if (FE_LoadMap(map) == -1) {
		warn("Failed to start game");
		FE_Menu_MainMenu();
		free(map);
		return;
	}
	free(map);

	// camera setup
	GameCamera = FE_CreateCamera();
	GameCamera->maxzoom = 2.0f;
	GameCamera->minzoom = 1.0f;

	// player setup
	GamePlayer = FE_CreatePlayer(1, 8, 18, (SDL_Rect){PresentGame->MapConfig.PlayerSpawn.x, PresentGame->MapConfig.PlayerSpawn.y, 120, 100});
	GameCamera->follow = &GamePlayer->render_rect;


	// test particle system
	SnowParticles = FE_CreateParticleSystem(
		(SDL_Rect){0, -20, PresentGame->MapConfig.MapWidth, 20}, // Position for the whole screen, slightly above the top to create more random
		250, // Emission rate
		3000, // Max particles
		10000, // Max lifetime
		true, // Particles to respawn once they go off screen
		"rain.png", // Texture
		(Vector2D){15, 15}, // Max size of each particle
		(Vector2D){3, 3}, // Set initial velocity so particle doesn't float until they accelerate
		false
	);

	world = FE_CreateRenderTexture(PresentGame->Window_width, PresentGame->Window_height);
	FE_ResetDT();

	FE_CreateGameObject((SDL_Rect){1200, 1200, 64, 128}, "torch.png", "", ENEMY, 0, false);
	FE_CreateLight((SDL_Rect){870,1050,712,512}, "torchglow.png");

	PresentGame->GameState = GAME_STATE_PLAY;
}


void FE_RenderGame()
{
	if (PresentGame->DebugConfig.LightingEnabled)
		SDL_SetRenderTarget(PresentGame->Renderer, world);
	SDL_RenderClear(PresentGame->Renderer);
	FE_RenderMapBackground(GameCamera);
	FE_RenderParticles(GameCamera);
	FE_RenderMap(GameCamera);
	FE_RenderGameObjects(GameCamera);
	FE_RenderPlayer(GamePlayer, GameCamera);
	if (PresentGame->DebugConfig.LightingEnabled)
		FE_RenderLighting(GameCamera, world);
	FE_RenderUI();
	SDL_RenderPresent(PresentGame->Renderer);
	
}

void FE_GameLoop()
{
	if (!PresentGame->MapConfig.Loaded || PresentGame->GameState != GAME_STATE_PLAY)
		error("Game not configured correctly");

	if (FE_FPS == 0) {
		return;
	}

	FE_DebugUI_Update(GamePlayer);
	
	PresentGame->Timing.EventTime = SDL_GetPerformanceCounter();
	PresentGame->Timing.EventTime = ((SDL_GetPerformanceCounter() - PresentGame->Timing.EventTime) / SDL_GetPerformanceFrequency()) * 1000;
	FE_GameEventHandler(GameCamera, GamePlayer);
	PresentGame->Timing.UpdateTime = SDL_GetPerformanceCounter();
	FE_UpdateTimers();
	FE_RunPhysics();
	FE_UpdateParticles(); // todo can we multithread this?
	FE_UpdatePlayer(GamePlayer);
	FE_UpdateAnimations();
	FE_UpdateCamera(GameCamera);
	PresentGame->Timing.UpdateTime = ((SDL_GetPerformanceCounter() - PresentGame->Timing.UpdateTime) / SDL_GetPerformanceFrequency()) * 1000;

	PresentGame->Timing.RenderTime = SDL_GetPerformanceCounter();
	FE_RenderGame();
	PresentGame->Timing.RenderTime = ((SDL_GetPerformanceCounter() - PresentGame->Timing.RenderTime) / SDL_GetPerformanceFrequency()) * 1000;
}