CC = gcc
CFLAGS = -Isrc/common/core/include -MD -Wall -Wextra -Wpedantic -g -fstack-protector -fno-common $$(sdl2-config --cflags) -lm
LD = $(CC) 

LIBS = -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer -lm