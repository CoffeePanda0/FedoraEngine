#ifndef H_VECTOR2D
#define H_VECTOR2D

#include <stdbool.h>

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


/** Compares two vectors
 * \param v1 The first vector
 * \param v2 The second vector
 * \return True if the vectors are equal
 */
bool FE_VecComp(Vector2D v1, Vector2D v2);


/** Checks if a vector is null
 * \param v The vector to check
 * \return True if the vector is null
 */
bool FE_VecNULL(Vector2D v);


/** Returns a vector with the given x and y. Used so we don't have to constantly cast to struct.
 * \param x - The x paramater of the vector
 * \param y - The y paramater of the vector
 * \return The new vector
*/
Vector2D FE_NewVector(float x, float y);


/** Sets the rect values to the Vector position
 *\param position The position of the rect
 *\param rect The rect to set
*/ 
void FE_DT_RECT(Vector2D position, SDL_Rect *rect);


#define VEC_EMPTY (Vector2D){0, 0}
#define VEC_NULL (Vector2D){-1, -1}

#endif