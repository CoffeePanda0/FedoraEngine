#ifndef _FONT_H
#define _FONT_H

#include <SDL_ttf.h>

/* Struct for storing a font and its properties*/
typedef struct {
    TTF_Font *font;
    uint8_t size;
    char *name;

    uint16_t refs;
} FE_Font;


/** Loads a font and returns the FE_Font struct, or returns a previously loaded font with the same name and size
 * \param path Path to the font file
 * \param size Size of the font
 * \returns The font struct
*/
FE_Font *FE_LoadFont(char *name, uint8_t size);


/** Destroys an FE_Font struct
 * \param font The font struct to destroy
*/
void FE_DestroyFont(FE_Font *font);


/** Gets the length in pixels of a string with the given font
 * \param font The font to use
 * \param str The string to measure
 * \returns The length of the string in pixels
*/
int FE_FontStringSize(FE_Font *font, char *str);


/* Destroys all loaded fonts */
void FE_CleanFonts();


/** Wrapper for TTF_RenderText_Bleneded to support FE_Font type
 * \param font The font to use
 * \param str The string to render
 * \param color The color of the text
*/
SDL_Surface *FE_RenderText(FE_Font *font, const char *str, SDL_Color color);


#endif