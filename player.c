// Player variables for physics and movement
#include "game.h"

SDL_Rect playerRect;
Mix_Chunk* JumpSound;

bool flipTex = false;
bool moving;
float acceleration = 1;
float velocity = 0;
float gravity = 3.0;

float movAmount = 3.0;
float maxAccel = 2.0;

void PlayerMove(int xAmount, int yAmount)
{
	playerRect.x += xAmount;
	playerRect.y += yAmount;
}

void PlayerJump()
{
	Mix_PlayChannel(-1, JumpSound, 0);
	jumping = true;
	velocity = -7;
}

void SpawnPlayer(int xPos, int yPos, int width, int height)
{
	playerRect.x = xPos;
	playerRect.y = yPos;
	playerRect.w = width;
	playerRect.h = height;
}