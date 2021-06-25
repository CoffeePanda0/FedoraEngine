#include "../game.h"

struct Animation {
    float speed; // speed of animation
    bool animated; // animation active?
    int frames; // amount of frames in the file
    SDL_Texture *spritesheet; // spritesheet
    int width;
    int height;
    int currentframe;
    int framespassed;
};

struct Animation CreateAnimation(int w, int h, int frames, float speed, char* text);
void RenderPlayerAnimation();