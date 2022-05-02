#ifndef H_UTILS
#define H_UTILS

#include "mem.h"
#include "../../include/game.h"

enum dietypes {
	DT_NONE,
	DT_ERRNO,
	DT_SDL,
};

enum logtypes {
	LT_INFO,
	LT_WARN,
	LT_ERROR,
};

/** Custom implementation of strsep to be universal
 * \param sp String to split
 * \param sep Delimiter to split on
 * \return Pointer to the first character of the next token
 */
char *strseps(char **sp, char *sep);

void die (enum dietypes, const char *, ...);

#define error(...) die(DT_NONE, __VA_ARGS__)
#define error_eno(...) die(DT_ERRNO,  __VA_ARGS__)
#define error_sdl(...) die(DT_SDL, __VA_ARGS__)

#define info(...) vlog(LT_INFO, __VA_ARGS__)
#define warn(...) vlog(LT_WARN, __VA_ARGS__)

char *IntToSTR (int i);


/* Adds one string to the end of the other, handling memory allocation
 * \param str String to add to
 * \param add String to add
 * \return Pointer to the destination string
 */
char *AddStr(const char *str, const char *add);


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


/** Checks if a string exists in an array of strings
 * \param arr The array of strings to check
 * \param n The number of elements in the array
 * \param str The string to compare against
*/
bool StrInArr(char **arr, size_t n, char *str);


/** Safe random function - catches floating point exception if 0 is passed
 * \param val The maximum random value
 * \return A number between 0 and val
*/
int s_Rand(int val);


void log_init(void);
void log_close(void);
void vlog (enum logtypes, const char *, ...);

#endif