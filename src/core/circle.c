#include "../include/game.h"
#include "include/circle.h"

static void FE_RenderDrawCircle(FE_Circle *circle)
{
    /* Use the midpoint circle algorithm */

    int32_t x = circle->radius;
    int32_t y = 0;
    int32_t err = 0;

    while (x >= y)
    {
        SDL_RenderDrawPoint(renderer, circle->x + x, circle->y + y);
        SDL_RenderDrawPoint(renderer, circle->x + y, circle->y + x);
        SDL_RenderDrawPoint(renderer, circle->x - y, circle->y + x);
        SDL_RenderDrawPoint(renderer, circle->x - x, circle->y + y);
        SDL_RenderDrawPoint(renderer, circle->x - x, circle->y - y);
        SDL_RenderDrawPoint(renderer, circle->x - y, circle->y - x);
        SDL_RenderDrawPoint(renderer, circle->x + y, circle->y - x);
        SDL_RenderDrawPoint(renderer, circle->x + x, circle->y - y);

        y++;
        err += 1 + 2 * y;
        if (2 * (err - x) + 1 > 0)
        {
            x--;
            err += 1 - 2 * x;
        }
    }
}

static void FE_RenderFillCircle(FE_Circle *circle)
{
    /* Use the midpoint circle algorithm */

    SDL_SetRenderDrawColor(renderer, circle->color.r, circle->color.g, circle->color.r, circle->color.r);
    int32_t x = circle->radius;
    int32_t y = 0;
    int32_t err = 0;

    while (x >= y)
    {
        SDL_RenderDrawLine(renderer, circle->x - x, circle->y + y, circle->x + x, circle->y + y);
        SDL_RenderDrawLine(renderer, circle->x - x, circle->y - y, circle->x + x, circle->y - y);
        SDL_RenderDrawLine(renderer, circle->x - y, circle->y + x, circle->x + y, circle->y + x);
        SDL_RenderDrawLine(renderer, circle->x - y, circle->y - x, circle->x + y, circle->y - x);

        y++;
        err += 1 + 2 * y;
        if (2 * (err - x) + 1 > 0)
        {
            x--;
            err += 1 - 2 * x;
        }
    }
}

void FE_RenderCircle(FE_Circle *circle)
{
    // get original render draw color
    SDL_Color color;
    SDL_GetRenderDrawColor(renderer, &color.r, &color.g, &color.b, &color.a);

    // set new color to draw
    SDL_SetRenderDrawColor(renderer, circle->color.r, circle->color.g, circle->color.b, circle->color.a);

    // draw circle
    FE_RenderDrawCircle(circle);
    if (circle->filled)
        FE_RenderFillCircle(circle);
    
    // go back to original draw color
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
}