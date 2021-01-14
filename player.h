#pragma once
#include <SDL.h>

extern SDL_Rect playerRect;

extern bool flipTex;

extern float acceleration;
extern bool moving;
extern float movAmount;
extern float maxAccel;
extern Mix_Chunk* JumpSound;

extern int MaxHealth;
extern int Health;
extern float velocity;
extern float gravity;

extern bool jumping;
void PlayerMove(int xAmount, int yAmount);
void PlayerJump();
void InitPlayer();
void PlayerDie();
void SpawnPlayer(int xPos, int yPos, int width, int height);
void HealthChange(int amount);

extern int scrollam;

extern SDL_Rect CollRect; // sep rect for collision