// Contains commonly used functions e.g logging
#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include "lib/string.h"
#include <errno.h>
#include "include/utils.h"
#include "include/fedoraengine.h"

#ifdef __has_include
    #if __has_include(<SDL.h>)
        #include <SDL.h>
		#define USE_SDL 1
    #endif
#endif

static FILE* f;

char *IntToSTR (int i)
{
	char *str = xmalloc(sizeof(int)*8);
	sprintf(str, "%i", i);
	return str;
}

void FE_Log_Init(void) {
	f = fopen("log.txt", "a");
	info("Started FedoraEngine");
	if (!f) {
		printf("Fatal Error: Failed to create log file\n");
		exit(-1);
	}
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
			len += 2 + mstrlen(err);
			break;
		case DT_SDL:
			#ifdef USE_SDL
				if (PresentGame->Client) {
					err = SDL_GetError();
					len += 2 + mstrlen(err);
				}
			#endif
			break;
	}

	out = xmalloc(len);
	vsprintf(out, s, ap2);
	if (err)
		sprintf(out+mstrlen(out), ": %s", err);

	vlog(LT_ERROR, "%s", out);
	
	#ifdef USE_SDL
		if (PresentGame->Client)
			SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Fatal Error", out, NULL);
	#endif

	fflush(f);
	free(out);

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

	free(out);
}

int s_Rand(int val)
{
	if (val == 0)
		return 0;
	return rand() % val;
}