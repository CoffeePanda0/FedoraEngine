#ifndef H_UIOBJECT
#define H_UIOBJECT

#include <SDL.h>
#include "../../core/include/texture.h"

typedef struct FE_UIObject {
    SDL_Rect r;
    FE_Texture *texture;
} FE_UIObject;

void FE_RenderUIObjects();
FE_UIObject *FE_CreateUIObject(int x, int y, int w, int h, char *texture_path);
int FE_DestroyUIObject(FE_UIObject *o);
int FE_CleanUIObjects();

#endif
