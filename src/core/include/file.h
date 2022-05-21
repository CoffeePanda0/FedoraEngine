#ifndef H_FILE
#define H_FILE

#include <stdio.h>

/** Checks if a directory exists
 * \param path The path to the directory
 * \return True if the directory exists, false otherwise
 */
bool FE_DirectoryExists(const char *path);

/** Uses fread to read a string with the length prefix before
 *\param f File to read from
 *\return Pointer to the string
 */
char *ReadStr(FILE *f);

/** Writes str to file, appending the length of the string
 * \param f File to write to
 * \param str String to write
 * \return true on success, false on failure
 */
bool WriteStr(FILE *F, const char *str);

#endif