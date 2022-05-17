#include "../core/include/include.h"
#include "include/lighting.h"

#define LIGHT_DIRECTORY "game/sprites/lightfx/"

static SDL_Texture *light_layer;
static bool light_layer_dirty = true;

static SDL_Texture *result_layer;

static FE_List *lights;

static bool initialised = false;

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

FE_Light *FE_CreateLight(SDL_Rect rect, const char *texture)
{
    if (!initialised) {
        warn("Trying to add world light before initialising");
        return NULL;
    }

    // Create new light object
    FE_Light *light = xmalloc(sizeof(FE_Light));
    light->Rect = rect;

    // Create absoloute path
    char *path = AddStr(LIGHT_DIRECTORY, texture);
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
    static float last_camera_x = 0;
    static float last_camera_y = 0;

    /* Check if brightness or camera position has changed */
    if (last_brightness != PresentGame->MapConfig.AmbientLight || last_camera_x != camera->x || last_camera_y != camera->y) {
        last_brightness = PresentGame->MapConfig.AmbientLight;
        last_camera_x = camera->x;
        last_camera_y = camera->y;
        light_layer_dirty = true;
        return;
    }
}

void FE_RenderLighting(FE_Camera *camera, SDL_Texture *world)
{
    /* NOTE: This function is not very well optimised and will use a lot of SDL_Render time. */

	Uint8 brightness = PresentGame->MapConfig.AmbientLight;

    /* Disable lighting if brightness is 0 */
    if (brightness == 255) {
        SDL_SetRenderTarget(PresentGame->Renderer, NULL);
        SDL_RenderCopy(PresentGame->Renderer, world, NULL, NULL);
        return;
    }

    CheckIfLightDirty(camera); // todo move rect instead of re-rendering

    /* Create an layer to render the lighting to. Only re-render if lighting has changed. */
    if (light_layer_dirty) {
        SDL_SetRenderTarget(PresentGame->Renderer, light_layer);
        SDL_SetTextureBlendMode(light_layer, SDL_BLENDMODE_MOD);
        SDL_SetRenderDrawColor(PresentGame->Renderer, brightness, brightness, brightness, 0);
        SDL_RenderClear(PresentGame->Renderer);
	
        // render the lights
        for (FE_List *l = lights; l; l = l->next) {
            FE_Light *light = l->data;
            FE_RenderCopy(camera, false, light->Texture, NULL, &light->Rect);
        }
        light_layer_dirty = false;
    }

    // Layer the world onto the lighting layer, using mod blending
    SDL_SetRenderTarget(PresentGame->Renderer, result_layer);
    SDL_SetRenderDrawColor(PresentGame->Renderer, 0, 0, 0, 0);
    SDL_SetRenderDrawBlendMode(PresentGame->Renderer, SDL_BLENDMODE_MOD);
    SDL_RenderClear(PresentGame->Renderer);

    // Render the world with the applied light effects
    SDL_RenderCopy(PresentGame->Renderer, world, NULL, NULL);
    SDL_RenderCopy(PresentGame->Renderer, light_layer, NULL, NULL);
    SDL_SetRenderTarget(PresentGame->Renderer, NULL);
    SDL_SetRenderDrawBlendMode(PresentGame->Renderer, SDL_BLENDMODE_NONE);
    SDL_RenderCopy(PresentGame->Renderer, result_layer, NULL, NULL);
}

void FE_DestroyLight(FE_Light *light)
{
    FE_List_Remove(&lights, light);
    FE_DestroyResource(light->Texture->path);
    free(light);
    light_layer_dirty = true;
}

void FE_InitLighting()
{
    light_layer = FE_CreateRenderTexture(PresentGame->Window_width, PresentGame->Window_height);
    result_layer = FE_CreateRenderTexture(PresentGame->Window_width, PresentGame->Window_height);
    initialised = true;
}

void FE_CleanLighting()
{
    if (!initialised) return;
    
    SDL_DestroyTexture(light_layer);
    SDL_DestroyTexture(result_layer);
    FE_List_Destroy(&lights);
    initialised = false;
}