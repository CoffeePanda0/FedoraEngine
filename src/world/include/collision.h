#ifndef _H_WORLD_COLLISION
#define _H_WORLD_COLLISION

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


#endif