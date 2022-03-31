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


void log_init(void);
void log_close(void);
void vlog (enum logtypes, const char *, ...);

#endif