#ifndef H_PHYSICS
#define H_PHYSICS

#include <SDL.h>
#include "../../core/include/vector2d.h"

#define GRAVITY 100
#define DRAG 1.2f
#define FRICTION 4;

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

void FE_RunPhysics();
void FE_ApplyForce(FE_PhysObj *o, Vector2D force);
int FE_AddPhysInteractable(FE_PhysObj *o);
int FE_RemovePhysInteractable(FE_PhysObj *o);

float clampf(float num, float min, float max);
int clampi(int num, int min, int max);

#endif