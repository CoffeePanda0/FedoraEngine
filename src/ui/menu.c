#include "../include/game.h"

// Contains preset menus

void (*MenuPage)();

static void loadmap_buf(FE_TextBox *t)
{
	if (!t->content || strlen(t->content) == 0)
		FE_ShowMessageBox("Warning", "Map name cannot be empty");
	else
		FE_StartGame(t->content);
}

void FE_Menu_ShowMaps()
{
	FE_FreeUI();
	
	FE_GameState = GAME_STATE_MENU;
	MenuPage = &FE_Menu_ShowMaps;

	FE_CreateLabel("FedoraEngine - Load Map", 142, 0, COLOR_BLACK);
	FE_CreateButton("Back", 156,400, BUTTON_MEDIUM, &FE_Menu_MainMenu, NULL);
	FE_CreateLabel("Enter Map Name", 72, 90, COLOR_BLACK);
	FE_TextBox *input_box = FE_CreateTextBox(72, 128, 356, 40, "");
	FE_ForceActiveTextBox(input_box);
	FE_CreateButton("Load", 156,198, BUTTON_MEDIUM, &loadmap_buf, input_box);

}

void FE_Menu_MainMenu()
{
	FE_FreeUI();

	FE_GameState = GAME_STATE_MENU;
	MenuPage = &FE_Menu_MainMenu;

	FE_CreateLabel("FedoraEngine - Main Menu", 128, 0, COLOR_BLACK);
	FE_CreateUIObject(192, 50, 128, 128, "jeffy.png");
	FE_CreateButton("Start Game", 156,200, BUTTON_MEDIUM, &FE_Menu_ShowMaps, NULL);
	FE_CreateButton("Options", 156,275, BUTTON_MEDIUM, &FE_Menu_OptionsMenu, NULL);
	FE_CreateButton("Map Editor", 156,350, BUTTON_MEDIUM, &FE_StartEditor, NULL);
	FE_CreateButton("Exit", 156,425, BUTTON_MEDIUM, &FE_Clean, NULL);
}

static void ChangeVol(int amount)
{
	FE_ChangeVolume(amount);
	FE_Menu_OptionsMenu();
}

void FE_Menu_OptionsMenu()
{
	FE_FreeUI();

	FE_GameState = GAME_STATE_MENU;
	MenuPage = &FE_Menu_OptionsMenu;

	FE_CreateLabel("FedoraEngine - Options Menu", 112, 0, COLOR_BLACK);

	FE_CreateLabel("Volume", 140, 156, COLOR_BLACK);

	FE_CreateButton("-", 156, 200, BUTTON_TINY, &ChangeVol, (void *)-10);
	FE_CreateButton("+", 256, 200, BUTTON_TINY, &ChangeVol, (void *)10);

	char *MusicVolStr = IntToSTR(FE_MusicVolume);
	FE_CreateLabel(MusicVolStr, 210, 200, COLOR_BLACK);
	free(MusicVolStr);

	FE_CreateCheckbox("Muted", 150, 250, FE_MusicMuted, &FE_MuteAudio, NULL);

	FE_CreateButton("Back", 156,400, BUTTON_MEDIUM, &FE_Menu_MainMenu, NULL);

}