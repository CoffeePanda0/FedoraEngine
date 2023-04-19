#include "../core/include/include.h"
#include "include/prefab.h"

#include "../world/include/lighting.h"
#include "../world/include/camera.h"

static void DestroyData(void *data)
{
    FE_Prefab *prefab = (FE_Prefab *)data;
    if (prefab->has_light)
        FE_Light_Destroy(prefab->light);
    if (prefab->has_particle)
        FE_ParticleSystem_Destroy(prefab->ps);
}

static void DestroyConfig(FE_PrefabConfig *config)
{
    if (config->name)
        free(config->name);
    if (config->texture_path)
        free(config->texture_path);
    if (config->light_effect)
        free(config->light_effect);
    if (config->particle_texture)
        free(config->particle_texture);
    free(config);
}

void FE_Prefab_Update()
{
    for (FE_List *o = FE_GamePrefabs; o; o = o->next) {
        FE_Prefab *obj = o->data;
        // update position for light effect and particles if it moves (this is pretty scuffed and not efficient)
        if (obj->last_location.x != obj->obj->phys->body.x || obj->last_location.y != obj->obj->phys->body.y) {
            if (obj->has_light) {
                FE_Light_Move(obj->light, obj->obj->phys->body.x, obj->obj->phys->body.y);
            }
            if (obj->has_particle) {
                float emission_radius = obj->ps->emission_area.w / 2;
                obj->ps->emission_area.x = obj->obj->phys->body.x - emission_radius + (obj->obj->phys->body.w / 2);
                obj->ps->emission_area.y = obj->obj->phys->body.y - emission_radius + (obj->obj->phys->body.h / 2);
            }
            obj->last_location.x = obj->obj->phys->body.x;
            obj->last_location.y = obj->obj->phys->body.y;
        }
    }
}

SDL_Texture *FE_Prefab_Thumbnail(const char *name)
{
    FE_PrefabConfig *data = FE_Prefab_Load(name);
    if (!data)
        return 0;

    if (!data->texture_path) {
        warn("Prefab is missing texture path %s", name);
        free(data);
        return 0;
    }

    char *path = mstradd("game/sprites/", data->texture_path);
    SDL_Texture *t = FE_TextureFromFile(path);

    free(path);
    DestroyConfig(data);

    return t;
}

FE_Prefab *FE_Prefab_Create(const char *name, int x, int y)
{
    FE_PrefabConfig *data = FE_Prefab_Load(name);
    if (!data)
        return 0;
        
    data->rect.x = x;
    data->rect.y = y;

    /* Create an FE_Prefab from the FE_PrefabConfig */
    FE_Prefab *new = xmalloc(sizeof(FE_Prefab));
    new->has_light = data->has_light;
    new->has_particle = data->has_particle;
    new->last_location = vec(0, 0);
    new->name = mstrdup(data->name);
    new->destroy_cb = &DestroyData;

    /* Attatches the gameobject, light and particles */
    new->obj = FE_GameObject_Create(data->rect, data->texture_path, data->mass, "prefab");

    if (new->has_light) {
        if (!data->light_effect)
            data->light_effect = mstrdup("");
        new->light = FE_Light_Create(new->obj->phys->body, data->light_radius, data->light_effect);
    }
    if (new->has_particle) {
        if (!data->particle_texture)
            data->particle_texture = mstrdup("");
        new->ps = FE_ParticleSystem_Create((SDL_Rect) {new->obj->phys->body.x - data->emission_radius, new->obj->phys->body.y - data->emission_radius, data->emission_radius * 2, data->emission_radius * 2}, data->emission_rate, data->max_particles, 2000, true, data->particle_texture, vec(15,15), vec(0,0), false);
    }
    DestroyConfig(data);

    FE_List_Add(&FE_GamePrefabs, new);
    return new;
}
