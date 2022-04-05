#ifndef H_MAP
#define H_MAP

#include <SDL.h>
#include "../../world/include/physics.h"

typedef struct FE_Map_Tile {
    Uint16 texture_index;
    Uint16 rotation;
    Vector2D position;
} FE_Map_Tile;

typedef struct FE_Map {
    char *name;

    Uint16 texturecount;
    char **texturepaths;

    char *bg_texturepath;

    Uint16 tilecount;
    FE_Map_Tile *tiles;

    Vector2D PlayerSpawn;
    Vector2D EndFlag;
} FE_Map;

typedef struct FE_LoadedMap {
    char *name;

    Uint16 texturecount;
    SDL_Texture **textures;

    SDL_Texture *bg;

    Uint16 tilecount;
    FE_Map_Tile *tiles;

    Vector2D PlayerSpawn;
    Vector2D EndFlag;
} FE_LoadedMap;

typedef struct FE_EndFlag {
    SDL_Texture *texture;
    SDL_Rect r;
} FE_EndFlag;


/** Loads map from file into memory
 * 
 * @param name Name of map to load.
 * @return 1 on success, -1 on failure.
 */
int FE_LoadMap(const char *name);


/** Renders the map tiles, background and end flag
*\param camera The camera to render the map from
*/
void FE_RenderMap(FE_Camera *camera);


/* Closes and frees resources from currently used map */
void FE_CloseMap();


/* Returns a Vector2D of the current map's spawn point */
Vector2D FE_GetSpawn();


extern Uint16 FE_Map_Width;
extern Uint16 FE_Map_Height;
extern Uint16 FE_Map_MinimumX;
extern Uint16 FE_Map_MinimumY;

#endif