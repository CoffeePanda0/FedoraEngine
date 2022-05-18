#include <SDL.h>
#include "include/texture.h"
#include "include/utils.h"
#include "include/fedoraengine.h"
#include "../world/include/camera.h"

#ifndef _MATH_H
    #include <math.h>
#endif

static SDL_Rect ApplyZoom(SDL_Rect *r, FE_Camera *camera, bool locked)
{
    // use ceil to prevent floating point rounding causing anomyous pixels

    SDL_Rect RenderRect = *r;
    if (!locked) {
        RenderRect.y -= camera->y;
        RenderRect.x -= camera->x;

        RenderRect.x = ceil(camera->zoom * RenderRect.x);
        RenderRect.y = ceil(camera->zoom * RenderRect.y);
    }

    RenderRect.w = ceil(camera->zoom * RenderRect.w);
    RenderRect.h = ceil(camera->zoom * RenderRect.h);

    return RenderRect;
}

int FE_RenderCopy(FE_Camera *camera, bool locked, FE_Texture *texture, SDL_Rect *src, SDL_Rect *dst) // Renders a texture to the screen if in camera bounds
{
    if (!texture || !dst || !texture->Texture) {
        error("FE_RenderCopy: NULL texture or dst");
        return -1;
    }

    SDL_Rect RenderRect = *dst;
    if (camera->zoom != 0)
        RenderRect = ApplyZoom(dst, camera, locked);
    else
        RenderRect = (SDL_Rect){RenderRect.x - camera->x, RenderRect.y - camera->y, RenderRect.w, RenderRect.h};

    SDL_Rect s;
    if (!src)
        s = SCREEN_RECT(camera);
    else
        s = *src;

    // Check if rect is in screen bounds
    if (FE_Camera_Inbounds(&RenderRect, &(SDL_Rect){0,0,PresentGame->Window_width, PresentGame->Window_height})) {
        return SDL_RenderCopy(PresentGame->Renderer, texture->Texture, &s, &RenderRect);
    } else
        return 0;
}

int FE_RenderCopyEx(FE_Camera *camera, bool locked, FE_Texture *texture, SDL_Rect *src, SDL_Rect *dst, double angle, SDL_RendererFlip flip)
{
    if (!texture || !dst || !texture->Texture) {
        error("FE_RenderCopyEx: NULL texture or dst");
        return -1;
    }

    SDL_Rect RenderRect = *dst;
    if (camera->zoom != 0)
        RenderRect = ApplyZoom(dst, camera, locked);
    else
        RenderRect = (SDL_Rect){RenderRect.x - camera->x, RenderRect.y - camera->y, RenderRect.w, RenderRect.h};

    SDL_Rect s;
    if (!src)
        s = SCREEN_RECT(camera);
    else
        s = *src;

    if (FE_Camera_Inbounds(&RenderRect, &(SDL_Rect){0,0, PresentGame->Window_width, PresentGame->Window_height})) {
        const SDL_Point center = (SDL_Point){RenderRect.w/2, RenderRect.h/2};
        return SDL_RenderCopyEx(PresentGame->Renderer, texture->Texture, &s, &RenderRect, angle, &center, flip);
    } else
        return 0;
}

int FE_RenderDrawLine(FE_Camera *camera, int x1, int y1, int x2, int y2, SDL_Color color) // Renders a line to the screen with camera offset
{
    if (!camera) {
        error("FE_RenderDrawLine: NULL camera");
        return -1;
    }

    // return if line is out of bounds
    if (x1 + camera->x < 0 || x1 > PresentGame->Window_width + camera->x || y1 + camera->y < 0 || y1 > PresentGame->Window_height + camera->y)
        return 0;

    // change the color of the renderer and restore it after drawing the line
    Uint8 prev_r, prev_g, prev_b, prev_a;
    SDL_GetRenderDrawColor(PresentGame->Renderer, &prev_r, &prev_g, &prev_b, &prev_a);
    SDL_SetRenderDrawColor(PresentGame->Renderer, color.r, color.g, color.b, color.a);

    int d = SDL_RenderDrawLine(PresentGame->Renderer,
        (x1 - camera->x) * camera->zoom,
        (y1 - camera->y) * camera->zoom,
        (x2 - camera->x) * camera->zoom,
        (y2 - camera->y) * camera->zoom
    );

    SDL_SetRenderDrawColor(PresentGame->Renderer, prev_r, prev_g, prev_b, prev_a);
    return d;
}

int FE_RenderRect(SDL_Rect *rect, SDL_Color color) // Renders a rect (filled) to the screen
{
    if (!rect) {
        error("FE_RenderRect: NULL rect");
        return -1;
    }

    Uint8 prev_r, prev_g, prev_b, prev_a;
    SDL_GetRenderDrawColor(PresentGame->Renderer, &prev_r, &prev_g, &prev_b, &prev_a);

    SDL_BlendMode b;
    SDL_GetRenderDrawBlendMode(PresentGame->Renderer, &b);

    SDL_SetRenderDrawBlendMode(PresentGame->Renderer, SDL_BLENDMODE_NONE);
    SDL_SetRenderDrawColor(PresentGame->Renderer, color.r, color.g, color.b, color.a);
    SDL_RenderDrawRect(PresentGame->Renderer, rect);
    SDL_RenderFillRect(PresentGame->Renderer, rect);

    SDL_SetRenderDrawColor(PresentGame->Renderer, prev_r, prev_g, prev_b, prev_a);
    SDL_SetRenderDrawBlendMode(PresentGame->Renderer, b);
    return 1;
}