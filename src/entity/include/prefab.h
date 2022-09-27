#ifndef _PREFAB_H
#define _PREFAB_H

#include <stdbool.h>

#include "gameobject.h"
#include "particle.h"
#include "../../world/include/lighting.h"
#include "../../core/include/linkedlist.h"

typedef struct {
    char *name;
    FE_GameObject *obj;

    bool has_particle;
    FE_ParticleSystem *ps;

    bool has_light;
    FE_Light *light;

    vec2 last_location;
} FE_Prefab;

/**
 * @brief Creates a new prefab.
 * 
 * @param The name of the prefab to create in the prefabs folder
 * @param x The x position of the prefab
 * @param y The y position of the prefab
 * @return The created prefab 
 */
FE_Prefab *FE_Prefab_Create(const char *name, int x, int y);


/** Destroys a prefab
 *\prefab The prefab to destroy
 *\ returns 0 failure, 1 success
 */
int FE_Prefab_Destroy(FE_Prefab *prefab);

/** Cleans and destroys all prefabs */
void FE_Prefab_Clean();


/* Updates the location of prefab attatchments when the body moves (lights and particles) */
void FE_Prefab_Update();


/** Generates a texture for the thumbnail of a prefab
 * \param name The name of the prefab
 * \returns the GPU_Image
*/
GPU_Image *FE_Prefab_Thumbnail(const char *name);


/** Returns the list of prefabs
 * \returns The list of prefabs
*/
FE_List *FE_Prefab_Get();


#endif