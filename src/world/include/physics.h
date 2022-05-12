#ifndef H_PHYSICS
#define H_PHYSICS

#include "../../core/include/vector2d.h"

#define DRAG 1.2f
#define FRICTION 0.3f
#define BOUNCE 0.3f

typedef struct FE_PhysObj {
    Uint16 mass;
    Vector2D velocity;
    Vector2D maxvelocity;

    SDL_Rect body;
    Vector2D position;
    
    bool moveable;
} FE_PhysObj;


/* The main update loop that carries out all physics calculations */
void FE_RunPhysics();


/** Applies a force to a PhysObj body.
 *\param o The object to apply the force to
 *\param force The force to apply
*/
void FE_ApplyForce(FE_PhysObj *o, Vector2D force);


/** Mallocs and creates a new physics object
 * \param mass The mass of the object
 * \param maxvelocity The maximum velocity of the object
 * \param body The body of the object
 * \param moveable Whether the object is moveable by other objects
 * \return The new object
*/
FE_PhysObj *FE_CreatePhysObj(Uint16 mass, Uint16 maxvelocity, SDL_Rect body, bool moveable);


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