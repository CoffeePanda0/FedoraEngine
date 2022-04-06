#ifndef H_VECTOR2D
#define H_VECTOR2D

typedef struct Vector2D {
    float x, y;
} Vector2D;


/** Adds two vectors together
 *\param v1 The first vector
 *\param v2 The second vector
 *\return The sum of the two vectors
 */
Vector2D FE_VecAdd(Vector2D v1, Vector2D v2);


/** Subtracts vector v2 from vector v1
*\param v1 Vector to subtract from
*\param v2 Vector to subtract
*\return Vector v1 - v2
*/
Vector2D FE_VecSub(Vector2D v1, Vector2D v2);


/** Multiplies vector v by scalar
 * \param v Vector to multiply
 * \param scalar Scalar to multiply vector by
 * \return Vector v * scalar
 */
Vector2D FE_VecMultiply(Vector2D v, float scalar);


#define VEC_EMPTY (Vector2D){0, 0}

#endif