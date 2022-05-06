#include <SDL.h>
#include "include/resourcemanager.h"
#include "../ext/hashtbl.h"
#include "include/utils.h"
#include "include/mem.h"

static hashtable Resources;
static bool resource_init = false;

/* Destroys the data held by the resource depending on the type */
static void DestroyResource(FE_Resource *res)
{
    if (!res->data) return;

    switch (res->type) {
        case FE_RESOURCE_TYPE_UNKNOWN:
            free(res->data);
            break;
        case FE_RESOURCE_TYPE_TEXTURE:
            FE_Texture *t = (FE_Texture *)res->data;
            FE_DestroyTexture(t);
            break;
        default:
            warn("Unknown resource type");
            break;
    }
}

/* Called to free memory held by items when removed from the hashtable */
static void DestroyData(char *key, void *val)
{
    if (!key || !val) {
        warn("Invalid key or value for ResourceManager");
        return;
    }
    
    xfree(key);

    FE_Resource *resource = (FE_Resource *)val;
    DestroyResource(resource);
    xfree(resource);
}

void FE_ResourceManager_Init()
{
    htinit(&Resources, DestroyData);
    resource_init = true;
}

void FE_ResourceManager_Destroy()
{
    htclear(&Resources);
    FE_CleanFonts();
}

/* Loads the data depending on the type */
static void *LoadResource(FE_RESOURCE_TYPE type, char *filepath)
{
    switch (type) {
        case FE_RESOURCE_TYPE_UNKNOWN:
            return NULL;
        case FE_RESOURCE_TYPE_TEXTURE:
            FE_Texture *t = xmalloc(sizeof(FE_Texture));
            t->path = strdup(filepath);
            t->Texture = FE_TextureFromFile(filepath);
            return t;
        case FE_RESOURCE_TYPE_SOUND:
            return FE_LoadSFX(filepath);
        default:
            warn("Unknown resource type");
            return NULL;
    }
}

/* Loads a resource to the game's memory */
void *FE_LoadResource(FE_RESOURCE_TYPE type, char *filepath)
{
    if (!resource_init) {
        error("ResourceManager has not yet been initalised");
    }

    char *key = strdup(filepath);

    /* Check if the resource is already loaded */
    FE_Resource *res = (FE_Resource *)htget(&Resources, key);
    if (res) {
        res->refcount++;
        free(key);
        return res->data;
    }

    /* Load the resource */
    res = xmalloc(sizeof(FE_Resource));
    res->type = type;
    res->refcount = 1;
    res->data = LoadResource(type, key);

    /* Add the resource to the hashtable */
    htset(&Resources, key, res);

    return res->data;
}

/* Destroys a resource from the filepath (only frees if resource is not used elsewhere) */
int FE_DestroyResource(char *filepath)
{
    if (!resource_init) {
        error("ResourceManager has not yet been initalised");
    }
    
    FE_Resource *res = (FE_Resource *)htget(&Resources, filepath);
    if (!res) {
        warn("Resource %s not found", filepath);
        return -1;
    }

    res->refcount--;
    if (res->refcount <= 0) {
        htunset(&Resources, filepath);
    }

    return 1;
}
