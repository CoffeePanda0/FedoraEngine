#include "../include/game.h"

Vector2D FE_VecAdd(Vector2D v1, Vector2D v2)
{
    return (Vector2D){v1.x + v2.x, v1.y + v2.y};
}

Vector2D FE_VecSub(Vector2D v1, Vector2D v2)
{
    return (Vector2D){v1.x - v2.x, v1.y - v2.y};
}

Vector2D FE_VecMultiply(Vector2D v, float scalar)
{
    return (Vector2D){v.x * scalar, v.y * scalar};
}

bool FE_VecComp(Vector2D v1, Vector2D v2)
{
    if (v1.x == v2.x && v1.y == v2.y)
        return true;
    return false;
}

bool FE_VecNULL(Vector2D v)
{
    if (v.x == -1 && v.y == -1)
        return true;
    return false;
}

Vector2D FE_NewVector(float x, float y)
{
    return (Vector2D){x,y};
}