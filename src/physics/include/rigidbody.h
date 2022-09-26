#ifndef _H_FE_Phys_Rigidbody
#define _H_FE_Phys_Rigidbody

#include "../../core/include/vector.h"
#include <SDL.h>

typedef struct {
    float mass; // Mass of the object
    float im; // Store the inverse mass for faster calculations

    bool grounded; // Is the object on the ground?

    vec2 force; // Force acting on the object
    vec2 velocity; // Velocity of the object

    float terminal_velocity; // Maximum velocity of the object freefalling
    float friction; // Static friction of the object

    float restitution; // Coefficient of restitution

    vec2 position; // Precise position of the object
    vec2 last_position; // Last position of the object (used internally for interpolation)
    vec2 last_collision; // Last collision check (used internally for collision detection)
    
    SDL_Rect body; // The body of the object (used for rendering)

} FE_Phys_Rigidbody;

typedef struct {
    vec2 min;
    vec2 max;
} Phys_AABB;

#endif