// Player variables for physics and movement
#include "game.h"

SDL_Rect playerRect;
Mix_Chunk* JumpSound;

bool moving;

float acceleration = 0;
float velocity = 0;
float maxvelocity = 7.0f;
float gravity = 4;

float movAmount = 3.2;
float maxAccel = 2.0;

int MaxHealth = 100;
int Health = 100;

SDL_Rect CollRect;

int scrollam = 0;
int hscrollam = 0;

void PlayerMove(int xAmount, int yAmount)
{ 

	if (playerRect.y + yAmount > 0) {

		CollRect.y += yAmount;
		
		if (playerRect.y > (screen_height / 2)) {
			hscrollam += yAmount;
		} else playerRect.y += yAmount;
		
			if (multiplayer) {
				char *packet = malloc(256);
				sprintf(packet, "MOV:%i,%i", CollRect.x, CollRect.y);
				send_packet(packet);
				free(packet);
			}
	}

	if ((!gRight(CollRect) && xAmount < 0) || (!gLeft(CollRect) && xAmount > 0)) {
		
		CollRect.x += xAmount;
		
		if (playerRect.x + 50 > (screen_width / 2) && scrollam + xAmount > 0) {  // Keep player centre screen
			scrollam += xAmount;
		} else
			playerRect.x += xAmount;
		
		if (multiplayer) {
			char *packet = malloc(256);
			sprintf(packet, "MOV:%i,%i", CollRect.x, CollRect.y);
			send_packet(packet);
			free(packet);
		}
	}
}

void PlayerDie()
{
	UpdateUI();
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
	velocity = -6;
}

void SpawnPlayer(int xPos, int yPos, int width, int height)
{
	CollRect.x = xPos + scrollam;
	CollRect.y = yPos + hscrollam;
	CollRect.w = width;
	CollRect.h = height;
	playerRect.x = xPos;
	playerRect.y = yPos;
	playerRect.w = width;
	playerRect.h = height;
}