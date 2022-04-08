#include "../include/game.h"

static FE_List *FE_PhysObjects = 0; // Linked list of all physics objects

/* TODO:
    - friction works less to the left?
    - nicer y movement on camera
    - going to sides causes increase in Y velocity?
    - longer player holds down space for, higher they jump
- */

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
            new_velocity = obj->velocity.y + (obj->velocity.y + GRAVITY) * dT;

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

            float new_y = obj->body.y + obj->velocity.y;
            
            if (obj->velocity.x != 0) {
                float new_x = obj->body.x + obj->velocity.x;

                // check for map boundies
                if (new_x <= 0 || new_x + obj->body.w > FE_Map_Width) {
                    obj->velocity.x = 0;
                    continue;
                }
                
                // check for collision from right
                if (obj->velocity.x > 0) { 
                    SDL_Rect tmp_r = obj->body;
                    tmp_r.x = new_x;
                    if (!FE_VecNULL(FE_CheckMapCollisionRight(&tmp_r))) {
                        obj->velocity.x = 0;
                        continue;
                    }
                }
                // check for collision from left
                if (obj->velocity.x < 0) {
                    SDL_Rect tmp_r = obj->body;
                    tmp_r.x = new_x;
                    if (!FE_VecNULL(FE_CheckMapCollisionLeft(&tmp_r))) {
                        obj->velocity.x = 0;
                        continue;
                    }
                }

                obj->body.x = new_x;
            }

            if (obj->velocity.y != 0) {
                SDL_Rect check_rect = {obj->body.x, new_y, obj->body.w, obj->body.h};
                Vector2D GroundCollision = FE_CheckMapCollisionAbove(&check_rect);
                if (!FE_VecNULL(GroundCollision)) {
                    if (obj->velocity.y > 4) {
                        obj->velocity.y = obj->velocity.y * -BOUNCE;
                    } else
                        obj->velocity.y = 0;
                }

                obj->body.y += obj->velocity.y;
            }

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
            if (obj->velocity.x > 0 && obj->velocity.x < 0.02) {
                obj->velocity.x = 0;
                continue;
            }
            if (obj->velocity.x < 0 && obj->velocity.x > -0.02) {
                obj->velocity.x = 0;
                continue;
            }

            obj->velocity.x = obj->velocity.x * FRICTION;
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
    if (o->velocity.x + force.x > 0)
        o->velocity.x = clamp(o->velocity.x + force.x, 0, o->maxvelocity.x);
    else if (o->velocity.x + force.x < 0)
        o->velocity.x = clamp(o->velocity.x + force.x, -o->maxvelocity.x, 0);

    o->velocity.x += force.x;
    o->velocity.y += force.y;
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
    static int frame_count = 0;
    static float last_time = 0;
    static float fps = 0;
    frame_count++;
    float current_time = SDL_GetTicks();
    if (current_time - last_time >= 1000) {
        fps = frame_count;
        frame_count = 0;
        last_time = current_time;
    }

    char title[64];    
    snprintf(title, 64, "FedoraEngine - FPS: %i", (int)fps);
    SDL_SetWindowTitle(window, title);
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