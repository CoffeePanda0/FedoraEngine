#ifndef H_MAP
#define H_MAP

#include "../../physics/include/vector.h"
#include "../../core/include/texture.h"

typedef struct FE_Map_Tile {
    uint16_t texture_x;
    uint16_t texture_y;
    
    Uint16 rotation;
    vec2 position;
} FE_Map_Tile;

/* The Map format when written to file */
typedef struct FE_Map {
    char *name;

    float gravity;

    char *atlaspath;
    Uint16 atlas_tilesize;

    char *bg_texturepath;

    Uint16 tilecount;
    Uint16 tilesize;
    FE_Map_Tile *tiles;

    vec2 PlayerSpawn;
    vec2 EndFlag;
} FE_Map;


/* The map format when loaded in RAM */
typedef struct FE_LoadedMap {
    char *name;

    float gravity;

    FE_TextureAtlas *atlas;
    FE_Texture *bg;

    Uint16 tilecount;
    Uint16 tilesize;
    FE_Map_Tile *tiles;

    vec2 PlayerSpawn;
    vec2 EndFlag;
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
FE_LoadedMap *FE_LoadMap(const char *name);


/** Renders the map tiles
 *\param m The map to render 
*\param camera The camera to render the map from
*/
void FE_RenderMap(FE_LoadedMap *m, FE_Camera *camera);


/** Renders the currently loaded map background
 * \param camera The camera to render the map from
 */
void FE_RenderMapBackground(FE_Camera *camera);


/* Closes and frees resources from a map */
void FE_CloseMap(FE_LoadedMap *map);

/* Destroys the currently loaded map */
void FE_CloseLoadedMap();

/** Checks if a rect is colliding above the map tiles
 *\param r The rectangle to check for collision
 *\returns A vector containing the x&y of the collided tile on collision, or VEC_NULL on no collision
*/
vec2 FE_CheckMapCollisionAbove(SDL_Rect *r);


/** Checks if a rect is colliding below the map tiles
 * \param r The rectangle to check for collision
 * \returns A vector containing the x&y of the collided tile on collision, or VEC_NULL on no collision
 */
vec2 FE_CheckMapCollisionBelow(SDL_Rect *r);


/** Checks if a rect is colliding with the right-hand side of the rect
 *\param r The rectangle to check for collision
 *\returns A vector containing the x&y of the collided tile on collision, or VEC_NULL on no collision
*/
vec2 FE_CheckMapCollisionRight(SDL_Rect *r);


/** Checks if a rect is colliding with the left-hand side of the rect
 *\param r The rectangle to check for collision
 *\returns A vector containing the x&y of the collided tile on collision, or VEC_NULL on no collision
*/
vec2 FE_CheckMapCollisionLeft(SDL_Rect *r);


/** Renders the currently loaded map
 *\param camera The camera to render from
*/
void FE_RenderLoadedMap(FE_Camera *camera);


/** Sets the world map
 *\param m The loaded map
*/
void FE_Game_SetMap(FE_LoadedMap *m);


#endif