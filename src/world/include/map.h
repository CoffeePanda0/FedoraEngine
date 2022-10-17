#ifndef H_MAP
#define H_MAP

#include <vector.h>
#include "../../common/physics/include/physics.h"

typedef struct FE_Map_Tile {
    uint16_t texture_x;
    uint16_t texture_y;
    
    uint16_t rotation;
    vec2 position;
} FE_Map_Tile;


typedef struct {
    vec2 position;
    vec2 normal;
    float penetration;
} TileCollision;


typedef struct {
    size_t count;
    TileCollision *collisions;
} FE_CollisionInfo;


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


/** Loads map from file
 * 
 *\param name Name of map to load.
 *\returns the loaded map
 */
FE_LoadedMap *FE_Map_Load(const char *name);


/* Closes and frees resources from a map */
void FE_Map_Close(FE_LoadedMap *map);


/** Checks the map for collisions with an AABB
 * @param aabb The AABB to check for collisions with
 * @param result The collision info to store the results in
 */
void FE_Map_Collisions(Phys_AABB *aabb, FE_CollisionInfo *result);


/* Gets the current active map*/
FE_LoadedMap *FE_Game_GetMap();


/* Destroys the currently loaded map */
void FE_Map_CloseLoaded();


/** Sets the world map
 *\param m The loaded map
*/
void FE_Game_SetMap(FE_LoadedMap *m);


#include "mapclient.h"


#endif