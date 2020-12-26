// RENDERS MAP AND HANDLES MAP COLLISION
#include "game.h"
static int tile_size;
static int rowcount;
static int columncount;
static int** array;

SDL_Texture* grass;
SDL_Texture* sky;
SDL_Texture* dirt;
SDL_Rect tilerect;

bool gAbove = false; bool gBelow= false; bool gLeft = false; bool gRight = false;
enum CollDir gDir;


SDL_Texture* LoadTile(const char* tiletext)
{
	SDL_Texture* tmptile = IMG_LoadTexture(renderer, tiletext);
	if (!tmptile)
		warn("Error loading tile texture. SDL Error: %s", IMG_GetError());
	return tmptile;
}

int MapLoaded() {
	if (!rowcount)
		error("No map was loaded");
	return 0;
}

void InitMap(char* map)
{
	gDir = DIR_NONE;
	// Read 2d array in from a text file for a map
	FILE *f = fopen(map, "r");
	if (!f)
		error("Could not open map %s", map);

	// Work out amount of rows and columns for memory allocation
	char ch;
	
	while ((ch = fgetc(f)) != EOF)
	{
		if (ch == ' ') {
			columncount += 1;
		}
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

    for (size_t i = 0; i < columncount; i++)
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
	grass = LoadTile("game/map/grass.png");
	sky = LoadTile("game/map/sky.png");
	dirt = LoadTile("game/map/dirt.png");

	tile_size = screen_height / rowcount;
	tilerect.h = tile_size;
	tilerect.w = tile_size;

}


void TileColl() // cursed code
{
	SDL_Rect tOut;
	
	if (SDL_IntersectRect(&playerRect, &tilerect, &tOut)) {

		if (tOut.y > 0) {
			gAbove = true;
		}
		if (playerRect.y + playerRect.h - tilerect.y > 20)
			gBelow = true;
	
		int TilePlayerHeight = tilerect.y - playerRect.y - playerRect.h + tilerect.h; // wtf is this help me
		int LeftDifference = playerRect.x - tilerect.x + tilerect.w;

		if (TilePlayerHeight < 0 && LeftDifference < 0) {
			gRight = true;
		} else if (TilePlayerHeight < 0 && LeftDifference > 0) {
			gLeft = true;
		}

	}
}

void DrawTile(int type, SDL_Texture* t) // GROUND COLLISION AND RENDER TILE
{
	SDL_RenderCopy(renderer, t, NULL, &tilerect);
	switch (type) {
		case 1:
			TileColl(); // CALL THIS IF YOU WANT TO CHECK FOR COLLISION FOR SOME TILES
			break;
		default:
			break;
	}
}

void RenderMap()
{
	gAbove = false;
	gRight = false;
	gLeft = false;
	gBelow = false;
	int type;
	for (int row = 0; row < rowcount; row++) {
		tilerect.y = row * tile_size;

		for (int column = 0; column < columncount; column++) {
			tilerect.x = column * tile_size;

			type = array[row][column];
			switch (type) {
				case 0:
					DrawTile(type, sky);
					break;
				case 1:
					DrawTile(type, grass);
					break;
				case 2:
					DrawTile(type, dirt);
					break;
				default:
					warn("Could not render tile. Map tile ID: %i", type);
					break;
			}
		}	

	}
}