#ifndef H_PHYSICS
#define H_PHYSICS

#include "../../physics/include/vector.h"

#define DRAG 1.2f
#define FRICTION 0.5f
#define BOUNCE 0.3f

typedef struct FE_PhysObj {
    float mass;
    float inv_mass;

    vec2 force;
    vec2 velocity;

    SDL_Rect body;
    vec2 position;
    
    bool moveable;
} FE_PhysObj;

typedef struct {
    vec2 min;
    vec2 max;
} FE_AABB;

typedef struct {
    
    float penetration;

    FE_PhysObj *a;
    FE_PhysObj *b;

} FE_CollisionInfo;

/* The main update loop that carries out all physics calculations */
void FE_RunPhysics();


/** Applies a force to a PhysObj body.
 *\param o The object to apply the force to
 *\param force The force to apply
*/
void FE_ApplyForce(FE_PhysObj *o, vec2 force);


/** Mallocs and creates a new physics object
 * \param mass The mass of the object
 * \param body The body of the object
 * \param moveable Whether the object is moveable by other objects
 * \return The new object
*/
FE_PhysObj *FE_CreatePhysObj(Uint16 mass, SDL_Rect body, bool moveable);


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


#endif