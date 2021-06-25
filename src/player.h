#pragma once
#include <SDL.h>

extern SDL_Rect playerRect;

extern float acceleration;
extern bool moving;
extern float movAmount;
extern float maxAccel;
extern Mix_Chunk* JumpSound;
extern bool jumping;

extern struct Animation PlayerAnimation;

extern int MaxHealth;
extern int Health;
extern float velocity;
extern float gravity;
extern float maxvelocity;

extern int hscrollam;

extern bool jumping;
void PlayerMove(int xAmount, int yAmount);
void PlayerJump();
void InitPlayer();
void PlayerDie();
void SpawnPlayer(int xPos, int yPos, int width, int height);
void HealthChange(int amount);


extern int scrollam;
extern SDL_RendererFlip playerFlip;

extern SDL_Rect CollRect; // sep rect for collision