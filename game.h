#pragma once
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include "SDL.h"
#include "SDL_image.h"
#include "SDL_mixer.h"
#include "SDL_ttf.h"
#include "gameobject.h"
#include "lib.h"
#include "player.h"
#include "uitext.h"
#include "map.h"

void init(const char* window_title, int xpos, int ypos, int window_width, int window_height);

void Clean();

void Update();
void Render();
void event_handler();
Mix_Music* LoadMusic(const char* path);
Mix_Chunk* LoadSFX(const char* path);

extern bool onGround;

extern Uint32 fps_current;

SDL_Texture* TextureManager(const char* texture, SDL_Renderer* ren);

extern SDL_Surface *s;
extern SDL_Window* window;
extern SDL_Renderer* renderer;
extern bool GameActive;

static int screen_height = 512;
static int screen_width = 512;

extern bool overlay;
extern bool vsync;