#include <FE_Common.h>
#include <file.h>
#include "include/include.h"

#include "../client/core/include/render.h"
#include "../client/core/include/texture.h"
#include "../client/ui/include/ui.h"
#include "../client/ui/include/menu.h"
#include "../client/entity/include/prefab.h"

#define TEXTURE_PATH "game/map/textures/"
#define BG_PATH "game/map/backgrounds/"
#define MAP_DIRECTORY "game/map/maps/"

static FE_Texture *flagtexture;

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
            SDL_RenderCopyEx(PresentGame->Renderer, m->r->atlas->atlas, &src, &r, m->tiles[i].rotation, NULL, SDL_FLIP_NONE);
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