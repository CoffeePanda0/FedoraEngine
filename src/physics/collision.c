#include <stdbool.h>
#include <math.h>
#include "include/collision.h"

void Raycast_Reset(Ray_Result *result)
{
    result->hit = false;
    result->collision_point = vec(0, 0);
}

bool Raycast_Plane(Ray *ray, Plane *plane, Ray_Result *result)
{
    Raycast_Reset(result);

    float x1 = ray->position.x;
    float y1 = ray->position.y;
    float x2 = ray->position.x + ray->direction.x;
    float y2 = ray->position.y + ray->direction.y;

    float x3 = plane->start.x;
    float y3 = plane->start.y;
    float x4 = plane->end.x;
    float y4 = plane->end.y;

    float denom = (x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4);
    if (denom == 0)
        return false;
    
    float t = ((x1 - x3) * (y3 - y4) - (y1 - y3) * (x3 - x4)) / denom;
    float u = -((x1 - x2) * (y1 - y3) - (y1 - y2) * (x1 - x3)) / denom;

    if (t > 0 && t < 1 && u > 0) {
        result->collision_point.x = x1 + t * (x2 - x1);
        result->collision_point.y = y1 + t * (y2 - y1);
        result->collision_dist = vec2_dist(result->collision_point, ray->position);
        result->hit = true;
        return true;
    }
    result->hit = false;
    return false;
}

bool Raycast_Polygon(Ray *ray, Polygon *polygon, Ray_Result *result)
{
    Raycast_Reset(result);

    for (size_t idx = 0; idx < polygon->point_count; idx++) {
        vec2 point = polygon->points[idx];
        vec2 next_point = polygon->points[(idx + 1) % polygon->point_count];
        Plane plane = {point, next_point};
        if (Raycast_Plane(ray, &plane, result))
            return true;
    }
    result->hit = false;
    return false;
}

bool Raycast_Circle(Ray *ray, Circle *circle, Ray_Result *result)
{
    Raycast_Reset(result);

    vec2 dir = vec2_sub(circle->position, ray->position);
    float proj = vec2_dot(dir, ray->direction);
    vec2 p = vec2_add(ray->position, vec2_scale(ray->direction, proj));

    if (vec2_dist(circle->position, p) > circle->radius)
        return false;

    vec2 diff1 = vec2_sub(p, ray->position);
    vec2 diff2 = vec2_sub(p, circle->position);
    float d1 = vec2_mag(diff1);
    float d2  = vec2_mag(diff2);

    float dist = d1 - sqrtf(square(circle->radius) - square(d2));

    result->collision_point = vec2_add(ray->position, vec2_scale(ray->direction, dist));
    result->collision_dist = vec2_dist(result->collision_point, ray->position);
    result->hit = true;
    return true;
}

bool Collision_AABB(AABB *a, AABB *b, Collision_Point *result)
{
    if(a->max.x < b->min.x || a->min.x > b->max.x) return false;
    if(a->max.y < b->min.y || a->min.y > b->max.y) return false;
    
    return true;
}

bool Collision_Circle_AABB(Circle *c, AABB *a, Collision_Point *result)
{
    // find closest point on a to c
    vec2 closest = vec2_clamp(c->position, a->min, a->max);
    // distance between closest and c
    float dist = vec2_dist(c->position, closest);
    // if distance is less than radius, collision
    if(dist < c->radius) return true;
    return false;
}


bool Collision_Circle(Circle *a, Circle *b, Collision_Point *result)
{
    // Calculate the distance between the two circles to avoid a square root
    float dist = square(b->position.x - a->position.x) + square(b->position.y - a->position.y);
    float radius = a->radius + b->radius;

    return (dist > square(radius));
}

void FE_DT_RECT(vec2 position, SDL_Rect *rect)
{
    rect->x = position.x;
    rect->y = position.y;
}