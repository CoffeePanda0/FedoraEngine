#ifndef H_LABEL
#define H_LABEL

#include <SDL.h>
#include "font.h"

typedef struct FE_Label {
    FE_Font *font;

    SDL_Texture *text;
    SDL_Color color;

    SDL_Rect r;
    SDL_Color backcolor;
    bool showbackground;
} FE_Label;


/* Renders all labels to the screen */
void FE_RenderLabels();


/** Creates a label with the given text and position 
 * \param font The font to use, or NULL for the default font
 * \param text The text to display
 * \param position The position to display the text
 * \param color The color of the text
 * \return The label created
*/
FE_Label *FE_CreateLabel(FE_Font *font, char *text, Vector2D position, SDL_Color color);


/** Updates an existing label's text
 * \param l The label to update
 * \param text The new text to display
*/
int FE_UpdateLabel(FE_Label *l, char *text);


/* Destroys all labels */
int FE_CleanLabels();


/** Destroys an existing label, freeing the label and stopping rendering of it
 * \param l The label to destroy
 * \return 1 on success, -1 on failure
*/
int FE_DestroyLabel(FE_Label *l);


/** Generates an SDL_Texture from the default font with given text
 *\param text The text to convert to a texture
 *\param color The color of the text
 *\returns A pointer to the new texture
*/
SDL_Texture *FE_TextureFromText(char *text, SDL_Color color);


#endif