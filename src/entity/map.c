#include "../include/game.h"

#define MAP_NAME_LENGTH 64
#define MAP_TEXURE_PATH_LENGTH 64
#define MAP_BG_TEXTURE_LENGTH 64
#define TILE_SIZE 32

#define TEXTURE_PATH "game/map/textures/"
#define BG_PATH "game/map/backgrounds/"

#define MAP_DIRECTORY "game/map/maps/"

static FE_LoadedMap map;
bool FE_MapLoaded = false;

int FE_LoadMap(const char *fp)
{
    // Combine map directory and map file path
    char *map_path = AddStr(MAP_DIRECTORY, fp);
    // Read map file from binary format
    FILE *f = fopen(map_path, "rb");
    if (f == NULL) {
        warn("Failed to open map file: %s", fp);
        FE_Menu_MainMenu(); // return to main menu when we ccan't open map
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
    
    char *buf = xmalloc(strlen(BG_PATH) + MAP_BG_TEXTURE_LENGTH + 1);
    strcpy(buf, BG_PATH);
    strcat(buf, bg_path);

    map.bg = FE_TextureFromFile(buf);
    free(bg_path);
    free(buf);

    // Read Map tiles
    if (fread(&map.tilecount, sizeof(Uint16), 1, f) != 1) goto err;
    map.tiles = xmalloc(sizeof(Map_Tile) * map.tilecount);
    for (int i = 0; i < map.tilecount; i++) {
        if (fread(&map.tiles[i].texture_index, sizeof(Uint16), 1, f) != 1) goto err;
        if (fread(&map.tiles[i].position, sizeof(Vector2D), 1, f) != 1) goto err;
    }
    
    // read player spawn
    if (fread(&map.PlayerSpawn, sizeof(Vector2D), 1, f) != 1) goto err;
    // read end flag
    if (fread(&map.EndFlag, sizeof(Vector2D), 1, f) != 1) goto err;

    fclose(f);

    info("Loaded map '%s'", map.name);

    return 1;

err:
    warn(feof(f) ? "Unexpected end of file in %s" : "Error loading map %s", fp);
    fclose(f);
    
    FE_CloseMap();
    return -1;
}

void FE_RenderMap()
{
    SDL_RenderCopy(renderer, map.bg, NULL, &(SDL_Rect){0,0,screen_width,screen_height});
    for (size_t i = 0; i < map.tilecount; i++)
        SDL_RenderCopy(renderer, map.textures[map.tiles[i].texture_index], NULL, &(SDL_Rect){map.tiles[i].position.x,map.tiles[i].position.y,TILE_SIZE,TILE_SIZE});
}

int FE_CloseMap()
{
    if (FE_MapLoaded) {
        if (map.name)
            free(map.name);

        // free and destroy texture array
        if (map.textures) {
            for (size_t i = 0; i < map.texturecount; i++)
                SDL_DestroyTexture(map.textures[i]);
            free(map.textures);
        }

        if (map.bg)
            SDL_DestroyTexture(map.bg);
        if (map.tiles)
            free(map.tiles);
        
        map.texturecount = 0;
        FE_MapLoaded = false;
    }
    return 1;
}

// TODO MAP EDITOR and test map loading and free map, finish flag 