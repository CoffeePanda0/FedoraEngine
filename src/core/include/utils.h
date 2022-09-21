#ifndef H_UTILS
#define H_UTILS

#include "mem.h"
#include <stdbool.h>
#include <stdio.h>

enum dietypes {
	DT_NONE,
	DT_ERRNO,
	DT_SDL
};

enum logtypes {
	LT_INFO,
	LT_WARN,
	LT_ERROR
};


/** Returns the width relative to the screen width
 * \param w The width to convert (as a percentage)
 * \return The width relative to the screen width
 */
uint16_t rel_w(uint16_t w);


/** Returns the height relative to the screen height
 * \param h The height to convert (as a percentage)
 * \return The height relative to the screen height
 */
uint16_t rel_h(uint16_t h);

void die (enum dietypes, const char *, ...);

#define error(...) die(DT_NONE, __VA_ARGS__)
#define error_eno(...) die(DT_ERRNO,  __VA_ARGS__)
#define error_sdl(...) die(DT_SDL, __VA_ARGS__)

#define info(...) vlog(LT_INFO, __VA_ARGS__)
#define warn(...) vlog(LT_WARN, __VA_ARGS__)


/** Returns a malloc'd string from an int
 * \param i The int to convert
 * \return The malloc'd string
 * \warning The caller is responsible for freeing the string
*/
char *IntToSTR (int i);

#define clamp(value, min, max)\
    (value < min ? min : (value > max ? max : value));


/** Safe random function - catches floating point exception if 0 is passed
 * \param val The maximum random value
 * \return A number between 0 and val
*/
int s_Rand(int val);


void FE_Log_Init(void);
void log_close(void);
void vlog (enum logtypes, const char *, ...);

#endif