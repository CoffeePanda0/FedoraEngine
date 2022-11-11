#ifndef _H_FE_Phys_Rigidbody
#define _H_FE_Phys_Rigidbody

#include <vector.h>

typedef struct {
    float mass; // Mass of the object
    float im; // Store the inverse mass for faster calculations

    bool grounded; // Is the object on the ground?
    bool colliding_ground; // Is the object grounded via collision?
    
    vec2 force; // Force acting on the object
    vec2 velocity; // Velocity of the object

    float terminal_velocity; // Maximum velocity of the object freefalling
    float friction; // Static friction of the object

    float restitution; // Coefficient of restitution

    vec2 position; // Precise position of the object
    vec2 last_position; // Last position of the object (used internally for interpolation)
    vec2 last_collision; // Last collision check (used internally for collision detection)
    
    SDL_Rect body; // The body of the object (used for collisions and rendering)

} FE_Phys_Rigidbody;

typedef struct {
    vec2 min;
    vec2 max;
} Phys_AABB;


/** Checks if two FRects intersect, and if they do, create the resultant collision rectangle
 * \returns true on intersection, false otherwise
*/
bool FE_AABB_Intersect(const SDL_FRect *a, const SDL_FRect *b, SDL_FRect *result);


#endif