#ifndef _H_PHYS_COLLISION
#define _H_PHYS_COLLISION

#include "manifold.h"

bool Phys_AABBvsAABB(FE_Phys_Rigidbody *a, FE_Phys_Rigidbody *b, Phys_Manifold *m);
bool FE_AABB_Collision(GPU_Rect *a, GPU_Rect *b);


#endif