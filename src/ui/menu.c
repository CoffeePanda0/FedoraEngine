#include "../include/game.h"

static FE_Texture *MenuTexture;
static FE_Camera *Camera;
static FE_ParticleSystem *Particles;

static inline int midx(int w)
{
    return (PresentGame->Window_width / 2) - (w / 2);
}

static inline int midy(int h)
{
    return (PresentGame->Window_height / 2) - (h / 2);
}

static void LoadMap_()
{
    FE_StartGame(FE_Messagebox_GetText());
}

static void LoadMap()
{
    FE_Messagebox_Show("Load Map", "Enter Map name", MESSAGEBOX_TEXTBOX);
    FE_Messagebox_AddCallback(&LoadMap_, NULL);
}

void FE_MenuPage_Main()
{
    FE_UI_Container *container = FE_UI_CreateContainer(midx(400), midy(500), 400, 500, "FedoraEngine", false);
    FE_UI_AddElement(FE_UI_CONTAINER, container);

    FE_UI_Button *start_btn = FE_UI_CreateButton("Load Map", 0, 0, BUTTON_LARGE, &LoadMap, NULL);
    FE_UI_Button *editor_btn = FE_UI_CreateButton("Map Editor", 0, 0, BUTTON_LARGE, &FE_StartEditor, NULL);
    FE_UI_Button *quit_btn = FE_UI_CreateButton("Quit", 0, 0, BUTTON_LARGE, &FE_Clean, NULL);

    FE_UI_AddChild(container, FE_UI_BUTTON, start_btn, FE_LOCATION_CENTRE);
    FE_UI_AddChild(container, FE_UI_BUTTON, editor_btn, FE_LOCATION_CENTRE);
    FE_UI_AddContainerSpacer(container, 100);
    FE_UI_AddChild(container, FE_UI_BUTTON, quit_btn, FE_LOCATION_CENTRE);
}

void FE_Menu_Render()
{
    SDL_SetRenderDrawColor(PresentGame->Renderer, 0, 0, 0, 255);
	SDL_RenderClear(PresentGame->Renderer);

    SDL_RenderCopy(PresentGame->Renderer, MenuTexture->Texture, NULL, NULL);

    FE_UpdateParticles();
    FE_RenderParticles(Camera);

    FE_UI_Render();
    FE_Console_Render();

    SDL_RenderPresent(PresentGame->Renderer);
}

void FE_Menu_LoadMenu(const char *page)
{
    PresentGame->GameState = GAME_STATE_MENU;
    FE_CleanAll();

    MenuTexture = FE_LoadResource(FE_RESOURCE_TYPE_TEXTURE, "game/ui/menu.jpg");
    Camera = FE_CreateCamera();
    
    Particles = FE_CreateParticleSystem(
		(SDL_Rect){0, -20, PresentGame->Window_width, 20}, // Position for the whole screen, slightly above the top to create more random
		300, // Emission rate
		3000, // Max particles
		10000, // Max lifetime
		true, // Particles to respawn once they go off screen
		"snow.png", // Texture
		(Vector2D){15, 15}, // Max size of each particle
		(Vector2D){10, 3}, // Set initial velocity so particle doesn't float until they accelerate
		true
	);

    if ((mstrcmp(page, "Main") == 0))
        FE_MenuPage_Main();

}

void FE_Menu_EventHandler()
{
    SDL_PumpEvents();
	SDL_Event event;
    const Uint8* keyboard_state = SDL_GetKeyboardState(NULL);

    while (SDL_PollEvent(&event)) {
        if (FE_UI_HandleEvent(&event, keyboard_state))
            break;
    }
}