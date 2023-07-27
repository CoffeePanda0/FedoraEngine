#ifndef _H_WORLD_COLLISION
#define _H_WORLD_COLLISION

#include <vector.h>
#include "../../physics/include/rigidbody.h"
#include "../../physics/include/collision.h"


typedef struct {
    vec2 position;
    vec2 normal;
    float penetration;
} TileCollision;


typedef struct {
    size_t count;
    TileCollision *collisions;
} FE_CollisionInfo;


/** Checks the map for collisions with an AABB
 * @param aabb The AABB to check for collisions with
 * @param result The collision info to store the results in
 */
void FE_Map_Collisions(Phys_AABB *aabb, FE_CollisionInfo *result);


/**
 * @brief A basic check to see if a collision has occured with the map with limited information
 * @param aabb The AABB to check for collisions with
 * @return The y position of the tile that the AABB is colliding with, or -1 if no collision
 */
int FE_Map_CollisionAbove(Phys_AABB *aabb);


#endif