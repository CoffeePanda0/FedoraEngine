#include <SDL_gpu.h>
#include "include/texture.h"
#include "include/utils.h"
#include "include/fedoraengine.h"
#include "../world/include/camera.h"

#ifndef _MATH_H
    #include <math.h>
#endif


GPU_Rect FE_ApplyZoom(GPU_Rect *r, FE_Camera *camera, bool locked)
{
    // use ceil to prevent floating point rounding causing anomyous pixels

    GPU_Rect RenderRect = *r;
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

int FE_RenderCopy(GPU_Target *target, FE_Camera *camera, bool locked, FE_Texture *texture, GPU_Rect *src, GPU_Rect *dst) // Renders a texture to the screen if in camera bounds
{
    GPU_Target *screen = target == 0 ? PresentGame->Screen : target;

    if (!texture || !dst || !texture->Texture) {
        error("FE_RenderCopy: NULL texture or dst");
        return -1;
    }

    GPU_Rect RenderRect = *dst;
    if (camera->zoom != 0)
        RenderRect = FE_ApplyZoom(dst, camera, locked);
    else
        RenderRect = (GPU_Rect){RenderRect.x - camera->x, RenderRect.y - camera->y, RenderRect.w, RenderRect.h};

    GPU_Rect s;
    if (!src)
        s = SCREEN_RECT(camera);
    else
        s = *src;

    // Check if rect is in screen bounds
    if (FE_Camera_Inbounds(&RenderRect, &(GPU_Rect){0,0,PresentGame->WindowWidth, PresentGame->WindowHeight})) {
        GPU_BlitRect(texture->Texture, &s, screen, &RenderRect);
        return 1;
    } else
        return 0;
}

int FE_RenderCopyEx(GPU_Target *target, FE_Camera *camera, bool locked, FE_Texture *texture, GPU_Rect *src, GPU_Rect *dst, double angle, GPU_FlipEnum flip)
{
    if (!texture || !dst || !texture->Texture) {
        error("FE_RenderCopyEx: NULL texture or dst");
        return -1;
    }

    GPU_Target *screen = target == 0 ? PresentGame->Screen : target;

    GPU_Rect RenderRect = *dst;
    if (camera->zoom != 0)
        RenderRect = FE_ApplyZoom(dst, camera, locked);
    else
        RenderRect = (GPU_Rect){RenderRect.x - camera->x, RenderRect.y - camera->y, RenderRect.w, RenderRect.h};

    GPU_Rect s;
    if (!src)
        s = SCREEN_RECT(camera);
    else
        s = *src;

    if (FE_Camera_Inbounds(&RenderRect, &(GPU_Rect){0,0, PresentGame->WindowWidth, PresentGame->WindowHeight})) {
        GPU_BlitRectX(texture->Texture, &s, screen, &RenderRect, angle, 0, 0, flip); // todo check pivot
        return 1;
    } else
        return 0;
}

void FE_RenderBorder(GPU_Target *target, int thickness, GPU_Rect r, SDL_Color color)
{
    GPU_Target *screen = target == 0 ? PresentGame->Screen : target;

    /* Border top */
    GPU_Rect top = (GPU_Rect){r.x, r.y, r.w, thickness};
    GPU_RectangleFilled2(screen, top, color);

    /* Border bottom */
    GPU_Rect bottom = (GPU_Rect){r.x, r.y + r.h - thickness, r.w, thickness};
    GPU_RectangleFilled2(screen, bottom, color);

    /* Border left */
    GPU_Rect left = (GPU_Rect){r.x, r.y, thickness, r.h};
    GPU_RectangleFilled2(screen, left, color);

    /* Border right */
    GPU_Rect right = (GPU_Rect){r.x + r.w - thickness, r.y, thickness, r.h};
    GPU_RectangleFilled2(screen, right, color);

}