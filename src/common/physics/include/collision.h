#ifndef _H_PHYS_COLLISION
#define _H_PHYS_COLLISION

#include "manifold.h"

bool Phys_AABBvsAABB(FE_Phys_Rigidbody *a, FE_Phys_Rigidbody *b, Phys_Manifold *m);
bool FE_AABB_Collision(SDL_Rect *a, SDL_Rect *b);
bool FE_AABB_FCollision(SDL_FRect *a, SDL_FRect *b);

#endif