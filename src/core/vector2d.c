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