#include "editor.h"
#include "../ui/include/ui.h"
#include "../core/include/include.h"
#include "../include/init.h"

#define ATLAS "tileset.png"

static size_t selectedtexture;

FE_Texture **editor_backgrounds; // The background atlas (stores 10)
static size_t bgcount; // amount of background textures loaded
static size_t selectedbackground; // the index of the currently selected background

static bool mode = true; // true = map, false = bg
static uint16_t rotation = 0; // persistent rotation when placing new tiles

static FE_LoadedMap newmap; // Current map for rendering from

/* UI objects for layout */
static FE_Camera *camera;
static FE_Label *coord;

static FE_Texture *spawntexture;
static FE_Texture *endtexture;

static bool saving = false;

static bool drag_place = false;
static bool drag_delete = false;

static int map_max_width = 4096;
static int map_max_height = 1024;

// TODO: loading previous maps, tile bg, change tilesize/gravity, nice UI, click anywhere in tile to delete, allow multiple atlas

void FE_RenderEditor()
{
    SDL_RenderClear(PresentGame->renderer);
	SDL_SetRenderDrawColor(PresentGame->renderer, 0, 0, 0, 0);

	/* render background, applying zoom */
	SDL_Rect bgrect = (SDL_Rect){0,0,0,0};
	FE_QueryTexture(newmap.bg, &bgrect.w ,&bgrect.h);
	bgrect.w *= camera->zoom;
	bgrect.h *= camera->zoom;
	bgrect.x -= camera->x * camera->zoom;
	bgrect.y -= camera->y * camera->zoom;
	SDL_RenderCopy(PresentGame->renderer, newmap.bg->Texture, NULL, &bgrect);

	// render all tiles
	for (size_t i = 0; i < newmap.tilecount; i++) {
		SDL_Rect r = (SDL_Rect){newmap.tiles[i].position.x, newmap.tiles[i].position.y, newmap.tilesize, newmap.tilesize};
		SDL_Rect src = FE_GetTexturePosition(newmap.atlas, newmap.tiles[i].texture_index);
		FE_RenderCopyEx(camera, false, newmap.atlas->atlas, &src, &r, newmap.tiles[i].rotation, SDL_FLIP_NONE);
	}

	// render spawn
	if (!FE_VecNULL(newmap.PlayerSpawn)) {
		SDL_Rect spawnrect = (SDL_Rect){newmap.PlayerSpawn.x, newmap.PlayerSpawn.y, newmap.tilesize, newmap.tilesize};
		FE_RenderCopy(camera, false, spawntexture, NULL, &spawnrect);
	}

	// render end flag
	if (!(newmap.EndFlag.x == -1 && newmap.EndFlag.y == -1)) {
		SDL_Rect endflagrect = (SDL_Rect){newmap.EndFlag.x, newmap.EndFlag.y, newmap.tilesize, newmap.tilesize};
		FE_RenderCopy(camera, false, endtexture, NULL, &endflagrect);
	}

	// render grid on map with camera
	for (int i = 0; i < map_max_width + PresentGame->window_width; i += newmap.tilesize)
		FE_RenderDrawLine(camera, i, 0, i, map_max_height + PresentGame->window_width, COLOR_WHITE);
	for (int i = 0; i < map_max_height + PresentGame->window_height; i += newmap.tilesize)
		FE_RenderDrawLine(camera, 0, i, map_max_width + PresentGame->window_width, i, COLOR_WHITE);


	// render panel for UI to go over
	FE_RenderRect(&(SDL_Rect){0, 0, PresentGame->window_width, 42}, COLOR_WHITE);
	FE_RenderUI();


	// render thumbnail previewing currently selected texture
	SDL_Rect thumbnail = {475, 6, 32, 32};
	if (mode) {
		SDL_Rect src = FE_GetTexturePosition(newmap.atlas, selectedtexture);
		SDL_RenderCopy(PresentGame->renderer, newmap.atlas->atlas->Texture, &src, &thumbnail);
	} else
		SDL_RenderCopy(PresentGame->renderer, editor_backgrounds[selectedtexture]->Texture, NULL, &thumbnail);


	SDL_RenderPresent(PresentGame->renderer);
}

static void GridSnap(int *x, int *y)
{
	*x = (*x / newmap.tilesize) * newmap.tilesize;
	*y = (*y / newmap.tilesize) * newmap.tilesize;
}

static void UpdateCoordinates() // updates the label position on screen
{
	char buffer[64];
	sprintf(buffer, "X: %i, Y: %i", (int)camera->x, (int)camera->y);
	FE_UpdateLabel(coord, buffer);
}

static void DeleteTile(int x, int y)
{
	// verify tile exists
	if (newmap.tilecount == 0)
		return;

	GridSnap(&x, &y);

	// find index of tile
	int index = -1;
	for (size_t i = 0; i < newmap.tilecount; i++) {
		if (newmap.tiles[i].position.x == x && newmap.tiles[i].position.y == y) {
			index = i;
			break;
		}
	}

	if (index == -1)
		return;

	// delete tile from newmap
	if (newmap.tilecount > 1) {
		FE_Map_Tile *temp = newmap.tiles;
		
		newmap.tiles = xmalloc(sizeof(FE_Map_Tile) * (--newmap.tilecount));
		memcpy(newmap.tiles, temp, sizeof(FE_Map_Tile) * index);
		memcpy(newmap.tiles + index, temp + index + 1, sizeof(FE_Map_Tile) * (newmap.tilecount - index));
		xfree(temp);
	} else {
		xfree(newmap.tiles);
		newmap.tiles = 0;
	}

}

static void AddTile(int x, int y)
{
	// snap x and y to a grid using tile size
	GridSnap(&x, &y);

	// check if tile in that location already exists, and it it does then deletes it
	for (size_t i = 0; i < newmap.tilecount; i++) {
		if (newmap.tiles[i].position.x == x && newmap.tiles[i].position.y == y) {
			DeleteTile(x,y);
		}
	}

	// Add tile to newmap
	if (newmap.tilecount == 0)
		newmap.tiles = xmalloc(sizeof(FE_Map_Tile));
	else
		newmap.tiles = xrealloc(newmap.tiles, sizeof(FE_Map_Tile) * (newmap.tilecount + 1));
	newmap.tiles[newmap.tilecount++] = (FE_Map_Tile){selectedtexture, 0, FE_NewVector(x, y)};

	// Apply persistent rotation
	newmap.tiles[newmap.tilecount-1].rotation = rotation;
}

static void SetSpawn(int x, int y)
{
	// snap x and y to a grid using tile size
	GridSnap(&x, &y);

	newmap.PlayerSpawn = FE_NewVector(x, y);
	info("Editor: Spawn set to (%d, %d)", x, y);
}

static void SetEnd(int x, int y)
{
	GridSnap(&x, &y);

	newmap.EndFlag = FE_NewVector(x, y);
	info("Editor: Spawn end flag to (%d, %d)", x, y);
}

static void RotateTile(int x, int y)
{
	// snap x and y to a grid using tile size
	GridSnap(&x, &y);


	// check if tile already exists
	for (size_t i = 0; i < newmap.tilecount; i++) {
		if (newmap.tiles[i].position.x == x && newmap.tiles[i].position.y == y) {
			if (newmap.tiles[i].rotation == 0)
				rotation = 90;
			else if (newmap.tiles[i].rotation == 90)
				rotation = 180;
			else if (newmap.tiles[i].rotation == 180)
				rotation = 270;
			else if (newmap.tiles[i].rotation == 270)
				rotation = 0;
			newmap.tiles[i].rotation = rotation;
			return;
		}
	}
}

static void Save()
{
	// Convert the map to the save format
	FE_Map save = {
		"",
		newmap.gravity,
		ATLAS,
		newmap.atlas->texturesize,
		editor_backgrounds[selectedbackground]->path,
		newmap.tilecount,
		newmap.tilesize,
		newmap.tiles,
		newmap.PlayerSpawn,
		newmap.EndFlag,
	};
	saving = Editor_CallSave(&save);
	saving = false;
}

static void ChangeSelection(size_t sel)
{
	rotation = 0;
	if (mode) { // change selected tile
		selectedtexture = sel;
	} else { // change selected bg
		if (bgcount -1 < sel)
			return;
		selectedbackground = sel;
	}
}

void FE_EventEditorHandler()
{
    const Uint8* keyboard_state = SDL_GetKeyboardState(NULL);
	SDL_PumpEvents();
	SDL_Event event;

	/* Allow user to drag to place tiles */
	if (drag_place) {
		int mouse_x, mouse_y;
		SDL_GetMouseState(&mouse_x, &mouse_y);
		if (mode)
			AddTile(mouse_x + camera->x, mouse_y + camera->y);
		else
			newmap.bg = editor_backgrounds[selectedbackground];
	} else if (drag_delete) {
		int mouse_x, mouse_y;
		SDL_GetMouseState(&mouse_x, &mouse_y);
		DeleteTile(mouse_x + camera->x, mouse_y + camera->y);
	}


    while (SDL_PollEvent(&event)) {
		if (PresentGame->ConsoleVisible) {
			FE_HandleConsoleInput(&event, keyboard_state);
		} else {
			switch (event.type) {
				case SDL_QUIT:
					FE_Clean();
				break;
				case SDL_MOUSEBUTTONDOWN:
					if (event.button.button == SDL_BUTTON_LEFT) {
						if (!FE_UI_HandleClick(&event)) {
							if (!saving) {
								drag_place = true;
								drag_delete = false;
							}
						}
					} else if (event.button.button == SDL_BUTTON_RIGHT) {
						if (!saving) {
							drag_delete = true;
							drag_place = false;
						}
					}
				break;

				case SDL_MOUSEBUTTONUP:
					if (event.button.button == SDL_BUTTON_LEFT) {
						drag_place = false;
					} else if (event.button.button == SDL_BUTTON_RIGHT) {
						drag_delete = false;
					}
				break;

				case SDL_KEYDOWN: // switch between textures and bgs
					if (keyboard_state[SDL_SCANCODE_ESCAPE]) {
						if (!saving) {
							saving = true;
							FE_ShowInputMessageBox("Map Editor", "Enter Map Name", &Save, NULL);
						} else {
							saving = false;
							FE_DestroyMessageBox();
						}
					}
					if (saving && !keyboard_state[SDL_SCANCODE_ESCAPE]) {
						FE_UpdateTextBox(event.key.keysym.sym);
						break;
					}

					// if a num key is pressed
					if (event.key.keysym.sym > 47 && event.key.keysym.sym < 58)
						ChangeSelection(event.key.keysym.sym - 48);
					
					if (keyboard_state[SDL_SCANCODE_Q]) {
						int mouse_x, mouse_y;
						SDL_GetMouseState(&mouse_x, &mouse_y);
						SetSpawn(mouse_x + camera->x, mouse_y + camera->y);
					}
					else if (keyboard_state[SDL_SCANCODE_E]) {
						int mouse_x, mouse_y;
						SDL_GetMouseState(&mouse_x, &mouse_y);
						SetEnd(mouse_x + camera->x, mouse_y + camera->y);
					} else if (keyboard_state[SDL_SCANCODE_R]) {
						int mouse_x, mouse_y;
						SDL_GetMouseState(&mouse_x, &mouse_y);
						RotateTile(mouse_x + camera->x, mouse_y + camera->y);
					}

				break;

			}
		}
	}
	// moving camera (outside of loop so movement is smooth)
	if (keyboard_state[SDL_SCANCODE_W]) {
		FE_MoveCamera(0, -10 * FE_DT_MULTIPLIER, camera);
		UpdateCoordinates();
	} else if (keyboard_state[SDL_SCANCODE_A]) {
		FE_MoveCamera(-10 * FE_DT_MULTIPLIER, 0, camera);
		UpdateCoordinates();
	} else if (keyboard_state[SDL_SCANCODE_S]) {
		FE_MoveCamera(0, 10 * FE_DT_MULTIPLIER, camera);
		UpdateCoordinates();
	} else if (keyboard_state[SDL_SCANCODE_D]) {
		FE_MoveCamera(10 * FE_DT_MULTIPLIER, 0, camera);
		UpdateCoordinates();
	}
}

void FE_CleanEditor()
{
	selectedtexture = 0;
	if (bgcount > 0) {
	// free editor backgrounds
		for (size_t i = 0; i < bgcount; i++)
			FE_DestroyResource(editor_backgrounds[i]->path);
		xfree(editor_backgrounds);

		bgcount = 0;
		selectedbackground = 0;
	}

	mode = true;

	if (spawntexture)
		FE_DestroyResource(spawntexture->path);
	spawntexture = 0;

	if (endtexture)
		FE_DestroyResource(endtexture->path);
	endtexture = 0;

	// free texturepaths
	if (newmap.atlas) {
		FE_DestroyResource(newmap.atlas->atlas->path);
		xfree(newmap.atlas);
	}
	newmap.atlas = 0;

	if (newmap.tiles) {
		xfree(newmap.tiles);
		newmap.tilecount = 0;
	}
	newmap.tiles = 0;
	newmap.PlayerSpawn = VEC_NULL;
	newmap.EndFlag = VEC_NULL;
}

static void Exit()
{
	FE_CleanEditor();
	FE_Menu_MainMenu();
}

static void ChangeMode()
{
	mode = !mode;
}

static void CreateUI()
{
	// UI elements
	FE_CreateButton("<", 10, 7, BUTTON_TINY, &Exit, NULL);
	FE_CreateButton("Clear", 60, 7, BUTTON_TINY, &FE_StartEditor, NULL);
	FE_CreateCheckbox("Tiles", 150, 7, mode, &ChangeMode, NULL);

	coord = FE_CreateLabel(NULL, "X: 0 Y: 0", FE_NewVector(340, 6), COLOR_BLACK);
	spawntexture = FE_LoadResource(FE_RESOURCE_TYPE_TEXTURE, "game/map/spawn.png");
	endtexture = FE_LoadResource(FE_RESOURCE_TYPE_TEXTURE, "game/map/end.png");
}

void FE_StartEditor() // cleans up from other game modes
{
	FE_CleanAll();

    PresentGame->GameState = GAME_STATE_EDITOR;

	if ((bgcount = Editor_LoadBackgrounds()) == 0) {
		warn("Unable to start map editor - No backgrounds found");
		Exit();
		return;
	}
	
	CreateUI();

	newmap.bg = editor_backgrounds[0];
	mode = true;

	camera = FE_CreateCamera();
	camera->x = 0; camera->y = 0;
	camera->x_bound = map_max_width; camera->y_bound = map_max_height;

	info("Editor: Started editor");

	newmap.PlayerSpawn = VEC_NULL;
	newmap.EndFlag = VEC_NULL;
	newmap.tilesize = 64;
	newmap.gravity = 70;
	newmap.atlas = FE_LoadTextureAtlas(ATLAS);
}