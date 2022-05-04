#ifndef H_ANIMATION
#define H_ANIMATION

#include <SDL.h>

typedef struct FE_Animation {
    FE_Texture *spritesheet; // The spritesheet

    Uint8 frame_count; // The number of frames in the spritesheet
    Uint16 frame_width; // The width of each frame
    Uint16 frame_height; // The height of each frame
    
    unsigned long frame_duration; // The duration of each frame (in ms)
    float time_passed; // The time passed since the last frame was changed
    
    Uint8 current_frame; // The current frame

    bool active; // Whether the animation is active
} FE_Animation;


/** Creates an animation
 *\param spritesheet_name The name of the spritesheet
 *\param frame_count The number of frames in the spritesheet
 *\param frame_width The width of each frame
 *\param frame_height The height of each frame
 *\param frame_duration The duration of each frame in milliseconds
 *\param active Whether the animation is active
 *\return The animation
*/
FE_Animation *FE_CreateAnimation(char *spritesheet_name, Uint8 frame_count, Uint16 frame_width, Uint16 frame_height, Uint16 frame_duration, bool active);


/* Updates the animations */
void FE_UpdateAnimations();

/* Pauses the animation */
void FE_PauseAnimation(FE_Animation *anim);

/* Starts an animation sequence */
void FE_PlayAnimation(FE_Animation *anim);

/* Stops and destroys an animation */
int FE_DestroyAnimation(FE_Animation *anim);


/** Gets the current frame of an animation
 *\param anim The animation
 *\return The current frame as a rectangle for rendering
*/
SDL_Rect FE_GetAnimationFrame(FE_Animation *anim);

int FE_CleanAnimations();

#endif