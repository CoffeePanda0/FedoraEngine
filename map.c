#include "game.h"
int tile_size;

SDL_Texture* grass;
SDL_Texture* sky;
SDL_Texture* dirt;
SDL_Rect tilerect;
int type = 0;

enum CollDir gDir;

SDL_Texture* LoadTile(const char* tiletext)
{
	SDL_Texture* tmptile = IMG_LoadTexture(renderer, tiletext);
	if (!tmptile)
		warn("Error loading tile texture. SDL Error: %s", IMG_GetError());
	return tmptile;
}
int rowcount;
int columncount;

int** array;

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
	columncount += 1;
	rowcount += 2;

	if (!rowcount) error("Map %s has no rows.", map);
	if (!columncount) error("Map %s has no columns", map);

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

void TileCollision()
{
	SDL_Rect out;
	SDL_IntersectRect(&playerRect, &tilerect, &out);

	int offset = -4;
	int TilePlayerHeight = tilerect.y - playerRect.y - playerRect.h - offset;

	int RightDifference = out.x - playerRect.x;
	int LeftDifference = out.x - playerRect.x - playerRect.w;

	if (TilePlayerHeight < 0) {
		if (RightDifference > 0) {
			gDir = DIR_RIGHT;
		} else if (LeftDifference < 0 && RightDifference == 0) {
			gDir = DIR_LEFT;
		}
		if (tilerect.y - tilerect.h < playerRect.y) {
			gDir = DIR_BELOW;
		}

	} else {
		onGround = true;
		dir = DIR_ABOVE;
	}
}

void DrawTile(SDL_Texture* t) // GROUND COLLISION AND RENDER TILE
{
	SDL_RenderCopy(renderer, t, NULL, &tilerect);

	if (SDL_HasIntersection(&playerRect, &tilerect)) {
		switch (type) {

			case 1:
				TileCollision(); // CALL THIS IF YOU WANT TO CHECK FOR COLLISION FOR SOME TILES
				break;

			default:
				onGround = false;
				break;
		}
	}
}

void RenderMap()
{
	for (int row = 0; row < rowcount; row++) {
		tilerect.y = row * tile_size;

		for (int column = 0; column < columncount; column++) {
			tilerect.x = column * tile_size;

			type = array[row][column];
			switch (type) {
				case 0:
					DrawTile(sky);
					break;
				case 1:
					DrawTile(grass);
					break;
				case 2:
					DrawTile(dirt);
					break;
				default:
					warn("Could not render tile. Map tile ID: %i", type);
					break;
			}
		}
	}
}