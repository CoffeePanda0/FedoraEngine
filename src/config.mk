CC = gcc
CFLAGS = -MD -Wall -Wextra -g -fno-common $$(sdl2-config --cflags) $$(pkg-config --cflags lua) -lm
LD = $(CC) 

LIBS = -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer -lm -ldl liblua.a
 
