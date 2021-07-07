// Player variables for physics and movement
#include "game.h"

SDL_Rect playerRect;
Mix_Chunk* JumpSound;

bool moving;

float acceleration = 0;
float velocity = 0;
float maxvelocity = 7.0f;
float gravity = 4;

float movAmount = 3.2f;
float maxAccel = 2.5f;

int MaxHealth = 100;
int Health = 100;

SDL_Rect CollRect;

int scrollam = 0;
int hscrollam = 0;

// Dash stuff
static float DashAmount = 10; 
static Uint64 LastDashTime = 0; // Stores the time of the last dash
static Uint64 DashTime = 75;
static int DashTimeOut = 2500; // Time between dashes (ms)
static Uint64 start_time;
static Uint64 time_elapsed;

static bool Dashing = false;
static bool DashTriggered = false;

void TriggerDash()
{
	if (!Dashing) {
		if (LastDashTime + DashTimeOut < now || LastDashTime == 0) {
			DashTriggered = true;
		}
	}
}

void Dash()
{
	if (DashTriggered) {
		if (LastDashTime + DashTimeOut < now || LastDashTime == 0) {
			// Make dash only last specified length
			if (!Dashing) {
				start_time = SDL_GetPerformanceCounter();
				time_elapsed = 0;
			}
			Dashing = true;
			if (time_elapsed < DashTime) {
				
				time_elapsed = (SDL_GetPerformanceCounter() - start_time) / 1000000;
				acceleration = maxAccel;
				if (playerFlip == SDL_FLIP_NONE) {
					NewParticleSystem(PARTICLE_DASH_RIGHT, 0, 0);
					PlayerMove(-(DashAmount * maxAccel), 0);
				} else {
					NewParticleSystem(PARTICLE_DASH_LEFT, 0, 0);
					PlayerMove(DashAmount * maxAccel, 0);
				}
			} else {
				Dashing = false;
				DashTriggered = false;
				LastDashTime = now;
			}
		}
	}
}

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