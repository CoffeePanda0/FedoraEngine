#include "../include/game.h"

bool FE_Camera_Inbounds(SDL_Rect *r, SDL_Rect *dsrct)
{
    if (r->x + r->w < dsrct->x) // to the left of dsrct
        return false;
    if (r->x > dsrct->w + dsrct->x) // to the right of dsrct
        return false;
    if (r->y + r->h < dsrct->y) // above dsrct
        return false;
    if (r->y > dsrct->y + dsrct->h) // below dsrct
        return false;
    return true;
}

void FE_MoveCamera(int x, int y, FE_Camera *c)
{
    if (!c)
        return;
    
    if (c->locked)
        return;

    if (c->x + x > c->x_bound)
        c->x = c->x_bound;
    else if (c->x + x < 0)
        c->x = 0;
    else
        c->x += x;

    if (c->y + y > c->y_bound)
        c->y = c->y_bound;
    else if (c->y + y < 0)
        c->y = 0;
    else
        c->y += y;
}