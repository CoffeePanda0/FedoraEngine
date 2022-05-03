#include "../include/game.h"

#ifdef _WIN32
    #include <math.h>
#endif

static FE_List *FE_PhysObjects = 0; // Linked list of all physics objects

/* TODO:
    - nicer y movement on camera
    - parallax
    - create nice pause UI (background, buttons to go to menu etc)
    - handle end of level
    - pushable objects
    - have to jump twice for big jump
    - fullscreen support
    -can we tie walking animation speed to player movement speed?
    -editor memory issues
    -particles on jumping odd location
    -going right first causes camera to zoom
    -holding space results in longer jump
*/

float clampf(float num, float min, float max) // Clamps a float value
{
    if (num < min)
        return min;
    if (num > max)
        return max;
    return num;
}

int clampi(int num, int min, int max) // Clamps an int value
{
    if (num < min)
        return min;
    if (num > max)
        return max;
    return num;
}

void FE_Gravity() // Applies gravity to all objects
{
    if (FE_PhysObjects != 0) {
        for (FE_List *t = FE_PhysObjects; t; t = t->next) {
            FE_PhysObj *obj = (FE_PhysObj *)t->data;

            float new_velocity = obj->velocity.y;

            /* calc accel and max accel */
            float terminal_velocity = sqrtf((2 * obj->mass * GRAVITY) / DRAG);
            new_velocity = obj->velocity.y + (obj->velocity.y + GRAVITY) * FE_DT;
            
            /* clamp to terminal velocity */
            new_velocity = clamp(new_velocity, -terminal_velocity, terminal_velocity);
            obj->velocity.y = new_velocity;
           
        }
    } 
}

void FE_PhysLoop() // Applies velocity forces in both directions to each object
{
    if (FE_PhysObjects != 0) {
        for (FE_List *t = FE_PhysObjects; t; t = t->next) {
            FE_PhysObj *obj = (FE_PhysObj *)t->data;

            /* Apply X force */
            if (obj->velocity.x != 0) {
                float new_x = obj->position.x + obj->velocity.x;
                // check for map boundies
                if (new_x <= 0 || new_x + obj->body.w > FE_Map_Width) {
                    obj->velocity.x = 0;
                }
                
                // check for collision on map from right
                if (obj->velocity.x > 0) { 
                    SDL_Rect tmp_r = (SDL_Rect){obj->position.x, obj->position.y, obj->body.w, obj->body.h};
                    tmp_r.x = new_x;
                    Vector2D collision = FE_CheckMapCollisionRight(&tmp_r);
                    if (!FE_VecNULL(collision)) {
                        obj->velocity.x = 0;
                        obj->position.x = collision.x - obj->body.w;
                    }
                }
                // check for collision on map from left
                if (obj->velocity.x < 0) {
                    SDL_Rect tmp_r = (SDL_Rect){obj->position.x, obj->position.y, obj->body.w, obj->body.h};
                    tmp_r.x = new_x;
                    Vector2D collision = FE_CheckMapCollisionLeft(&tmp_r);
                    if (!FE_VecNULL(collision)) {
                        obj->velocity.x = 0;
                        obj->position.x = collision.x;
                    }

                }

                // check for collision with other gameobjects
                // TODO

                obj->position.x += (obj->velocity.x * FE_DT_MULTIPLIER);
            }

            /* Apply Y force  */
            if (obj->velocity.y != 0) {
                float new_y = obj->position.y + obj->velocity.y;
                SDL_Rect check_rect = {obj->body.x, new_y, obj->body.w, obj->body.h};
                Vector2D GroundCollision = FE_CheckMapCollisionAbove(&check_rect);

                if (!FE_VecNULL(GroundCollision)) { // If we collide with the ground
                    
                    obj->position.y = GroundCollision.y - obj->body.h;
                    // If we hit the ground, bounce
                    if (obj->velocity.y > 8) {

                        // emit ground particles for high velocity bounce effect
                        float force = obj->mass * obj->velocity.y;
                        if (force > 500) {
                            FE_CreateParticleSystem(
                                (SDL_Rect){GroundCollision.x - obj->body.w, GroundCollision.y - 5, obj->body.w, 5},
                                0,
                                20,
                                1000,
                                false,
                                "impact.png",
                                FE_NewVector(10,10),
                                FE_NewVector(-obj->velocity.y / 5, -obj->velocity.y / 10),
                                false
                            );
                        }

                        obj->velocity.y = obj->velocity.y * -BOUNCE;
                    } else { // or come to rest if velocity not enough
                        obj->velocity.y = 0;
                    }
                }

                obj->position.y += (obj->velocity.y * FE_DT_MULTIPLIER);
                
            }
            // calculate new position
            FE_DT_RECT(obj->position, &obj->body);
        }
    }
}

void FE_Friction()
{
    if (FE_PhysObjects != 0) {
        for (FE_List *t = FE_PhysObjects; t; t = t->next) {
            FE_PhysObj *obj = (FE_PhysObj *)t->data;

            if (obj->velocity.x == 0)
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
}

void FE_ApplyForce(FE_PhysObj *o, Vector2D force)
{
    if (!o) {
        warn("Passing nullptr (ApplyForce)");
        return;
    }
    
    // clamp to max velocity
    o->velocity.x = clampf(o->velocity.x + force.x, -o->maxvelocity.x, o->maxvelocity.x);

    o->velocity.y += force.y;
}

FE_PhysObj *FE_CreatePhysObj(Uint16 mass, Uint16 maxvelocity, SDL_Rect body, bool moveable)
{
    FE_PhysObj *o = xmalloc(sizeof(FE_PhysObj));
    o->mass = mass;
    o->velocity = FE_NewVector(0, 0);
    o->maxvelocity = FE_NewVector(maxvelocity, 0);
    o->body = body;
    o->position = FE_NewVector(body.x, body.y);
    o->moveable = moveable;

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

static void FE_FPSCounter()
{
    // only update every second
    static float fps_timer = 0;
    fps_timer += FE_DT;
    if (fps_timer > 1) {
        fps_timer = 0;
        char title[64];    
        snprintf(title, 64, "FedoraEngine - FPS: %i", FE_FPS);
        SDL_SetWindowTitle(window, title);
    }
}

void FE_RunPhysics()
{    
    FE_FPSCounter();
    FE_Gravity();
    FE_Friction();
    FE_PhysLoop();
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

