#ifndef _H_CLIENT_PREFAB
#define _H_CLIENT_PREFAB

#include "../../../common/entity/include/prefab.h"
#include "gameobject.h"
#include "particle.h"

/**
 * @brief Creates a new prefab.
 * 
 * @param The name of the prefab to create in the prefabs folder
 * @param x The x position of the prefab
 * @param y The y position of the prefab
 * @return The created prefab 
 */
FE_Prefab *FE_Prefab_Create(const char *name, int x, int y);


/* Updates the location of prefab attatchments when the body moves (lights and particles) */
void FE_Prefab_Update();


/** Generates a texture for the thumbnail of a prefab
 * \param name The name of the prefab
 * \returns the SDL_Texture
*/
SDL_Texture *FE_Prefab_Thumbnail(const char *name);


#endif