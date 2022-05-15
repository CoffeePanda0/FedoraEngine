#ifndef H_CAMERA
#define H_CAMERA

#include <SDL.h>
#include <stdbool.h>

typedef struct FE_Camera {
    float x, y;
    float zoom, maxzoom, minzoom;
    
    int x_min, y_min;
    int x_bound, y_bound;
    bool movement_locked;

    SDL_Rect *follow;
} FE_Camera;

// Moves camera taking clamps into account
void FE_MoveCamera(float x, float y, FE_Camera *c);

// Returns false if r is not inside dsrct
bool FE_Camera_Inbounds(SDL_Rect *r, SDL_Rect *dsrct);


/** Sets the zoom level of the camera
 * \param camera The camera to set the zoom level of
 * \param zoom The zoom level to set the camera to
*/
void FE_Camera_SetZoom(FE_Camera *camera, float zoom); 


/** Smoothly zooms the camera over a period of time
 * \param camera The camera to zoom
 * \param amount The total change of the camera zoom level
 * \param time The total time to zoom the camera (in ms)
*/
void FE_Camera_SmoothZoom(FE_Camera *camera, float amount, uint16_t time);


/** Updates the camera to follow 
 * \param camera The camera to update
*/
void FE_UpdateCamera(FE_Camera *camera);


/** Sets a camera to follow a given rect
 * \param camera The camera to move
 * \param rect The rect to folloe
*/
void FE_Camera_Follow(FE_Camera *camera, SDL_Rect *rect);


/* Creates and returns a new camera */
FE_Camera *FE_CreateCamera();


/** Safely destroys a camera
 * \param camera The camera to destroy
*/
void FE_FreeCamera(FE_Camera *camera);


/* Destroys all cameras that have been loaded into the game so far */
void FE_CleanCameras();

#endif