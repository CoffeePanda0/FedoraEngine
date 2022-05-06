// Contains all code for managing UI Objects (e.g HUD). Pretty much the same as GameObjects.
#include "../include/game.h"

#define AssetPath "game/ui/"

static FE_List *UIObjects;

int FE_DestroyUIObject(FE_UIObject *o) // Frees resources and removes from linked list
{
    if (!o) {
        warn("Passing nullptr (FE_DestroyUIObject)");
        return 1;
    }
    FE_DestroyResource(o->texture->path);
    FE_List_Remove(&UIObjects, o);
     
    xfree(o);
    return 1;
}

int FE_CleanUIObjects() // removes all UI object nodes and frees memory
{
    for (struct FE_List *l = UIObjects; l; l = l->next) {
        FE_UIObject *tmp = l->data;
        FE_DestroyResource(tmp->texture->path);
        xfree(tmp);
    }

    FE_List_Destroy(&UIObjects);

    UIObjects = 0;
    return 1;
} 

FE_UIObject *FE_CreateUIObject(int x, int y, int w, int h, char *texture_path)  // returns the new object. Adds to render list
{
    FE_UIObject *tmp = xmalloc(sizeof(FE_UIObject));

    // combine the path with the asset path
    char *path = xmalloc(strlen(AssetPath) + strlen(texture_path) + 1);
    strcpy(path, AssetPath);
    strcat(path, texture_path);
    tmp->texture = FE_LoadResource(FE_RESOURCE_TYPE_TEXTURE, path);
    xfree(path);

    SDL_Rect r = {x, y, h, w};
    tmp->r = r;

    FE_List_Add(&UIObjects, tmp);
    
    return tmp;
}

void FE_RenderUIObjects()
{
    for (FE_List *o = UIObjects; o; o = o->next) {
        FE_UIObject *tmp = o->data;
        SDL_RenderCopy(PresentGame->renderer, tmp->texture->Texture, NULL, &tmp->r);
    }
}