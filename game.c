//Main loops and functions for game operation
#include "game.h"

SDL_Texture* playerText;
SDL_RendererFlip playerFlip = SDL_FLIP_NONE;

SDL_Window* window;
SDL_Renderer* renderer;
SDL_Texture* text;

struct GameObject doge;

bool GameActive;
bool onGround;

void Update()
{
	if (moving) {
		if (acceleration < maxAccel)
			acceleration += 0.02f;
	}
	else {
		acceleration = 1.0;
	}

	if (playerRect.y <= (screen_height - playerRect.h - 1) || jumping) {
		playerRect.y += gravity + velocity;
		onGround = false;
	}
	else if (playerRect.y > screen_height) {
		playerRect.y = 0;
	} else 
		onGround = true;

	if (jumping) {
		velocity += 0.05f;
		if (playerRect.y < screen_height / 1.5) {
			jumping = false;
			velocity = 0;
		}
	}
}

SDL_Texture* TextureManager(const char* texture, SDL_Renderer* ren)
{
	SDL_Surface* tmpSurface = IMG_Load(texture); 
	SDL_Texture* text = SDL_CreateTextureFromSurface(ren, tmpSurface);
	SDL_FreeSurface(tmpSurface);
	return text;
}

void Render()
{
	SDL_RenderClear(renderer);
	RenderObject(doge);
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
				info("Window size changed");
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
				Clean();
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

void init(const char* window_title, int xpos, int ypos, int window_width, int window_height, bool fullscreen)
{
	int flags = 0;
	if (fullscreen)
		flags = SDL_WINDOW_FULLSCREEN;

	if (SDL_Init(SDL_INIT_EVERYTHING) == 0)
	{

		info("SDL Fully Initialised");

		window = SDL_CreateWindow(window_title, xpos, ypos, window_width, window_height, SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
		if (window) {
			info("Window Created");
		}
		else if (window == NULL) {
			error("Window could not be created! SDL_Error: %s", SDL_GetError());
			Clean();
		}

		renderer = SDL_CreateRenderer(window, -1, 0);
	
		if (renderer) {
			info("Renderer Created");
			
		}
		else {
			error("Renderer could not be created! SDL_Error: %s", SDL_GetError());
			Clean();
		}

		GameActive = true;
		InitPlayer(50, 50, 100, 100);
		
		playerText = TextureManager("game/player.png", renderer);
		CreateObject(300, 300, 100, 100, "game/doge.png", &doge);
	}
	else {
		GameActive = false;
	}
}

void Clean()
{
	SDL_DestroyWindow(window);
	SDL_DestroyRenderer(renderer);
	SDL_Quit();
	info("SDL Exited");
	log_close();
}
