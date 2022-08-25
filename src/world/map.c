#include "../core/include/include.h"
#include "include/include.h"
#include "../ui/include/ui.h"
#include "../ui/include/menu.h"
#include "../core/include/file.h"
#include "../entity/include/prefab.h"

#define TEXTURE_PATH "game/map/textures/"
#define BG_PATH "game/map/backgrounds/"
#define MAP_DIRECTORY "game/map/maps/"

static FE_LoadedMap *map;

static FE_Texture *flagtexture;

FE_LoadedMap *FE_LoadMap(const char *name)
{
    if (!name || strlen(name) < 1)
        return 0;
    
    FE_LoadedMap *m = xmalloc(sizeof(FE_LoadedMap));

    // Combine map directory and map file path
    char *map_path = mstradd(MAP_DIRECTORY, name);
    // Read map file from binary format
    FILE *f = fopen(map_path, "rb");
    if (f == NULL) {
        warn("Failed to open map file: %s", name);
        FE_Menu_LoadMenu("Main"); // return to main menu when we can't open map
        free(map_path);
        return 0;
    }
    free(map_path);

    // Read map name
    if (!(m->name = FE_File_ReadStr(f))) goto err;
    
    // Read author
    if (!(m->author = FE_File_ReadStr(f))) goto err;

    // read gravity
    if (fread(&m->gravity, sizeof(float), 1, f) != 1) goto err;
    
    // load texture atlas
    char *atlas_path = 0;
    if (!(atlas_path = FE_File_ReadStr(f))) goto err;
    m->atlas = FE_LoadResource(FE_RESOURCE_TYPE_ATLAS, atlas_path);
    free(atlas_path);

    if (fread(&m->atlas->texturesize, sizeof(Uint16), 1, f) != 1) goto err;

    // load background image
    if (fread(&m->static_bg, sizeof(bool), 1, f) != 1) goto err;
    char *bg_path = 0;
    if (!(bg_path = FE_File_ReadStr(f))) goto err;
    
    char *parallax_path = 0;
    if (!(parallax_path = FE_File_ReadStr(f))) goto err;
    
    if (m->static_bg) {
        m->bg = FE_LoadResource(FE_RESOURCE_TYPE_TEXTURE, bg_path);
    } else {
        m->bg = 0;
        if (!parallax_path || mstrlen(parallax_path) == 0)
            goto err;
        FE_Parallax_Load(parallax_path);
        free(parallax_path);
    }
    if (bg_path) free(bg_path);

    // read ambient light
    if (fread(&m->ambientlight, sizeof(uint8_t), 1, f) != 1) goto err;

    // Read Map tiles and sizes
    if (fread(&m->tilecount, sizeof(Uint16), 1, f) != 1) goto err;
    if (fread(&m->tilesize, sizeof(Uint16), 1, f) != 1) goto err;
    
    bool setminX = false; // check if we have set the map minimum values yet

    m->tiles = xmalloc(sizeof(FE_Map_Tile) * m->tilecount);
    for (int i = 0; i < m->tilecount; i++) {
        if (fread(&m->tiles[i].texture_x, sizeof(Uint16), 1, f) != 1) goto err;
        if (fread(&m->tiles[i].texture_y, sizeof(Uint16), 1, f) != 1) goto err;
        if (fread(&m->tiles[i].rotation, sizeof(Uint16), 1, f) != 1) goto err;
        if (fread(&m->tiles[i].position, sizeof(vec2), 1, f) != 1) goto err;

        // calculate map height and width
        if ((Uint16)m->tiles[i].position.x + m->tilesize > PresentGame->MapConfig.MapWidth) PresentGame->MapConfig.MapWidth = m->tiles[i].position.x + m->tilesize;
        if ((Uint16)m->tiles[i].position.y - PresentGame->WindowHeight + m->tilesize > PresentGame->MapConfig.MapHeight) PresentGame->MapConfig.MapHeight = m->tiles[i].position.y + m->tilesize - PresentGame->MapConfig.MapHeight;
    
        // calculate minimum x point and minimum y point for camera bounds
        if ((Uint16)m->tiles[i].position.x < PresentGame->MapConfig.MinimumX || !setminX) {
            PresentGame->MapConfig.MinimumX = m->tiles[i].position.x;
            setminX = true;
        }
        
        // calculate highest Y value
        if ((Uint16)m->tiles[i].position.y + m->tilesize > PresentGame->MapConfig.MapHeight) PresentGame->MapConfig.MapHeight = m->tiles[i].position.y + m->tilesize;
    }

    // Read Prefabs
    Uint16 prefabcount;
    if (fread(&prefabcount, sizeof(Uint16), 1, f) != 1) goto err;
    for (int i = 0; i < prefabcount; i++) {
        uint16_t x = 0;
        uint16_t y = 0;
        char *_name = 0;
        if (fread(&x, sizeof(Uint16), 1, f) != 1) goto err;
        if (fread(&y, sizeof(Uint16), 1, f) != 1) goto err;
        if (!(_name = FE_File_ReadStr(f))) goto err;
        FE_Prefab_Create(_name, x, y);
        free(_name);
    }

    PresentGame->MapConfig.Gravity = m->gravity;
    PresentGame->MapConfig.AmbientLight = m->ambientlight;

    // read player spawn
    if (fread(&m->PlayerSpawn, sizeof(vec2), 1, f) != 1) goto err;
    PresentGame->MapConfig.PlayerSpawn = m->PlayerSpawn;
    
    // read end flag
    if (fread(&m->EndFlag, sizeof(vec2), 1, f) != 1) goto err;

    fclose(f);

    info("Loaded map: %s by %s", m->name, m->author);

    return m;
    
err:
    warn(feof(f) ? "Unexpected end of file in %s" : "Error loading map %s", name);
    fclose(f);
    
    FE_CloseMap(m);
    
    return 0;
}

void FE_Game_SetMap(FE_LoadedMap *m)
{
    if (!m) {
        warn("Passed NULL map to SetMap");
        return;
    }
    if (PresentGame->MapConfig.Loaded)
        FE_CloseMap(map);
    map = m;
    PresentGame->MapConfig.Loaded = true;
}

void FE_RenderLoadedMap(FE_Camera *camera)
{
    if (!PresentGame->MapConfig.Loaded)
        return;
    FE_RenderMap(map, camera);
}

void FE_RenderMap(FE_LoadedMap *m, FE_Camera *camera)
{
    if (!m)
        return;

    if (!flagtexture)
        flagtexture = FE_LoadResource(FE_RESOURCE_TYPE_TEXTURE, "game/map/end.png");

	// render all tiles
    for (size_t i = 0; i < m->tilecount; i++) {
		SDL_Rect r = {m->tiles[i].position.x, m->tiles[i].position.y, m->tilesize, m->tilesize};
        SDL_Rect src = {m->tiles[i].texture_x, m->tiles[i].texture_y, m->atlas->texturesize, m->atlas->texturesize};
        r = FE_ApplyZoom(&r, camera, false);
        // check if we need to render the tile
        if (FE_Camera_Inbounds(&r, &(SDL_Rect){0,0,PresentGame->WindowWidth, PresentGame->WindowHeight})) {
            SDL_RenderCopyEx(PresentGame->Renderer, m->atlas->atlas, &src, &r, m->tiles[i].rotation, NULL, SDL_FLIP_NONE);
        }
	}

    // render finish flag
    if (!vec2_null(m->EndFlag)) {
        SDL_Rect r = (SDL_Rect){m->EndFlag.x, m->EndFlag.y, m->tilesize, m->tilesize};
        FE_RenderCopy(camera, false, flagtexture, NULL, &r);
    }
}

void FE_RenderMapBackground(FE_Camera *camera) {
    FE_RenderMapBG(camera, map);
}

void FE_CloseLoadedMap()
{
    if (!PresentGame->MapConfig.Loaded)
        return;

    FE_CloseMap(map);

    if (flagtexture)
        FE_DestroyResource(flagtexture->path);
    flagtexture = 0;

    PresentGame->MapConfig.Loaded  = false;
    map = 0;
}

void FE_CloseMap(FE_LoadedMap *map)
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
    if (map->atlas && map->atlas->path) {
        FE_DestroyResource(map->atlas->path);
    }
    map->atlas = 0;
    
    if (map->static_bg && map->bg)
        FE_DestroyResource(map->bg->path);
    map->bg = 0;

    if (map->tiles) {
        free(map->tiles);
        map->tiles = 0;
    }
    map->tilecount = 0;
    free(map);

    return;
}

static bool InBounds(SDL_Rect *r)// checks if rect is in map bounds
{
    if (!PresentGame->MapConfig.Loaded)
        return false;

     if (!r)
        return false;

    if (r->y < 0)
        return false;
    if (r->x < 0)
        return false;
    if (r->x + r->w > PresentGame->MapConfig.MapWidth)
        return false;
    if (r->y + r->h > PresentGame->MapConfig.MapHeight)
        return false;
    
    return true;
}

static size_t LeftTileRange(SDL_Rect *r) // calculates the left-most tile near the player to check for collision
{
    /*  check if the rect hasn't moved, if so we can just return the previous value
        Stores the last 5 calculations as we can return for 5 objects if there is no change. This is more
        efficient than searching the list every time when there has been no x movement
    */
    static uint16_t prev_x[5];
    static uint16_t prev_mid[5];
    static uint8_t arr_pt = 0;
    for (size_t i = 0; i < 5; i++) {
        if (r->x == prev_x[i]) {
            return prev_mid[i];
        }
    }
    if (arr_pt == 4)
        arr_pt = 0;
    else
        arr_pt++;

    /* find the tile closest to the player's left side using a binary search */
    Uint16 left = 0;
    Uint16 right = map->tilecount - 1;
    Uint16 mid = 0;
    while (left <= right && right < map->tilecount) {
        mid = (left + right) / 2;
        if (map->tiles[mid].position.x + map->tilesize == r->x) {
            break;
        } else if (map->tiles[mid].position.x + map->tilesize < r->x)
            left = mid + 1;
        else
            right = mid - 1;
    }

    prev_x[arr_pt] = r->x;
    prev_mid[arr_pt] = mid;

    return mid;
}

static size_t RightTileRange(size_t left, SDL_Rect *r) /* calculates the right-most tile near the player to check for collision */
{
    /* check if the rect hasn't moved, if so we can just return the previous value */
    static uint16_t prev_x[5];
    static uint16_t prev_mid[5];
    static uint8_t arr_pt = 0;
    
    for (size_t i = 0; i < 5; i++) {
        if (r->x == prev_x[i]) {
            return prev_mid[i];
        }
    }
    if (arr_pt == 4)
        arr_pt = 0;
    else
        arr_pt++;

    Uint16 r_right = r->x + r->w;
    prev_x[arr_pt] = r_right;

    // find the tile closest to the player's right side
    for (size_t i = left; i < map->tilecount; i++) {
        if (map->tiles[i].position.x > r_right)
            return i;
    }
    prev_mid[arr_pt] = map->tilecount -1;
    return map->tilecount -1;
}

vec2 FE_CheckMapCollisionLeft(SDL_Rect *r)
{
    if (!InBounds(r)) return VEC_NULL;

    size_t left = LeftTileRange(r);
    size_t right = RightTileRange(left, r);

    for (size_t i = left; i < right; i++) {
        // continue if tile is above or below player
        if (map->tiles[i].position.y > r->h + r->y || map->tiles[i].position.y + map->tilesize < r->y)
            continue;
        
        SDL_Rect tilerect = (SDL_Rect){map->tiles[i].position.x, map->tiles[i].position.y, map->tilesize, map->tilesize};
        SDL_Rect out;
        if (!SDL_IntersectRect(r, &tilerect, &out)) {
            continue;
        }
        if (out.h < 10 || out.w == 0)
            continue;
        if (tilerect.x < r->x && out.w > 0)
            return vec(map->tiles[i].position.x + map->tilesize, map->tiles[i].position.y);
    } 
    return VEC_NULL;
}

vec2 FE_CheckMapCollisionRight(SDL_Rect *r)
{
    if (!InBounds(r)) return VEC_NULL;

    size_t left = LeftTileRange(r);
    size_t right = RightTileRange(left, r);

    for (size_t i = left; i < right; i++) {
        // continue if tile is above or below player
        if (map->tiles[i].position.y > r->h + r->y || map->tiles[i].position.y + map->tilesize < r->y)
            continue;

        SDL_Rect tilerect = (SDL_Rect){map->tiles[i].position.x, map->tiles[i].position.y, map->tilesize, map->tilesize};
        SDL_Rect out;
        if (!SDL_IntersectRect(r, &tilerect, &out))
            continue;
        
        if (out.h < 5 || out.w == 0)
            continue;
        if (tilerect.x > r->x && out.w > 0) {
            return map->tiles[i].position;
        }
    } 
    
    return VEC_NULL;
}

vec2 FE_CheckMapCollisionAbove(SDL_Rect *r)
{
    if (!InBounds(r)) return VEC_NULL;
    
    // Only checks the tiles closest to the player's X coordinate to save performance
    size_t left_tile = LeftTileRange(r);
    size_t right_tile = RightTileRange(left_tile, r);

    vec2 position = VEC_NULL;

    // check if each tile is colliding from above with the rect
    for (size_t i = left_tile; i < right_tile; i++) {
        SDL_Rect tilerect = (SDL_Rect){map->tiles[i].position.x, map->tiles[i].position.y, map->tilesize, map->tilesize};
        SDL_Rect collrect;

        if (!SDL_IntersectRect(&tilerect, r, &collrect))
            continue;

        if (r->x + r->w < tilerect.x || r->x > tilerect.x + tilerect.w || collrect.h > map->tilesize)
            continue;

        if (collrect.h > 0) {
            if (map->tiles[i].position.y > position.y)
                position = map->tiles[i].position;
        }

    }
    return position;
}

vec2 FE_CheckMapCollisionBelow(SDL_Rect *r)
{
    if (!InBounds(r)) return VEC_NULL;

    size_t left_tile = LeftTileRange(r);
    size_t right_tile = RightTileRange(left_tile, r);


    // check if each tile is colliding from below with the rect
    for (size_t i = left_tile; i < right_tile; i++) {
        SDL_Rect tilerect = (SDL_Rect){map->tiles[i].position.x, map->tiles[i].position.y, map->tilesize, map->tilesize};
        SDL_Rect collrect;

        if (!SDL_IntersectRect(&tilerect, r, &collrect))
            continue;

        if (collrect.h > 0 && (r->y > tilerect.y + (tilerect.h / 2)))
            return vec(map->tiles[i].position.x, map->tiles[i].position.y + map->tilesize);
    }
    return VEC_NULL;

}