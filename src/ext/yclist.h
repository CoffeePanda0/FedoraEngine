/*
 * Derived from yclib.
 *
 * Copyright (c) 2022 Elaine Gibson, et al.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * All function-like macros defined in this header evaluate the list argument
 * more than once. Other parameters are only evaluated once, unless otherwise
 * specified.
 *
 * Due to the nature of these macros, they do not handle allocation errors.
 */

#ifndef Y_LIST_H
#define Y_LIST_H

#include "../core/include/mem.h"

#ifndef Y_LIST_ALLOC
	#define Y_LIST_ALLOC xmalloc
	#define Y_LIST_REALLOC xrealloc
#endif

#ifndef Y_LIST_FREE
	#define Y_LIST_FREE free
#endif

/* Must be >= 2. */
#ifndef Y_LIST_DEFAULTSIZE
	#define Y_LIST_DEFAULTSIZE 16
#endif

#include <string.h>

/* Sugar to identify lists. Lists must be initialised to NULL before use. */
#define yList(T) T*

#define y_listsetcap(list,cap) ((size_t*)(list))[-1] = cap
#define y_listsetlen(list,len) ((size_t*)(list))[-2] = len

/* Returns the current capacity of a list (number of allocated slots). */
#define y_listcap(list) ((list) ? ((size_t*)(list))[-1] : 0)

/* Returns the number of elements currently in a list. */
#define y_listlen(list) ((list) ? ((size_t*)(list))[-2] : 0)

/* Resizes a list to the given capacity, which must be >= 2. */
#define y_listresize(list,cap) \
	do { \
		size_t y_Lnewcap = cap; \
		size_t y_Lsize = y_Lnewcap*sizeof(*(list)) + sizeof(size_t)*2; \
		size_t *y_Lnew; \
		if (!(list)) { \
			y_Lnew = Y_LIST_ALLOC(y_Lsize); \
			y_Lnew[0] = 0; \
		} else { \
			y_Lnew = Y_LIST_REALLOC(&((size_t*)(list))[-2], y_Lsize); \
		} \
		y_Lnew[1] = y_Lnewcap; \
		list = (void*)&y_Lnew[2]; \
	} while (0)

/* Appends a value to a list. */
#define y_listpush(list,val) \
	do { \
		size_t y_Lcap, y_Llen; \
		if (!(list)) \
			y_listresize(list, Y_LIST_DEFAULTSIZE); \
		y_Lcap = y_listcap(list); \
		y_Llen = y_listlen(list); \
		if (y_Lcap <= y_Llen) \
			y_listresize(list, y_Lcap + y_Lcap/2); \
		(list)[y_Llen] = val; \
		y_listsetlen(list, y_Llen+1); \
	} while (0)

/* Removes a value from a list by index. */
#define y_listerase(list,idx) \
	do { \
		size_t y_Lidx = idx; \
		size_t y_Llen = y_listlen(list); \
		if (y_Llen > y_Lidx) { \
			size_t y_Lnewlen = y_Llen - 1; \
			y_listsetlen(list, y_Lnewlen); \
			memmove(list+y_Lidx, list+y_Lidx+1, (y_Lnewlen-y_Lidx) * sizeof(*(list))); \
		} \
	} while (0)

/* Releases the memory used by a list. */
#define y_listfree(list) Y_LIST_FREE((list) ? &((size_t*)(list))[-2] : NULL)

/* Returns a pointer to one past the end of a list (useful in for loops). */
#define y_listend(list) ((list) ? &((list)[y_listlen(list)]) : NULL)

#endif
