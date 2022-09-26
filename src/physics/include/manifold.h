#ifndef _H_PHYS_MANIFOLD
#define _H_PHYS_MANIFOLD

#include "../../core/include/vector.h"

typedef struct {
    FE_Phys_Rigidbody *a;
    FE_Phys_Rigidbody *b;

    float penetration;
    vec2 normal;
} Phys_Manifold;


/* Reset the manifold to default values */
void Phys_ResetManifold(Phys_Manifold *m); 

/* Apply impulse to a rigidbody */
void Phys_ApplyImpulse(FE_Phys_Rigidbody *b, vec2 impulse);

/* Resolve collision between two rigidbodies */
void Phys_ResolveCollision(Phys_Manifold *m);

/* Correct position of two rigidbodies using baumgarte stabilization */
void Phys_CorrectPosition(Phys_Manifold *m);

#endif