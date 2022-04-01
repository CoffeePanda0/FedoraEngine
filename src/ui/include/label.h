#ifndef H_LABEL
#define H_LABEL

#include <SDL.h>

typedef struct FE_Label {
    SDL_Texture *text;
    SDL_Rect r;
    SDL_Color color;
} FE_Label;

void FE_RenderLabels();
FE_Label *FE_CreateLabel(char *text, int x, int y, SDL_Color color);
int FE_UpdateLabel(FE_Label *l, char *text);
int FE_CleanLabels();
int FE_DestroyLabel(FE_Label *l);


/** Generates an SDL_Texture from the default font with given text
 *\param text The text to convert to a texture
 *\param color The color of the text
 *\returns A pointer to the new texture
*/
SDL_Texture *FE_TextureFromText(char *text, SDL_Color color);

#endif