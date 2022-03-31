#ifndef H_UI_EVENT
#define H_UI_EVENT

#include <SDL.h>

bool FE_UI_HandleClick(SDL_Event *e);
void FE_HandleConsoleInput(SDL_Event *e, const Uint8* keyboard_state);

extern bool FE_StartedInput;

#endif