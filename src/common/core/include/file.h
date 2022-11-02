#ifndef H_FILE
#define H_FILE

#include <stdio.h>
#include <stdbool.h>
#include "strarr.h"

/** Checks if a directory exists
 * \param path The path to the directory
 * \return True if the directory exists, false otherwises
 */
bool FE_File_DirectoryExists(const char *path);

/** Uses fread to read a string with the length prefix before
 *\param f File to read from
 *\return Pointer to the string
 */
char *FE_File_ReadStr(FILE *f);

/** Writes str to file, appending the length of the string
 * \param f File to write to
 * \param str String to write
 * \return true on success, false on failure
 */
bool FE_File_WriteStr(FILE *F, const char *str);


/**
 * @brief Deletes a file
 * 
 * @param path 
 * @return true on success, false on failure
*/
bool FE_File_Delete(const char *path);


/** Returns an array of all files found in a directory
 * \param dir The directory to search
 * \return the string array
*/
FE_StrArr *FE_File_GetFiles(const char *dir);


#endif