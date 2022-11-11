// Contains all code for managing UI Objects (e.g HUD). Pretty much the same as GameObjects.
#include "../core/include/include.h"
#include "include/uiobject.h"

#define AssetPath "game/ui/"

void FE_UI_DestroyObject(FE_UI_Object *o, bool global) // Frees resources and removes from linked list
{
    if (!o) {
        warn("Passing nullptr (FE_DestroyUIObject)");
        return;
    }
    FE_DestroyResource(o->texture->path);

    if (global) {
        int r = FE_List_Remove(&PresentGame->UIConfig->ActiveElements->Objects, o);
        if (r == 1) PresentGame->UIConfig->ActiveElements->Count--;
    }

    free(o);
}

FE_UI_Object *FE_UI_CreateObject(int x, int y, int w, int h, char *texture_path)
{
    FE_UI_Object *tmp = xmalloc(sizeof(FE_UI_Object));

    // combine the path with the asset path
    char *path = xmalloc(mstrlen(AssetPath) + mstrlen(texture_path) + 1);
    mstrcpy(path, AssetPath);
    mstrcat(path, texture_path);
    tmp->texture = FE_LoadResource(FE_RESOURCE_TYPE_TEXTURE, path);
    free(path);

    SDL_Rect r = {x, y, w, h};
    tmp->r = r;
    
    return tmp;
}

void FE_UI_RenderObject(FE_UI_Object *o)
{
    SDL_RenderCopy(PresentGame->Client->Renderer, o->texture->Texture, NULL, &o->r);
}