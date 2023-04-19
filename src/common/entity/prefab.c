#include <FE_Common.h>
#include "../../common/ext/inih/ini.h"

#include "include/prefab.h"
#include "include/gameobject.h"

#define PREFAB_DIRECTORY "game/prefabs/"

FE_List *FE_GamePrefabs = 0; // list of all prefabs

static FE_PrefabConfig *NewConfig()
{
    FE_PrefabConfig *c = xmalloc(sizeof(FE_PrefabConfig));
    c->name = 0;
    c->mass = 0;
    c->rect = (SDL_Rect){0,0,0,0};
    c->texture_path = 0;

    c->light_effect = 0;
    c->has_light = false;
    c->light_radius = 0;

    c->has_particle = false;
    c->particle_texture = 0;
    c->emission_rate = 0;
    c->max_particles = 0;
    c->emission_radius = 0;

    return c;
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

FE_List *FE_Prefab_Get()
{
    return FE_GamePrefabs;
}

int FE_Prefab_Destroy(FE_Prefab *prefab)
{
    if (!prefab) {
        warn("Passing NULL prefab to FE_Prefab_Destroy");
        return 0;
    }

    free(prefab->name);
    if (prefab->destroy_cb)
        prefab->destroy_cb(prefab);

    FE_GameObject_Destroy(prefab->obj);
    FE_List_Remove(&FE_GamePrefabs, prefab);
    free(prefab);

    return 1;
}

/* Loads the prefab INI data into a FE_PrefabConfig */
static int ConfigParser(void *user, const char *section, const char *name, const char *value)
{
    FE_PrefabConfig *prefab = (FE_PrefabConfig*)user;

    // Prefab Info
    if (MATCH("PREFAB", "name"))
        prefab->name = mstrdup(value);
    else if (MATCH("PREFAB", "mass"))
        prefab->mass = atoi(value);
    else if (MATCH("PREFAB", "height"))
        prefab->rect.h = atoi(value);
    else if (MATCH("PREFAB", "width"))
        prefab->rect.w = atoi(value);
    else if (MATCH("PREFAB", "texture"))
        prefab->texture_path = mstrdup(value);

    // Light info
    if (MATCH("LIGHT", "enabled"))
        prefab->has_light = (mstrcmp(value, "true") == 0) ? true : false;
    if (MATCH("LIGHT", "radius"))
        prefab->light_radius = atoi(value);
    if (MATCH("LIGHT", "texture"))
        prefab->light_effect = mstrdup(value);

    // Particle info
    if (MATCH("PARTICLE", "enabled"))
        prefab->has_particle = (mstrcmp(value, "true") == 0) ? true : false;
    else if (MATCH("PARTICLE", "texture"))
        prefab->particle_texture = mstrdup(value);
    else if (MATCH("PARTICLE", "emission_rate"))
        prefab->emission_rate = atoi(value);
    else if (MATCH("PARTICLE", "max_particles"))
        prefab->max_particles = atoi(value);
    else if (MATCH("PARTICLE", "emission_radius"))
        prefab->emission_radius = atoi(value);
    
    return 1;
}

FE_Prefab *FE_Prefab_Create_Basic(const char *name, int x, int y)
{
    FE_PrefabConfig *data = FE_Prefab_Load(name);
    if (!data)
        return 0;
        
    data->rect.x = x;
    data->rect.y = y;

    /* Create an FE_Prefab from the FE_PrefabConfig */
    FE_Prefab *new = xmalloc(sizeof(FE_Prefab));
    new->has_light = false;
    new->has_particle = false;
    new->last_location = vec(0, 0);
    new->name = mstrdup(data->name);
    new->destroy_cb = NULL;

    /* Attatches the gameobject, light and particles */
    new->obj = FE_GameObject_Create_Basic(data->rect, data->mass, data->texture_path, "prefab");

    DestroyConfig(data);

    FE_List_Add(&FE_GamePrefabs, new);
    return new;
}

FE_PrefabConfig *FE_Prefab_Load(const char *name)
{
    FE_PrefabConfig *data = NewConfig();

    /* Load the prefab INI data into a prefab config */
    char *full_path = mstradd(PREFAB_DIRECTORY, name);
    if (ini_parse(full_path, ConfigParser, data) < 0) {
        warn("Can't load prefab %s", name);
        free(full_path);
        free(data);
        return 0;
    }
    free(full_path);
    return data;
}

void FE_Prefab_Clean()
{
    // destroy all prefab children inside every node
	for (FE_List *o = FE_GamePrefabs; o; o = o->next) {
		FE_Prefab *obj = o->data;
        if (obj->name)
		    free(obj->name);
        if (obj->destroy_cb)
            obj->destroy_cb(obj);
		free(obj);
	}

	FE_List_Destroy(&FE_GamePrefabs);
    FE_GamePrefabs = 0;
}
