#ifndef H_MESSAGEBOX
#define H_MESSAGEBOX

#include "../../include/game.h"

enum FE_MBTYPE {
    MESSAGEBOX_TEXT,
    MESSAGEBOX_TEXTBOX
};

extern bool FE_MBShown;

typedef struct FE_MessageBox {
    enum FE_MBTYPE type;

    SDL_Rect displayrect;

    SDL_Texture *texture;

    SDL_Texture *title_texture;
    SDL_Rect title_rect;

    SDL_Texture *dialogue_texture;
    SDL_Rect dialogue_rect;

    FE_TextBox *textbox;

    FE_Button *button;
    
} FE_MessageBox;

int FE_ShowInputMessageBox(char *title, char *dialogue, void (*onclick), void *onclick_data);
int FE_ShowMessageBox(char *title, char *body);

void FE_RenderMessageBox();
void FE_DestroyMessageBox();
char *FE_GetMessageBoxInput();

#endif