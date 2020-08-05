//Main loops and functions for game operation
#include "game.h"

SDL_Texture* playerText;
SDL_RendererFlip playerFlip = SDL_FLIP_NONE;

SDL_Window* window;
SDL_Renderer* renderer;
TTF_Font* Sans;
SDL_Texture* title;
SDL_Surface* s;

struct GameObject doge; // using doge as an example gameobject because why not :P
struct TextObject test;
Mix_Music* bgMusic;

bool GameActive;
bool onGround = false;
bool jumping = false;

enum CollDir dir;

void InitGame() // initialises the things like game objects and maps
{	
	Sans = TTF_OpenFont("game/baloo.ttf", 20);
	if (overlay) InitDebugOverlay();
	
	InitPlayer(50, 50, 100, 100);
	playerText = TextureManager("game/player.png", renderer); // EXAMPLE PLAYER
	
	CreateObject(300, 370, 45, 45, "game/doge.png", &doge); // EXAMPLE GAMEOBJECT

	NewText(&test, "FedoraEngine!", Black, 350 , 0); // EXAMPLE TEXT
	
	InitMap("game/map/testmap.txt"); // YOU HAVE TO CALL THIS FOR A MAP TO RENDER AND BE LOADED

	bgMusic = Mix_LoadMUS("game/audio/CoffeeTime.mp3"); // load in background music
	if( Mix_PlayingMusic() == 0 ) {
    	if( Mix_PlayMusic( bgMusic, -1 ) == -1 )
			warn("Could not play music %s", Mix_GetError());
    }

	SDL_FreeSurface(s);
}

void Update()
{
	CollisionDetection();

	if (moving) {
		if (acceleration < maxAccel)
			acceleration += 0.02f;
	} else
		acceleration = 1.0;

	if (!onGround || jumping) 
		playerRect.y += gravity + velocity;

	if (jumping) {
		if (velocity < 0.1)
			velocity += 0.1f;
		if (velocity >= 0) {
			jumping = false;
			velocity = 0;
		} 
	}
}

SDL_Texture* TextureManager(const char* texture, SDL_Renderer* ren)
{
	s = IMG_Load(texture);
	if (s) {
		SDL_Surface* tmpSurface = IMG_Load(texture); 
		SDL_Texture* text = SDL_CreateTextureFromSurface(ren, tmpSurface);
		SDL_FreeSurface(tmpSurface);
		return text;
	} else {
		warn("Texture %s not found", texture);
		SDL_Surface* tmpSurface = SDL_CreateRGBSurface(0,50,50,32,0,0,0,0);
		SDL_Texture* text = SDL_CreateTextureFromSurface(ren, tmpSurface);
		SDL_FreeSurface(tmpSurface);
		return text;
	}
	SDL_FreeSurface(s);
}

void LoadMusic(Mix_Music* m, const char* path)
{
	m = Mix_LoadMUS(path);
	if (!m)
		warn("Could not load music %s, SDL_Error: %s", path, Mix_GetError());
}

void LoadSFX(Mix_Chunk* c, const char* path)
{
	c = Mix_LoadWAV(path);
	if (!c)
		warn("Could not load SFX %s, SDL_Error: %s", path, Mix_GetError());
}

void Render()
{
	SDL_RenderClear(renderer);
	RenderMap();
	if (overlay) {TextDebugOverlay();}
	RenderText();
	RenderObject();
	SDL_RenderCopyEx(renderer, playerText, NULL, &playerRect, 0, NULL, playerFlip);
	SDL_SetRenderDrawColor(renderer, 255, 255, 255, 0);
	SDL_RenderPresent(renderer);
}

void event_handler() {
	const Uint8* keyboard_state = SDL_GetKeyboardState(NULL);

	SDL_Event event;
	SDL_PollEvent(&event);

	switch (event.type)
	{
		case SDL_QUIT:
			GameActive = false;
			break;
		case SDL_WINDOWEVENT:
			switch (event.window.event) {
			case SDL_WINDOWEVENT_SIZE_CHANGED:
				screen_width = event.window.data1;
				screen_height = event.window.data2;
				break;
			}
			break;

		case SDL_KEYDOWN: // ALL KEYBOARD INPUTS HANDLED HERE

			if (keyboard_state[SDL_SCANCODE_LEFT] && dir != DIR_RIGHT) {
				if (playerRect.x >= movAmount) {
					PlayerMove(-movAmount * acceleration, 0);
					playerFlip = SDL_FLIP_NONE;
					moving = true;
				}
			}
			else if (keyboard_state[SDL_SCANCODE_RIGHT] && dir != DIR_LEFT) {
				if (playerRect.x <= (screen_width - playerRect.w - 1)) {
					PlayerMove(movAmount * acceleration, 0);
					playerFlip = SDL_FLIP_HORIZONTAL;
					moving = true;
				}

			}
			else if (keyboard_state[SDL_SCANCODE_SPACE]) {
				if (onGround)
					PlayerJump();
			}

			else if (keyboard_state[SDL_SCANCODE_C]) {
				if (overlay) {
					overlay = false;
					FreeOverlay();
				} else {
					overlay = true;
					InitDebugOverlay();
				}
			}
		break;
	
		case SDL_KEYUP:
			if (!keyboard_state[SDL_SCANCODE_LEFT])
				moving = false;
			if (!keyboard_state[SDL_SCANCODE_RIGHT])
				moving = false;
		break;
	}
}

void init(const char* window_title, int xpos, int ypos, int window_width, int window_height)
{

	if (SDL_Init(SDL_INIT_EVERYTHING) == 0)
	{
		// Check that everything has loaded in correctly
		info("SDL Fully Initialised");

		window = SDL_CreateWindow(window_title, xpos, ypos, window_width, window_height, SDL_WINDOW_ALLOW_HIGHDPI);
		
		if (window) {
			info("Window Created");
		} else 
			error("Window could not be created! SDL_Error: %s", SDL_GetError());

		if (vsync)
			renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC);
		else
			renderer = SDL_CreateRenderer(window, -1, 0);
		
		if (renderer) {
			info("Renderer Created");
		} else
			error("Renderer could not be created! SDL_Error: %s", SDL_GetError());

		if (TTF_Init() == -1)
    		error("TTF Failed to initialize. SDL_Error: %s", TTF_GetError());
		else
			info("Created TTF");

		if (IMG_Init(IMG_INIT_PNG) == 0)
    		error("IMG Failed to initialize. SDL_Error: %s", TTF_GetError());
		else
			info("Created IMG");

		if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096) < 0)
            error("SDL_mixer failed to initialize! SDL_Error: %s", Mix_GetError());
		else
			info("Initialized Mixer");
		
		// Set up the game here
		InitGame();
		GameActive = true;
		
	}
	else {
		GameActive = false;
		error("Could not start SDL. SDL_Error: %s", SDL_GetError());
	}
}

void Clean()
{
	TTF_CloseFont(Sans);
	SDL_DestroyWindow(window);
	SDL_DestroyRenderer(renderer);
	TTF_Quit();
	IMG_Quit();
	Mix_Quit();
	info("SDL Exited\n");
	log_close();
	SDL_Quit();
}
