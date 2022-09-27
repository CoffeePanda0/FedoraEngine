#include "include/prefab.h"
#include "../core/lib/string.h"
#include "../core/include/include.h"
#include "../ext/inih/ini.h"

#define PREFAB_DIRECTORY "game/prefabs/"

static FE_Prefab *buffer_prefab = 0; // used for loading a new prefab to return a pointer
static GPU_Rect buffer_rect;

static FE_List *prefab_list = 0; // list of all prefabs

// buffer data to create objects filled in when variables have been read
static int mass = 0;
static char *texture_path = 0;
static char *light_effect = 0;
static char *particle_texture = 0;
static int emission_rate = 100;
static int max_particles = 0;
static int emission_radius = 0;
static int light_radius = 100;

FE_List *FE_Prefab_Get()
{
    return prefab_list;
}

int FE_Prefab_Destroy(FE_Prefab *prefab)
{
    if (!prefab) {
        warn("Passing NULL prefab to FE_Prefab_Destroy");
        return 0;
    }

    free(prefab->name);
    FE_GameObject_Destroy(prefab->obj);
    if (prefab->has_light)
        FE_Light_Destroy(prefab->light);
    if (prefab->has_particle)
        FE_ParticleSystem_Destroy(prefab->ps);
    
    FE_List_Remove(&prefab_list, prefab);
    free(prefab);

    return 1;
}

static int ConfigParser(void *user, const char *section, const char *name, const char *value)
{
    (void)user;

    // Prefab Info
    if (MATCH("PREFAB", "name"))
        buffer_prefab->name = mstrdup(value);
    else if (MATCH("PREFAB", "mass"))
        mass = atoi(value);
    else if (MATCH("PREFAB", "height"))
        buffer_rect.h = atoi(value);
    else if (MATCH("PREFAB", "width"))
        buffer_rect.w = atoi(value);
    else if (MATCH("PREFAB", "texture"))
        texture_path = mstrdup(value);

    // Light info
    if (MATCH("LIGHT", "enabled"))
        buffer_prefab->has_light = (mstrcmp(value, "true") == 0) ? true : false;
    if (MATCH("LIGHT", "radius"))
        light_radius = atoi(value);
    if (MATCH("LIGHT", "texture"))
        light_effect = mstrdup(value);

    // Particle info
    if (MATCH("PARTICLE", "enabled"))
        buffer_prefab->has_particle = (mstrcmp(value, "true") == 0) ? true : false;
    else if (MATCH("PARTICLE", "texture"))
        particle_texture = mstrdup(value);
    else if (MATCH("PARTICLE", "emission_rate"))
        emission_rate = atoi(value);
    else if (MATCH("PARTICLE", "max_particles"))
        max_particles = atoi(value);
    else if (MATCH("PARTICLE", "emission_radius"))
        emission_radius = atoi(value);
    
    return 1;
}

GPU_Image *FE_Prefab_Thumbnail(const char *name)
{
    // Load prefab file
    buffer_prefab = xmalloc(sizeof(FE_Prefab));
    char *full_path = mstradd(PREFAB_DIRECTORY, name);
    if (ini_parse(full_path, ConfigParser, NULL) < 0) {
        warn("Can't load prefab %s", name);
        free(full_path);
        free(buffer_prefab);
        return 0;
    }

    if (buffer_prefab->name)
        free(buffer_prefab->name);
    if (light_effect)
        free(light_effect);
    if (particle_texture)
        free(particle_texture);
    free(full_path);
    free(buffer_prefab);

    char *path = mstradd("game/sprites/", texture_path);
    GPU_Image *t = FE_TextureFromFile(path);

    if (texture_path)
        free(texture_path);
    free(path);

    return t;
}

FE_Prefab *FE_Prefab_Create(const char *name, int x, int y)
{
    buffer_rect = (GPU_Rect){x,y,0,0};
    buffer_prefab = xmalloc(sizeof(FE_Prefab));
    buffer_prefab->has_light = false;
    buffer_prefab->has_particle = false;

    // Load prefab file
    char *full_path = mstradd(PREFAB_DIRECTORY, name);
    if (ini_parse(full_path, ConfigParser, NULL) < 0) {
        warn("Can't load prefab %s", name);
        free(full_path);
        free(buffer_prefab);
        return 0;
    }

     // Create the attatched objects
    if (!texture_path) {
        warn("No texture path for prefab %s", name);
        free(full_path);
        free(buffer_prefab);
        return 0;
    }
    buffer_prefab->obj = FE_GameObject_Create(buffer_rect, texture_path, mass);
    buffer_prefab->last_location = vec(0, 0);

    if (buffer_prefab->has_light) {
        if (!light_effect) light_effect = mstrdup("");
        buffer_prefab->light = FE_Light_Create(buffer_prefab->obj->phys->body, light_radius, light_effect);
    }
    if (buffer_prefab->has_particle) {
        if (!light_effect) particle_texture = mstrdup("");
        buffer_prefab->ps = FE_ParticleSystem_Create((GPU_Rect){buffer_prefab->obj->phys->body.x - emission_radius, buffer_prefab->obj->phys->body.y - emission_radius, emission_radius * 2, emission_radius * 2}, emission_rate, max_particles, 2000, true, particle_texture, vec(15,15), vec(0,0), false);
    }
    if (light_effect)
        free(light_effect);
    if (texture_path)
        free(texture_path);
    if (particle_texture)
        free(particle_texture);

    free(full_path);
    
    FE_List_Add(&prefab_list, buffer_prefab);
    return buffer_prefab;
}

void FE_Prefab_Clean()
{
    // destroy all prefab children inside every node
	for (FE_List *o = prefab_list; o; o = o->next) {
		FE_Prefab *obj = o->data;
        if (obj->name)
		    free(obj->name);
        if (obj->has_light)
            FE_Light_Destroy(obj->light);
        if (obj->has_particle)
            FE_ParticleSystem_Destroy(obj->ps);
		free(obj);
	}

	FE_List_Destroy(&prefab_list);
    prefab_list = 0;
}

void FE_Prefab_Update()
{
    for (FE_List *o = prefab_list; o; o = o->next) {
        FE_Prefab *obj = o->data;
        // update position for light effect and particles if it moves (this is pretty scuffed and not efficient)
        if (obj->last_location.x != obj->obj->phys->body.x || obj->last_location.y != obj->obj->phys->body.y) {
            if (obj->has_light) {
                FE_Light_Move(obj->light, obj->obj->phys->body.x, obj->obj->phys->body.y);
            }
            if (obj->has_particle) {
                obj->ps->emission_area.x = obj->obj->phys->body.x - emission_radius + (obj->obj->phys->body.w / 2);
                obj->ps->emission_area.y = obj->obj->phys->body.y - emission_radius + (obj->obj->phys->body.h / 2);
            }
            obj->last_location.x = obj->obj->phys->body.x;
            obj->last_location.y = obj->obj->phys->body.y;
        }
    }
}