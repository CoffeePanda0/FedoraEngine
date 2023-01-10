src := $(wildcard *.c */*.c */*/*.c */*/*/*.c */*/*/*/*.c */*/*/*/*/*.c)
obj = $(src:.c=.o)
out = FedoraEngine

include linux.mk

all: $(out)

$(out): $(obj)
	$(LD) $(LDFLAGS) -o $@ $(obj) $(LIBS)

.c.o:
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(out) $(obj) $(src:.c=.d)

fresh: clean all

vg-s:
	valgrind --leak-check=full --show-reachable=no --show-possibly-lost=no --track-origins=yes ./$(out) -s

vg:
	valgrind --leak-check=full --show-reachable=no --show-possibly-lost=no --track-origins=yes ./$(out)

-include $(src:.c=.d)