#include "game.h"

SDL_RendererFlip playerFlip = SDL_FLIP_NONE;

static bool fullscreen = false;
static bool startedinput = false;

static char content[512];

void event_handler() {

	const Uint8* keyboard_state = SDL_GetKeyboardState(NULL);
	SDL_Event event;

	while (SDL_PollEvent(&event)) {
		
		if (!intext) {
			// Code for main control operation

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
						DialogueInteract(0);
					if (keyboard_state[SDL_SCANCODE_F] && event.key.repeat == 0) {
						if (!fullscreen) {
							SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);
							fullscreen = true;
						} else {
							SDL_SetWindowFullscreen(window, 0);
							fullscreen = false;
						}
					}
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
					if (keyboard_state[SDL_SCANCODE_GRAVE] && event.key.repeat == 0) {
						if (!intext) {
							intext = true;
							if (!ConsoleVisible)
								ShowConsole();
						}
					}
					startedinput = false;

				break;

				case SDL_KEYUP:
					if (!keyboard_state[SDL_SCANCODE_LEFT])
						moving = false;
					if (!keyboard_state[SDL_SCANCODE_RIGHT])
						moving = false;
				break;

			}
		} else {
			// Code for key handling here for UI
			if (!startedinput) {
				SDL_StartTextInput();
				startedinput = true;
				memset(content, 0, strlen(content));
				return;
			}

			if (keyboard_state[SDL_SCANCODE_GRAVE]) {
				SDL_StopTextInput();
				startedinput = false;
				intext = false;
				HideConsole();
			}

			else if (keyboard_state[SDL_SCANCODE_BACKSPACE]) {
				content[strlen(content) -1] = 0;
				TextBoxUpdate(content);
			}

			else if (keyboard_state[SDL_SCANCODE_RETURN]) {
				if (ConsoleVisible) {
					RunLua(content);
					memset(content, 0, strlen(content)); // empty console after command sent
				}
			}

			else if (event.type == SDL_TEXTINPUT) {
				if (strlen(content) < 256) {
					strcat(content, event.text.text);
					TextBoxUpdate(content);
				}
			}

			switch (event.type)
			{
				case SDL_QUIT: // basic window controls need to stay
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
				
			}
		}
	}

	if (!TextPaused && !intext) {
		// MORE IMPORTANT MULTI PRESS OUT THE FUNCTION
		if (keyboard_state[SDL_SCANCODE_LEFT] && dir != DIR_RIGHT && !gRight(CollRect)) {
			if (playerRect.x >= 0) {
				moving = true;
				playerFlip = SDL_FLIP_NONE;
				PlayerMove(-movAmount * acceleration, 0);
			}
		}

		if (keyboard_state[SDL_SCANCODE_RIGHT] && dir != DIR_LEFT && !gLeft(CollRect)) {
			if (playerRect.x <= (map_width - playerRect.w - 1)) {
				moving = true;
				playerFlip = SDL_FLIP_HORIZONTAL;
				PlayerMove(movAmount * acceleration, 0);
			}
		}

		if (keyboard_state[SDL_SCANCODE_SPACE] && !gBelow(CollRect) && !jumping && (gAbove(CollRect) || dir == DIR_ABOVE))
			PlayerJump();
	}

}