#include "../include/game.h"

#define TEXTURE_PATH "game/map/textures/"

#define MAP_DIRECTORY "game/map/maps/"

static FE_LoadedMap map;
bool FE_MapLoaded = false;

static SDL_Texture *flagtexture;

Uint16 FE_Map_Width = 0;
Uint16 FE_Map_Height = 0;
Uint16 FE_Map_MinimumX = 0;
Uint16 FE_Map_MinimumY = 0;

float GRAVITY;

Vector2D FE_GetSpawn()
{
    return map.PlayerSpawn;
}

int FE_LoadMap(const char *name)
{
    FE_MapLoaded = true;
    if (!flagtexture)
        flagtexture = FE_LoadTexture("game/map/end.png");

    FE_Map_Width = 0;
    FE_Map_Height = 0;
    FE_Map_MinimumX = 0;
    FE_Map_MinimumY = screen_height;

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

    // Read map texture paths
    if (fread(&map.texturecount, sizeof(Uint16), 1, f) != 1) goto err;

    char **texturepaths = xmalloc(sizeof(char*) * map.texturecount);
    for (int i = 0; i < map.texturecount; i++) {
        if (!(texturepaths[i] = ReadStr(f))) goto err;
    }
    
    // load texture from path to textures array
    map.textures = xmalloc(sizeof(SDL_Texture*) * map.texturecount);
    for (int i = 0; i < map.texturecount; i++) {
        map.textures[i] = FE_TextureFromFile(texturepaths[i]);
    }

    // free texturepaths array
    for (int i = 0; i < map.texturecount; i++) 
        free(texturepaths[i]);
    free(texturepaths);

    // load background image
    char *bg_path = 0;
    if (!(bg_path = ReadStr(f))) goto err;

    map.bg = FE_TextureFromFile(bg_path);
    free(bg_path);

    // Read Map tiles and sizes
    if (fread(&map.tilecount, sizeof(Uint16), 1, f) != 1) goto err;
    if (fread(&map.tilesize, sizeof(Uint16), 1, f) != 1) goto err;
    
    bool setminX = false; // check if we have set the map minimum values yet

    map.tiles = xmalloc(sizeof(FE_Map_Tile) * map.tilecount);
    for (int i = 0; i < map.tilecount; i++) {
        if (fread(&map.tiles[i].texture_index, sizeof(Uint16), 1, f) != 1) goto err;
        if (fread(&map.tiles[i].rotation, sizeof(Uint16), 1, f) != 1) goto err;
        if (fread(&map.tiles[i].position, sizeof(Vector2D), 1, f) != 1) goto err;

        // calculate map height and width
        if ((Uint16)map.tiles[i].position.x + map.tilesize > FE_Map_Width) FE_Map_Width = map.tiles[i].position.x + map.tilesize;
        if ((Uint16)map.tiles[i].position.y - screen_height + map.tilesize > FE_Map_Height) FE_Map_Height = map.tiles[i].position.y + map.tilesize - screen_height;
    
        // calculate minimum x point and minimum y point for camera bounds
        if ((Uint16)map.tiles[i].position.x < FE_Map_MinimumX || !setminX) {
            FE_Map_MinimumX = map.tiles[i].position.x;
            setminX = true;
        }
        if ((Uint16)map.tiles[i].position.y < FE_Map_MinimumY) {
            FE_Map_MinimumY = map.tiles[i].position.y;
        }
    }

    FE_Map_MinimumY = screen_height - FE_Map_MinimumY  + (map.tilesize * 2);

    GRAVITY = map.gravity;

    // read player spawn
    if (fread(&map.PlayerSpawn, sizeof(Vector2D), 1, f) != 1) goto err;
    // read end flag
    if (fread(&map.EndFlag, sizeof(Vector2D), 1, f) != 1) goto err;

    fclose(f);

    info("Loaded map '%s'", map.name);

    FE_FreeUI();
    FE_GameState = GAME_STATE_PLAY;
    return 1;
    
err:
    warn(feof(f) ? "Unexpected end of file in %s" : "Error loading map %s", name);
    fclose(f);
    
    FE_CloseMap();
    
    return -1;
}

void FE_RenderMap(FE_Camera *camera)
{
	// render all tiles
	for (size_t i = 0; i < map.tilecount; i++) {
		SDL_Rect r = (SDL_Rect){map.tiles[i].position.x - camera->x, map.tiles[i].position.y - camera->y, map.tilesize, map.tilesize};
		FE_RenderCopyEx(map.textures[map.tiles[i].texture_index], NULL, &r, map.tiles[i].rotation, SDL_FLIP_NONE);
	} 

    // render finish flag
    SDL_Rect r = (SDL_Rect){map.EndFlag.x - camera->x, map.EndFlag.y - camera->y, map.tilesize, map.tilesize};
    FE_RenderCopy(flagtexture, NULL, &r);

}

void FE_RenderMapBackground(FE_Camera *camera)
{
    // render background
	SDL_Rect bgrect = (SDL_Rect){0,0,0,0};
	SDL_QueryTexture(map.bg, NULL, NULL, &bgrect.w ,&bgrect.h);
	bgrect.x -= camera->x;
	bgrect.y -= camera->y;
	SDL_RenderCopy(renderer, map.bg, NULL, &bgrect);
}

void FE_CloseMap()
{
    if (!FE_MapLoaded)
        return;

    if (flagtexture)
        FE_FreeTexture(flagtexture);
    flagtexture = 0;

    if (map.name)
        xfree(map.name);
    map.name = 0;
    

    if (map.textures) {
        for (size_t i = 0; i < map.texturecount; i++) {
            if (map.textures[i])
                SDL_DestroyTexture(map.textures[i]);
        }
        free(map.textures);
        map.textures = 0;
    }
    map.texturecount = 0;
    
    if (map.bg)
        SDL_DestroyTexture(map.bg);
    map.bg = 0;

    if (map.tiles) {
        free(map.tiles);
        map.tiles = 0;
    }
    map.tilecount = 0;

    FE_MapLoaded = false;
    return;
}

static bool InBounds(SDL_Rect *r)// checks if rect is in map bounds
{
    if (r->y < 0)
        return false;
    
    if (r->x < 0)
        return false;

    if (r->x + r->w > FE_Map_Width)
        return false;

    return true;
}

static size_t LeftTileRange(SDL_Rect *r) // calculates the left-most tile near the player to check for collision
{
    int r_left = r->x;

    // find the tile closest to the player's left side using a binary search
    int left = 0;
    int right = map.tilecount - 1;
    int mid = 0;
    while (left <= right) {
        mid = (left + right) / 2;
        if (map.tiles[mid].position.x == r_left)
            break;
        else if (map.tiles[mid].position.x < r_left)
            left = mid + 1;
        else
            right = mid - 1;
    }
    return clamp(mid - 2, 0, map.tilecount);
}

static size_t RightTileRange(size_t left, SDL_Rect *r) // calculates the right-most tile near the player to check for collision
{
    int r_right = r->x + r->w;

    size_t right_tile = left;
    while (right_tile < map.tilecount && map.tiles[right_tile].position.x <= r_right) {
        right_tile++;
    }
    return right_tile;
}

Vector2D FE_CheckMapCollisionLeft(SDL_Rect *r)
{
    if (!r || !InBounds(r)) return VEC_NULL;

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
            return map.tiles[i].position;
    } 
    return VEC_NULL;
}

Vector2D FE_CheckMapCollisionRight(SDL_Rect *r)
{
    if (!r || !InBounds(r)) return VEC_NULL;

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
        if (tilerect.x > r->x && out.w > 0)
            return map.tiles[i].position;
    } 
    return VEC_NULL;
}

Vector2D FE_CheckMapCollisionAbove(SDL_Rect *r)
{
    if (!r || !InBounds(r)) return VEC_NULL;

    // Check if the object is in map bounds
    if (r->x < FE_Map_MinimumX || r->x + r->w > FE_Map_Width)
        return VEC_NULL;
    
    // Only checks the tiles closest to the player's X coordinate to save performance
    size_t left_tile = LeftTileRange(r);
    size_t right_tile = RightTileRange(left_tile, r);

    // check if each tile is colliding from above with the rect
    for (size_t i = left_tile; i < right_tile; i++) {
        SDL_Rect tilerect = (SDL_Rect){map.tiles[i].position.x, map.tiles[i].position.y, map.tilesize, map.tilesize};
        SDL_Rect collrect;

        if (!SDL_IntersectRect(&tilerect, r, &collrect))
            continue;

        if (collrect.h > 0)
            return map.tiles[i].position;
    }
    return VEC_NULL;
}