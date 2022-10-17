#ifndef _H_PHYS
#define _H_PHYS

// define SDL vars if not defined (as in the case of the server)
#include "rigidbody.h"

/* TODO:
    bouncing off walls
    stop players being sticky on walls
    fix object-on-object collisions
    fix jumping
    benchmark
*/

extern const float PHYS_SCALE;

/** Creates and returns a Rigidbody
 * \param mass The mass of the rigidbody
 * \param body The body of the rigidbody
 * \param returns The rigidbody
 */
FE_Phys_Rigidbody *FE_Physics_CreateBody(float mass, SDL_Rect body);


/** Adds a rigidbody to the physics engine
 * \param body The rigidbody to add
 */
void FE_Physics_AddBody(FE_Phys_Rigidbody *body);


/** Applies a force to a rigidbody
 * \param o The rigidbody to apply the force to
 * \param force The force to apply
 */
void FE_Physics_ApplyForce(FE_Phys_Rigidbody *o, vec2 force);


/* Cleans the physics engine */
void FE_Physics_Clean();


/** Removes and frees a rigidbody from the physics engine
 * \param body The rigidbody to remove
 */
void FE_Physics_Remove(FE_Phys_Rigidbody *o);


/* Updates the physics engine */
void FE_Physics_Update();


#endif