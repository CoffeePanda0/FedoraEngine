//Main loops and functions for game operation
#include "game.h"

SDL_Texture* playerText;
static SDL_RendererFlip playerFlip = SDL_FLIP_NONE;

SDL_Window* window;
SDL_Renderer* renderer;
TTF_Font* Sans;
SDL_Texture* title;

struct TextObject testText;
Mix_Music* bgMusic;

bool GameActive;
bool jumping = false;
bool TextPaused = false;
enum CollDir dir;

int map_width, map_height;

void InitGame() // initializes the things like game objects and maps
{
	Sans = TTF_OpenFont("game/baloo.ttf", 20);

	InitPlayer();
	NewText(&testText, "FedoraEngine!", Black, 350 , 0); // EXAMPLE TEXT
	
	InitMap("game/map/testmap.txt"); // YOU HAVE TO CALL THIS FOR A MAP TO RENDER AND BE LOADED
	MapLoaded(); // make sure user loaded in a map

	bgMusic = LoadMusic("game/audio/CoffeeTime.mp3"); // load in background music
	if (Mix_PlayingMusic() == 0) {
    	if (Mix_PlayMusic( bgMusic, -1 ) == -1)
			warn("Could not play music %s", Mix_GetError());
    }
	UIText("Welcome To FEDORA ENGINE", "fedora man");
}

void InitPlayer() // change this for your player
{
	SpawnPlayer(0, 0, 75, 90);
	playerText = TextureManager("game/duck.png", renderer); // EXAMPLE PLAYER
	JumpSound = LoadSFX("game/audio/jump.wav");
}

void Physics() { // handles player movement
	if (!paused && !TextPaused) {

		if (moving && !jumping) {
		
			if (acceleration < maxAccel)
				acceleration += 0.01f;
		} else if (jumping && acceleration > 1.0f)
			acceleration -= 0.03f; // decel player left-right in the air so they are not too speedy
		else
			acceleration = 1.0;

		if (!gAbove() && dir != DIR_ABOVE)
			playerRect.y += gravity;

		if (jumping)
				playerRect.y += velocity;

		if (jumping && (gBelow() || dir == DIR_BELOW)) {
				velocity = 1;
				jumping = false;
		}
		
		if (jumping) {
			if (playerRect.y < 0 ) { // check bounds
				velocity = 0;
				jumping = false;
			}

			if (gRight() || gLeft()) { // make sure player wont clip through tiles
					jumping = false;
					velocity = 0;
			}

			if (velocity < 0.1) // decrease velocity so player will eventually fall
				velocity += 0.1f;

			if (velocity >= 0) {
				jumping = false;
				velocity = 0;
			} 
		}

		if (playerRect.y > screen_height) { // if player falls through hole in ground
			info("Player died\n");
			PlayerMove(200,0);
		}
	}
}

void Update()
{
	CollisionDetection(); // checks for collision with each game objects
	Physics();
}

SDL_Texture* TextureManager(const char* texture, SDL_Renderer* ren)
{
	SDL_Surface* s = IMG_Load(texture);
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
}

Mix_Music* LoadMusic(const char* path)
{
	Mix_Music* tmp = Mix_LoadMUS(path);
	if (!tmp) {
		warn("Could not load music %s, SDL_Error: %s", path, Mix_GetError());
	} else
		info("Loaded music %s", path);
	
	return tmp;
}

Mix_Chunk* LoadSFX(const char* path)
{
	Mix_Chunk* tmp = Mix_LoadWAV(path);
	if (!tmp) {
		warn("Could not load SFX %s, SDL_Error: %s", path, Mix_GetError());
	} else
		info("Loaded SFX %s", path);

	return tmp;
}

void Render()
{
	SDL_RenderClear(renderer);
	RenderMap();
	RenderObject();
	RenderText();
	SDL_RenderCopyEx(renderer, playerText, NULL, &playerRect, 0, NULL, playerFlip); // renders the player
	SDL_SetRenderDrawColor(renderer, 255, 255, 255, 0);
	SDL_RenderPresent(renderer);
	
}

void event_handler() {
	const Uint8*  keyboard_state = SDL_GetKeyboardState(NULL);

	SDL_Event event;
	while (SDL_PollEvent(&event)) {

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
			break;

			case SDL_KEYDOWN: // SINGLE KEY PRESS NON IMPORTANT HERE
			
				if (keyboard_state[SDL_SCANCODE_X] && TextPaused) 
					UITextInteract(0);
				if (keyboard_state[SDL_SCANCODE_M] && event.key.repeat == 0) {
					if (Mix_PausedMusic()) {
						Mix_PlayMusic(bgMusic, -1);
					} else 
						Mix_PauseMusic();
				}
				if (keyboard_state[SDL_SCANCODE_P] && event.key.repeat == 0) {
						CreateMenu(); 
						paused = true;
				}

			case SDL_KEYUP:
				if (!keyboard_state[SDL_SCANCODE_LEFT])
					moving = false;
				if (!keyboard_state[SDL_SCANCODE_RIGHT])
					moving = false;
			break;
		}
	}
	if (!TextPaused) {
		// MORE IMPORTANT MULTI PRESS OUT THE FUNCTION
		if (keyboard_state[SDL_SCANCODE_LEFT] && dir != DIR_LEFT && !gRight()) {
			if (playerRect.x >= 0) {
				moving = true;
				playerFlip = SDL_FLIP_NONE;
				PlayerMove(-movAmount * acceleration, 0);
			}
		}
		if (keyboard_state[SDL_SCANCODE_RIGHT] && dir != DIR_RIGHT && !gLeft()) {
			if (playerRect.x <= (map_width - playerRect.w - 1)) {
				moving = true;
				playerFlip = SDL_FLIP_HORIZONTAL;
				PlayerMove(movAmount * acceleration, 0);
			}
		}

		if (keyboard_state[SDL_SCANCODE_SPACE]) {
			if (!gBelow() && dir != DIR_BELOW && !jumping && gAbove())
				PlayerJump();
		}
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

		renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC);
		if (renderer)
			info("Renderer Created");
		else
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

		paused = false;

		// Set up the game here
		InitGame();
		GameActive = true;
		
	} else {
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
