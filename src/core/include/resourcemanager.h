#ifndef H_RESOURCEMANAGER
#define H_RESOURCEMANAGER

#include <stdio.h>

typedef enum FE_RESOURCE_TYPE {
    FE_RESOURCE_TYPE_UNKNOWN,
    FE_RESOURCE_TYPE_TEXTURE,
    FE_RESOURCE_TYPE_SOUND
} FE_RESOURCE_TYPE;

typedef struct FE_Resource {
    enum FE_RESOURCE_TYPE type; // Type of resource
    void *data; // pointer to the data
    uint32_t refcount; // number of times this resource is referenced
} FE_Resource;


/** Loads a new resource into memory, or returns a pointer to the resource if it has been loaded before
 * This can also be used for pre-loading and caching resources
 * \param type The type of resource
 * \param filepath Path to the resource to load
*/
void *FE_LoadResource(FE_RESOURCE_TYPE type, char *filepath);


/** Destroys a resource of any type. If multiple other objects reference this resource, it will not be destroyed
 * until no objects reference the resource
 * \param filepath The filepath of the original resource to destroy
*/
int FE_DestroyResource(char *filepath);


/* Initialises the resource manager */
void FE_ResourceManager_Init();

#endif