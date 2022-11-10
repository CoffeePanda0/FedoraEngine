#include "include/include.h"

void FE_CalculateFPS()
{
    if (PresentGame->config->ShowFPS && !PresentGame->config->Headless) {
        // only update every second
        static float fps_timer = 0;
        fps_timer += FE_DT;
        if (fps_timer > 1) {
            fps_timer = 0;
            char title[64];    
            snprintf(title, 64, "%s - FPS: %i", PresentGame->config->WindowTitle, FE_FPS);
            SDL_SetWindowTitle(PresentGame->Client->Window, title);
        }
    }
}