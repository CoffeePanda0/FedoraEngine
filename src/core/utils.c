// Contains commonly used functions e.g logging
#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <string.h>
#include <errno.h>
#include <SDL.h>
#include "include/utils.h"

static FILE* f;

char *strseps(char **sp, char *sep)
{
	char *p, *s;
	if (sp == NULL || *sp == NULL || **sp == '\0') return(NULL);
	s = *sp;
	p = s + strcspn(s, sep);
	if (*p != '\0') *p++ = '\0';
	*sp = p;
	return(s);
}

void log_init(void) {
	f = fopen("log.txt", "a");
}

void log_close(void) {
	fclose(f);
}

void die (enum dietypes type, const char *s, ...) {
	va_list ap, ap2;
	char *out;
	const char *err;
	size_t len;

	va_start(ap, s);
	va_copy(ap2, ap);
	err = 0;
	len = vsnprintf(NULL, 0, s, ap) + 1;
	switch (type) {
		case DT_NONE:
			break;
		case DT_ERRNO:
			err = strerror(errno);
			len += 2 + strlen(err);
			break;
		case DT_SDL:
			err = SDL_GetError();
			len += 2 + strlen(err);
			break;
	}

	out = xmalloc(len);
	vsprintf(out, s, ap2);
	if (err)
		sprintf(out+strlen(out), ": %s", err);

	vlog(LT_ERROR, "%s", out);
	SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Fatal Error", out, NULL);
	fflush(f);
	FE_Clean();
	SDL_free(out);

	exit(1);
}

void vlog (enum logtypes type, const char *s, ...) {

	va_list ap, ap2, ap3;
	time_t t;
	struct tm *tm;
	char tstr[12];

	if (!f)
		return;

	va_start(ap, s);
	time(&t);
	tm = gmtime(&t);
	strftime(tstr, 12, "[%H:%M:%S] ", tm);

	fputs(tstr, f);

	switch (type) {
		case LT_INFO:
			fputs(" INFO: ", f);
			break;
		case LT_WARN:
			fputs(" WARN: ", f);
			break;
		case LT_ERROR:
			fputs(" ERROR: ", f);
			break;
	}
	
	va_copy(ap2, ap);
	va_copy(ap3, ap2);

	vfprintf(f, s, ap2);

	char *out = xmalloc(512);
	vsprintf(out, s, ap3);

	vprintf(s, ap);
	printf("\n");
	va_end(ap2);
	va_end(ap);
	fputc('\n', f);
	fflush(f);
	FE_ConsoleSetText(out);
	free(out);
}

char *IntToSTR (int i) {
	char *str = xmalloc(sizeof(int)*8);
	sprintf(str, "%i", i);
	return str;
}

char *AddStr(const char *str, const char *add)
{
	char *newstr = xmalloc(strlen(str) + strlen(add) + 1);
	strcpy(newstr, str);
	strcat(newstr, add);
	return newstr;
}