#ifndef H_PHYSICS
#define H_PHYSICS

#include <SDL.h>
#include "../../core/include/vector2d.h"

#define DRAG 1.2f
#define FRICTION 0.8f;
#define BOUNCE 0.2f;

extern float GRAVITY;

#define clamp(num, min, max) _Generic((num), \
        int: clampi, \
        float: clampf \
    )((num), (min), (max))

typedef struct FE_PhysObj {
    Uint16 mass;
    Vector2D velocity;
    Vector2D maxvelocity;
    SDL_Rect body;
} FE_PhysObj;


/* The main update loop that carries out all physics calculations */
void FE_RunPhysics();


/** Applies a force to a PhysObj body.
 *\param o The object to apply the force to
 *\param force The force to apply
*/
void FE_ApplyForce(FE_PhysObj *o, Vector2D force);


/** Adds a given physics interactable to the physics system.
 * If this is not called, the object will not have any calculations applied 
 *\param o The object to add to the physics system
 */
int FE_AddPhysInteractable(FE_PhysObj *o);


/** Removes a physics object from the physics system
 *\param o The object to remove
*/
int FE_RemovePhysInteractable(FE_PhysObj *o);


/** AABB collision detection
 *\param a The first rect
 *\param b The second rect
 *\return true if the objects are colliding, false otherwise
*/
bool FE_AABB_Collision(SDL_Rect *a, SDL_Rect *b);


float clampf(float num, float min, float max);
int clampi(int num, int min, int max);

#endif