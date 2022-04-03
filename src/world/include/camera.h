#ifndef H_CAMERA
#define H_CAMERA

#include <SDL.h>

typedef struct FE_Camera {
    int x, y;
    int x_bound, y_bound;
    bool locked;
} FE_Camera;

// Moves camera taking clamps into account
void FE_MoveCamera(int x, int y, FE_Camera *c);

// Returns false if r is not inside dsrct
bool FE_Camera_Inbounds(SDL_Rect *r, SDL_Rect *dsrct);

#endif