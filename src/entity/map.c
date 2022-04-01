#include "../include/game.h"

#define MAP_NAME_LENGTH 64
#define MAP_TEXURE_PATH_LENGTH 64
#define MAP_BG_TEXTURE_LENGTH 64
#define TILE_SIZE 64

#define TEXTURE_PATH "game/map/textures/"

#define MAP_DIRECTORY "game/map/maps/"

static FE_LoadedMap map;
bool FE_MapLoaded = false;

static SDL_Texture *flagtexture;

size_t FE_Map_Width = 0;
size_t FE_Map_Height = 0;

int FE_LoadMap(const char *name)
{
    if (!flagtexture)
        flagtexture = FE_TextureFromFile("game/map/end.png");

    FE_Map_Width = 0;
    FE_Map_Height = 0;

    // Combine map directory and map file path
    char *map_path = AddStr(MAP_DIRECTORY, name);
    // Read map file from binary format
    FILE *f = fopen(map_path, "rb");
    if (f == NULL) {
        warn("Failed to open map file: %s", name);
        FE_Menu_MainMenu(); // return to main menu when we can't open map
        xfree(map_path);
        return -1;
    }
    xfree(map_path);
    FE_MapLoaded = true;

    // Read map name
    map.name = xmalloc(MAP_NAME_LENGTH);
    if (fread(map.name, sizeof(char), MAP_NAME_LENGTH, f) != MAP_NAME_LENGTH) goto err;
    
    // Read map texture paths
    if (fread(&map.texturecount, sizeof(Uint16), 1, f) != 1) goto err;

    char **texturepaths = xmalloc(sizeof(char*) * map.texturecount);
    for (int i = 0; i < map.texturecount; i++) {
        texturepaths[i] = xmalloc(MAP_TEXURE_PATH_LENGTH);
        if (fread(texturepaths[i], sizeof(char), MAP_TEXURE_PATH_LENGTH, f) != MAP_TEXURE_PATH_LENGTH) goto err;
    }
    
    // load texture from path to textures array
    map.textures = xmalloc(sizeof(SDL_Texture*) * map.texturecount);
    for (int i = 0; i < map.texturecount; i++) {
        // buffer for texture path
        char *texturepath = xmalloc(MAP_TEXURE_PATH_LENGTH);
        strcpy(texturepath, TEXTURE_PATH);
        strcat(texturepath, texturepaths[i]);

        map.textures[i] = FE_TextureFromFile(texturepaths[i]);
    }

    // free texturepaths array
    for (int i = 0; i < map.texturecount; i++) 
        free(texturepaths[i]);
    free(texturepaths);

    // load background image
    char *bg_path = xmalloc(MAP_BG_TEXTURE_LENGTH);
    if (fread(bg_path, sizeof(char), MAP_BG_TEXTURE_LENGTH, f) != MAP_BG_TEXTURE_LENGTH) goto err;

    map.bg = FE_TextureFromFile(bg_path);
    free(bg_path);

    // Read Map tiles
    if (fread(&map.tilecount, sizeof(Uint16), 1, f) != 1) goto err;
    map.tiles = xmalloc(sizeof(Map_Tile) * map.tilecount);
    for (int i = 0; i < map.tilecount; i++) {
        if (fread(&map.tiles[i].texture_index, sizeof(Uint16), 1, f) != 1) goto err;
        if (fread(&map.tiles[i].position, sizeof(Vector2D), 1, f) != 1) goto err;

        // calculate map height and width
        if ((size_t)map.tiles[i].position.x > FE_Map_Width) FE_Map_Width = map.tiles[i].position.x + TILE_SIZE;
        if ((size_t)map.tiles[i].position.y > FE_Map_Height) FE_Map_Height = map.tiles[i].position.y + TILE_SIZE;
    }
 
    // read player spawn
    if (fread(&map.PlayerSpawn, sizeof(Vector2D), 1, f) != 1) goto err;
    // read end flag
    if (fread(&map.EndFlag, sizeof(Vector2D), 1, f) != 1) goto err;

    fclose(f);

    info("Loaded map '%s'", map.name);
    FE_FreeUI();
    FE_GameState = GAME_STATE_PLAY;
    return 1;

err:
    warn(feof(f) ? "Unexpected end of file in %s" : "Error loading map %s", name);
    fclose(f);
    
    FE_CloseMap();
    
    return -1;
}

void FE_RenderMap(FE_Camera *camera)
{
	// render background
	SDL_Rect bgrect = (SDL_Rect){0,0,0,0};
	SDL_QueryTexture(map.bg, NULL, NULL, &bgrect.w ,&bgrect.h);
	bgrect.x -= camera->x;
	bgrect.y -= camera->y;
	SDL_RenderCopy(renderer, map.bg, NULL, &bgrect);

	// render all tiles
	for (size_t i = 0; i < map.tilecount; i++) {
		SDL_Rect r = (SDL_Rect){map.tiles[i].position.x - camera->x, map.tiles[i].position.y - camera->y, TILE_SIZE, TILE_SIZE};
		FE_RenderCopy(map.textures[map.tiles[i].texture_index], NULL, &r);
	} 

    // render finish flag
    SDL_Rect r = (SDL_Rect){map.EndFlag.x - camera->x, map.EndFlag.y - camera->y, TILE_SIZE, TILE_SIZE};
    FE_RenderCopy(flagtexture, NULL, &r);

} // TODO here last: event camera moving

void FE_CloseMap()
{
    return;
}

// TODO MAP EDITOR and test map loading and free map