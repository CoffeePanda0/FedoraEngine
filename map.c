#include "game.h"
int tile_size = 32;

int GroundCollideHeight = 96; // CHANGE THIS FOR YOUR MAP 

SDL_Texture* grass;
SDL_Texture* sky;
SDL_Texture* dirt;

SDL_Rect tilerect;

int testmap[16][16] = {
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
{2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2},
{2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2},
};

char buffer[20];

void InitMap()
{
	grass = IMG_LoadTexture(renderer, "game/grass.png");
	sky = IMG_LoadTexture(renderer, "game/sky.png");
	dirt = IMG_LoadTexture(renderer, "game/dirt.png");
	tilerect.h = tile_size;
	tilerect.w = tile_size;
}

void RenderMap(int map[16][16])
{
	int type = 0;
	for (int row = 0; row < 16; row++) {
		tilerect.y = row * tile_size;

		for (int column = 0; column < 16; column++) {
			tilerect.x = column * tile_size;

			type = map[row][column];
			
			switch (type) {
				case 0:
					SDL_RenderCopy(renderer, sky, NULL, &tilerect);
					break;
				case 1:
					SDL_RenderCopy(renderer, grass, NULL, &tilerect);
					break;
				case 2:
					SDL_RenderCopy(renderer, dirt, NULL, &tilerect);
					break;
			}
		}
	}
}