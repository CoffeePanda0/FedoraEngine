#pragma once
enum dietypes {
	DT_NONE,
	DT_ERRNO,
	DT_SDL,
};

void die (enum dietypes, const char *, ...);

#define error(...) die(DT_NONE, __VA_ARGS__)
#define error_eno(...) die(DT_ERRNO,  __VA_ARGS__)
#define error_sdl(...) die(DT_SDL, __VA_ARGS__)

enum logtypes {
	LT_INFO,
	LT_WARN,
	LT_ERROR,
};

void log_init(void);
void log_close(void);
void vlog (enum logtypes, const char *, ...);

#define info(...) vlog(LT_INFO, __VA_ARGS__);
#define warn(...) vlog(LT_WARN, __VA_ARGS__);