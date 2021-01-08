// RENDERS MAP AND HANDLES MAP COLLISION
#include "game.h"
#include "ext/json/tiny-json.h" // credit https://github.com/rafagafe/tiny-json

static int rowcount;
static int columncount;
static int** array;

static SDL_Texture* grass;
static SDL_Texture* sky;
static SDL_Texture* dirt;
static SDL_Rect tilerect;

static SDL_Texture** arr;

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

void LoadTiles(char* map) // Loads in location for each tile from map.txt.tiles
{
 	char *fp = malloc(sizeof(map) + 6); // allocate memory for string and .json extension
    strcpy(fp, map);
    strcat(fp, ".json");
	
	// load in json file to string
	FILE *f = fopen(fp, "r"); 
	char s[1000]; // TODO: needs fixing - making into dynamic array

	size_t i = 0;
	char ch;
	int lines;
	while ((ch=fgetc(f)) != EOF) {
		if (ch == '\n')
			lines++;
		
		s[i] = ch;
		i++;
	}

	printf("%i\n",lines);
	// parse json
    json_t mem[32];
    const json_t* json = json_create(s, mem, sizeof mem / sizeof *mem );
    if (!json)
        error("Error parsing json for tiles: %s",fp);

	arr = malloc(sizeof(SDL_Texture*) * (lines - 1));

	for (int i = 0; i < lines-1; i++) {
		char a[10];
		sprintf(a, "%i", i);
		const json_t* textitem = json_getProperty( json, a);
		if (textitem || JSON_TEXT != json_getType(textitem)) {
			const char* textval = json_getValue( textitem );
			printf( "Loaded Texture: %s.\n", textval );
			arr[i] = LoadTile(textval);
		}
	}
	free(fp);

}

void InitMap(char* map)
{
	LoadTiles(map);
	// Read 2d array in from a text file for a map

	char *fp = malloc(sizeof(map) + 6); // allocate memory for string and .json extension
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
	rowcount += 2;
	
 	array = malloc(rowcount * sizeof(*array));

    for (size_t i = 0; i < rowcount; i++)
        array[i] = malloc(columncount * sizeof(**array));

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
}

bool gLeft() { // Ground collide from left of tile
	int bottomA = playerRect.y + playerRect.h;
	int tx = (CollRect.x + playerRect.w) / tile_size;
	int ty = (bottomA / tile_size);

	if (ty >= rowcount || tx >= columncount) // todo - this sucks and is a temp fix to prevent segfault
		return true;

	if (array[ty -1][tx] > 0) 
		return true;
	else
		return false;
}

bool gRight() { // Ground collision from right of tile
	int bottomA = playerRect.y + playerRect.h;
	int tx = CollRect.x / tile_size;
	int ty = (bottomA / tile_size);

	if (ty >= rowcount || tx >= columncount)
		return true;

	if (array[ty - 1][tx] > 0)
		return true;
	else 
		return false;
}

bool gAbove() { // Ground collision from above tile
	int bottomA = playerRect.y + playerRect.h;

	int tx = CollRect.x / tile_size;
	int ty = bottomA / tile_size;

	if (ty >= rowcount || tx >= columncount)
		return true;

	bool col = false;

	for (int i = CollRect.x; i < CollRect.x + playerRect.w; i++) { // loop through each tile in range of player
		int tx = i / tile_size;
		if (array[ty][tx] > 0)
			col = true;
	}
	if (col)
		return true;
	else
		return false;
}

bool gBelow() { // Ground collision from below tile
	int bottomA = playerRect.y;
	int ty = bottomA / tile_size;
	int tx = CollRect.x / tile_size;
	
	if (ty >= rowcount || tx >= columncount)
		return true;

	bool col = false;

	for (int i = CollRect.x; i < CollRect.x + playerRect.w; i++) { // loop through each tile in range of player
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
	if (array != NULL) {
		columncount = 0;
		rowcount = 0;
		free(array);
	}
	if (arr != NULL)
		free(arr);
}

void RenderMap()
{
	if (array != NULL) {
		int maxrows = screen_width / tile_size;
		int maxcols = screen_height / tile_size;

		int type;
		for (int row = 0; row < rowcount; row++) {
			tilerect.y = (row * tile_size);
			
			for (int column = 0; column < columncount; column++) {
				tilerect.x = (column * tile_size) - scrollam;
				type = array[row][column];

				if (arr[type])
					SDL_RenderCopy(renderer, arr[type], NULL, &tilerect);
				else
					warn("Could not render tile. Map tile ID: %i", type);
			}	

		}
	}
}