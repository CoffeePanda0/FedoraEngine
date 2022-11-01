#ifndef SDL_rect_h_
#define SDL_rect_h_

/* Provides implementation of SDL_Rects for when
we create a server-only version without any SDL*/

typedef struct SDL_Rect {
    int x, y;
    int w, h;
} SDL_Rect;

typedef struct SDL_Point {
    int x, y;
} SDL_Point;

typedef struct SDL_FPoint {
    float x, y;
} SDL_FPoint;

typedef struct SDL_FRect {
    float x, y;
    float w, h;
} SDL_FRect;

#endif