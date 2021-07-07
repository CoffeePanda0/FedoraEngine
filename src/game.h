#pragma once

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>
#include "entity/gameobject.h"
#include "lib.h"
#include "player.h"
#include "ui/ui.h"
#include "map.h"
#include "ui/menu.h"
#include "entity/enemy.h"
#include "client.h"
#include "entity/particle.h"
#include "ui/console.h"
#include "lua.h"
#include "entity/animation.h"

#define LEN(x) (sizeof(x)/sizeof(x[0]))

extern int screen_height;
extern int screen_width;

extern Uint64 now;

void init(const char* window_title, int xpos, int ypos, int window_width, int window_height);
void SaveGame(const char *name);
void Clean();
void LoadSave(const char *name);

void Update();
void Render();

void event_handler();

extern Mix_Music* bgMusic;
Mix_Music* LoadMusic(const char* path);
Mix_Chunk* LoadSFX(const char* path);

extern SDL_Rect BgRect;

extern bool paused;
extern bool TextPaused;
extern bool intext;

SDL_Texture* TextureManager(const char* texture, SDL_Renderer* ren);
Mix_Music* LoadMusic(const char* path);
Mix_Chunk* LoadSFX(const char* path);

char *strseps(char **sp, char *sep);

extern SDL_Surface *s;
extern SDL_Window* window;
extern SDL_Renderer* renderer;
extern char *save_path;
extern bool GameActive;
extern bool multiplayer;