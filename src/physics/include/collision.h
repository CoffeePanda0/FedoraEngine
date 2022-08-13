#ifndef _COLLISION_H
#define _COLLISION_H

#include "vector.h"
#include <stdlib.h>

#define square(x) ((x) * (x))

typedef struct {
    vec2 position;
    vec2 direction;
} Ray;

typedef struct {
    bool hit;
    vec2 collision_point;
    float collision_dist;
} Ray_Result;

typedef struct {
    vec2 local_a;
    vec2 local_b;
    float penetration;
} Collision_Point;

#define ray(position, direction) (Ray){position, direction}

typedef enum {
    PHYS_COLL_AABB,
    PHYS_COLL_CIRCLE,
    PHYS_COLL_POLYGON
} Physics_CollType;

typedef struct {
    vec2 min;
    vec2 max;
} AABB;

typedef struct {
    float radius;
    vec2 position;
} Circle;

typedef struct {
    vec2 start;
    vec2 end;
} Plane;

typedef struct {
    vec2 *points;
    size_t point_count;
} Polygon;

void Raycast_Reset(Ray_Result *result);
bool Raycast_Plane(Ray *ray, Plane *plane, Ray_Result *result);
bool Raycast_Polygon(Ray *ray, Polygon *polygon, Ray_Result *result);
bool Raycast_Circle(Ray *ray, Circle *circle, Ray_Result *result);
bool Collision_AABB(AABB *a, AABB *b, Collision_Point *result);
bool Collision_Circle_AABB(Circle *c, AABB *a, Collision_Point *result);
bool Collision_Circle(Circle *a, Circle *b, Collision_Point *result);


#endif