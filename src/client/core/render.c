#include "include/include.h"
#include "../world/include/camera.h"

#ifndef _MATH_H
    #include <math.h>
#endif

SDL_Rect FE_ApplyZoom(SDL_Rect *r, FE_Camera *camera, bool locked)
{
    // use ceil to prevent floating point rounding causing anomyous pixels

    SDL_Rect RenderRect = *r;
    if (!locked) {
        RenderRect.x -= camera->x;
        RenderRect.y -= camera->y;

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
        RenderRect = FE_ApplyZoom(dst, camera, locked);
    else
        RenderRect = (SDL_Rect){RenderRect.x - camera->x, RenderRect.y - camera->y, RenderRect.w, RenderRect.h};

    SDL_Rect s;
    if (!src)
        s = SCREEN_RECT(camera);
    else
        s = *src;

    // Check if rect is in screen bounds
    if (FE_Camera_Inbounds(&RenderRect, &(SDL_Rect){0,0,PresentGame->WindowWidth, PresentGame->WindowHeight})) {
        return SDL_RenderCopy(PresentGame->Client->Renderer, texture->Texture, &s, &RenderRect);
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
        RenderRect = FE_ApplyZoom(dst, camera, locked);
    else
        RenderRect = (SDL_Rect){RenderRect.x - camera->x, RenderRect.y - camera->y, RenderRect.w, RenderRect.h};

    SDL_Rect s;
    if (!src)
        s = SCREEN_RECT(camera);
    else
        s = *src;

    if (FE_Camera_Inbounds(&RenderRect, &(SDL_Rect){0,0, PresentGame->WindowWidth, PresentGame->WindowHeight})) {
        return SDL_RenderCopyEx(PresentGame->Client->Renderer, texture->Texture, &s, &RenderRect, angle, NULL, flip);
    } else
        return 0;
}

int FE_RenderDrawLine(FE_Camera *camera, int x1, int y1, int x2, int y2, SDL_Color color) // Renders a line to the screen with camera offset
{
    if (!camera) {
        error("FE_RenderDrawLine: NULL camera");
        return -1;
    }

    // change the color of the renderer and restore it after drawing the line
    Uint8 prev_r, prev_g, prev_b, prev_a;
    SDL_GetRenderDrawColor(PresentGame->Client->Renderer, &prev_r, &prev_g, &prev_b, &prev_a);
    SDL_SetRenderDrawColor(PresentGame->Client->Renderer, color.r, color.g, color.b, color.a);

    int d = SDL_RenderDrawLine(PresentGame->Client->Renderer,
        (x1 - camera->x) * camera->zoom,
        (y1 - camera->y) * camera->zoom,
        (x2 - camera->x) * camera->zoom,
        (y2 - camera->y) * camera->zoom
    );

    SDL_SetRenderDrawColor(PresentGame->Client->Renderer, prev_r, prev_g, prev_b, prev_a);
    return d;
}

int FE_RenderRect(SDL_Rect *rect, SDL_Color color) // Renders a rect (filled) to the screen
{
    if (!rect) {
        error("FE_RenderRect: NULL rect");
        return -1;
    }

    Uint8 prev_r, prev_g, prev_b, prev_a;
    SDL_GetRenderDrawColor(PresentGame->Client->Renderer, &prev_r, &prev_g, &prev_b, &prev_a);

    SDL_BlendMode b;
    SDL_GetRenderDrawBlendMode(PresentGame->Client->Renderer, &b);

    SDL_SetRenderDrawBlendMode(PresentGame->Client->Renderer, SDL_BLENDMODE_NONE);
    SDL_SetRenderDrawColor(PresentGame->Client->Renderer, color.r, color.g, color.b, color.a);
    SDL_RenderDrawRect(PresentGame->Client->Renderer, rect);
    SDL_RenderFillRect(PresentGame->Client->Renderer, rect);

    SDL_SetRenderDrawColor(PresentGame->Client->Renderer, prev_r, prev_g, prev_b, prev_a);
    SDL_SetRenderDrawBlendMode(PresentGame->Client->Renderer, b);
    return 1;
}

void FE_RenderBorder(int thickness, SDL_Rect r, SDL_Color color)
{
    Uint8 prev_r, prev_g, prev_b, prev_a;

    SDL_GetRenderDrawColor(PresentGame->Client->Renderer, &prev_r, &prev_g, &prev_b, &prev_a);
    SDL_SetRenderDrawColor(PresentGame->Client->Renderer, color.r, color.g, color.b, 255);

    /* Border top */
    SDL_Rect top = (SDL_Rect){r.x, r.y, r.w, thickness};
    SDL_RenderFillRect(PresentGame->Client->Renderer, &top);

    /* Border bottom */
    SDL_Rect bottom = (SDL_Rect){r.x, r.y + r.h - thickness, r.w, thickness};
    SDL_RenderFillRect(PresentGame->Client->Renderer, &bottom);

    /* Border left */
    SDL_Rect left = (SDL_Rect){r.x, r.y, thickness, r.h};
    SDL_RenderFillRect(PresentGame->Client->Renderer, &left);

    /* Border right */
    SDL_Rect right = (SDL_Rect){r.x + r.w - thickness, r.y, thickness, r.h};
    SDL_RenderFillRect(PresentGame->Client->Renderer, &right);

    SDL_SetRenderDrawColor(PresentGame->Client->Renderer, prev_r, prev_g, prev_b, prev_a);
}

uint16_t rel_w(uint16_t w)
{
	return (w * PresentGame->WindowWidth) / 100;
}

uint16_t rel_h(uint16_t h)
{
	return (h / 100) * PresentGame->WindowHeight;
}