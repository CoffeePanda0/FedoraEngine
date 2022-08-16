#include "include/include.h"
#include "../core/include/include.h"
#include "../entity/include/particle.h"
#include <math.h>

static FE_List *FE_PhysObjects = 0; // Linked list of all physics objects

static void FE_Gravity() // Applies gravity to all objects
{
    if (FE_PhysObjects != 0) {
        for (FE_List *t = FE_PhysObjects; t; t = t->next) {
            FE_PhysObj *obj = (FE_PhysObj *)t->data;

            if (obj->mass == 0) continue; // Don't apply gravity to objects with no mass

            float new_velocity = obj->velocity.y;
            new_velocity = obj->velocity.y + (obj->velocity.y + PresentGame->MapConfig.Gravity) * FE_DT;

            /* calc accel and max accel */
            float terminal_velocity = sqrtf((2 * obj->mass * PresentGame->MapConfig.Gravity) / DRAG);
            
            /* clamp to terminal velocity */
            new_velocity = clamp(new_velocity, -terminal_velocity, terminal_velocity);
            obj->velocity.y = new_velocity;
           
        }
    } 
}

FE_PhysObj *FE_CreatePhysObj(Uint16 mass, SDL_Rect body)
{
    FE_PhysObj *o = xmalloc(sizeof(FE_PhysObj));
    o->mass = mass;
    o->inv_mass = 1.0f / mass;
    o->velocity = vec(0, 0);
    o->body = body;
    o->force = VEC_NULL;
    o->position = vec(body.x, body.y);

    return o;
}

int FE_AddPhysInteractable(FE_PhysObj *o) // Create new node with pointer to object, adds to linked list
{
    if (!o) {
        warn("Invalid PhysObj passed! (AddPhysInteractable)");
        return -1;
    }
    FE_List_Add(&FE_PhysObjects, o);
    return 1;
}

int FE_RemovePhysInteractable(FE_PhysObj *o)
{
    if (!o) {
        warn("Invalid PhysObj passed! (RemovePhysInteractable)");
        return -1;
    }
    FE_List_Remove(&FE_PhysObjects, o);
    free(o);

    return 1;
}

int FE_CleanPhys() // Removes all objects from linked list
{
    if (!FE_PhysObjects)
        return 0;
    
    // free all objects first
    for (FE_List *t = FE_PhysObjects; t; t = t->next) {
        FE_PhysObj *obj = (FE_PhysObj *)t->data;
        free(obj);
    }

    // destroy list
    FE_List_Destroy(&FE_PhysObjects);

    return 1;
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

static void Friction()
{
    for (FE_List *t = FE_PhysObjects; t; t = t->next) {
        FE_PhysObj *obj = (FE_PhysObj *)t->data;

        if (obj->velocity.x == 0 || obj->mass == 0)
            continue;

        // don't bother with negligble amounts
        if (!(obj->velocity.x > 0.01 || obj->velocity.x < -0.01)) {
            obj->velocity.x = 0;
            continue;
        }

        if (obj->velocity.x > 0) {
            float new_velocity = obj->velocity.x - (FRICTION * FE_DT_MULTIPLIER);
            obj->velocity.x = clamp(new_velocity, 0, obj->velocity.x);
        } else {
            float new_velocity = obj->velocity.x + (FRICTION * FE_DT_MULTIPLIER);
            obj->velocity.x = clamp(new_velocity, obj->velocity.x, 0);
        }

    }
}

/* Applies acceleration to velocity */
static void IntegrateAcceleration(FE_PhysObj *o)
{
    vec2 accel = vec2_scale(o->force, o->inv_mass);
    o->velocity = vec2_add(accel, o->velocity);
}

/* Applies velocity to object location */
static void IntegrateVelocity(FE_PhysObj *o)
{    
    vec2 added_position = vec2_scale(o->velocity, FE_DT_MULTIPLIER);
    o->position = vec2_add(o->position, added_position);
}

/* Applies a force to an onject */
void FE_ApplyForce(FE_PhysObj *o, vec2 force)
{
    o->force = vec2_add(o->force, force);
}

/* Clears force at end of each frame so that we only apply the force once */
void ClearForce(FE_PhysObj *o)
{
    o->force = VEC_EMPTY;
}

static void MapCollision(FE_PhysObj *o)
{
    bool action = false;

    if (o->mass == 0)
        return;

    /* Check screen / map boundries */
    if (o->position.x < 0) {
        o->position.x = 0;
        o->velocity.x = 0;
        action = true;
    } else if (o->position.x + o->body.w > PresentGame->MapConfig.MapWidth) {
        o->position.x = PresentGame->MapConfig.MapWidth - o->body.w;
        o->velocity.x = 0;
        action = true;
    }

    // Check for tiles colliding below player
    if (o->velocity.y > 0) {
        vec2 col = FE_CheckMapCollisionAbove(&o->body);
        if (!vec2_null(col)) {
            action = true;
            float dif = vec2_sub(col, vec(o->body.x, o->body.y + o->body.h - 1)).y;
            o->position.y += dif;
            if (o->velocity.y > 12) {
                // emit ground particles for high velocity bounce effect
                float force = o->mass * o->velocity.y;
                if (force > 1000) {
                    FE_CreateParticleSystem(
                        (SDL_Rect){col.x, col.y - 5, o->body.w, 5},
                        0,
                        100,
                        1000,
                        false,
                        "impact.png",
                        vec(5,5),
                        vec(-o->velocity.y / 10, -o->velocity.y / 20),
                        false
                    );
                }
                o->velocity.y = o->velocity.y * -BOUNCE;
            } else o->velocity.y = 0;
        }
    } else {
        // If player is going up
        vec2 CeilingCollision = FE_CheckMapCollisionBelow(&o->body);
        if (!vec2_null(CeilingCollision)) { // If we collide with the ceiling
            o->position.y = CeilingCollision.y;
            o->velocity.y = 0;
        }
    }

    // check for collision on map from right
    if (o->velocity.x > 0) { 
        action = true;
        SDL_Rect tmp_r = (SDL_Rect){o->position.x, o->position.y, o->body.w, o->body.h};
        vec2 collision = FE_CheckMapCollisionRight(&tmp_r);
        if (!vec2_null(collision)) {
            o->velocity.x = 0;
            o->position.x = collision.x - o->body.w + 1;
        }
    }
    // check for collision on map from left
    else if (o->velocity.x < 0) {
        action = true;
        SDL_Rect tmp_r = (SDL_Rect){o->position.x, o->position.y, o->body.w, o->body.h};
        vec2 collision = FE_CheckMapCollisionLeft(&tmp_r);
        if (!vec2_null(collision)) {
            o->velocity.x = 0;
            o->position.x = collision.x;
        }
    }
    if (action)
        FE_DT_RECT(o->position, &o->body);
}

static void AABB_Collision(FE_PhysObj *p1, FE_PhysObj *p2)
{
    /* Check for collision */
    if (!FE_AABB_Collision(&p1->body, &p2->body)) return;

    // todo
}   

void FE_RunPhysics()
{
    FE_Gravity();

    for (FE_List *l = FE_PhysObjects; l; l = l->next) {
        FE_PhysObj *body = l->data;
        
        if (body->mass == 0) continue;

        IntegrateAcceleration(body);
        IntegrateVelocity(body);

        FE_DT_RECT(body->position, &body->body); // resolve the float location to displayable rect

        MapCollision(body);
        
        ClearForce(body);
    }

    /* Check each of the physics objects for collision with each other */
    for (FE_List *l = FE_PhysObjects; l; l = l->next) {
        for (FE_List *l2 = l->next; l2; l2 = l2->next) {
            FE_PhysObj *body = l->data;
            if (body->mass == 0) continue;
            AABB_Collision(l->data, l2->data);
        }
    }

    Friction();
}
