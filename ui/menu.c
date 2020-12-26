// Pause menu and event handling for pause menu
#include "../menu.h"

static int selection = 0;
static const char* labels[] = {"Resume", "Quit"};
static const int LabelCount = LEN(labels);
static SDL_Texture* LabelText[LEN(labels)];

void RenderMenu()
{
	SDL_RenderClear(renderer);
	SDL_Rect TmpRect;
	int txtw, txth;
	TmpRect.x = 50;
	TmpRect.y = 50;

	for (int i = 0; i < LabelCount; i++) {

		SDL_QueryTexture(LabelText[i], NULL, NULL, &txtw, &txth); 
		TmpRect.w = txtw;
		TmpRect.h = txth;

		SDL_RenderCopy(renderer, LabelText[i], NULL, &TmpRect);
		TmpRect.y += txth + 5;
	}

	SDL_SetRenderDrawColor(renderer, 255, 255, 255, 0);
	SDL_RenderPresent(renderer);

}


void CreateMenu()
{ 	
	SDL_Surface* tmp;
	for (int i = 0; i < LabelCount; i++) {
		
		if (i == selection)
			tmp = TTF_RenderText_Blended(Sans, labels[i], blue);
		else
			tmp = TTF_RenderText_Blended(Sans, labels[i], Black);
		
		LabelText[i] = SDL_CreateTextureFromSurface(renderer, tmp);
		SDL_FreeSurface(tmp);
	}
}

void MenuAction() // Add your menu events here
{
	switch (selection) {
		case 0:
			DestroyMenu();
			paused = false;
			break;
		case 1:
			GameActive = false;
			break;
		default:
			warn("A command does not exist for this label");

	}
}

void DestroyMenu()
{
	for (int i = 0; i < LabelCount; i++)
		SDL_DestroyTexture(LabelText[i]);
}

void MenuEventHandle()
{
	
	const Uint8* keyboard_state = SDL_GetKeyboardState(NULL);
	SDL_PumpEvents();
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

			case SDL_KEYDOWN:
				if (keyboard_state[SDL_SCANCODE_DOWN] && event.key.repeat == 0 && selection != LabelCount -1) {
					selection += 1;
					DestroyMenu();
					CreateMenu();
				}
				else if (keyboard_state[SDL_SCANCODE_P]) {
					DestroyMenu();
					paused = false;
				}
				else if (keyboard_state[SDL_SCANCODE_UP] && event.key.repeat == 0 && selection != 0) {
					selection -= 1;
					DestroyMenu();
					CreateMenu();
				}

				if (keyboard_state[SDL_SCANCODE_SPACE]) {
					MenuAction();
				}

			break;
			
		}
	}
}