#ifndef H_MAP
#define H_MAP

#include "../../core/include/vector2d.h"
#include "../../core/include/texture.h"

typedef struct FE_Map_Tile {
    Uint16 texture_index;
    Uint16 rotation;
    Vector2D position;
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

    Vector2D PlayerSpawn;
    Vector2D EndFlag;
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

    Vector2D PlayerSpawn;
    Vector2D EndFlag;
} FE_LoadedMap;

typedef struct FE_EndFlag {
    FE_Texture *texture;
    SDL_Rect r;
} FE_EndFlag;


/** Loads map from file into memory
 * 
 *\param name Name of map to load.
 *\returns 1 on success, -1 on failure.
 */
int FE_LoadMap(const char *name);


/** Renders the map tiles, background and end flag
*\param camera The camera to render the map from
*/
void FE_RenderMap(FE_Camera *camera);


/** Renders the map background
 * \param camera The camera to render the map from
 */
void FE_RenderMapBackground(FE_Camera *camera);


/* Closes and frees resources from currently used map */
void FE_CloseMap();


/** Checks if a rect is colliding above the map tiles
 *\param r The rectangle to check for collision
 *\returns A vector containing the x&y of the collided tile on collision, or VEC_NULL on no collision
*/
Vector2D FE_CheckMapCollisionAbove(SDL_Rect *r);


/** Checks if a rect is colliding below the map tiles
 * \param r The rectangle to check for collision
 * \returns A vector containing the x&y of the collided tile on collision, or VEC_NULL on no collision
 */
Vector2D FE_CheckMapCollisionBelow(SDL_Rect *r);


/** Checks if a rect is colliding with the right-hand side of the rect
 *\param r The rectangle to check for collision
 *\returns A vector containing the x&y of the collided tile on collision, or VEC_NULL on no collision
*/
Vector2D FE_CheckMapCollisionRight(SDL_Rect *r);


/** Checks if a rect is colliding with the left-hand side of the rect
 *\param r The rectangle to check for collision
 *\returns A vector containing the x&y of the collided tile on collision, or VEC_NULL on no collision
*/
Vector2D FE_CheckMapCollisionLeft(SDL_Rect *r);


#endif