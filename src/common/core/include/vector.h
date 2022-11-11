#ifndef _VEC2_H
#define _VEC2_H

#include <stdbool.h>
#include <math.h>

#include "rect.h"

typedef struct vec2 {
    float x, y;
} vec2;

#define vec(x, y) (vec2){x, y}
#define VEC_NULL (vec2){-1,-1}
#define VEC_EMPTY (vec2){0,0}

#define vec2_add(a, b) (vec2){a.x + b.x, a.y + b.y}
#define vec2_sub(a, b) (vec2){a.x - b.x, a.y - b.y}
#define vec2_mul(a, b) (vec2){a.x * b.x, a.y * b.y}
#define vec2_div(a, b) (vec2){a.x / b.x, a.y / b.y}
#define vec2_scale(a, s) (vec2){a.x * s, a.y * s}
#define vec2_mag(a) sqrtf(a.x * a.x + a.y * a.y)
#define vec2_setmag(a, l) vec2_scale(a, l / vec2_mag(a))
#define vec2_normalize(a) vec2_scale(a, 1.0f / vec2_mag(a))
#define vec2_dot(a, b) (a.x * b.x + a.y * b.y)
#define vec2_angle(a, b) acosf(vec2_dot(a, b) / (vec2_mag(a) * vec2_mag(b)))
#define vec2_dist(a, b) vec2_mag(vec2_sub(a, b))
#define vec2_clamp(a, min, max) (vec2){fmaxf(min.x, fminf(max.x, a.x)), fmaxf(min.y, fminf(max.y, a.y))}
#define vec2_null(a) (a.x == -1 && a.y == -1)
#define vec2_cmp(a, b) ((a.x == b.x) && (a.y == b.y))
#define vec2_lerp(a, b, t) (vec2){a.x + t * (b.x - a.x), a.y + t * (b.y - a.y)}
#define vec2_abs(a) (vec2){fabsf(a.x), fabsf(a.y)}

#endif