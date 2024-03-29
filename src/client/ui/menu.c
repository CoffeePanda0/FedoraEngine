#include <FE_Client.h>
#include "../editor/editor.h"
#include "../../common/net/include/net.h"


static FE_Texture *MenuTexture;
static FE_Camera *Camera;
static FE_ParticleSystem *Particles;

struct JoinInfo {
    FE_UI_Textbox *addr;
    FE_UI_Textbox *port;
    FE_UI_Textbox *username;
};
static struct JoinInfo joininfo;

void FE_Menu_LoadMenu(const char *page);

static void JoinAction()
{
    FE_Multiplayer_InitClient(joininfo.addr->content, atoi(joininfo.port->content), joininfo.username->content);
}

static void FE_MenuPage_JoinGame()
{
    /* Firstly, check if a file containing last server info exists */
    char *lastserver = 0;
    char *lastport = 0;
    char *lastusername = 0;
    if (FE_File_DirectoryExists("serverinfo.txt")) {
        FILE *f = fopen("serverinfo.txt", "r");
        if (f) {
            char line[128];
            while (fgets(line, 128, f)) {
                line[strlen(line) - 1] = 0; // strip newline
                if (!lastserver) lastserver = mstrndup(line, 20);
                else if (!lastport) lastport = mstrndup(line, 6);
                else if (!lastusername) lastusername = mstrndup(line, 18);
            }
            fclose(f);
        }
    }

    /* Create UI */
    FE_UI_Container *container = FE_UI_CreateContainer(midx(400), midy(500), 400, 500, "Join Game", false);
    FE_UI_AddElement(FE_UI_CONTAINER, container);

    /* Autofill with previous connection info if exists*/
    FE_UI_Textbox *addr = FE_UI_CreateTextbox(0, 0, 256, lastserver == 0 ? "127.0.0.1" : lastserver);
    FE_UI_Textbox *port = FE_UI_CreateTextbox(0, 0, 256, lastport == 0 ? "7777" : lastport);
    FE_UI_Textbox *username = FE_UI_CreateTextbox(0,0, 256, lastusername == 0 ? "player" : lastusername);
    FE_UI_Button *host_btn = FE_UI_CreateButton("Join", 0, 0, BUTTON_LARGE, &JoinAction, 0);
    FE_UI_Button *back_btn = FE_UI_CreateButton("Back", 0, 0, BUTTON_LARGE, &FE_Menu_LoadMenu, "Main");

    FE_UI_AddChild(container, FE_UI_LABEL, FE_UI_CreateLabel(0, "Server Address", 256, 0, 0, COLOR_WHITE), FE_LOCATION_CENTRE);
    FE_UI_AddChild(container, FE_UI_TEXTBOX, addr, FE_LOCATION_CENTRE);
    FE_UI_AddChild(container, FE_UI_LABEL, FE_UI_CreateLabel(0, "Server Port", 256, 0, 0, COLOR_WHITE), FE_LOCATION_CENTRE);
    FE_UI_AddChild(container, FE_UI_TEXTBOX, port, FE_LOCATION_CENTRE);
    FE_UI_AddChild(container, FE_UI_LABEL, FE_UI_CreateLabel(0, "Username", 256, 0, 0, COLOR_WHITE), FE_LOCATION_CENTRE);
    FE_UI_AddChild(container, FE_UI_TEXTBOX, username, FE_LOCATION_CENTRE);
    FE_UI_AddChild(container, FE_UI_BUTTON, host_btn, FE_LOCATION_CENTRE);
    FE_UI_AddContainerSpacer(container, 64);
    FE_UI_AddChild(container, FE_UI_BUTTON, back_btn, FE_LOCATION_CENTRE);

    /* Create struct so that we can keep track of the entered data when the UI is reset */
    joininfo.addr = addr;
    joininfo.port = port;
    joininfo.username = username;
    
    /* Free loaded server info */
    if (lastserver) free(lastserver);
    if (lastport) free(lastport);
    if (lastusername) free(lastusername);
}

static void LoadEditor(char *m)
{
    if (m) {
        FE_Editor_Init(FE_Messagebox_GetText());
    } else {
        FE_Messagebox_Show("Load Editor", "Enter Map name", MESSAGEBOX_TEXTBOX);
        FE_Messagebox_AddCallback(&LoadEditor, "editor");
    }
}

static void FE_MenuPage_Editor()
{
    FE_UI_Container *container = FE_UI_CreateContainer(midx(400), midy(500), 400, 500, "Map Editor", false);
    FE_UI_AddElement(FE_UI_CONTAINER, container);

    FE_UI_Button *new_btn = FE_UI_CreateButton("New Map", 0, 0, BUTTON_LARGE, &FE_Editor_Init, NULL);
    FE_UI_Button *load_btn = FE_UI_CreateButton("Load Map", 0, 0, BUTTON_LARGE, &LoadEditor, 0);
    FE_UI_Button *back_btn = FE_UI_CreateButton("Back", 0, 0, BUTTON_LARGE, &FE_Menu_LoadMenu, "Main");

    FE_UI_AddChild(container, FE_UI_BUTTON, new_btn, FE_LOCATION_CENTRE);
    FE_UI_AddChild(container, FE_UI_BUTTON, load_btn, FE_LOCATION_CENTRE);
    FE_UI_AddChild(container, FE_UI_BUTTON, back_btn, FE_LOCATION_CENTRE);
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

static void KeyChange(char *key)
{
    SDL_Scancode scancode = 0;
    while (scancode == 0) {
        // Wait for next input, take it in as scancode
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_KEYDOWN) {
                scancode = event.key.keysym.scancode;
                break;
            }
        }
    }
    if (FE_Key_Assign(key, scancode))
        FE_Menu_LoadMenu("Keyeditor");
}

static FE_UI_Label *GenKeyLabel(const char *input)
{
    char *label = xmalloc(sizeof(char) * 100);
    char *key = (char*)SDL_GetKeyName(SDL_GetKeyFromScancode(FE_Key_Get(input)));
    sprintf(label, "%s: %s", (char*)input, key);
    
    FE_UI_Label *l = FE_UI_CreateLabel(PresentGame->Client->Font, label, 300, 0, 0, COLOR_WHITE);
    free(label);
    return l;
}

static void FE_MenuPage_KeyEditor()
{
    FE_UI_Container *container = FE_UI_CreateContainer(midx(400), midy(500), 400, 500, "FedoraEngine", false);
    FE_UI_AddElement(FE_UI_CONTAINER, container);

    FE_UI_Button *return_btn = FE_UI_CreateButton("Return", 0, 0, BUTTON_SMALL, &FE_Menu_LoadMenu, "Main");
    FE_UI_Button *save_btn = FE_UI_CreateButton("Save", 0, 0, BUTTON_SMALL, &FE_Key_Save, NULL);

    FE_UI_Label *enter_l = GenKeyLabel("JUMP");
    FE_UI_Label *left_l = GenKeyLabel("LEFT");
    FE_UI_Label *right_l = GenKeyLabel("RIGHT");
    FE_UI_Label *zoomin_l = GenKeyLabel("ZOOM IN");
    FE_UI_Label *zoomout_l = GenKeyLabel("ZOOM OUT");

    FE_UI_Button *enter_btn = FE_UI_CreateButton("Change", 0, 0, BUTTON_SMALL, &KeyChange, "JUMP");
    FE_UI_Button *left_btn = FE_UI_CreateButton("Change", 0, 0, BUTTON_SMALL, &KeyChange, "LEFT");
    FE_UI_Button *right_btn = FE_UI_CreateButton("Change", 0, 0, BUTTON_SMALL, &KeyChange, "RIGHT");
    FE_UI_Button *zoomin_btn = FE_UI_CreateButton("Change", 0, 0, BUTTON_SMALL, &KeyChange, "ZOOM IN");
    FE_UI_Button *zoomout_btn = FE_UI_CreateButton("Change", 0, 0, BUTTON_SMALL, &KeyChange, "ZOOM OUT");

    FE_UI_AddChild(container, FE_UI_BUTTON, return_btn, FE_LOCATION_CENTRE);
    FE_UI_AddChild(container, FE_UI_BUTTON, save_btn, FE_LOCATION_CENTRE);
    FE_UI_AddContainerSpacer(container, 10);

    FE_UI_AddChild(container, FE_UI_LABEL, enter_l, FE_LOCATION_CENTRE);
    FE_UI_AddChild(container, FE_UI_BUTTON, enter_btn, FE_LOCATION_CENTRE);
    FE_UI_AddContainerSpacer(container, 10);

    FE_UI_AddChild(container, FE_UI_LABEL, left_l, FE_LOCATION_CENTRE);
    FE_UI_AddChild(container, FE_UI_BUTTON, left_btn, FE_LOCATION_CENTRE);
    FE_UI_AddContainerSpacer(container, 10);

    FE_UI_AddChild(container, FE_UI_LABEL, right_l, FE_LOCATION_CENTRE);
    FE_UI_AddChild(container, FE_UI_BUTTON, right_btn, FE_LOCATION_CENTRE);
    FE_UI_AddContainerSpacer(container, 10);

    FE_UI_AddChild(container, FE_UI_LABEL, zoomin_l, FE_LOCATION_CENTRE);
    FE_UI_AddChild(container, FE_UI_BUTTON, zoomin_btn, FE_LOCATION_CENTRE);
    FE_UI_AddContainerSpacer(container, 10);

    FE_UI_AddChild(container, FE_UI_LABEL, zoomout_l, FE_LOCATION_CENTRE);
    FE_UI_AddChild(container, FE_UI_BUTTON, zoomout_btn, FE_LOCATION_CENTRE);
}

void FE_MenuPage_Main()
{
    FE_UI_Container *container = FE_UI_CreateContainer(midx(400), midy(500), 400, 500, "FedoraEngine", false);
    FE_UI_AddElement(FE_UI_CONTAINER, container);

    FE_UI_Button *start_btn = FE_UI_CreateButton("Load Map", 0, 0, BUTTON_LARGE, &LoadMap, NULL);
    FE_UI_Button *editor_btn = FE_UI_CreateButton("Map Editor", 0, 0, BUTTON_LARGE, &FE_Menu_LoadMenu, "Editor");
    FE_UI_Button *multiplayer_btn = FE_UI_CreateButton("Join Game", 0, 0, BUTTON_LARGE, &FE_Menu_LoadMenu, "JoinGame");
    FE_UI_Button *key_btn = FE_UI_CreateButton("Keybinds", 0, 0, BUTTON_LARGE, &FE_Menu_LoadMenu, "Keyeditor");
    FE_UI_Button *quit_btn = FE_UI_CreateButton("Quit", 0, 0, BUTTON_LARGE, &FE_Clean, NULL);

    FE_UI_AddChild(container, FE_UI_BUTTON, start_btn, FE_LOCATION_CENTRE);
    FE_UI_AddChild(container, FE_UI_BUTTON, editor_btn, FE_LOCATION_CENTRE);
    FE_UI_AddChild(container, FE_UI_BUTTON, multiplayer_btn, FE_LOCATION_CENTRE);
    FE_UI_AddChild(container, FE_UI_BUTTON, key_btn, FE_LOCATION_CENTRE);
    FE_UI_AddContainerSpacer(container, 64);
    FE_UI_AddChild(container, FE_UI_BUTTON, quit_btn, FE_LOCATION_CENTRE);
}

void FE_Menu_Render()
{
    SDL_SetRenderDrawColor(PresentGame->Client->Renderer, 0, 0, 0, 255);
	SDL_RenderClear(PresentGame->Client->Renderer);

    SDL_RenderCopy(PresentGame->Client->Renderer, MenuTexture->Texture, NULL, NULL);

    FE_Particles_Update(Camera);
    FE_Particles_Render(Camera);

    FE_UI_Render();
    FE_Console_Render();

    SDL_RenderPresent(PresentGame->Client->Renderer);
}

void FE_Menu_LoadMenu(const char *page)
{
    PresentGame->GameState = GAME_STATE_MENU;
    FE_CleanAll();

    MenuTexture = FE_LoadResource(FE_RESOURCE_TYPE_TEXTURE, "game/ui/menu.jpg");
    Camera = FE_CreateCamera();
    
    Particles = FE_ParticleSystem_Create(
		(SDL_Rect){0, -20, PresentGame->WindowWidth, 20}, // Position for the whole screen, slightly above the top to create more random
		300, // Emission rate
		3000, // Max particles
		10000, // Max lifetime
		true, // Particles to respawn once they go off screen
		"snow.png", // Texture
		(vec2){15, 15}, // Max size of each particle
		(vec2){10, 3}, // Set initial velocity so particle doesn't float until they accelerate
		true
	);

    // if any server messages exist, display and then clear them
    if (PresentGame->DisconnectInfo.set) {
        PresentGame->DisconnectInfo.set = false;
        switch (PresentGame->DisconnectInfo.type) {
            case DISC_SERVER:
                FE_Messagebox_Show("Multiplayer", "Connection to server timed out", MESSAGEBOX_TEXT);
            break;
            case DISC_KICK:
                FE_Messagebox_Show("You were kicked", PresentGame->DisconnectInfo.reason, MESSAGEBOX_TEXT);
            break;
            case DISC_BAN:
                FE_Messagebox_Show("You were banned", PresentGame->DisconnectInfo.reason, MESSAGEBOX_TEXT);
            break;
            case DISC_NOCON:
                FE_Messagebox_Show("Connection Denied", "You have been banned from this server", MESSAGEBOX_TEXT);
            break;
        }
    }


    if ((mstrcmp(page, "Main") == 0))
        FE_MenuPage_Main();
    else if ((mstrcmp(page, "Keyeditor") == 0))
        FE_MenuPage_KeyEditor();
    else if ((mstrcmp(page, "Editor") == 0))
        FE_MenuPage_Editor();
    else if ((mstrcmp(page, "JoinGame") == 0))
        FE_MenuPage_JoinGame();
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
