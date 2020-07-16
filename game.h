#pragma once
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "SDL.h"
#include "SDL_image.h"
#include "SDL_ttf.h"
#include "gameobject.h"
#include "lib.h"
#include "player.h"

void init(const char* window_title, int xpos, int ypos, int window_width, int window_height);

void Clean();
void Update();
void Render();
void event_handler();
SDL_Texture* TextureManager(const char* texture, SDL_Renderer* ren);

extern SDL_Window* window;
extern SDL_Renderer* renderer;
extern bool GameActive;

static int screen_height = 500;
static int screen_width = 500;

extern bool vsync;