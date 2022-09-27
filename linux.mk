CC = gcc
CFLAGS = -MD -Wall -Wextra -Wpedantic -g -fstack-protector -O3 -fno-common $$(sdl2-config --cflags) -lm
LD = $(CC) 

LIBS = -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer -lm -lSDL2_gpu