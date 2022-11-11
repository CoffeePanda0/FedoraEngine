#ifndef _H_CLIENTCONFIG
#define _H_CLIENTCONFIG

#include "../../../client/ui/include/font.h"

typedef struct FE_Client {
    struct FE_Font *Font;

    struct SDL_Window *Window;
    struct SDL_Renderer *Renderer;

    bool FullScreen;
    bool ConsoleVisible;
    bool StartedInput;

    int DialogueSpeed;
} FE_Client;

typedef struct FE_UIConfig {
    FE_UIList *ActiveElements;
    FE_Font *UIFont;
    SDL_Color UIFontColor;

    bool DialogueActive;
    bool InText;
    bool MBShown;

} FE_UIConfig;

#endif