#include "game.h"

bool flipTex = false;
bool moving;
SDL_Rect playerRect;

float acceleration = 1;
float velocity = 0;
float gravity = 3.0;

float movAmount = 5.0;
float maxAccel = 4.0;

void PlayerMove(int xAmount, int yAmount)
{
	playerRect.x += xAmount;
	playerRect.y += yAmount;
}

void PlayerJump()
{
	jumping = true;
	moving = true;
	velocity = -6;
}

void InitPlayer(int xPos, int yPos, int width, int height)
{
	playerRect.x = xPos;
	playerRect.y = yPos;
	playerRect.w = width;
	playerRect.h = height;
}
