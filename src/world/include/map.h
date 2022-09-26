#ifndef H_MAP
#define H_MAP

#include "../../core/include/vector.h"
#include "../../physics/include/physics.h"
#include "../../core/include/texture.h"

typedef struct FE_Map_Tile {
    uint16_t texture_x;
    uint16_t texture_y;
    
    Uint16 rotation;
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

/* The Map format when written to file */
typedef struct FE_Map {
    char *name;
    char *author;
    
    float gravity;

    char *atlaspath;
    Uint16 atlas_tilesize;

    bool static_bg;
    char *bg_texturepath;
    char *parallax;
    
    uint8_t ambientlight;

    Uint16 tilecount;
    Uint16 tilesize;
    FE_Map_Tile *tiles;

    Uint16 prefabcount;
    FE_Map_Prefab *prefabs;

    vec2 PlayerSpawn;
    vec2 EndFlag;
} FE_Map;


/* The map format when loaded in RAM */
typedef struct FE_LoadedMap {
    char *name;
    char *author;

    float gravity;

    FE_TextureAtlas *atlas;
    
    bool static_bg;
    FE_Texture *bg;

    uint8_t ambientlight;

    Uint16 tilecount;
    Uint16 tilesize;
    FE_Map_Tile *tiles;

    vec2 PlayerSpawn;
    vec2 EndFlag;

    int MapWidth;
    int MapHeight;
    int MinimumX;
} FE_LoadedMap;

typedef struct FE_EndFlag {
    FE_Texture *texture;
    SDL_Rect r;
} FE_EndFlag;


/** Loads map from file
 * 
 *\param name Name of map to load.
 *\returns the loaded map
 */
FE_LoadedMap *FE_Map_Load(const char *name);


/** Renders the map tiles
 *\param m The map to render 
*\param camera The camera to render the map from
*/
void FE_Map_Render(FE_LoadedMap *m, FE_Camera *camera);


/** Renders the currently loaded map background
 * \param camera The camera to render the map from
 */
void FE_Map_RenderBackground(FE_Camera *camera);


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


/** Renders the currently loaded map
 *\param camera The camera to render from
*/
void FE_Map_RenderLoaded(FE_Camera *camera);


/** Sets the world map
 *\param m The loaded map
*/
void FE_Game_SetMap(FE_LoadedMap *m);


#endif