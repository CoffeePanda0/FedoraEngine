// RENDERS MAP AND HANDLES MAP COLLISION
#include "game.h"
#include "ext/json/tiny-json.h" // credit https://github.com/rafagafe/tiny-json

static int rowcount;
static int columncount;
static int** array;

static SDL_Rect tilerect;

static SDL_Texture** arr;
size_t arr_elements = 0;

static bool mapopen; // use this to prevent segfault during map load

SDL_Texture* LoadTile(const char* tiletext)
{
	SDL_Texture* tmptile = IMG_LoadTexture(renderer, tiletext);
	if (!tmptile)
		warn("Error loading tile texture. SDL Error: %s", IMG_GetError());
	return tmptile;
}

int MapLoaded()
{
	if (!rowcount) {
		error("No map was loaded");
		return -1;
	}
	return 0;
}

void DestroyTiles()
{
	for (size_t i = 0; i < arr_elements; i++) {
		SDL_DestroyTexture(arr[i]);
		if (i != arr_elements -1)
			arr[i] = arr[i+1];
	}
	arr_elements = 0;
}

void LoadTiles(const char* map) // Loads in location for each tile from map.txt.tiles
{
 	char *fp = malloc(strlen(map) + 6); // allocate memory for string and .json extension
    strcpy(fp, map);
    strcat(fp, ".json");
	
	// load in json file to string
	FILE *f = fopen(fp, "r"); 
	if (!f)
		error("Error opening map file %s", fp);

	char s[1000];
	size_t i = 0;
	char ch;
 	int lines = 0;
	int chars = 0;
	while ((ch=fgetc(f)) != EOF) {
		if (ch == '\n')
			lines++;
		
		chars++;
		s[i] = ch;

		i++;
	}

	lines--;

	// parse json
	json_t mem[32];
	const json_t* json = json_create(s, mem, sizeof mem / sizeof *mem);
	if (!json)
    	error("Error parsing json for tiles: %s",fp);

	arr = malloc(sizeof(SDL_Texture*) * (lines - 1));

	for (int i = 0; i < lines-1; i++) { // loop through each line (not the best way but itll do)
		char a[10];
		sprintf(a, "%i", i);
		const json_t* textitem = json_getProperty(json, a);
		if (textitem || JSON_TEXT != json_getType(textitem)) { // get value of each node 
			const char* textval = json_getValue( textitem );
			printf("Loaded Texture: %s.\n", textval); // load node into array
			arr[i] = LoadTile(textval);
			arr_elements++;
		}
	}
	free(fp);
	fclose(f);
}

void InitMap(const char* map)
{
	mapopen = false;
	if (rowcount > 0)
		DestroyMap();
	
	LoadTiles(map);
	// Read 2d array in from a text file for a map

	char *fp = malloc(strlen(map) + 5); // allocate memory for string and .json extension
    strcpy(fp, map);
    strcat(fp, ".txt");

	FILE *f = fopen(fp, "r");
	if (!f)
		error("Could not open map %s", map);
	
	// Work out amount of rows and columns for memory allocation
	char ch;
	
	while ((ch = fgetc(f)) != EOF)
	{
		if (ch == ' ')
			columncount += 1;
		if (ch == '\n')
			break;
	}
	while ((ch = fgetc(f)) != EOF)
	{
		if (ch == '\n')
			rowcount +=1;
	}
	
	if (!rowcount) error("Map %s has no rows.", map);
	if (!columncount) error("Map %s has no columns", map);

	columncount += 1;
	rowcount += 1;
	
 	array = malloc(rowcount * sizeof(*array));

    for (int i = 0; i < rowcount; i++) {
        array[i] = malloc(columncount * sizeof(**array));
	}
	
	if (!array)
		error("Could not allocate memory for 2D map array %s", map);

	rewind(f);
	for (int row = 0; row < rowcount; row++) {
		for (int col = 0; col < columncount; col++)
			fscanf(f, " %d ", &array[row][col]);
	}

	info("Loaded map %s to memory succesfully", map);
	
	fclose(f);
	// Load in textures
	tilerect.h = tile_size;
	tilerect.w = tile_size;

	map_width = columncount * tile_size;
	map_height = rowcount * tile_size;
	free(fp);
	mapopen = true;
	SpawnPlayer(80, 50, 75, 90);
}

bool gLeft(SDL_Rect r) { // Ground collide from left of tile
	if (!mapopen)
		return false;
	
	int tx = (r.x + r.w) / tile_size;

	if (r.y < 3)
		return true;

	for (int i = r.y; i < (r.h + r.y -4); i++) {
		int g = (i / tile_size);

		if (array[g][tx] > 0)
			return true;
		
	}

	return false;
}

bool gRight(SDL_Rect r) { // Ground collision from right of tile
	if (!mapopen)
		return false;

	int tx = CollRect.x / tile_size;

	if (r.y < 3)
		return true;

	for (int i = r.y; i < (r.h + r.y -4); i++) {
		int g = (i / tile_size);

		if (array[g][tx] > 0)
			return true;

	}
	return false;
}

bool gAbove(SDL_Rect r) { // Ground collision from above tile
	if (!mapopen)
		return false;

	int bottomA = r.y + r.h;
	int tx = r.x / tile_size;
	int ty = bottomA / tile_size;

	if (r.y < 3) // i cant remember what this does but it does something
		return true;

	if (ty >= rowcount || tx >= columncount || ty < 0) { // if player at bottom of screen with no tile
		if (&r == &playerRect)
			PlayerDie();
		return false; // kill player 
	}
	
	bool col = false;

	for (int i = r.x; i < r.x + r.w; i++) { // loop through each tile in range of player
		int tx = i / tile_size;

		if (array[ty][tx] > 0)
			col = true;

	}

	if (col)
		return true;
	else
		return false;
}

void calculate_scroll()
{
	
}

bool gBelow(SDL_Rect r)
{ // Ground collision from below tile
	if (!mapopen)
		return false;

	int bottomA = r.y;
	int ty = bottomA / tile_size;
	int tx = r.x / tile_size;
	
	if (bottomA + r.h < 3)
		return true;

	if (ty >= rowcount || tx >= columncount || ty < 0)
		return true;

	bool col = false;

	for (int i = r.x; i < r.x + r.w; i++) { // loop through each tile in range of player
		int tx = i / tile_size;
		if (array[ty][tx] > 0)
			col = true;
	}
	if (col)
		return true;
	else
		return false;
}


void DestroyMap()
{
	mapopen = false;
	if (rowcount > 0) {
		DestroyTiles();
		for (int x = 0; x < rowcount; x++)
			free(array[x]);

		if (array != NULL) {
			columncount = 0;
			rowcount = 0;
			free(array);
		}


		map_width = 0;
		scrollam = 0;
		hscrollam = 0;
		map_height = 0;
	}
}

void RenderMap()
{
	if (array != NULL) {
		int type;
		for (int row = 0; row < rowcount; row++) {
			tilerect.y = (row * tile_size);
			
			for (int column = 0; column < columncount; column++) {
				tilerect.x = (column * tile_size) - scrollam;
				tilerect.y = (row *  tile_size) - hscrollam;
				BgRect.x = 0 - scrollam;
				type = array[row][column];

				if (type > 0)
					SDL_RenderCopy(renderer, arr[type], NULL, &tilerect);
			}	

		}
	}
}
