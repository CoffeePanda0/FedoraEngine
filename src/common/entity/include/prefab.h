#ifndef _PREFAB_H
#define _PREFAB_H

#include <stdbool.h>
#include <linkedlist.h>

extern FE_List *FE_GamePrefabs;

// Contains the loaded INI data before creating the prefab itself
typedef struct {
    int mass;
    SDL_Rect rect;
    char *texture_path;
    char *name;

    bool has_light;
    char *light_effect;
    int light_radius;

    bool has_particle;
    char *particle_texture;
    int emission_rate;
    int max_particles;
    int emission_radius;
} FE_PrefabConfig;


// Contains the prefab
typedef struct FE_Prefab {
    char *name;
    struct FE_GameObject *obj;

    bool has_particle;
    struct FE_ParticleSystem *ps;

    bool has_light;
    struct FE_Light *light;

    vec2 last_location;

    void (*destroy_cb)(void*);
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


/** Returns the list of prefabs
 * \returns The list of prefabs
*/
FE_List *FE_Prefab_Get();


/**
 * @brief Loads the prefab file into a prefab config
 * 
 * @param name the name of the prefab
 * @return 0 on failure, the new prefab config otherwise
 */
FE_PrefabConfig *FE_Prefab_Load(const char *name);


#endif