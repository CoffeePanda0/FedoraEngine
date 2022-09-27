#include "include/fedoraengine.h"
#include "include/circle.h"

static void FE_RenderDrawCircle(FE_Circle *circle)
{
    GPU_Circle(PresentGame->Screen, circle->x, circle->y, circle->radius, circle->color);
}

static void FE_RenderFillCircle(FE_Circle *circle)
{
    GPU_CircleFilled(PresentGame->Screen, circle->x, circle->y, circle->radius, circle->color);
}

void FE_RenderCircle(FE_Circle *circle)
{
    if (circle->filled)
        FE_RenderFillCircle(circle);
    else
        FE_RenderDrawCircle(circle);
}