#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <string.h>
#include <errno.h>
#include <SDL.h>
#include <SDL_image.h>

#include "lib.h"
#include "game.h"

static FILE* f;

void log_init(void) {
	f = fopen("log.txt", "w");
}

void log_close(void) {
	fclose(f);
}

void die (enum dietypes type, const char *s, ...) {
	va_list ap;
	char *out;
	const char *err;
	size_t len;

	va_start(ap, s);
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
	vsprintf(out, s, ap);
	if (err)
		sprintf(out+strlen(out), ": %s", err);

	fprintf(stderr, "%s\n", out);
	vlog(LT_ERROR, "%s", out);
	SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Fatal Error", out, NULL);

	if (renderer)
		SDL_DestroyRenderer(renderer);
	if (window)
		SDL_DestroyWindow(window);

	IMG_Quit();
	SDL_Quit();

	exit(1);
}

void vlog (enum logtypes type, const char *s, ...) {
	va_list ap;
	time_t t;
	struct tm *tm;
	char tstr[11];

	if (!f)
		return;

// time doesn't work /shrug -gibson
	time(&t);
	tm = gmtime(&t);
	strftime(tstr, 11, "[%H:%M:%S] ", tm);

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

	va_start(ap, s);
	vfprintf(f, s, ap);
	vprintf(s, ap);
	printf("\n");
	va_end(ap);
	fputc('\n', f);
}

