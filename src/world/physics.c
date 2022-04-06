#include "../include/game.h"

static FE_List *FE_PhysObjects = 0; // Linked list of all physics objects

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

            if (obj->velocity.y < 0)
                warn("bees");
                
            float new_velocity = obj->velocity.y;
            float new_y = obj->body.y;

            
            /* calc accel and max accel */
            float terminal_velocity = sqrtf((2 * obj->mass * GRAVITY) / DRAG);
            new_velocity = obj->velocity.y + (obj->velocity.y + GRAVITY) * dT;


            /* clamp to terminal velocity */
            new_velocity = clamp(new_velocity, -terminal_velocity, terminal_velocity);    

            /* Check that location is inside map boundries */
            new_y += (new_velocity / 4);

            if (obj->body.h + obj->body.y > screen_height) {
                new_velocity = 0;
                obj->velocity.y = new_velocity;
                continue;
            }
            
            /* check that new location is not inside another object */
            for (FE_List *t2 = FE_PhysObjects; t2; t2 = t2->next) {
                if (t2 != t) {
                    FE_PhysObj *obj2 = (FE_PhysObj *)t2->data;
                    if (obj2->body.y < new_y && new_y < obj2->body.y + obj2->body.h) {
                        new_velocity = 0;
                        obj2->velocity.y = new_velocity;
                        continue;
                    }
                }
            }
            
            /* apply force to object */
            obj->body.y = new_y;
            obj->velocity.y = new_velocity;
        }
    } 
}

void FE_Friction()
{
    /*
    if (head != 0) {
        for (struct Node *t = head; t; t = t->next) {
            if (t->o->velocity.x == 0)
                continue;


            float new_velocity = t->o->velocity.x - FRICTION;
            if (new_velocity < 0)
                new_velocity = 0;
            

            t->o->velocity.x = new_velocity;
            t->o->body.x += new_velocity;
        }
    }
    */
}

void FE_ApplyForce(FE_PhysObj *o, Vector2D force)
{
    if (!o) {
        warn("Passing nullptr (ApplyForce)");
        return;
    }
    
    // todo : check for collision

    // check that object is inside map boundries
    if (o->body.x + o->body.w + force.x > screen_width) {
        force.x = 0;
        o->velocity.x = 0;
    }
    if (o->body.x + force.x <= 0) {
        force.x = 0;
        o->velocity.x = 0;
    }

    // clamp to max velocity
    if (force.x + o->velocity.x > o->maxvelocity.x) {
        force.x = o->maxvelocity.x - o->velocity.x;
    }

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

void FE_FPSCounter()
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
}