#include "../core/include/include.h"
#include "../entity/include/prefab.h"

#include "include/map.h"
#include "include/background.h"

#define MAP_DIRECTORY "game/map/maps/"

static FE_Texture *flagtexture;

static void DestroyRenderer(void *data)
{
    FE_LoadedMap *map = (FE_LoadedMap*)data;

    if (!map) {
        warn("Passing NULL map to FE_Map_DestroyRenderer");
        return;
    }

    if (map->r) {
        if (map->r->atlas && map->r->atlas->path) {
            FE_DestroyResource(map->r->atlas->path);
        }
        map->r->atlas = 0;
        
        if (map->r->static_bg && map->r->bg)
            FE_DestroyResource(map->r->bg->path);
        map->r->bg = 0;
        free(map->r);
    }
}

void FE_Map_RenderLoaded(FE_Camera *camera)
{
    if (!PresentGame->MapConfig.Loaded)
        return;
    FE_Map_Render(PresentMap, camera);
}

void FE_Map_Render(FE_LoadedMap *m, FE_Camera *camera)
{
    if (!m)
        return;

    if (!flagtexture)
        flagtexture = FE_LoadResource(FE_RESOURCE_TYPE_TEXTURE, "game/map/end.png");

	// render all tiles
    for (size_t i = 0; i < m->tilecount; i++) {
		SDL_Rect r = {m->tiles[i].position.x, m->tiles[i].position.y, m->tilesize, m->tilesize};
        SDL_Rect src = {m->tiles[i].texture_x, m->tiles[i].texture_y, m->r->atlas->texturesize, m->r->atlas->texturesize};
        r = FE_ApplyZoom(&r, camera, false);
        // check if we need to render the tile
        if (FE_Camera_Inbounds(&r, &(SDL_Rect){0,0,PresentGame->WindowWidth, PresentGame->WindowHeight})) {
            SDL_RenderCopyEx(PresentGame->Client->Renderer, m->r->atlas->atlas, &src, &r, m->tiles[i].rotation, NULL, SDL_FLIP_NONE);
        }
	}

    // render finish flag
    if (!vec2_null(m->EndFlag)) {
        SDL_Rect r = (SDL_Rect){m->EndFlag.x, m->EndFlag.y, m->tilesize, m->tilesize};
        FE_RenderCopy(camera, false, flagtexture, NULL, &r);
    }
}

void FE_Map_RenderBackground(FE_Camera *camera) {
    FE_Map_RenderBG(camera, PresentMap);
}

FE_LoadedMap *FE_Map_Load(const char *name)
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

    // Create map renderer struct
    m->r = xmalloc(sizeof(FE_MapRenderer));
    m->destroy_cb = &DestroyRenderer;

    // Read map name
    if (!(m->name = FE_File_ReadStr(f))) goto err;
    
    // Read author
    if (!(m->author = FE_File_ReadStr(f))) goto err;

    // read gravity
    if (fread(&m->gravity, sizeof(float), 1, f) != 1) goto err;
    
    // load texture atlas
    char *atlas_path = 0;
    if (!(atlas_path = FE_File_ReadStr(f))) goto err;
    m->r->atlas = FE_LoadResource(FE_RESOURCE_TYPE_ATLAS, atlas_path);
    free(atlas_path);

    if (fread(&m->r->atlas->texturesize, sizeof(uint16_t), 1, f) != 1) goto err;

    // load background image
    if (fread(&m->r->static_bg, sizeof(bool), 1, f) != 1) goto err;
    char *bg_path = 0;
    if (!(bg_path = FE_File_ReadStr(f))) goto err;
    
    char *parallax_path = 0;
    if (!(parallax_path = FE_File_ReadStr(f))) goto err;
    
    if (m->r->static_bg) {
        m->r->bg = FE_LoadResource(FE_RESOURCE_TYPE_TEXTURE, bg_path);
    } else {
        m->r->bg = 0;
        if (!parallax_path || mstrlen(parallax_path) == 0)
            goto err;
        FE_Parallax_Load(parallax_path);
        free(parallax_path);
    }
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
        FE_Prefab_Create(_name, x, y);
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

