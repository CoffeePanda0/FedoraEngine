#include <FE_Common.h>

#include "include/rigidbody.h"
#include "include/manifold.h"
#include "include/collision.h"

#include "../world/include/collision.h"

static FE_List *RigidBodies = 0;

const float PHYS_SCALE = 10.0f; // to save us having to use huge forces

static float dt = 1.0f / 100;
static float friction_coef = 0.75f;

void FE_UPDATE_RECT(vec2 position, SDL_Rect *rect)
{
    rect->x = position.x;
    rect->y = position.y;
}

static void CheckCollisions()
{
    static Phys_Manifold m;
    for (FE_List *l = RigidBodies; l; l = l->next) {
        for (FE_List *l2 = l->next; l2; l2 = l2->next) {
            FE_Phys_Rigidbody *a = (FE_Phys_Rigidbody *)l->data;
            FE_Phys_Rigidbody *b = (FE_Phys_Rigidbody *)l2->data;

            if (a->mass == 0 || b->mass == 0) continue;

            if (Phys_AABBvsAABB(a, b, &m)) {
                Phys_ResolveCollision(&m);
                Phys_CorrectPosition(&m);
            }
        }
    }
}

static void CalculateTerminalVelocity(FE_Phys_Rigidbody *body)
{
    const float density = 1.29f;
    const float drag_coef = 0.2f; // yeilds most realistic results

    int area = body->body.w * body->body.h;

    body->terminal_velocity = sqrtf(
        2 * body->mass * PresentGame->MapConfig.Gravity /
        density * area * drag_coef
    );
}

static void CalculateFriction(FE_Phys_Rigidbody *body)
{
    body->friction = friction_coef * body->mass * 9.81f;
}

static float ReboundCollision(FE_Phys_Rigidbody *o, vec2 normal)
{
    /* If collision is above, bounce */
    if (normal.y == 1) {
        if (o->velocity.y > 0) {
            float momentum = o->mass * o->velocity.y;
            if (momentum > 2 && o->velocity.y > 3) {
                return -o->velocity.y * o->restitution;
            }
        }
    }
    return 0; 
}

static void CheckFE_Map_Collisions()
{
    for (FE_List *l = RigidBodies; l; l = l->next) {
        FE_Phys_Rigidbody *o = (FE_Phys_Rigidbody *)l->data;
        if (o->mass == 0) continue;

        /* Only bother checking if the object has moved */
        if (vec2_cmp(o->last_collision, o->position)) {
            o->velocity.y = 0;
            continue;
        }

        /* Only bother if we aren't on top of another grounded object */
        if (o->colliding_ground) {
            o->grounded = true;
            continue;
        }
        
        o->last_collision = o->position;

        /* Check that user is within map bounds */
        if (o->body.x < 0) {
            o->body.x = 0;
            o->velocity.x = 0;
        }
        else if (o->body.x + o->body.w > PresentGame->MapConfig.MapWidth) {
            o->body.x = PresentGame->MapConfig.MapWidth - o->body.w;
            o->velocity.x = 0;
        }

        /* AABB for where the user will be next frame (pre-emptive collision) */
        Phys_AABB next = {
            .min = (vec2) {
                o->position.x + (o->velocity.x * dt * PHYS_SCALE),
                o->position.y + (o->velocity.y * dt * PHYS_SCALE)
            },
            .max = (vec2) {
                o->position.x + (o->velocity.x * dt * PHYS_SCALE) + o->body.w,
                o->position.y + (o->velocity.y * dt * PHYS_SCALE) + o->body.h
            }
        };

        FE_CollisionInfo collisions = {0, 0};
        FE_Map_Collisions(&next, &collisions);

        o->grounded = false;

        /* Handle collisions */
        for (size_t i = 0; i < collisions.count; i++) {
            TileCollision *c = &collisions.collisions[i];

            if (vec2_cmp(c->normal, vec(0, 1))) { // player is above tile
                o->velocity.y = ReboundCollision(o, c->normal);
                o->position.y = c->position.y - o->body.h;
                o->grounded = true;
            }
            else if (vec2_cmp(c->normal, vec(0, -1))) { // player is below tile
                o->velocity.y = ReboundCollision(o, c->normal);
                o->position.y = c->position.y + 64;
            }
            else if (vec2_cmp(c->normal, vec(1, 0))) { // player is to the left of tile
                // apply rebound if player is moving into
                o->velocity.x = ReboundCollision(o, c->normal);
                o->position.x = c->position.x - o->body.w;
            }
            else if (vec2_cmp(c->normal, vec(-1, 0))) { // player is to the right of tile
                o->velocity.x = ReboundCollision(o, c->normal);
                o->position.x = c->position.x + 64;
            }
        } 


        free(collisions.collisions);
    }
}

static void IntegrateForces()
{
    for (FE_List *l = RigidBodies; l; l = l->next) {
        FE_Phys_Rigidbody *o = l->data;
        if (o->mass == 0) continue; // don't integrate forces for static objects

        vec2 accel = vec2_scale(o->force, o->im);
        if (!PresentGame->DebugConfig.NoClip)
            accel.y += PresentGame->MapConfig.Gravity;

        // Check that we don't exceed terminal velocity, cap at terminal velocity
        float new_y = o->velocity.y + (accel.y * dt * PHYS_SCALE);

        if (new_y > o->terminal_velocity)
            o->velocity.y = o->terminal_velocity;
        else
            o->velocity = vec2_add(o->velocity, vec2_scale(accel, dt * PHYS_SCALE));

    }
}

static void IntegrateVelocities()
{
    for (FE_List *l = RigidBodies; l; l = l->next) {
        FE_Phys_Rigidbody *o = l->data;
        if (o->mass == 0) continue; // don't integrate velocities for static objects

        o->position = vec2_add(o->position, vec2_scale(o->velocity, dt * PHYS_SCALE));
    }
}

static void ClearForces()
{
    for (FE_List *l = RigidBodies; l; l = l->next) {
        FE_Phys_Rigidbody *o = l->data;
        o->colliding_ground = false;
        o->force = VEC_EMPTY;
    }
}

void FE_Physics_ApplyForce(FE_Phys_Rigidbody *o, vec2 force)
{
    o->force = vec2_add(o->force, vec2_scale(force, PHYS_SCALE));
}

static void IntegrateFriction()
{
    for (FE_List *l = RigidBodies; l; l = l->next) {
        FE_Phys_Rigidbody *o = l->data;
        if (o->mass == 0) continue; // don't integrate friction for static objects

        if (fabs(o->velocity.x) < 1) {
            o->velocity.x = 0;
            continue;
        }

        if (o->grounded) { // apply lots more friction if on the ground
            float momentum = o->mass * o->velocity.x;
            float val = 0;
        
            val = momentum > 0 ? clamp(momentum - o->friction, 0, momentum) : clamp(momentum + o->friction, momentum, 0);
            val -= (o->mass * o->velocity.x);
            val *= 0.1;
            o->velocity.x += (val * o->im); // this really sucks but i can't be bothered anymore, friction is hell
        } else { // apply very small amount of friction in the air
            float friction_speed = o->friction * o->im;
            float val = 0;

            if (o->velocity.x > 0) {
                val = clamp(o->velocity.x - friction_speed, 0, o->velocity.x);
            } else if (o->velocity.x < 0) {
                val = clamp(o->velocity.x + friction_speed, o->velocity.x, 0);
            }
            float dif = o->velocity.x - val;
            o->velocity.x -= (dif * dt);
        }
    }
}

static void InterpolateStates()
{
    for (FE_List *l = RigidBodies; l; l = l->next) {
        FE_Phys_Rigidbody *o = l->data;
        if (o->mass == 0) continue; // don't interpolate states for static objects

        o->last_position = o->position;
    }
}

static void FE_PhysLoop()
{
    InterpolateStates();
    IntegrateForces();
    CheckCollisions();
    if (!PresentGame->DebugConfig.NoClip)
        CheckFE_Map_Collisions();
    IntegrateVelocities();
    IntegrateFriction();
    ClearForces();
}

FE_Phys_Rigidbody *FE_Physics_CreateBody(float mass, SDL_Rect body)
{
    FE_Phys_Rigidbody *p = xmalloc(sizeof(FE_Phys_Rigidbody));
    p->mass = mass;
    p->im = 1.0f / mass;
    p->body = body;
    p->position = vec(body.x, body.y);
    p->last_position = vec(0, 0);
    p->last_collision = vec(0, 0);
    p->velocity = VEC_EMPTY;
    p->force = VEC_EMPTY;
    p->restitution = 0.4f;
    p->grounded = false;
    p->colliding_ground = false;

    CalculateTerminalVelocity(p);
    CalculateFriction(p);

    return p;
}

void FE_Physics_AddBody(FE_Phys_Rigidbody *body)
{
    if (!body) {
        warn("Attempted to add null body to physics engine");
        return;
    }
    FE_List_Add(&RigidBodies, body);
}

void FE_Physics_Clean()
{
    if (RigidBodies) {
        for (FE_List *l = RigidBodies; l; l = l->next) {
            free(l->data);
        }
        FE_List_Destroy(&RigidBodies);
    }
}

void FE_Physics_Remove(FE_Phys_Rigidbody *o)
{
    if (!o) {
        warn("Attempted to remove null body from physics engine");
        return;
    }
    FE_List_Remove(&RigidBodies, o);
    free(o);
}

void FE_Physics_Update()
{
    // Use a fixed timestep for physics to avoid instability
    static float accumulator = 0.0f;
    static uint64_t last_time = 0;

    uint64_t current_time = FE_GetTicks64();

    accumulator += (current_time - last_time) / 1000.0f;

    last_time = current_time;

    // clamp accumulator to avoid spiral of death
    if (accumulator > 0.20f) accumulator = 0.20f;

    while (accumulator >= dt) {
        // Time how long it takes to run the physics loop
        FE_PhysLoop();
        accumulator -= dt;
    }

    // Interpolate the positions of the objects to smooth out the movement
    const float alpha = accumulator / dt;
    for (FE_List *l = RigidBodies; l; l = l->next) {        
        FE_Phys_Rigidbody *o = l->data;
        if (o->mass == 0) return;

        vec2 position = vec2_lerp(o->last_position, o->position, alpha);
        FE_UPDATE_RECT(position, &o->body); // set interpolated rect for rendering
    } /* todo velocity sometimes clips to -1.6 ?? */
}

