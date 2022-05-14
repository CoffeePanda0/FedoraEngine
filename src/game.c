//Core game loops and functions for game operation
#include "include/game.h"

static FE_Camera *GameCamera;
static FE_Player *GamePlayer;

static FE_ParticleSystem *SnowParticles;
SDL_Texture *world;


/* Starts the main game */
void FE_StartGame(const char *mapname)
{
	if (FE_LoadMap(mapname) == -1) {
		warn("Failed to start game");
		FE_Menu_MainMenu();
		return;
	}

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

	world = FE_CreateRenderTexture(PresentGame->window_width, PresentGame->window_height);

	PresentGame->GameState = GAME_STATE_PLAY;
}

void FE_RenderGame()
{
	SDL_RenderClear(PresentGame->renderer);

	SDL_SetRenderTarget(PresentGame->renderer, world);
	SDL_RenderClear(PresentGame->renderer);

	// Render the game to the world texture
	FE_RenderMapBackground(GameCamera);
	FE_RenderParticles(GameCamera);
	FE_RenderMap(GameCamera);
	FE_RenderGameObjects(GameCamera);
	FE_RenderPlayer(GamePlayer, GameCamera);

	FE_RenderLighting(GameCamera, world);

	FE_RenderUI();

	SDL_RenderPresent(PresentGame->renderer);
	
}

void FE_GameLoop()
{
	FE_CalculateDT();

	if (FE_FPS == 0) {
		return;
	}
	
	FE_GameEventHandler(GameCamera, GamePlayer);
	FE_UpdateTimers();
	FE_RunPhysics();
	FE_UpdateParticles();
	FE_UpdatePlayer(GamePlayer);
	FE_UpdateAnimations();
	FE_UpdateCamera(GameCamera);
	FE_RenderGame();
}