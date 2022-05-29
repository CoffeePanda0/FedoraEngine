src := $(wildcard *.c */*.c */*/*.c */*/*/*.c)
obj := $(src:.c=.o)
out = FedoraEngine

CC = gcc
CFLAGS = -MD --pedantic -Wall -Wextra -g -fstack-protector -O2 -fno-common $$(sdl2-config --cflags) -lm
LD = $(CC) 

LIBS = -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer -lm
 

all: $(out)

$(out): $(obj)
	$(LD) $(LDFLAGS) -o $@ $(obj) $(LIBS)

.c.o:
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(out) $(obj) $(src:.c=.d)

fresh: clean all

vg-m:
	valgrind --leak-check=full --show-reachable=no --show-possibly-lost=no ./$(out) -m test

vg:
	valgrind --leak-check=full --show-reachable=no --show-possibly-lost=yes ./$(out)

-include $(src:.c=.d)
