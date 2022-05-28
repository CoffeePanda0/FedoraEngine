#include "../core/include/include.h"
#include "include/lighting.h"

#define LIGHT_DIRECTORY "game/sprites/lightfx/"

static SDL_Texture *light_layer;
static bool light_layer_dirty = true;

static FE_List *lights;

void FE_MoveLight(FE_Light *light, int x, int y)
{
    if (!light) {
        warn("NULL light passed! (FE_MoveLight)");
        return;
    }
    light->Rect.x = x;
    light->Rect.y = y;
    light_layer_dirty = true;
}

void FE_ToggleLight(FE_Light *light)
{
    if (!light) {
        warn("NULL light passed! (FE_ToggleLight)");
        return;
    }
    light->enabled = !light->enabled;
    light_layer_dirty = true;
}

FE_Light *FE_CreateLight(SDL_Rect rect, const char *texture)
{
    if (!light_layer) {
        light_layer = FE_CreateRenderTexture(PresentGame->WindowWidth, PresentGame->WindowHeight);
    }

    // Create new light object
    FE_Light *light = xmalloc(sizeof(FE_Light));
    light->Rect = rect;
    light->enabled = true;
    light->intensity = 255;

    // Create absoloute path
    char *path = mstradd(LIGHT_DIRECTORY, texture);
    light->Texture = FE_LoadResource(FE_RESOURCE_TYPE_TEXTURE, path);
    free(path);

    // Add to list
    FE_List_Add(&lights, light);

    light_layer_dirty = true;

    return light;
}

static void CheckIfLightDirty(FE_Camera *camera)
{
    static Uint8 last_brightness = 0;
    static float last_zoom = 1.0f;

    /* Check if brightness or camera zoom has changed */
    if (last_brightness != PresentGame->MapConfig.AmbientLight || last_zoom != camera->zoom) {
        last_brightness = PresentGame->MapConfig.AmbientLight;
        last_zoom = camera->zoom;
        light_layer_dirty = true;
        return;
    }
}

void FE_RenderLighting(FE_Camera *camera, SDL_Texture *world)
{
	Uint8 brightness = PresentGame->MapConfig.AmbientLight;
    SDL_Rect vis_rect = SCREEN_RECT(camera);

    /* Disable lighting if brightness is 0 */
    if (brightness == 255) {
        SDL_SetRenderTarget(PresentGame->Renderer, NULL);
        // Only render the part of the world that is visible
        SDL_RenderCopy(PresentGame->Renderer, world, &vis_rect, NULL);
        return;
    }

    CheckIfLightDirty(camera);

    /* Create an layer to render the lighting to. Only re-render if lighting has changed. */
    if (light_layer_dirty || 1) {
        SDL_SetRenderTarget(PresentGame->Renderer, light_layer);
        SDL_SetTextureBlendMode(light_layer, SDL_BLENDMODE_MOD);
        SDL_SetRenderDrawColor(PresentGame->Renderer, brightness, brightness, brightness, 0);
        SDL_RenderClear(PresentGame->Renderer);
	
        // render the lights
        for (FE_List *l = lights; l; l = l->next) {
            FE_Light *light = l->data;
            if (!light->enabled) continue;
            // apply intensity
            SDL_SetTextureAlphaMod(light->Texture->Texture, light->intensity);
            FE_RenderCopy(camera, false, light->Texture,  NULL, &light->Rect);
            SDL_SetTextureAlphaMod(light->Texture->Texture, 255);
        }
        light_layer_dirty = false;
    }

    // Render the world with the applied light effects
    SDL_SetRenderTarget(PresentGame->Renderer, NULL);
    SDL_RenderCopy(PresentGame->Renderer, world,  &vis_rect, NULL);
    SDL_RenderCopy(PresentGame->Renderer, light_layer,  &vis_rect, NULL);
    SDL_SetRenderDrawBlendMode(PresentGame->Renderer, SDL_BLENDMODE_NONE);
}

void FE_DestroyLight(FE_Light *light)
{
    FE_List_Remove(&lights, light);
    FE_DestroyResource(light->Texture->path);
    free(light);
    light_layer_dirty = true;
}

void FE_CleanLighting()
{
    if (light_layer)    
        SDL_DestroyTexture(light_layer);
    light_layer = 0;

    if (lights) {
        for (FE_List *l = lights; l; l = l->next) {
            FE_Light *light = l->data;
            if (!light || !light->Texture) continue;
            FE_DestroyResource(light->Texture->path);
            free(light);
        }
        FE_List_Destroy(&lights);
    }
    
    lights = 0;
}