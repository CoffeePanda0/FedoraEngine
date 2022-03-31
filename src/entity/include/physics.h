#ifndef H_PHYSICS
#define H_PHYSICS

#include <SDL.h>

#define GRAVITY 100
#define DRAG 1.2f
#define FRICTION 4;

#define clamp(num, min, max) _Generic((num), \
        int: clampi, \
        float: clampf \
    )((num), (min), (max))

typedef struct Vector2D {
    int x;
    int y;
} Vector2D;

typedef struct FE_PhysObj {
    Uint16 mass;
    Vector2D velocity;
    SDL_Rect body;
} FE_PhysObj;

void FE_RunPhysics();
void FE_ApplyForce(FE_PhysObj *o, int x, int y);
int FE_AddPhysInteractable(FE_PhysObj *o);
int FE_RemovePhysInteractable(FE_PhysObj *o);

float clampf(float num, float min, float max);
int clampi(int num, int min, int max);

#endif