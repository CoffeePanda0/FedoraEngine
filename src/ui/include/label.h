#ifndef _LABEL_H
#define _LABEL_H

#include <SDL.h>
#include <stdbool.h>
#include "../../physics/include/vector.h"
#include "font.h"

typedef struct {
    FE_Font *font;
    uint16_t linewidth;

    SDL_Texture *texture;
    SDL_Color color;

    char *text;

    SDL_Rect r;
    SDL_Color backcolor;
    bool showbackground;
} FE_UI_Label;


/** Creates a label with the given text and position 
 * \param font The font to use, or NULL for the default font
 * \param text The text to display
 * \param linewidth The maximum width of each line of the label
 * \param position The position to display the text
 * \param color The color of the text
 * \return The label created
*/
FE_UI_Label *FE_UI_CreateLabel(FE_Font *font, char *text, uint16_t linewidth, vec2 position, SDL_Color color);


/** Updates an existing label's text
 * \param l The label to update
 * \param text The new text to display
*/
void FE_UI_UpdateLabel(FE_UI_Label *l, char *text);


/** Destroys an existing label, freeing the label and stopping rendering of it
 * \param l The label to destroy
 * \param global Whether to free the label from the global list of labels
*/
void FE_UI_DestroyLabel(FE_UI_Label *l, bool global);


/** Generates an SDL_Texture from the default font with given text
 *\param text The text to convert to a texture
 *\param color The color of the text
 *\returns A pointer to the new texture
*/
SDL_Texture *FE_TextureFromText(char *text, SDL_Color color);


/** Renders a label to the screen
 * \param l The label to render
*/
void FE_UI_RenderLabel(FE_UI_Label *l);


#endif