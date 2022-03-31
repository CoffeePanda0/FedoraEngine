#ifndef H_MAP
#define H_MAP

#include <SDL.h>

typedef struct Map_Tile {
    Uint16 texture_index;
    Vector2D position;
} Map_Tile;

typedef struct FE_Map {
    char *name;

    Uint16 texturecount;
    char **texturepaths;

    char *bg_texturepath;

    Uint16 tilecount;
    Map_Tile *tiles;

    Vector2D PlayerSpawn;
    Vector2D EndFlag;
} FE_Map;

typedef struct FE_LoadedMap {
    char *name;

    Uint16 texturecount;
    SDL_Texture **textures;

    SDL_Texture *bg;

    Uint16 tilecount;
    Map_Tile *tiles;

    Vector2D PlayerSpawn;
    Vector2D EndFlag;
} FE_LoadedMap;

typedef struct FE_EndFlag {
    SDL_Texture *texture;
    SDL_Rect r;
} FE_EndFlag;

/* Map loading */
int FE_CloseMap();
int FE_LoadMap(const char *fp);
void FE_RenderMap();

#endif