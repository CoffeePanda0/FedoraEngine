CC = gcc
CFLAGS = -MD -Wall -g -fno-common $$(sdl2-config --cflags)
LD = $(CC)

LIBS = -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer
