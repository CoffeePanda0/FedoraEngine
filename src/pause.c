#include "core/include/include.h"
#include "ui/include/include.h"
#include "ui/include/menu.h"
#include "include/init.h"

static FE_UIList *GameList = 0;
static FE_Texture *BGTexture = 0;

void FE_Pause_Render()
{
    SDL_SetRenderDrawColor(PresentGame->Renderer, 0, 0, 0, 255);
	SDL_RenderClear(PresentGame->Renderer);

    SDL_RenderCopy(PresentGame->Renderer, BGTexture->Texture, NULL, NULL);

    FE_UI_Render();
    FE_Console_Render();

    SDL_RenderPresent(PresentGame->Renderer);
}

void FE_ResumeGame()
{
    // Free the paused elements and restore the game elements
    FE_UI_ClearElements(PresentGame->UIConfig.ActiveElements);
    free(PresentGame->UIConfig.ActiveElements);

    PresentGame->UIConfig.ActiveElements = GameList;

    PresentGame->GameState = GAME_STATE_PLAY;

    FE_DestroyResource(BGTexture->path);
    
    info("Game resumed");
}

static void PauseUI()
{
    FE_UI_Container *container = FE_UI_CreateContainer(midx(400), midy(500), 400, 500, "Paused", false);
    FE_UI_AddElement(FE_UI_CONTAINER, container);

    FE_UI_Button *resume_btn = FE_UI_CreateButton("Resume", 0, 0, BUTTON_LARGE, &FE_ResumeGame, NULL);
    FE_UI_Button *menu_btn = FE_UI_CreateButton("Main Menu", 0, 0, BUTTON_LARGE, &FE_Menu_LoadMenu, "Main");
    FE_UI_Button *quit_btn = FE_UI_CreateButton("Quit", 0, 0, BUTTON_LARGE, &FE_Clean, NULL);

    FE_UI_AddChild(container, FE_UI_BUTTON, resume_btn, FE_LOCATION_CENTRE);
    FE_UI_AddContainerSpacer(container, 100);
    FE_UI_AddChild(container, FE_UI_BUTTON, menu_btn, FE_LOCATION_CENTRE);
    FE_UI_AddContainerSpacer(container, 100);
    FE_UI_AddChild(container, FE_UI_BUTTON, quit_btn, FE_LOCATION_CENTRE);

    BGTexture = FE_LoadResource(FE_RESOURCE_TYPE_TEXTURE, "game/ui/pause.png");
}

void FE_PauseGame()
{
    PresentGame->GameState = GAME_STATE_PAUSE;   
    // Temporarily move all active UI to separate list
    GameList = PresentGame->UIConfig.ActiveElements;

    // Clear all active UI
    PresentGame->UIConfig.ActiveElements = 0;
    FE_UI_ControlContainerLocked = false;

    FE_UI_InitUI();
    PauseUI();
    info("Game paused");
}