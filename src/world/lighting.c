#include "../core/include/include.h"
#include "include/lighting.h"

#define LIGHT_DIRECTORY "game/sprites/lightfx/"

static GPU_Image *light_layer;
static bool light_layer_dirty = true;

static FE_List *lights;

void FE_Light_SetIntensity(FE_Light *light, uint8_t intensity)
{
    light->intensity = intensity;
    light_layer_dirty = true;
}

void FE_Light_Move(FE_Light *light, int x, int y)
{
    if (!light) {
        warn("NULL light passed! (FE_Light_Move)");
        return;
    }
    // set the light to the new position, taking into account the light's radius
    light->Rect = (GPU_Rect) {
        x + light->x_offset - (light->Rect.w / 2),
        y + light->y_offset - (light->Rect.h / 2),
        light->Rect.w,
        light->Rect.h
    };

    light_layer_dirty = true;
}

void FE_Light_Toggle(FE_Light *light)
{
    if (!light) {
        warn("NULL light passed! (FE_Light_Toggle)");
        return;
    }
    light->enabled = !light->enabled;
    light_layer_dirty = true;
}

FE_Light *FE_Light_Create(GPU_Rect rect, int radius, const char *texture)
{
    if (!light_layer) {
        light_layer = GPU_CreateImage(PresentGame->WindowWidth, PresentGame->WindowHeight, GPU_FORMAT_RGBA);
    }

    // Create new light object
    FE_Light *light = xmalloc(sizeof(FE_Light));
    light->x_offset = rect.w / 2;
    light->y_offset = rect.h / 2;

    light->Rect = (GPU_Rect) {
        rect.x + light->x_offset - radius,
        rect.y + light->y_offset - radius,
        radius * 2,
        radius * 2
    };

    light->enabled = true;
    light->intensity = 225;

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

void FE_Light_Render(FE_Camera *camera, GPU_Image *world)
{
	Uint8 brightness = PresentGame->MapConfig.AmbientLight;
    GPU_Rect vis_rect = SCREEN_RECT(camera);

    /* Disable lighting if brightness is 0 */
    if (brightness == 255) {
        // Only render the part of the world that is visible
        GPU_BlitRect(world, &vis_rect, PresentGame->Screen, NULL);
        GPU_Clear(camera->target);
        return;
    }

    CheckIfLightDirty(camera);

    /* Create an layer to render the lighting to. Only re-render if lighting has changed. */
    if (light_layer_dirty || PresentGame->GameState == GAME_STATE_EDITOR) {
        GPU_Target *target = GPU_LoadTarget(light_layer);

        GPU_Clear(target);

        // render the lights
        for (FE_List *l = lights; l; l = l->next) {
            FE_Light *light = l->data;
            if (!light->enabled) continue;
            // apply intensity
            FE_SetTextureAlphaMod(light->Texture->Texture, light->intensity);
            FE_RenderCopy(target, camera, false, light->Texture,  NULL, &light->Rect);
            GPU_SetRGBA(light->Texture->Texture, light->Texture->Texture->color.r, light->Texture->Texture->color.g, light->Texture->Texture->color.b, 255);
        }
        light_layer_dirty = false;

    }

    // Render the world with the applied light effects
    GPU_BlitRect(world, &vis_rect, PresentGame->Screen, NULL);
    GPU_BlitRect(light_layer, &vis_rect, PresentGame->Screen, NULL);
    GPU_Clear(camera->target);
}

void FE_Light_Destroy(FE_Light *light)
{
    FE_List_Remove(&lights, light);
    FE_DestroyResource(light->Texture->path);
    free(light);
    light_layer_dirty = true;
}

void FE_Light_Clean()
{
    if (light_layer) {
        GPU_FreeImage(light_layer); // for some reason this leaks memory from the target. which is great
                                    // as sdl_gpu is meant to free the target when the image is freed.
                                    // but it doesn't. infact all targets seem to leak memory.
    }
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