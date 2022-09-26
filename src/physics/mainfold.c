#include "include/rigidbody.h"
#include "include/manifold.h"

void Phys_ResetManifold(Phys_Manifold *m)
{
    if (m) {
        m->a = 0;
        m->b = 0;
        m->penetration = 0;
        m->normal = (vec2){0, 0};
    }
}

void Phys_ApplyImpulse(FE_Phys_Rigidbody *b, vec2 impulse)
{
    b->velocity = vec2_sub(b->velocity, vec2_scale(impulse, b->im));
}

void Phys_ResolveCollision(Phys_Manifold *m)
{
    FE_Phys_Rigidbody *a = m->a;
    FE_Phys_Rigidbody *b = m->b;

    /* Return if objects are static */
    if (a->mass == 0 || b->mass == 0) return; 


    /* Calculate relative velocity */
    vec2 relative_vel = vec2_sub(a->velocity, b->velocity);
    float vel_along_normal = vec2_dot(relative_vel, m->normal);

    /* Return if objects are separating */
    if (vel_along_normal > 0) return;

    /* Calculate restitution */
    float e = fmin(a->restitution, b->restitution);

    /* Calculate impulse scalar */
    float j = -(1 + e) * vel_along_normal;
    j /= a->im + b->im;

    /* Apply impulse */
    vec2 impulse = vec2_scale(m->normal, j);

    Phys_ApplyImpulse(a, impulse);
    Phys_ApplyImpulse(b, vec2_scale(impulse, -1));
}

void Phys_CorrectPosition(Phys_Manifold *m)
{
    const float percent = 0.1f; // percent of penetration to correct
    const float slop = 0.05f; // allow some penetration to avoid jitter

    vec2 correction = vec2_scale(m->normal, fmax(m->penetration - slop, 0) / (m->a->im + m->b->im) * percent);
    m->a->position = vec2_sub(m->a->position, vec2_scale(correction, m->a->im));
    m->b->position = vec2_add(m->b->position, vec2_scale(correction, m->b->im));
}
