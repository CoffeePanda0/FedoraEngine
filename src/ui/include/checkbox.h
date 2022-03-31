#ifndef H_CHECKBOX
#define H_CHECKBOX

#include <SDL.h>

typedef struct FE_Checkbox{ 
    SDL_Rect r; // rect containing the checkbox
    bool checked; // is the checkbox checked?
    void (*onclick)(void *data); // callback function on checkbox click
    void *onclick_data; // data passed to callback func

    SDL_Texture *label;
    SDL_Rect label_rect;
} FE_CheckBox;

FE_CheckBox *FE_CreateCheckbox(const char *label, int x, int y, bool checked, void (*onclick), void *onclick_data);
int FE_CleanCheckBoxes();
void FE_RenderCheckBoxes();
bool FE_CheckboxClick(int x, int y);

#endif
