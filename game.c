//Main loops and functions for game operation
#include "game.h"
#include <sys/stat.h>

SDL_Texture* playerText;
SDL_RendererFlip playerFlip = SDL_FLIP_NONE;

SDL_Window* window;
SDL_Renderer* renderer;
TTF_Font* Sans;
SDL_Texture* title;
SDL_Surface *s;

struct GameObject doge; // using doge as an example gameobject because why not :P

bool GameActive;
bool onGround;
struct TextObject test;

void Update()
{
	if (moving) {
		if (acceleration < maxAccel)
			acceleration += 0.02f;
	} else
		acceleration = 1.0;

	if (playerRect.y <= (screen_height - GroundCollideHeight - playerRect.h - 1) || jumping) {
		playerRect.y += gravity + velocity;
		onGround = false;
		
	} else 
		onGround = true;

	if (jumping) {
		if (velocity > -0.1)
			velocity += 0.07f;
		if (playerRect.y + GroundCollideHeight < screen_height / 1.5) {
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
			//std::cout << "X: " << playerRect.x << " Y: " << playerRect.y << " Accel: " << acceleration << std::endl; // Debugging purposes	

			if (keyboard_state[SDL_SCANCODE_LEFT]) {
				if (playerRect.x >= movAmount) {
					PlayerMove(-movAmount * acceleration, 0);
					playerFlip = SDL_FLIP_NONE;
					moving = true;
				}
			}
			else if (keyboard_state[SDL_SCANCODE_RIGHT]) {
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
			else if (keyboard_state[SDL_SCANCODE_ESCAPE]) {
				FreeText(&ui_fps);
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

		// Set up the game here

		Sans = TTF_OpenFont("game/baloo.ttf", 20);
		if (overlay) InitDebugOverlay();

		InitPlayer(50, 50, 100, 100);
		playerText = TextureManager("game/player.png", renderer); // EXAMPLE PLAYER

		CreateObject(300, 300, 100, 100, "game/doge.png", &doge); // EXAMPLE GAMEOBJECT

		NewText(&test, "FedoraEngine!", Black, 350 , 0); // EXAMPLE TEXT
		
		InitMap("game/map/testmap.txt"); // YOU HAVE TO CALL THIS FOR A MAP TO RENDER AND BE LOADED
		
		SDL_FreeSurface(s);
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
	info("SDL Exited\n");
	log_close();
	SDL_Quit();
}
