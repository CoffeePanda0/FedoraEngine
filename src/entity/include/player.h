#ifndef H_PLAYER
#define H_PLAYER

#include "../entity/include/physics.h"

typedef struct FE_Player {
    FE_PhysObj *PhysObj;
    float movespeed;
    float jumpforce;

    SDL_Rect render_rect; // the location displayed on the screen
    SDL_Texture *texture;
} FE_Player;

#endif