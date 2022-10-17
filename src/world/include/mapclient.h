#ifndef _H_CLIENT_MAP
#define _H_CLIENT_MAP

#include "../../client/world/include/camera.h"

/* The Map format when written to file */
typedef struct FE_Map {
    char *name;
    char *author;
    
    float gravity;

    char *atlaspath;
    uint16_t atlas_tilesize;

    bool static_bg;
    char *bg_texturepath;
    char *parallax;
    
    uint8_t ambientlight;

    uint16_t tilecount;
    uint16_t tilesize;
    FE_Map_Tile *tiles;

    uint16_t prefabcount;
    FE_Map_Prefab *prefabs;

    vec2 PlayerSpawn;
    vec2 EndFlag;
} FE_Map;


typedef struct FE_MapRenderer {
    struct FE_TextureAtlas *atlas;

    bool static_bg;
    struct FE_Texture *bg;
} FE_MapRenderer;


typedef struct FE_EndFlag {
    struct FE_Texture *texture;
    SDL_Rect r;
} FE_EndFlag;


/** Renders the map tiles
 *\param m The map to render 
*\param camera The camera to render the map from
*/
void FE_Map_Render(FE_LoadedMap *m, FE_Camera *camera);


/** Renders the currently loaded map background
 * \param camera The camera to render the map from
 */
void FE_Map_RenderBackground(FE_Camera *camera);


/** Renders the currently loaded map
 *\param camera The camera to render from
*/
void FE_Map_RenderLoaded(FE_Camera *camera);


#endif