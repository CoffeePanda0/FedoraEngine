#include "game.h"

void SaveGame(const char *name)
{
	FILE *f;

	f = fopen(name, "wb");
	if (!f) {
		warn("Unable to save file");
		return;
	}

	uint16_t len = strlen(lua_script);

	fwrite(&Health, sizeof(int), 1, f);
	fwrite(&CollRect.x, sizeof(int), 1, f);
	fwrite(&CollRect.y, sizeof(int), 1, f);
	fwrite(&scrollam, sizeof(int), 1, f);
	fwrite(&hscrollam, sizeof(int), 1, f);
	fwrite(&len, sizeof(uint16_t), 1, f);
	fwrite(lua_script, strlen(lua_script), 1, f);
	info("Saved game!");
	fclose(f);
}

void LoadSave(const char *name)
{
	FILE *f;
	f = fopen(name, "rb");
	if (!f) {
		warn("Save does not exist or is corrupted");
		return;
	}

	int tmp_x = 0;
	int tmp_y = 0;
	int tmp_scrollam = 0; // we need these temp values as initialising a new map resets scrolling
	int tmp_hscrollam = 0;


	uint16_t len = 0; // used for reading in size of string

	fread(&Health, sizeof(int), 1, f);
	fread(&tmp_x, sizeof(int), 1, f);
	fread(&tmp_y, sizeof(int), 1, f);
	fread(&tmp_scrollam, sizeof(int), 1, f);
	fread(&tmp_hscrollam, sizeof(int), 1, f);
	fread(&len, sizeof(uint16_t), 1, f);
	fread(lua_script, len, 1, f);

	HealthChange(0); // refresh UI
	RunLuaFile(lua_script);

	CollRect.x = tmp_x;
	CollRect.y = tmp_y;
	hscrollam = tmp_hscrollam;
	scrollam = tmp_scrollam;

	playerRect.x = tmp_x - tmp_scrollam;
	playerRect.y = tmp_y - tmp_hscrollam;
	
	fclose(f);
	
	info("Loaded save!");
}