#include "../include/game.h"

#define ANIMATION_PATH "game/sprites/animations/"

static FE_List *animation_list = NULL;
static long last_update;

void FE_UpdateAnimations()
{
    // loops through each animation, changes frame if needed
    for (FE_List *l = animation_list; l; l = l->next) {
        FE_Animation *anim = (FE_Animation *)l->data;
        if (anim->active) {
            anim->time_passed += SDL_GetTicks() - last_update;
            if (anim->time_passed >= anim->frame_duration) {
                anim->current_frame = (anim->current_frame + 1) < anim->frame_count ? anim->current_frame + 1 : 0;
                anim->time_passed = 0;
            }
        }
    }
    last_update = SDL_GetTicks();
}

FE_Animation *FE_CreateAnimation(char *spritesheet_name, Uint8 frame_count, Uint16 frame_width, Uint16 frame_height, Uint16 frame_duration, bool active)
{
    FE_Animation *anim = xmalloc(sizeof(FE_Animation));

    char *temp_path = AddStr(ANIMATION_PATH, spritesheet_name);
    anim->spritesheet = FE_TextureFromFile(temp_path);
    xfree(temp_path);

    anim->frame_count = frame_count;
    anim->frame_width = frame_width;
    anim->frame_height = frame_height;
    anim->frame_duration = frame_duration;
    anim->active = active;

    anim->current_frame = 0;
    anim->time_passed = 0;

    FE_List_Add(&animation_list, anim);
    return anim;
}

SDL_Rect FE_GetAnimationFrame(FE_Animation *anim)
{
    SDL_Rect frame;
    frame.x = anim->current_frame * anim->frame_width;
    frame.y = 0;
    frame.w = anim->frame_width;
    frame.h = anim->frame_height;
    return frame;
}

void FE_PauseAnimation(FE_Animation *anim)
{
    anim->active = false;
}

void FE_PlayAnimation(FE_Animation *anim)
{
    anim->active = true;
}

int FE_DestroyAnimation(FE_Animation *anim)
{
    if (!anim) {
        warn("Tried to destroy a NULL animation");
        return -1;
    }
    FE_DestroyTexture(anim->spritesheet);
    FE_List_Remove(&animation_list, anim);
    xfree(anim);

    return 1;
}

int FE_CleanAnimations()
{
    if (!animation_list)
        return 0;

    for (FE_List *l = animation_list; l; l = l->next) {
        FE_Animation *anim = (FE_Animation *)l->data;
        SDL_DestroyTexture(anim->spritesheet);
        xfree(anim);
    }

    FE_List_Destroy(&animation_list);
    animation_list = 0;

    return 1;
}