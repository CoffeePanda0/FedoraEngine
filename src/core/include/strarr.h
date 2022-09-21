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



/** Checks if a string exists in a string array
 * @param arr the Array to check
 * @param string The string to find
 * @returns -1 if not found, otherwise, the index of the item
*/
ssize_t FE_StrArr_Exists(FE_StrArr *arr, char *str);


/** Removes a string from a string array
 * @param arr The array to remove from
 * @param index The index of the item to remove
*/
void FE_StrArr_RemoveAt(FE_StrArr *arr, ssize_t index);


#endif