CC = gcc
CFLAGS = -MD --pedantic -Wall -Wextra -g -fstack-protector -O2 -fno-common $$(sdl2-config --cflags) -lm
LD = $(CC) 

LIBS = -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer -lm