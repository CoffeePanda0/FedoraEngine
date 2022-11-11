#include <FE_Common.h>
#include "include/map.h"

#include "../entity/include/prefab.h"

#define MAP_DIRECTORY "game/map/maps/"
FE_LoadedMap *PresentMap;


void FE_Game_SetMap(FE_LoadedMap *m)
{
    if (!m) {
        warn("Passed NULL map to SetMap");
        return;
    }
    if (PresentGame->MapConfig.Loaded)
        FE_Map_Close(PresentMap);

    PresentMap = m;

    PresentGame->MapConfig = (FE_MapConfig) {
        .Loaded = true,
        .MapWidth = m->MapWidth,
        .MapHeight = m->MapHeight,
        .MinimumX = m->MinimumX,
        .Gravity = m->gravity,
        .AmbientLight = m->ambientlight,
        .PlayerSpawn = m->PlayerSpawn
    };

}

FE_LoadedMap *FE_Game_GetMap()
{
    return PresentMap;
}

void FE_Map_CloseLoaded()
{
    if (!PresentGame->MapConfig.Loaded)
        return;

    FE_Map_Close(PresentMap);

    PresentGame->MapConfig.Loaded  = false;
    PresentMap = 0;
}

FE_LoadedMap *FE_Map_Load_Basic(const char *name)
{
    if (!name || mstrlen(name) < 1)
        return 0;
    
    FE_LoadedMap *m = xmalloc(sizeof(FE_LoadedMap));

    // Combine map directory and map file path
    char *map_path = mstradd(MAP_DIRECTORY, name);
    // Read map file from binary format
    FILE *f = fopen(map_path, "rb");
    if (f == NULL) {
        warn("Failed to open map file: %s", name);
        free(map_path);
        return 0;
    }
    free(map_path);

    // No map renderer for server only
    m->r = NULL;
    m->destroy_cb = NULL;

    // Read map name
    if (!(m->name = FE_File_ReadStr(f))) goto err;
    
    // Read author
    if (!(m->author = FE_File_ReadStr(f))) goto err;

    // read gravity
    if (fread(&m->gravity, sizeof(float), 1, f) != 1) goto err;
    
    // load texture atlas info
    char *atlas_path = 0;
    if (!(atlas_path = FE_File_ReadStr(f))) goto err;
    free(atlas_path);

    uint16_t texturesize = 0;
    if (fread(&texturesize, sizeof(uint16_t), 1, f) != 1) goto err;

    // load background image
    char static_bg[256];
    if (fread(static_bg, sizeof(bool), 1, f) != 1) goto err;
    char *bg_path = 0;
    if (!(bg_path = FE_File_ReadStr(f))) goto err;
    
    char *parallax_path = 0;
    if (!(parallax_path = FE_File_ReadStr(f))) goto err;
    if (parallax_path) free(parallax_path);

    if (bg_path) free(bg_path);

    // read ambient light
    if (fread(&m->ambientlight, sizeof(uint8_t), 1, f) != 1) goto err;

    // Read Map tiles and sizes
    if (fread(&m->tilecount, sizeof(uint16_t), 1, f) != 1) goto err;
    if (fread(&m->tilesize, sizeof(uint16_t), 1, f) != 1) goto err;
    
    // set empty values
    m->MinimumX = 0;
    m->MapHeight = 0;
    m->MapWidth = 0;

    bool setminX = false; // check if we have set the map minimum values yet

    m->tiles = xmalloc(sizeof(FE_Map_Tile) * m->tilecount);
    for (int i = 0; i < m->tilecount; i++) {
        if (fread(&m->tiles[i].texture_x, sizeof(uint16_t), 1, f) != 1) goto err;
        if (fread(&m->tiles[i].texture_y, sizeof(uint16_t), 1, f) != 1) goto err;
        if (fread(&m->tiles[i].rotation, sizeof(uint16_t), 1, f) != 1) goto err;
        if (fread(&m->tiles[i].position, sizeof(vec2), 1, f) != 1) goto err;


        // calculate map height and width
        if ((uint16_t)m->tiles[i].position.x + m->tilesize > m->MapWidth) m->MapWidth = m->tiles[i].position.x + m->tilesize;
        if ((uint16_t)m->tiles[i].position.y - PresentGame->WindowHeight + m->tilesize > m->MapHeight) m->MapHeight = m->tiles[i].position.y + m->tilesize - m->MapHeight;
    
        // calculate minimum x point and minimum y point for camera bounds
        if ((uint16_t)m->tiles[i].position.x < m->MinimumX || !setminX) {
            m->MinimumX = m->tiles[i].position.x;
            setminX = true;
        }
        
        // calculate highest Y value
        if ((uint16_t)m->tiles[i].position.y + m->tilesize > m->MapHeight) m->MapHeight = m->tiles[i].position.y + m->tilesize;
    }

    // Read Prefabs
    uint16_t prefabcount;
    if (fread(&prefabcount, sizeof(uint16_t), 1, f) != 1) goto err;
    for (int i = 0; i < prefabcount; i++) {
        uint16_t x = 0;
        uint16_t y = 0;
        char *_name = 0;
        if (fread(&x, sizeof(uint16_t), 1, f) != 1) goto err;
        if (fread(&y, sizeof(uint16_t), 1, f) != 1) goto err;
        if (!(_name = FE_File_ReadStr(f))) goto err;
        FE_Prefab_Create_Basic(_name, x, y);
        free(_name);
    }

    // read player spawn
    if (fread(&m->PlayerSpawn, sizeof(vec2), 1, f) != 1) goto err;
    
    // read end flag
    if (fread(&m->EndFlag, sizeof(vec2), 1, f) != 1) goto err;

    fclose(f);

    info("Loaded map: %s by %s", m->name, m->author);

    return m;
    
err:
    warn(feof(f) ? "Unexpected end of file in %s" : "Error loading map %s", name);
    fclose(f);
    
    FE_Map_Close(m);
    
    return 0;
}

void FE_Map_Close(FE_LoadedMap *map)
{
    if (!map) {
        warn("Passing NULL map to CloseMap");
        return;
    }

    if (map->name)
        free(map->name);
    if (map->author)
        free(map->author);
    map->name = 0;
    map->PlayerSpawn = VEC_NULL;

    if (map->destroy_cb)
        map->destroy_cb(map);
    map->r = 0;

    if (map->tiles) {
        free(map->tiles);
        map->tiles = 0;
    }
    map->tilecount = 0;
    free(map);

    PresentGame->MapConfig.Loaded = false;

    return;
}

