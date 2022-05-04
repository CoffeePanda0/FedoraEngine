//Core game loops and functions for game operation
#include "include/game.h"

enum FE_GAMESTATE FE_GameState;

static FE_Camera GameCamera;
static FE_Player *GamePlayer;

static FE_GameObject *test;
static FE_ParticleSystem *SnowParticles;

/* Starts the main game */
void FE_StartGame(const char *mapname)
{
	if (FE_LoadMap(mapname) == -1) {
		warn("Failed to start game");
		FE_Menu_MainMenu();
		return;
	}

	// camera setup
	int spawnx = FE_GetSpawn().x;
	int spawny = FE_GetSpawn().y;
	GameCamera = (FE_Camera){spawnx, spawny, FE_Map_MinimumX, FE_Map_Height, FE_Map_Width, FE_Map_Height, false};

	// player setup
	GamePlayer = FE_CreatePlayer(1, 8, 4, (SDL_Rect){0,0, 120, 100});
	FE_SetPlayerWorldPos(GamePlayer, &GameCamera, FE_NewVector(spawnx, spawny)); // todo spawn

	// test object
	//test = FE_CreateGameObject((SDL_Rect){200, 50, 50, 50}, "doge.png", "deez", ENEMY, 20, true);

	// test particle system
	SnowParticles = FE_CreateParticleSystem(
		(SDL_Rect){0, -20, FE_Map_Width, 20}, // Position for the whole screen, slightly above the top to create more random
		240, // Emission rate
		3000, // Max particles
		10000, // Max lifetime
		true, // Particles to respawn once they go off screen
		"snow.png", // Texture
		(Vector2D){15, 15}, // Max size of each particle
		(Vector2D){15, 3}, // Set initial velocity so particle doesn't float until they accelerate
		false
	);

	FE_GameState = GAME_STATE_PLAY;

}

void FE_RenderGame()
{	
	SDL_RenderClear(renderer);
	FE_RenderMapBackground(&GameCamera);
	FE_RenderParticles(&GameCamera);
	FE_RenderMap(&GameCamera);
	FE_RenderGameObjects(&GameCamera);
	FE_RenderLightObjects(&GameCamera);
	FE_RenderPlayer(GamePlayer);
	FE_RenderUI();
	SDL_RenderPresent(renderer);
}

void FE_GameLoop()
{
	FE_CalculateDT();

	if (FE_FPS == 0) {
		return;
	}
	
	FE_GameEventHandler(&GameCamera, GamePlayer);
	FE_UpdateTimers();
	FE_RunPhysics();
	FE_UpdateParticles();
	FE_UpdatePlayer(GamePlayer, &GameCamera);
	FE_UpdateAnimations();
	FE_RenderGame();
}