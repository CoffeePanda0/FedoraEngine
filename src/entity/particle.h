#pragma once
#include "SDL.h"

enum ParticleTypes {
    PARTICLE_SNOW,
    PARTICLE_FIRE
};

void GenerateParticles(int num, int maxsize, int minsize, SDL_Rect area, char *texture, int speed, int death, bool respawn); // Creates each particle rect and add to list arr
void RenderParticles();

void NewParticleSystem(enum ParticleTypes t, int x, int y); // Use this whenever possible - easy preset particles

