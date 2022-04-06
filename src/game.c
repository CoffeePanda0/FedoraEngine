//Core game loops and functions for game operation
#include "include/game.h"

enum FE_GAMESTATE FE_GameState;

static FE_Camera GameCamera;

/* Starts the main game */
void FE_StartGame(const char *mapname)
{
	if (FE_LoadMap(mapname) == -1) {
		warn("Failed to start game");
		FE_Menu_MainMenu();
		return;
	}

	// camera setup
	int spawnx = clamp(FE_GetSpawn().x, FE_Map_MinimumX, FE_Map_Width);
	int spawny = clamp(FE_GetSpawn().y, FE_Map_MinimumY, FE_Map_Height);
	GameCamera = (FE_Camera){spawnx, spawny, FE_Map_MinimumX, FE_Map_MinimumY, FE_Map_Width, FE_Map_Height, false};

	FE_GameState = GAME_STATE_PLAY;
}

void FE_RenderGame()
{
	SDL_RenderClear(renderer);
	FE_RenderMap(&GameCamera);
	FE_RenderGameObjects(&GameCamera);
	FE_RenderLightObjects(&GameCamera);
	FE_RenderUI();
	SDL_RenderPresent(renderer);
}

void FE_GameLoop()
{
	FE_UpdateTimers();
	FE_GameEventHandler(&GameCamera);
	FE_RunPhysics();
	FE_UpdateAnimations();
	FE_RenderGame();
}
