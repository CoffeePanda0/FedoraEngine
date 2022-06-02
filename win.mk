CC = gcc
CFLAGS = -MD -I "D:\mingw\mingw64\include\SDL2" --pedantic -Wall -Wextra -g
LD = $(CC) 
LIBS = -lmingw32 -lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer -lm -lWs2_32