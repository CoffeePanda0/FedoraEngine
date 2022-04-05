/* Contains wrappers for built in memory management functions - including pointer checking and error handling */
#include "include/mem.h"
#include "include/utils.h"

void *xmalloc(size_t size)
{
	void *pt = malloc(size);
	if (!pt)
		error("Unable to malloc %li memory", size);

	return pt;
}

void *xrealloc(void *pt, size_t size)
{
	if (!pt)
		return xmalloc(size);

	if (size == 0) {
		free(pt);
		return 0;
	}
	
	void *p = realloc(pt, size);
	if (!p)
		error("Unable to realloc %li memory", size);
	
	return p;
}

void *xcalloc(size_t num, size_t size)
{
	void *pt = calloc(num, size);
	if (!pt)
		error("Failed to calloc %li memory of %li", num, size);
	
	return pt;
}

void xfree(void *pt)
{
	if (!pt)
		error("Trying to free NULL");
	free(pt);
}

void FE_DestroyTexture(SDL_Texture *texture)
{
    if (!texture)
        error("Trying to destroy NULL texture");

    SDL_DestroyTexture(texture);
}