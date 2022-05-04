#ifndef H_MEM
#define H_MEM

#include <stdlib.h>
#include <SDL.h>

/* Custom memory allocation, returns a pointer to allocated memory
*\param size size of memory to allocate
*/
void *xmalloc(size_t size);


/* Returns a pointer to reallocated memory, xmallocs if pointer is NULL
*\param ptr pointer to reallocate
*\param size size of memory to allocate
*/
void *xrealloc(void *pt, size_t size);


/** Returns a pointer to calloc'd memory
 * \param count Number of elements to allocate and set to 0
 * \param size Size of each element
*/
void *xcalloc(size_t num, size_t size);


/** Frees memory, sets pointer to 0
*\param pt Pointer to memory to free
*/
void xfree(void *pt);

#endif