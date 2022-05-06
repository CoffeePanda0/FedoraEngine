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

    // check if in x bounds
    if (x != 0) {
        if (c->x + x > c->x_bound - PresentGame->window_width)
            c->x = c->x_bound - PresentGame->window_width;
        else if (c->x + x < c->x_min)
            c->x = c->x_min;
        else
            c->x += x;
    }

    // check if in y bounds
    if (y != 0) {
        if (c->y + y > c->y_bound)
            c->y = c->y_bound;
        else if (c->y + y < c->y_min)
            c->y = c->y_min;
        else
            c->y += y;
    }
}