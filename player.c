// Player variables for physics and movement
#include "game.h"

SDL_Rect playerRect;
Mix_Chunk* JumpSound;

bool flipTex = false;
bool moving;

float acceleration = 10;
float velocity = 0;
float gravity = 4;

float movAmount = 3.0;
float maxAccel = 2.0;

int MaxHealth = 100;
int Health = 100;

SDL_Rect CollRect;

int scrollam;

void PlayerMove(int xAmount, int yAmount)
{
	if (!gLeft() && !gRight()) {
		playerRect.y += yAmount;
		
		CollRect.x += xAmount;
		CollRect.y += yAmount;

		if (CollRect.x > ((screen_width / 2)) && scrollam < screen_width) {
			scrollam += xAmount;
		} else playerRect.x += xAmount;
	}
}

void PlayerDie()
{
	UpdateUI();
	info("Player died health");
	SpawnPlayer(0, 0, 75, 90);
}

void HealthChange(int amount) // handles damage and health from items
{
	if (amount + Health <= 0) {
		Health = 100;
		PlayerDie();
	} else {
		if (Health + amount <= MaxHealth)
			Health += amount;
		else
			Health = MaxHealth;
		UpdateUI();
	}
}

void PlayerJump()
{
	Mix_PlayChannel(-1, JumpSound, 0);
	jumping = true;
	velocity = -7;
}

void SpawnPlayer(int xPos, int yPos, int width, int height)
{
	CollRect.x = xPos;
	CollRect.y = yPos;
	playerRect.x = xPos;
	playerRect.y = yPos;
	playerRect.w = width;
	playerRect.h = height;
}