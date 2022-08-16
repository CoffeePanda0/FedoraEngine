#ifndef _H_STRARR
#define _H_STRARR

#include <stddef.h>

typedef struct {
    char **data;
    size_t items;
} FE_StrArr;


/** Adds a string to the array.
 * @param arr The array to add the string to.
 * @param str The string to add to the array.
 * @return The index of the string in the array.
 */
size_t FE_StrArr_Add(FE_StrArr *arr, char *str);


/* Creates a new string array.
 * @return The new string array.
 */
FE_StrArr *FE_StrArr_Create();


/** Frees the memory used by the string array.
 * @param arr The string array to free.
 */
void FE_StrArr_Destroy(FE_StrArr *arr);


#endif