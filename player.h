#pragma once
#include <SDL.h>

extern SDL_Rect playerRect;

extern bool flipTex;

extern float acceleration;
extern bool moving;
extern float movAmount;
extern float maxAccel;

extern bool jumping;
extern float velocity;
extern float gravity;

void PlayerMove(int xAmount, int yAmount);
void PlayerJump();
void InitPlayer(int xPos, int yPos, int width, int height);
