#include "../core/include/include.h"
#include "include/include.h"
#include "../ui/include/ui.h"

#define TEXTURE_PATH "game/map/textures/"
#define BG_PATH "game/map/backgrounds/"
#define MAP_DIRECTORY "game/map/maps/"

static FE_LoadedMap map;

static FE_Texture *flagtexture;

int FE_LoadMap(const char *name)
{
    PresentGame->MapConfig.Loaded  = true;
    if (!flagtexture)
        flagtexture = FE_LoadResource(FE_RESOURCE_TYPE_TEXTURE, "game/map/end.png");

    PresentGame->MapConfig.MapWidth = 0;
    PresentGame->MapConfig.MapHeight = 0;
    PresentGame->MapConfig.MinimumX = 0;

    // Combine map directory and map file path
    char *map_path = AddStr(MAP_DIRECTORY, name);
    // Read map file from binary format
    FILE *f = fopen(map_path, "rb");
    if (f == NULL) {
        warn("Failed to open map file: %s", name);
        FE_Menu_MainMenu(); // return to main menu when we can't open map
        xfree(map_path);
        return -1;
    }
    xfree(map_path);

    // Read map name
    if (!(map.name = ReadStr(f))) goto err;
    
    // read gravity
    if (fread(&map.gravity, sizeof(float), 1, f) != 1) goto err;
    
    // load texture atlas
    char *atlas_path = 0;
    if (!(atlas_path = ReadStr(f))) goto err;
    map.atlas = FE_LoadTextureAtlas(atlas_path);
    xfree(atlas_path);


    if (fread(&map.atlas->texturesize, sizeof(Uint16), 1, f) != 1) goto err;
    FE_QueryTexture(map.atlas->atlas, &map.atlas->width, &map.atlas->height);

    // load background image
    char *bg_path = 0;
    if (!(bg_path = ReadStr(f))) goto err;

    map.bg = FE_LoadResource(FE_RESOURCE_TYPE_TEXTURE, bg_path);
    xfree(bg_path);

    // Read Map tiles and sizes
    if (fread(&map.tilecount, sizeof(Uint16), 1, f) != 1) goto err;
    if (fread(&map.tilesize, sizeof(Uint16), 1, f) != 1) goto err;
    
    bool setminX = false; // check if we have set the map minimum values yet

    map.tiles = xmalloc(sizeof(FE_Map_Tile) * map.tilecount);
    for (int i = 0; i < map.tilecount; i++) {
        if (fread(&map.tiles[i].texture_x, sizeof(Uint16), 1, f) != 1) goto err;
        if (fread(&map.tiles[i].texture_y, sizeof(Uint16), 1, f) != 1) goto err;
        if (fread(&map.tiles[i].rotation, sizeof(Uint16), 1, f) != 1) goto err;
        if (fread(&map.tiles[i].position, sizeof(Vector2D), 1, f) != 1) goto err;

        // calculate map height and width
        if ((Uint16)map.tiles[i].position.x + map.tilesize > PresentGame->MapConfig.MapWidth) PresentGame->MapConfig.MapWidth = map.tiles[i].position.x + map.tilesize;
        if ((Uint16)map.tiles[i].position.y - PresentGame->Window_height + map.tilesize > PresentGame->MapConfig.MapHeight) PresentGame->MapConfig.MapHeight = map.tiles[i].position.y + map.tilesize - PresentGame->MapConfig.MapHeight;
    
        // calculate minimum x point and minimum y point for camera bounds
        if ((Uint16)map.tiles[i].position.x < PresentGame->MapConfig.MinimumX || !setminX) {
            PresentGame->MapConfig.MinimumX = map.tiles[i].position.x;
            setminX = true;
        }
        
        // calculate highest Y value
        if ((Uint16)map.tiles[i].position.y + map.tilesize > PresentGame->MapConfig.MapHeight) PresentGame->MapConfig.MapHeight = map.tiles[i].position.y + map.tilesize;
    }

    PresentGame->MapConfig.Gravity = map.gravity;

    // read player spawn
    if (fread(&map.PlayerSpawn, sizeof(Vector2D), 1, f) != 1) goto err;
    PresentGame->MapConfig.PlayerSpawn = map.PlayerSpawn;
    
    // read end flag
    if (fread(&map.EndFlag, sizeof(Vector2D), 1, f) != 1) goto err;

    fclose(f);
    
    info("Loaded map '%s'", map.name);

    return 1;
    
err:
    warn(feof(f) ? "Unexpected end of file in %s" : "Error loading map %s", name);
    fclose(f);
    
    FE_CloseMap();
    
    return -1;
}

void FE_RenderMap(FE_Camera *camera)
{
    if (!PresentGame->MapConfig.Loaded )
        return;

	// render all tiles
    for (size_t i = 0; i < map.tilecount; i++) {
		SDL_Rect r = {map.tiles[i].position.x, map.tiles[i].position.y, map.tilesize, map.tilesize};
        SDL_Rect src = {map.tiles[i].texture_x, map.tiles[i].texture_y, map.atlas->texturesize, map.atlas->texturesize};
		FE_RenderCopyEx(camera, false, map.atlas->atlas, &src, &r, map.tiles[i].rotation, SDL_FLIP_NONE);
	} 

    // render finish flag
    SDL_Rect r = (SDL_Rect){map.EndFlag.x, map.EndFlag.y, map.tilesize, map.tilesize};
    FE_RenderCopy(camera, false, flagtexture, NULL, &r);

}

void FE_RenderMapBackground(FE_Camera *camera) {
    FE_RenderMapBG(camera, &map);
}

void FE_CloseMap()
{
    if (!PresentGame->MapConfig.Loaded )
        return;

    if (flagtexture)
        FE_DestroyResource(flagtexture->path);
    flagtexture = 0;

    if (map.name)
        xfree(map.name);
    map.name = 0;
    
    map.PlayerSpawn = VEC_NULL;

    if (map.atlas) {
        FE_DestroyResource(map.atlas->atlas->path);
        free(map.atlas);
    }
    map.atlas = 0;
    
    if (map.bg)
        FE_DestroyResource(map.bg->path);
    map.bg = 0;

    if (map.tiles) {
        free(map.tiles);
        map.tiles = 0;
    }
    map.tilecount = 0;

    PresentGame->MapConfig.Loaded  = false;
    return;
}

static bool InBounds(SDL_Rect *r)// checks if rect is in map bounds
{
    if (!PresentGame->MapConfig.Loaded )
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

    // find the tile closest to the player's left side using a binary search
    Uint16 left = 0;
    Uint16 right = map.tilecount - 1;
    Uint16 mid = 0;
    while (left <= right && right < map.tilecount) {
        mid = (left + right) / 2;
        if (map.tiles[mid].position.x + map.tilesize == r->x) {
            break;
        } else if (map.tiles[mid].position.x + map.tilesize < r->x)
            left = mid + 1;
        else
            right = mid - 1;
    }

    prev_x[arr_pt] = r->x;
    prev_mid[arr_pt] = mid;

    return mid;
}

static size_t RightTileRange(size_t left, SDL_Rect *r) // calculates the right-most tile near the player to check for collision
{
    // check if the rect hasn't moved, if so we can just return the previous value
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
    for (size_t i = left; i < map.tilecount; i++) {
        if (map.tiles[i].position.x > r_right)
            return i;
    }
    prev_mid[arr_pt] = map.tilecount -1;
    return map.tilecount -1;
}

Vector2D FE_CheckMapCollisionLeft(SDL_Rect *r)
{
    if (!InBounds(r)) return VEC_NULL;

    size_t left = LeftTileRange(r);
    size_t right = RightTileRange(left, r);

    for (size_t i = left; i < right; i++) {
        // continue if tile is above or below player
        if (map.tiles[i].position.y > r->h + r->y || map.tiles[i].position.y + map.tilesize < r->y)
            continue;
        
        SDL_Rect tilerect = (SDL_Rect){map.tiles[i].position.x, map.tiles[i].position.y, map.tilesize, map.tilesize};
        SDL_Rect out;
        if (!SDL_IntersectRect(r, &tilerect, &out)) {
            continue;
        }
        if (out.h < 10 || out.w == 0)
            continue;
        if (tilerect.x < r->x && out.w > 0)
            return FE_NewVector(map.tiles[i].position.x + map.tilesize, map.tiles[i].position.y);
    } 
    return VEC_NULL;
}

Vector2D FE_CheckMapCollisionRight(SDL_Rect *r)
{
    if (!InBounds(r)) return VEC_NULL;

    size_t left = LeftTileRange(r);
    size_t right = RightTileRange(left, r);

    for (size_t i = left; i < right; i++) {
        // continue if tile is above or below player
        if (map.tiles[i].position.y > r->h + r->y || map.tiles[i].position.y + map.tilesize < r->y)
            continue;

        SDL_Rect tilerect = (SDL_Rect){map.tiles[i].position.x, map.tiles[i].position.y, map.tilesize, map.tilesize};
        SDL_Rect out;
        if (!SDL_IntersectRect(r, &tilerect, &out))
            continue;
        
        if (out.h < 5 || out.w == 0)
            continue;
        if (tilerect.x > r->x && out.w > 0) {
            return map.tiles[i].position;
        }
    } 
    
    return VEC_NULL;
}

Vector2D FE_CheckMapCollisionAbove(SDL_Rect *r)
{
    if (!InBounds(r)) return VEC_NULL;
    
    // Only checks the tiles closest to the player's X coordinate to save performance
    size_t left_tile = LeftTileRange(r);
    size_t right_tile = RightTileRange(left_tile, r);

    Vector2D position = VEC_NULL;

    // check if each tile is colliding from above with the rect
    for (size_t i = left_tile; i < right_tile; i++) {
        SDL_Rect tilerect = (SDL_Rect){map.tiles[i].position.x, map.tiles[i].position.y, map.tilesize, map.tilesize};
        SDL_Rect collrect;

        if (!SDL_IntersectRect(&tilerect, r, &collrect))
            continue;

        if (r->x + r->w < tilerect.x || r->x > tilerect.x + tilerect.w || collrect.h > map.tilesize)
            continue;

        if (collrect.h > 0) {
            if (map.tiles[i].position.y > position.y)
                position = map.tiles[i].position;
        }

    }
    return position;
}

Vector2D FE_CheckMapCollisionBelow(SDL_Rect *r)
{
    if (!InBounds(r)) return VEC_NULL;

    size_t left_tile = LeftTileRange(r);
    size_t right_tile = RightTileRange(left_tile, r);


    // check if each tile is colliding from below with the rect
    for (size_t i = left_tile; i < right_tile; i++) {
        SDL_Rect tilerect = (SDL_Rect){map.tiles[i].position.x, map.tiles[i].position.y, map.tilesize, map.tilesize};
        SDL_Rect collrect;

        if (!SDL_IntersectRect(&tilerect, r, &collrect))
            continue;

        if (collrect.h > 0 && (r->y > tilerect.y + (tilerect.h / 2)))
            return FE_NewVector(map.tiles[i].position.x, map.tiles[i].position.y + map.tilesize);
    }
    return VEC_NULL;

}