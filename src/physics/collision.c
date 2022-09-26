#include "include/rigidbody.h"
#include "include/manifold.h"

bool Phys_AABBvsAABB(FE_Phys_Rigidbody *a, FE_Phys_Rigidbody *b, Phys_Manifold *m)
{
    // Set manifold for collision
    Phys_ResetManifold(m);
    m->a = a;
    m->b = b;

    // calculate if objects are colliding, if so, calculate the collision normal and penetration

    Phys_AABB aabb1 = {
        .min = (vec2){a->body.x, a->body.y},
        .max = (vec2){a->body.x + a->body.w, a->body.y + a->body.h}
    };
    Phys_AABB aabb2 = {
        .min = (vec2){b->body.x, b->body.y},
        .max = (vec2){b->body.x + b->body.w, b->body.y + b->body.h}
    };

    // Calculate collision overlap on x axis
    float x_overlap = fmin(aabb1.max.x, aabb2.max.x) - fmax(aabb1.min.x, aabb2.min.x);
    

    if (x_overlap > 0) {
        // Calculate collision extent on Y axis
        float y_extent = ((aabb1.max.y - aabb1.min.y) + (aabb2.max.y - aabb2.min.y)) / 2;
        float y_overlap = y_extent - fabs(aabb1.max.y - aabb2.min.y);

        // Return the smallest overlap
        if (y_extent > 0) {
            if (x_overlap > y_overlap) {
                if (aabb1.max.y > aabb2.max.y)
                    m->normal = (vec2){0, -1};
                else
                    m->normal = (vec2){0, 1};

                m->penetration = y_overlap;
                return true;
            } else {
                if (aabb1.max.x > aabb2.max.x)
                    m->normal = (vec2){-1, 0};
                else
                    m->normal = (vec2){1, 0};

                m->penetration = x_overlap;
                return true;
            }
        }
    }
    return false;
}

bool FE_AABB_Collision(SDL_Rect *a, SDL_Rect *b)
{
    if (a->x + a->w < b->x) // to the left of b
        return false;
    if (a->x > b->x + b->w) // to the right of b
        return false;
    if (a->y + a->h < b->y) // above b
        return false;
    if (a->y > b->y + b->h) // below b
        return false;
    return true;
}