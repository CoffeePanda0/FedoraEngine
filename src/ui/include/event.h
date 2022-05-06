#ifndef H_UI_EVENT
#define H_UI_EVENT

#include <SDL.h>

/** Handles a click event to determine if any UI (button, checkbox, textbox) was pressed
* \param event The SDL_Event to handle
* \return true if any UI was pressed, false otherwise
*/
bool FE_UI_HandleClick(SDL_Event *e);


/** Handles text input to the console 
 * \param e The SDL_Event to handle
 * \param keyboard_state The SDL_KeyboardState to use
*/
void FE_HandleConsoleInput(SDL_Event *e, const Uint8* keyboard_state);

#endif