#include "../include/game.h"

#define TILE_SIZE 64

SDL_Texture **editor_textures; // The texture atlas (stores 10)
char **editor_texturepaths;
static size_t texturecount;
static size_t selectedtexture;

SDL_Texture **editor_backgrounds; // The background atlas (stores 10)
char **editor_backgroundpaths;
static size_t bgcount;
static size_t selectedbackground;

static bool mode = true; // true = map, false = bg
static bool grid = true;

static FE_Map mapsave; // for exporting to file
static FE_LoadedMap newmap; // for rendering

static SDL_Rect thumbnail;
static FE_Camera camera;
static FE_Label *coord;

static SDL_Texture *spawntexture;
static SDL_Texture *endtexture;

static bool saving = false;

// TODO: free textures if they are deleted and not used in map, loading previous maps, tile bg, drag and click?

void FE_RenderEditor()
{
    SDL_RenderClear(renderer);
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);

	// render background
	SDL_Rect bgrect = (SDL_Rect){0,0,0,0};
	SDL_QueryTexture(newmap.bg, NULL, NULL, &bgrect.w ,&bgrect.h);
	bgrect.x -= camera.x;
	bgrect.y -= camera.y;
	SDL_RenderCopy(renderer, newmap.bg, NULL, &bgrect);

	// render all tiles
	for (size_t i = 0; i < newmap.tilecount; i++) {
		SDL_Rect r = (SDL_Rect){newmap.tiles[i].position.x - camera.x, newmap.tiles[i].position.y - camera.y, TILE_SIZE, TILE_SIZE};
		FE_RenderCopy(newmap.textures[newmap.tiles[i].texture_index], NULL, &r);
	}

	// render spawn
	if (!(newmap.PlayerSpawn.x == -1 && newmap.PlayerSpawn.y == -1)) {
		SDL_Rect spawnrect = (SDL_Rect){newmap.PlayerSpawn.x - camera.x, newmap.PlayerSpawn.y - camera.y, TILE_SIZE, TILE_SIZE};
		FE_RenderCopy(spawntexture, NULL, &spawnrect);
	}

	// render end flag
	if (!(newmap.EndFlag.x == -1 && newmap.EndFlag.y == -1)) {
		SDL_Rect endflagrect = (SDL_Rect){newmap.EndFlag.x - camera.x, newmap.EndFlag.y - camera.y, TILE_SIZE, TILE_SIZE};
		FE_RenderCopy(endtexture, NULL, &endflagrect);
	}

	// render grid on map with camera
	if (grid) {
		SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
		for (int i = 0; i < 4096 + screen_width; i += TILE_SIZE) {
			SDL_RenderDrawLine(renderer, i - camera.x, 0, i - camera.x, screen_height);
		}
		for (int i = 0; i < 1024 + screen_height; i += TILE_SIZE) {
			SDL_RenderDrawLine(renderer, 0, i - camera.y, screen_width, i - camera.y);
		}
	}

	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);

	FE_RenderUI();

	if (mode)
		SDL_RenderCopy(renderer, editor_textures[selectedtexture], NULL, &thumbnail);
	else
		SDL_RenderCopy(renderer, editor_backgrounds[selectedbackground], NULL, &thumbnail);

	SDL_RenderPresent(renderer);
}

static void DeleteTile(int x, int y)
{
	// verify tile exists
	if (newmap.tilecount == 0)
		return;
	
	if (grid) {
		x = (x / TILE_SIZE) * TILE_SIZE;
		y = (y / TILE_SIZE) * TILE_SIZE;
	}

	// find index of tile texture
	int index = -1;
	// size_t used_texture = 0;
	for (size_t i = 0; i < newmap.tilecount; i++) {
		if (newmap.tiles[i].position.x == x && newmap.tiles[i].position.y == y) {
			index = i;
			// used_texture = newmap.tiles[i].texture_index;
			break;
		}
	}

	if (index == -1)
		return;

	// delete tile from newmap
	Map_Tile *temp = newmap.tiles;
	newmap.tiles = xmalloc(sizeof(Map_Tile) * (newmap.tilecount - 1));
	memcpy(newmap.tiles, temp, sizeof(Map_Tile) * index);
	memcpy(newmap.tiles + index, temp + index + 1, sizeof(Map_Tile) * (newmap.tilecount - index - 1));
	xfree(temp);
	newmap.tilecount--;

	// delete tile from mapsave
	temp = mapsave.tiles;
	mapsave.tiles = xmalloc(sizeof(Map_Tile) * (mapsave.tilecount - 1));
	memcpy(mapsave.tiles, temp, sizeof(Map_Tile) * index);
	memcpy(mapsave.tiles + index, temp + index + 1, sizeof(Map_Tile) * (mapsave.tilecount - index - 1));
	xfree(temp);
	mapsave.tilecount--;

	/* check if we need to delete the texture - TODO 
	bool delete = true;
	for (size_t i = 0; i < newmap.tilecount; i++) {
		if (newmap.tiles[i].texture_index == used_texture) {
			delete = false;
			break;
		}
	}
	*/

}
static void SetBG(int x, int y)
{
	if (grid) {
		x = (x / TILE_SIZE) * TILE_SIZE;
		y = (y / TILE_SIZE) * TILE_SIZE;
	}

	if (mapsave.bg_texturepath)
		xfree(mapsave.bg_texturepath);
	mapsave.bg_texturepath = 0;

	newmap.bg = editor_backgrounds[selectedbackground];
	mapsave.bg_texturepath = strdup(editor_backgroundpaths[selectedbackground]);
}

static void AddTile(int x, int y)
{
	// snap x and y to a grid using tile size
	if (grid) {
		x = (x / TILE_SIZE) * TILE_SIZE;
		y = (y / TILE_SIZE) * TILE_SIZE;
	}

	// check if tile already exists
	for (size_t i = 0; i < newmap.tilecount; i++) {
		if (newmap.tiles[i].position.x == x && newmap.tiles[i].position.y == y) {
			// check if new texture exists in mapsave
			int in = -1;
			for (size_t i = 0; i < mapsave.texturecount; i++) {
				if (strcmp(editor_texturepaths[selectedtexture], mapsave.texturepaths[i]) == 0) {
					in = i;
				}
			}
			if (in == -1) {
				// add texture path to mapsave
				if (mapsave.texturecount == 0)
					mapsave.texturepaths = xmalloc(sizeof(char *));
				else
					mapsave.texturepaths = xrealloc(mapsave.texturepaths, sizeof(char*) * (mapsave.texturecount + 1));
				mapsave.texturepaths[mapsave.texturecount] = xcalloc(64, 1);
				mapsave.texturepaths[mapsave.texturecount] = strcpy(mapsave.texturepaths[mapsave.texturecount], editor_texturepaths[selectedtexture]);
				mapsave.texturecount++;

				// add tile texture to newmap
				if (newmap.texturecount == 0)
					newmap.textures = xmalloc(sizeof(SDL_Texture*));
				else
					newmap.textures = xrealloc(newmap.textures, sizeof(SDL_Texture*) * (newmap.texturecount + 1));
				newmap.textures[newmap.texturecount++] = FE_TextureFromFile(mapsave.texturepaths[mapsave.texturecount-1]);
				in = newmap.texturecount -1;
			}
			newmap.tiles[i].texture_index = in;
			return;
		}
	}
	
	int in = -1;
	// see if selected texture has been added to newmap texture paths. if it has already been added, return the index
	for (size_t i = 0; i < mapsave.texturecount; i++) {
		if (strcmp(editor_texturepaths[selectedtexture], mapsave.texturepaths[i]) == 0) {
			in = i;
			break;
		}
	}
	if (in == -1) {
		// add texture path to mapsave
		if (mapsave.texturecount == 0)
			mapsave.texturepaths = xmalloc(sizeof(char*));
		else
			mapsave.texturepaths = xrealloc(mapsave.texturepaths, sizeof(char*) * (mapsave.texturecount + 1));
		mapsave.texturepaths[mapsave.texturecount] = xcalloc(64, 1);
		mapsave.texturepaths[mapsave.texturecount] = strcpy(mapsave.texturepaths[mapsave.texturecount], editor_texturepaths[selectedtexture]);
		mapsave.texturecount++;

		// add tile texture to newmap
		if (newmap.texturecount == 0)
			newmap.textures = xmalloc(sizeof(SDL_Texture*));
		else
			newmap.textures = xrealloc(newmap.textures, sizeof(SDL_Texture*) * (newmap.texturecount + 1));
		newmap.textures[newmap.texturecount++] = FE_TextureFromFile(mapsave.texturepaths[mapsave.texturecount-1]);
		in = newmap.texturecount -1;
	}

	// Add tile to mapsave
	if (mapsave.tilecount == 0)
		mapsave.tiles = xmalloc(sizeof(Map_Tile));
	else
		mapsave.tiles = xrealloc(mapsave.tiles, sizeof(Map_Tile) * (mapsave.tilecount + 1));
	mapsave.tiles[mapsave.tilecount++] = (Map_Tile){in, (Vector2D){x, y}};
	
	// Add tile to newmap
	if (newmap.tilecount == 0)
		newmap.tiles = xmalloc(sizeof(Map_Tile));
	else
		newmap.tiles = xrealloc(newmap.tiles, sizeof(Map_Tile) * (newmap.tilecount + 1));
	newmap.tiles[newmap.tilecount++] = (Map_Tile){in, (Vector2D){x, y}};
}

static void SetSpawn(int x, int y)
{
	// snap x and y to a grid using tile size
	if (grid) {
		x = (x / TILE_SIZE) * TILE_SIZE;
		y = (y / TILE_SIZE) * TILE_SIZE;
	}

	newmap.PlayerSpawn = (Vector2D){x, y};
	mapsave.PlayerSpawn = (Vector2D){x, y};
	info("Editor: Spawn set to (%d, %d)", x, y);
}

static void SetEnd(int x, int y)
{
	if (grid) {
		x = (x / TILE_SIZE) * TILE_SIZE;
		y = (y / TILE_SIZE) * TILE_SIZE;
	}

	newmap.EndFlag = (Vector2D){x, y};
	mapsave.EndFlag = (Vector2D){x, y};
	info("Editor: Spawn end flag to (%d, %d)", x, y);
}

static void Save()
{
	saving = Editor_CallSave(&mapsave);
}

static void Reset()
{
	FE_CleanEditor();
	FE_StartEditor();
}

static void ChangeSelection(size_t sel)
{
	if (mode) { // change selected tile
		if (texturecount -1 < sel)
			return;
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

    while (SDL_PollEvent(&event)) {
		if (FE_ConsoleVisible) {
			FE_HandleConsoleInput(&event, keyboard_state);
		} else {
			switch (event.type)
			{
				case SDL_QUIT:
					FE_Clean();
				break;
				case SDL_MOUSEBUTTONDOWN:
					if (event.button.button == SDL_BUTTON_LEFT) {
						if (!FE_UI_HandleClick(&event)) {
							int mouse_x, mouse_y;
							SDL_GetMouseState(&mouse_x, &mouse_y);
							if (!saving) {
								if (mode)
									AddTile(mouse_x + camera.x, mouse_y + camera.y);
								else
									SetBG(mouse_x + camera.x, mouse_y + camera.y);
							}
						}
					} else if (event.button.button == SDL_BUTTON_RIGHT) {
						int mouse_x, mouse_y;
						SDL_GetMouseState(&mouse_x, &mouse_y);
						if (!saving)
							DeleteTile(mouse_x + camera.x, mouse_y + camera.y);
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
						SetSpawn(mouse_x + camera.x, mouse_y + camera.y);
					}
					else if (keyboard_state[SDL_SCANCODE_E]) {
						int mouse_x, mouse_y;
						SDL_GetMouseState(&mouse_x, &mouse_y);
						SetEnd(mouse_x + camera.x, mouse_y + camera.y);
					}

					// moving camera
					if (keyboard_state[SDL_SCANCODE_W])
						FE_MoveCamera(0, -10, &camera);
					else if (keyboard_state[SDL_SCANCODE_A])
						FE_MoveCamera(-10, 0, &camera);
					else if (keyboard_state[SDL_SCANCODE_S])
						FE_MoveCamera(0, 10, &camera);
					else if (keyboard_state[SDL_SCANCODE_D])
						FE_MoveCamera(10, 0, &camera);

					// update label with camera x and y
					char buffer[64];
					sprintf(buffer, "X: %d, Y: %d", camera.x, camera.y);
					FE_UpdateLabel(coord, buffer);

				break;
			}
		}
	}
}

void FE_CleanEditor()
{
	if (texturecount > 0) {
		// free editor textures
		for (size_t i = 0; i < texturecount; i++)
			FE_DestroyTexture(editor_textures[i]);
		xfree(editor_textures);
		
		// free texturepaths
		for (size_t i = 0; i < texturecount; i++)
			xfree(editor_texturepaths[i]);
		xfree(editor_texturepaths);

		texturecount = 0;
		selectedtexture = 0;
	}

	if (bgcount > 0) {
	// free editor backgrounds
		for (size_t i = 0; i < bgcount; i++)
			FE_DestroyTexture(editor_backgrounds[i]);
		xfree(editor_backgrounds);

		// free backgroundpaths
		for (size_t i = 0; i < bgcount; i++)
			xfree(editor_backgroundpaths[i]);
		xfree(editor_backgroundpaths);

		bgcount = 0;
		selectedbackground = 0;
	}

	mode = true; grid = true;

	if (spawntexture)
		FE_DestroyTexture(spawntexture);
	spawntexture = 0;

	if (endtexture)
		FE_DestroyTexture(endtexture);
	endtexture = 0;

	// free memory held by mapsave
	if (mapsave.name)
		xfree(mapsave.name);
	mapsave.name = 0;

	// free texturepaths
	if (mapsave.texturecount > 0) {
		for (size_t i = 0; i < mapsave.texturecount; i++)
			xfree(mapsave.texturepaths[i]);
		xfree(mapsave.texturepaths);
		mapsave.texturecount = 0;
	}

	if (mapsave.bg_texturepath)
		xfree(mapsave.bg_texturepath);
	mapsave.bg_texturepath = 0;

	// free tiles
	if (mapsave.tilecount > 0) {
		xfree(mapsave.tiles);
		mapsave.tilecount = 0;
	}
	mapsave.PlayerSpawn = (Vector2D){0, 0};
	mapsave.EndFlag = (Vector2D){0, 0};

	// free memory held by newmap
	if (newmap.name)
		xfree(newmap.name);

	// free textures
	if (newmap.texturecount > 0) {
		for (size_t i = 0; i < newmap.texturecount; i++)
			FE_DestroyTexture(newmap.textures[i]);
		xfree(newmap.textures);
		newmap.texturecount = 0;
	}

	if (newmap.tilecount > 0) {
		xfree(newmap.tiles);
		newmap.tilecount = 0;
	}
	newmap.PlayerSpawn = (Vector2D){-1, -1};
	newmap.EndFlag = (Vector2D){-1, -1};
	mapsave.PlayerSpawn = (Vector2D){-1, -1};
	mapsave.EndFlag = (Vector2D){-1, -1};
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

static void ToggleGrid()
{
	grid = !grid;
}

static void CreateUI()
{
	// UI elements
	FE_CreateButton("<", 10, 7, BUTTON_TINY, &Exit, NULL);
	FE_CreateButton("Clear", 60, 7, BUTTON_TINY, &Reset, NULL);
	FE_CreateCheckbox("Tiles", 150, 7, mode, &ChangeMode, NULL);
	FE_CreateCheckbox("Grid", 250, 7, mode, &ToggleGrid, NULL);
	FE_CreateUIObject(0, 0, 42, screen_width, "white.png");

	thumbnail = (SDL_Rect){475, 6, 32, 32};
	coord = FE_CreateLabel("X: 0 Y: 0", 340, 6, COLOR_BLACK);
	spawntexture = FE_TextureFromFile("game/map/spawn.png");
	endtexture = FE_TextureFromFile("game/map/end.png");
}

void FE_StartEditor() // cleans up from other game modes
{
   FE_CleanAll();

    FE_GameState = GAME_STATE_EDITOR;

	texturecount = Editor_LoadTextures();
	if (texturecount == 0) {
		warn("Unable to start map editor - No textures found");
		Exit();
		return;
	}
	bgcount = Editor_LoadBackgrounds();
	if (bgcount == 0) {
		warn("Unable to start map editor - No backgrounds found");
		Exit();
		return;
	}
	
	CreateUI();

	newmap.bg = editor_backgrounds[0];
	mapsave.bg_texturepath = strdup(editor_backgroundpaths[0]);

	mode = true;

	camera = (FE_Camera){0, 0, 4096, 1024, false};

	info("Editor: Started editor");

	mapsave.PlayerSpawn = (Vector2D){-1,-1};
	newmap.PlayerSpawn = (Vector2D){-1,-1};
	mapsave.EndFlag = (Vector2D){-1,-1};
	newmap.EndFlag = (Vector2D){-1,-1};
}