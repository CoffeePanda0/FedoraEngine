#include <SDL.h>
#include "include/camera.h"
#include "include/map.h"
#include "include/mapbg.h"
#include "../core/include/include.h"

#define PARALLAX_DIRECTORY "game/map/parallax/"

static const int layer_width = 1920;
static const int layer_height = 1080; 

static float last_x;
static float last_y;

static FE_Map_Parallax *parallax;
static size_t parallax_layers;
static float parallax_speed = 1.0f;
static bool parallax_dirty = false;

void FE_Parallax_SetSpeed(float speed)
{
    parallax_speed = speed;
    parallax_dirty = true;
}

void FE_Parallax_Add(const char *layer_name, float scale)
{
    if (!parallax)
        parallax = xmalloc(sizeof(FE_Map_Parallax));
    else
        parallax = xrealloc(parallax, sizeof(FE_Map_Parallax) * (parallax_layers + 1));
    
    char *path = AddStr(PARALLAX_DIRECTORY, layer_name);
    parallax[parallax_layers].texture = FE_LoadResource(FE_RESOURCE_TYPE_TEXTURE, path);
    free(path);

    parallax[parallax_layers].scale = scale;
    parallax[parallax_layers].r1 = (SDL_Rect){0, 600, layer_width, layer_height};
    parallax[parallax_layers++].r2 = (SDL_Rect){-layer_width, 600, layer_width, layer_height};
    parallax_dirty = true;
}

static void FE_Parallax_Render(FE_Camera *camera)
{
    if (!parallax) return;

    SDL_Rect vis_rect = SCREEN_RECT(camera);

    static SDL_Texture *buff = 0;

    // Render to a buffer so we can just re-render texture if camera hasn't moved
    if (!buff) {
        buff = FE_CreateRenderTexture(PresentGame->Window_width, PresentGame->Window_width);
        SDL_SetTextureBlendMode(buff, SDL_BLENDMODE_BLEND);
    }

    if (last_x != camera->x || last_y != camera->y || parallax_dirty) {
        last_x = camera->x; last_y = camera->y;

        for (size_t i = 0; i < parallax_layers; i++) {
            float scale = parallax[i].scale * parallax_speed;
            
            // Apply scale to the camera position
            SDL_Rect dst1 = {
                .x = (parallax[i].r1.x - (camera->x * scale)) * camera->zoom,
                .y = (parallax[i].r1.y -camera->y) * camera->zoom,
                .w = layer_width * camera->zoom,
                .h = layer_height * camera->zoom
            };
            
            // Apply scale to the camera position (second rect for infinite scrolling)
            SDL_Rect dst2 = {
                .x = (parallax[i].r2.x - (camera->x * scale)) * camera->zoom,
                .y = dst1.y,
                .w = layer_width * camera->zoom,
                .h = layer_height * camera->zoom
            };

            // Check if the rects need to be moved to scroll horizontally
            if (dst2.x + dst2.w < 0)
                parallax[i].r2.x = parallax[i].r1.x + parallax[i].r1.w;
            if (dst1.x + dst1.w < 0)
                parallax[i].r1.x = parallax[i].r2.x + parallax[i].r2.w;
            if (dst1.x > PresentGame->Window_width)
                parallax[i].r1.x = parallax[i].r2.x - parallax[i].r2.w;
            if (dst2.x > PresentGame->Window_width)
                parallax[i].r2.x = parallax[i].r1.x - parallax[i].r1.w;

            SDL_Texture *prev = SDL_GetRenderTarget(PresentGame->Renderer);
            SDL_SetRenderTarget(PresentGame->Renderer, buff);

            SDL_RenderCopy(PresentGame->Renderer, parallax[i].texture->Texture, &vis_rect, &dst1);
            SDL_RenderCopy(PresentGame->Renderer, parallax[i].texture->Texture, &vis_rect, &dst2);

            SDL_SetRenderTarget(PresentGame->Renderer, prev);
            parallax_dirty = false;
        }
    }
    SDL_RenderCopy(PresentGame->Renderer, buff, &vis_rect, NULL);
}

void FE_RenderMapBG(FE_Camera *camera, FE_LoadedMap *map)
{
    /*  Render background twice so that we can loop background for infinite effect
        This is not hugely efficient and will use a lot of SDL_Render time.
        NOTE: BG width must be at least screen size
        We render to a buffer so that we can just copy the texture if not dirty to improve render time
    */

    if (parallax) {
        FE_Parallax_Render(camera);
        return;
    }
    
    if (!map || !camera) {
        warn("Passing NULL on rendering BG");
        return;
    }

    SDL_Rect vis_rect = SCREEN_RECT(camera);

    static SDL_Texture *buffer = 0;
    static SDL_Rect r1 = {0, 800, layer_width, layer_height};
    static SDL_Rect r2 = {-layer_width, 800, layer_width, layer_height};

    if (!buffer)
        buffer = FE_CreateRenderTexture(PresentGame->Window_width, PresentGame->Window_height);
    
    SDL_Texture *prev = SDL_GetRenderTarget(PresentGame->Renderer);
    SDL_SetRenderTarget(PresentGame->Renderer, buffer);

    if (last_x != camera->x || last_y != camera->y) {
        // todo: this code is a monster and should not be reckoned with
        last_x = camera->x;
        last_y = camera->y;

        SDL_Rect bg1 = r1;
        SDL_Rect bg2 = r2;

        if (bg2.x - camera->x + bg2.w < 0)
            r2.x = r1.x + bg1.w;
        if (bg1.x - camera->x + bg1.w < 0)
            r1.x = r2.x + r1.w;
        if (bg1.x - camera->x + bg1.w > camera->x + PresentGame->Window_width)
            r1.x = r2.x - r2.w;
        
        bg1.y -= camera->y;
        bg1.x -= camera->x * camera->zoom;
        bg1.y *= camera->zoom;
        bg1.w *= camera->zoom;
        bg1.h *= camera->zoom;
        bg2 = bg1;
        bg2.x = (r2.x - camera->x) * camera->zoom;

        SDL_RenderCopy(PresentGame->Renderer, map->bg->Texture, &vis_rect, &bg1);
        SDL_RenderCopy(PresentGame->Renderer, map->bg->Texture, &vis_rect, &bg2);
    }
    
    SDL_SetRenderTarget(PresentGame->Renderer, prev);
    SDL_RenderCopy(PresentGame->Renderer, buffer, &vis_rect, NULL);
    
}