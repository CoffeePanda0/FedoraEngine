#include "include/camera.h"
#include "../../common/ext/yclist.h"
#include "../core/include/include.h"

static bool zooming = false;
static float newzoom;
static float zoom_rate;

// keeps track of cameras for freeing if we want to later on
static yList(FE_Camera*) cameras = 0;

bool FE_Camera_Inbounds(SDL_Rect *r, SDL_Rect *dsrct)
{
    if (r->x + r->w < dsrct->x) // to the left of dsrct
        return false;
    if (r->x > dsrct->w + dsrct->x) // to the right of dsrct
        return false;
    if (r->y + r->h < dsrct->y) // above dsrct
        return false;
    if (r->y > dsrct->y + dsrct->h) // below dsrct
        return false;
    return true;
}

void FE_Camera_SetZoom(FE_Camera *camera, float zoom) // Sets the zoom of the camera
{
    if (!camera) {
        error("FE_Camera_SetZoom: NULL camera");
        return;
    }

    float newzoom = clamp(zoom, 0.5f, 10.0f);
    camera->zoom = newzoom;
}

void FE_CleanCameras()
{
    if (!cameras || y_listlen(cameras) == 0)
        return;

    for (size_t i = 0; i < y_listlen(cameras); i++) {
        if (cameras[i]) 
            free(cameras[i]);
    }
    y_listfree(cameras);
    cameras = 0;
    zooming = false;
    zoom_rate = 0;
    newzoom = 0;
}

void FE_FreeCamera(FE_Camera *camera)
{
    if (!camera) {
        warn("Passing NULL to FE_FreeCamera");
        return;
    }

    for (size_t i = 0; i < y_listlen(cameras); i++) {
        if (cameras[i] == camera) {
            y_listerase(cameras, i);
            zooming = false;
            return;
        }
    }

    warn("Camera not found in list");
}

FE_Camera *FE_CreateCamera()
{
    FE_Camera *c = xmalloc(sizeof(FE_Camera));

    c->x = (PresentGame->WindowWidth / 2);
    c->y = (PresentGame->WindowHeight / 2);
    c->zoom = 1;
    c->maxzoom = 5.0f;
    c->minzoom = 0.5f;
    if (PresentGame->MapConfig.Loaded) {
        c->x_min = PresentGame->MapConfig.MinimumX;
        c->x_bound = PresentGame->MapConfig.MapWidth;
        c->y_bound = PresentGame->MapConfig.MapHeight;
    } else {
        c->x_min = 0;
        c->x_bound = PresentGame->WindowWidth * 28;
        c->y_bound = PresentGame->WindowHeight * 8;
    }
    c->movement_locked = false;
    c->follow = 0;
    c->y_min = 0;

    y_listpush(cameras, c);
    return c;
} 

void FE_UpdateCamera(FE_Camera *camera)
{
    if (zooming) {
        camera->zoom += (zoom_rate * FE_DT);
        if ((camera->zoom >= newzoom && zoom_rate > 0) || (camera->zoom <= newzoom && zoom_rate < 0)) {
            camera->zoom = newzoom;
            zooming = false;
        }
    }

    /* Follow object */
    if (camera->movement_locked || !camera->follow)
        return;

    SDL_Rect *r = camera->follow;
    
    if (r) {
        int initial_x = r->x;
        int initial_y = r->y;
        int win_height = (PresentGame->WindowHeight / camera->zoom);
        int win_width = (PresentGame->WindowWidth / camera->zoom);
        
        // Centre the player X on the screen (accounting for zoom)
        r->x = (win_width / 2) - (r->w * camera->zoom);
        camera->x = initial_x - r->x;

        // Check X bounds
        if (camera->x <= camera->x_min) {
            r->x = initial_x - camera->x_min;
            camera->x = camera->x_min;
        }
        if (camera->x >= camera->x_bound - win_width) {
            r->x = (initial_x - camera->x_bound + win_width);
            camera->x = initial_x - r->x;
        }

        // Centre the player Y on the screen (accounting for zoom)
        r->y = ((win_height / 2) - (r->h * camera->zoom));
        camera->y = initial_y - r->y;
    
        // Check Y bounds
        if (camera->y + win_height >= camera->y_bound) {
            r->y = (initial_y - camera->y_bound + win_height);
            camera->y = initial_y - r->y;
        } 
    } else {
        // Centre the camera on the screen (accounting for zoom)
        int dif_x = (PresentGame->WindowWidth / 2) - camera->x;
        int dif_y = (PresentGame->WindowHeight / 2) - camera->y;
        dif_x = dif_x / camera->zoom;
        dif_y = dif_y / camera->zoom;
        camera->x += dif_x;
        camera->y += dif_y;
    }

}

void FE_Camera_SmoothZoom(FE_Camera *camera, float amount, uint16_t time)
{
    if (!zooming) {
        zooming = true;
        newzoom = clamp(camera->zoom + amount, camera->minzoom, camera->maxzoom);
        zoom_rate = amount / (time / 1000.0f);
    }
}

SDL_Rect SCREEN_RECT(FE_Camera *camera)
{
    return (SDL_Rect){0, 0, PresentGame->WindowWidth / camera->zoom, PresentGame->WindowHeight / camera->zoom};
}

void FE_MoveCamera(float x, float y, FE_Camera *c)
{
    if (!c)
        return;
    
    if (c->movement_locked)
        return;

    // check if in x bounds
    if (x != 0) {
        if (c->x + x > c->x_bound - PresentGame->WindowWidth)
            c->x = c->x_bound - PresentGame->WindowWidth;
        else if (c->x + x < c->x_min)
            c->x = c->x_min;
        else
            c->x += x;
    }

    // check if in y bounds
    if (y != 0) {
        if (c->y + y > c->y_bound)
            c->y = c->y_bound;
        else if (c->y + y < c->y_min)
            c->y = c->y_min;
        else
            c->y += y;
    }
}
