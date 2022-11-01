#include <FE_Common.h>
#include <SDL.h>
#include <SDL_image.h>

#include "ui/include/include.h"
#include "include/keymap.h"

void FE_Client_Init(FE_InitConfig *InitConfig)
{
	if (SDL_Init(SDL_INIT_EVERYTHING) == 0)
	{			
		// Check that everything has loaded in correctly
		PresentGame->Window = SDL_CreateWindow(
			InitConfig->WindowTitle,
			SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
			InitConfig->WindowWidth, InitConfig->WindowHeight,
			SDL_WINDOW_ALLOW_HIGHDPI
		);
		
		PresentGame->WindowHeight = InitConfig->WindowHeight;
		PresentGame->WindowWidth = InitConfig->WindowWidth;

		if (PresentGame->Window)
			info("Window Created");
		else 
			error("Window could not be created! SDL_Error: %s", SDL_GetError());
		
		if (InitConfig->Vsync)
			PresentGame->Renderer = SDL_CreateRenderer(PresentGame->Window, -1, SDL_RENDERER_PRESENTVSYNC);
		else
			PresentGame->Renderer = SDL_CreateRenderer(PresentGame->Window, -1, SDL_RENDERER_ACCELERATED);
		

		if (PresentGame->Renderer)
			info("Renderer Created");
		else
			error("Renderer could not be created! SDL_Error: %s", SDL_GetError());

		if (!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0"))
			warn("Nearest texture filtering not enabled! SDL_Error: %s", SDL_GetError());

		if (!SDL_SetHint(SDL_HINT_RENDER_BATCHING, "1"))
			warn("Texture batching could not be enabled! SDL_Error: %s", SDL_GetError());

		if (IMG_Init(IMG_INIT_PNG) == 0)
    		error("IMG Failed to initialize. SDL_Error: %s", IMG_GetError());

		if (TTF_Init() != 0)
			error("TTF Failed to initialize. SDL_Error: %s", TTF_GetError());

		PresentGame->font = FE_Font_Load(InitConfig->default_font, 24);
		
		FE_Console_Init();
		FE_UI_InitUI();
		FE_Key_Init();

		info("FedoraEngine started successfully");

		FE_GameInitialised = true;
		PresentGame->GameActive = true;

	} else {
		error("Could not start SDL. SDL_Error: %s", SDL_GetError());
		FE_Clean();
	}
}