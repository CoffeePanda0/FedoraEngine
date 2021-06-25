#include "../game.h"

struct Animation CreateAnimation(int w, int h, int frames, float speed, char* text)
{
    struct Animation tmp;
    tmp.width = w;
    tmp.height = h;
    tmp.currentframe = 1;
    tmp.spritesheet = TextureManager(text, renderer);
    tmp.animated = true;
    tmp.frames = frames;
    tmp.speed = speed;
    tmp.framespassed = 0;
    return tmp;
}

void RenderPlayerAnimation()
{
    SDL_Rect TextureRect;
    SDL_Rect h;

    h.h = PlayerAnimation.height;
    h.w = PlayerAnimation.width;
    h.x = playerRect.x;
    h.y = playerRect.y;

    if (moving && !jumping) {
        if (PlayerAnimation.framespassed < PlayerAnimation.speed)
            PlayerAnimation.framespassed++;
        else {
            PlayerAnimation.framespassed = 0;
            if (PlayerAnimation.currentframe < PlayerAnimation.frames)
                PlayerAnimation.currentframe += 1;
            else
                PlayerAnimation.currentframe = 1;
        }
    } else {
        PlayerAnimation.currentframe = 1;
    }
    
    SDL_QueryTexture(PlayerAnimation.spritesheet, NULL, NULL, &TextureRect.w, &TextureRect.h);

    TextureRect.x = (PlayerAnimation.currentframe -1) * (TextureRect.w / PlayerAnimation.frames);
    TextureRect.w = h.w;

    SDL_RenderCopyEx(renderer, PlayerAnimation.spritesheet, &TextureRect, &h, 0, NULL, playerFlip);
}