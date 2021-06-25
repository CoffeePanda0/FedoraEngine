// Contains commonly used functions e.g logging
#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <string.h>
#include <errno.h>
#include <SDL.h>

#include "lib.h"
#include "game.h"

static FILE* f;

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

	out = SDL_malloc(len);
	vsprintf(out, s, ap2);
	if (err)
		sprintf(out+strlen(out), ": %s", err);

	vlog(LT_ERROR, "%s", out);
	SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Fatal Error", out, NULL);
	fflush(f);
	Clean();
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

	char *out = malloc(512);
	vsprintf(out, s, ap3);
	UpdateConsole(out);

	vprintf(s, ap);
	printf("\n");
	va_end(ap2);
	va_end(ap);
	fputc('\n', f);
	fflush(f);
	free(out);
}