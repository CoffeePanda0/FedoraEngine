#include <SDL.h>
#include "include/camera.h"
#include "include/map.h"
#include "../core/include/fedoraengine.h"
#include "../core/include/utils.h"

static const int layer_width = 1920;
static const int layer_height = 1080;

static SDL_Rect r1 = {0, 800, layer_width, layer_height};
static SDL_Rect r2 = {-layer_width, 800, layer_width, layer_height};

static SDL_Texture *buffer = 0;
static float last_x = 0;
static float last_y = 0;

void FE_RenderMapBG(FE_Camera *camera, FE_LoadedMap *map)
{

    /*  Render background twice so that we can loop background for infinite effect
        This is not hugely efficient and will use a lot of SDL_Render time.
        NOTE: BG width must be at least screen size
        We render to a buffer so that we can just copy the texture if not dirty to improve render time
    */
 
    if (!map || !camera) {
        warn("Passing NULL on rendering BG");
        return;
    }

    if (!buffer)
        buffer = FE_CreateRenderTexture(layer_width, layer_height);
    
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

        SDL_RenderCopy(PresentGame->Renderer, map->bg->Texture, NULL, &bg1);
        SDL_RenderCopy(PresentGame->Renderer, map->bg->Texture, NULL, &bg2);
    }
    
    SDL_SetRenderTarget(PresentGame->Renderer, prev);
    SDL_RenderCopy(PresentGame->Renderer, buffer, NULL, NULL);
    
}