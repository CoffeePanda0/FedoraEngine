//Core game loops and functions for game operation
#include "include/game.h"

SDL_Window* window;
SDL_Renderer* renderer;

enum FE_GAMESTATE FE_GameState;
void (*MenuPage)();

bool FE_GameActive;
static FE_Camera GameCamera;

	/* Features!
		- Labels
		- Textboxes
		- Linked list system
		- Buttons
		- Checkboxes
		- Messagebox and input box
		- HUD UIObjects
		- Game Objects and sprites
		- Physics objects
		- Timers
		- Basic hard-coded menus
		- Audio controls
		- Dialogue system
		- Viewing logs in game console
	*/

    /* fedoraengine todo
        this
		check return codes on all functions
        menu JSON implementation
		console execute lua
		add (working) deltatime to physics
		make a game and player maybe?? idk
    */


void FE_StartGame(const char *mapname)
{
	if (FE_LoadMap(mapname) == -1) {
		warn("Failed to start game");
		FE_Menu_MainMenu();
		return;
	}

	FE_GameState = GAME_STATE_PLAY;
	GameCamera = (FE_Camera){0, 0, FE_Map_Width, FE_Map_Height, false};
	// todo: pause menu, tiles loading incorrect texture
}

void FE_RenderGame()
{
	SDL_RenderClear(renderer);
	FE_RenderMap(&GameCamera);
	FE_RenderGameObjects(&GameCamera);
	FE_RenderUI();
	SDL_RenderPresent(renderer);
}

void FE_GameLoop()
{
	FE_UpdateTimers();
	FE_GameEventHandler(&GameCamera);
	FE_RunPhysics();
	FE_RenderGame();
}

void FE_init(const char* window_title, int xpos, int ypos, int window_width, int window_height)
{
	if (SDL_Init(SDL_INIT_EVERYTHING) == 0)
	{	
		FE_ConsoleInit();

		// Check that everything has loaded in correctly
		window = SDL_CreateWindow(window_title, xpos, ypos, window_width, window_height, SDL_WINDOW_ALLOW_HIGHDPI);
		
		if (window) {
			info("Window Created");
		} else 
			error("Window could not be created! SDL_Error: %s", SDL_GetError());

		renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC);

		if (renderer)
			info("Renderer Created");
		else
			error("Renderer could not be created! SDL_Error: %s", SDL_GetError());

		if (IMG_Init(IMG_INIT_PNG) == 0)
    		error("IMG Failed to initialize. SDL_Error: %s", TTF_GetError());

		Sans = TTF_OpenFont("game/fonts/baloo.ttf", 20);
	 	if (!Sans)
		 	warn("Failed to load font baloo.ttf");

		FE_Menu_MainMenu();

		FE_GameActive = true;

	} else {
		error("Could not start SDL. SDL_Error: %s", SDL_GetError());
		FE_Clean();
	}
}

void FE_CleanAll() // Cleans all resources possible without exiting
{
	FE_CloseMap();
	FE_FreeUI();
	FE_FreeDialogue();
	FE_CleanEditor();
	FE_CleanGameObjects();
	FE_DestroyMessageBox();
	FE_CleanAudio();
	FE_CleanTimers();
}

void FE_Clean() // Exits the game cleanly, freeing all resources
{
	if (FE_GameActive) {
		FE_GameActive = false;
		info("Exiting FedoraEngine \n");
		IMG_Quit();
		FE_CleanAll();
		log_close();
		SDL_DestroyWindow(window);
		TTF_CloseFont(Sans);
		FE_DestroyConsole();
		SDL_Quit();
	}
}
