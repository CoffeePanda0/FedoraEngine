#pragma once
#include <SDL.h>

extern SDL_Rect playerRect;

extern bool flipTex;

extern float acceleration;
extern bool moving;
extern float movAmount;
extern float maxAccel;
Mix_Chunk* JumpSound;

extern float velocity;
extern float gravity;

extern bool jumping;
void PlayerMove(int xAmount, int yAmount);
void PlayerJump();
void InitPlayer();
void SpawnPlayer(int xPos, int yPos, int width, int height);
