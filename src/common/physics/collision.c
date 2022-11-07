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
        float y_overlap = fmin(aabb1.max.y, aabb2.max.y) - fmax(aabb1.min.y, aabb2.min.y);

        // Return the smallest overlap
        if (y_overlap > 0) {
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

bool FE_AABB_Intersect(const SDL_FRect *a, const SDL_FRect *b, SDL_FRect *result)
{
    /* Custom implementation of SDL_IntersectFRect for portability */
    float x1 = fmax(a->x, b->x);
    float x2 = fmin(a->x + a->w, b->x + b->w);
    float y1 = fmax(a->y, b->y);
    float y2 = fmin(a->y + a->h, b->y + b->h);

    if (x1 < x2 && y1 < y2) {
        result->x = x1;
        result->y = y1;
        result->w = x2 - x1;
        result->h = y2 - y1;
        return true;
    }
    return false;
}

// make prefabs run on server too
// fix warnings