#include "../core/include/include.h"
#include "include/lighting.h"

#define LIGHT_DIRECTORY "game/sprites/lightfx/"

static SDL_Texture *light_layer;
static SDL_Texture *result_layer;

static FE_List *lights;

static bool initialised = false;

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

    return light;
}

void FE_RenderLighting(FE_Camera *camera, SDL_Texture *world)
{
	Uint8 brightness = PresentGame->MapConfig.AmbientLight;

    if (brightness == 255) {
        SDL_SetRenderTarget(PresentGame->renderer, NULL);
        SDL_RenderCopy(PresentGame->renderer, world, NULL, NULL);
        return;
    }

    // Create an layer to render the lighting to
	SDL_SetRenderTarget(PresentGame->renderer, light_layer);
	SDL_SetTextureBlendMode(light_layer, SDL_BLENDMODE_MOD);
	SDL_SetRenderDrawColor(PresentGame->renderer, brightness, brightness, brightness, 0);
	SDL_RenderClear(PresentGame->renderer);
	
	// render the lights
    for (FE_List *l = lights; l; l = l->next) {
        FE_Light *light = l->data;
        FE_RenderCopy(camera, false, light->Texture, NULL, &light->Rect);
    }

    // Layer the world onto the lighting layer, using mod blending
    SDL_SetRenderTarget(PresentGame->renderer, result_layer);
    SDL_SetRenderDrawColor(PresentGame->renderer, 0, 0, 0, 0);
    SDL_SetRenderDrawBlendMode(PresentGame->renderer, SDL_BLENDMODE_MOD);
    SDL_RenderClear(PresentGame->renderer);

    // Render the world with the applied light effects
    SDL_RenderCopy(PresentGame->renderer, world, NULL, NULL);
    SDL_RenderCopy(PresentGame->renderer, light_layer, NULL, NULL);
    SDL_SetRenderTarget(PresentGame->renderer, NULL);
    SDL_RenderCopy(PresentGame->renderer, result_layer, NULL, NULL);
}

void FE_DestroyLight(FE_Light *light)
{
    FE_List_Remove(&lights, light);
    FE_DestroyResource(light->Texture->path);
    free(light);
}

void FE_InitLighting()
{
    light_layer = FE_CreateRenderTexture(PresentGame->window_width, PresentGame->window_height);
    result_layer = FE_CreateRenderTexture(PresentGame->window_width, PresentGame->window_height);
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