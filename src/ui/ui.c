#include "../core/include/fedoraengine.h"
#include "../core/include/texture.h"
#include "../include/init.h"
#include "../entity/include/timer.h"
#include "include/ui.h"

void FE_RenderUI()
{
	FE_RenderUIObjects();
	FE_RenderCheckBoxes();
	FE_RenderTextBox();
	FE_RenderLabels();
	FE_RenderButtons();
	FE_RenderConsole();
	FE_UpdateTimers();
	FE_RenderMessageBox();
}

void FE_MenuEventHandle() // we use a different event handler so it doesnt move the player
{
	const Uint8* keyboard_state = SDL_GetKeyboardState(NULL);
	SDL_PumpEvents();
	SDL_Event event;

	while (SDL_PollEvent(&event)) {
		if (PresentGame->ConsoleVisible) {
			FE_HandleConsoleInput(&event, keyboard_state);
		} else {
			switch (event.type)
			{
				case SDL_QUIT:
					FE_Clean();
				break;

				case SDL_KEYDOWN:
						if (keyboard_state[SDL_SCANCODE_GRAVE] && event.key.repeat == 0) {
							SDL_StartTextInput();
							PresentGame->StartedInput = true;
							FE_ConsoleShow();
						}
						else if (PresentGame->DialogueActive)
							FE_DialogueInteract();
						else {
							FE_TextBox *t = FE_GetActiveTextBox();
							if (t) {
								FE_UpdateTextBox(event.key.keysym.sym);
							}
						}
				break;

				case SDL_MOUSEBUTTONDOWN:
					FE_UI_HandleClick(&event);
				break;
			}
		}
	}
}

void FE_CentreRect(FE_UI_CENTRE_TYPE type, SDL_Rect *r)
{
	switch (type) {
		case FE_UI_CENTRE_NONE:
			break;
		case FE_UI_CENTRE_HORIZONTAL:
			r->x = (PresentGame->window_width - r->w) / 2;
			break;
		case FE_UI_CENTRE_VERTICAL:
			r->y = (PresentGame->window_height - r->h) / 2;
			break;
		case FE_UI_CENTRE_ALL:
			r->x = (PresentGame->window_width - r->w) / 2;
			r->y = (PresentGame->window_height - r->h) / 2;
		break;
	}
}

void FE_FreeUI() // Destroys all UI objects and dialogue
{
	FE_DestroyMessageBox();
	FE_FreeDialogue();
	FE_CleanTextBoxes();
	FE_CleanLabels();
	FE_CleanCheckBoxes();
	FE_CleanButtons();
	FE_CleanUIObjects();
}

void FE_RenderMenu()
{
    SDL_RenderClear(PresentGame->renderer);
	SDL_SetRenderDrawColor(PresentGame->renderer, 0, 255, 255, 0);
	FE_RenderUI();
	SDL_RenderPresent(PresentGame->renderer);
}