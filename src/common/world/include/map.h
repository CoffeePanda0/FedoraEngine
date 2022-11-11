#ifndef _H_COMMON_MAP
#define _H_COMMON_MAP

#include <vector.h>
#include "../../physics/include/physics.h"

typedef struct FE_Map_Tile {
    uint16_t texture_x;
    uint16_t texture_y;
    
    uint16_t rotation;
    vec2 position;
} FE_Map_Tile;


typedef struct FE_Map_Prefab {
    uint16_t x;
    uint16_t y;
    char *name;
} FE_Map_Prefab;


/* The map format when loaded in RAM */
typedef struct FE_LoadedMap {
    char *name;
    char *author;

    float gravity;

    struct FE_MapRenderer *r;
    void (*destroy_cb)(void*);

    uint8_t ambientlight;

    uint16_t tilecount;
    uint16_t tilesize;
    FE_Map_Tile *tiles;

    vec2 PlayerSpawn;
    vec2 EndFlag;

    int MapWidth;
    int MapHeight;
    int MinimumX;
} FE_LoadedMap;

extern FE_LoadedMap *PresentMap;


/* Gets the current active map*/
FE_LoadedMap *FE_Game_GetMap();


/* Destroys the currently loaded map */
void FE_Map_CloseLoaded();


/** Sets the world map
 *\param m The loaded map
*/
void FE_Game_SetMap(FE_LoadedMap *m);


/* Closes and frees resources from a map */
void FE_Map_Close(FE_LoadedMap *map);


/** Loads and returns a map in basic mode (no rendering or resources for server use)
    *\param path The path to the map file
    *\return The loaded map
*/
FE_LoadedMap *FE_Map_Load_Basic(const char *name);


#endif