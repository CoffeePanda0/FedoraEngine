#include "../include/game.h"
#include "include/light.h"

#define EFFECT_TEXTURE_PATH "game/sprites/lightfx/"

static FE_List *LightList = 0;

/* todo this really is not great - redo later with proper map support */

FE_LightObject *FE_CreateLightObject(char *obj_image, SDL_Rect obj_rect, char *effect_name, int effect_offset_x, int effect_offset_y, int size)
{
    FE_LightObject *o = xmalloc(sizeof(FE_LightObject));

    o->obj_texture = FE_LoadResource(FE_RESOURCE_TYPE_TEXTURE, obj_image);
    o->obj_rect = obj_rect;
    
    char *texture_path = xmalloc(strlen(EFFECT_TEXTURE_PATH) + strlen(effect_name) + 5);
    texture_path = strcpy(texture_path, EFFECT_TEXTURE_PATH);
    texture_path = strcat(texture_path, effect_name);
    texture_path = strcat(texture_path, ".png");
    
    o->effect_texture = FE_LoadResource(FE_RESOURCE_TYPE_TEXTURE, texture_path);
    o->effect_rect = (SDL_Rect){obj_rect.x + effect_offset_x, obj_rect.y + effect_offset_y, size, size};

    xfree(texture_path);
    FE_List_Add(&LightList, o);

    return o;
}

void FE_DestroyLightObject(FE_LightObject *o)
{
    if (!o) {
        warn("FE_DestroyLightObject: Light object is null");
        return;
    }
    FE_DestroyResource(o->effect_texture->path);
    FE_DestroyResource(o->obj_texture->path);

    FE_List_Remove(&LightList, o);

    free(o);
}

void FE_RenderLightObjects(FE_Camera *camera)
{
    FE_List *l = LightList;
    while (l)
    {
        FE_LightObject *o = l->data;

        FE_RenderCopy(o->effect_texture, 0, &(SDL_Rect){o->effect_rect.x - camera->x, o->effect_rect.y - camera->y, o->effect_rect.w, o->effect_rect.h});
        FE_RenderCopy(o->obj_texture, 0, &(SDL_Rect){o->obj_rect.x - camera->x, o->obj_rect.y - camera->y, o->obj_rect.w, o->obj_rect.h});     
        l = l->next;
    }
}

void FE_CleanLightObjects()
{
    FE_List *l = LightList;
    while (l)
    {
        FE_DestroyLightObject(l->data);
        l = l->next;
    }
    LightList = 0;
}