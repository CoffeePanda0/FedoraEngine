src = game.c lib.c main.c player.c gameobject.c ui/ui.c map.c ui/menu.c
obj = $(src:.c=.o)
out = FedoraEngine

include config.mk

all: $(out)

$(out): $(obj)
	$(LD) $(LDFLAGS) -o $@ $(obj) $(LIBS)

.c.o:
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(out) $(obj) $(src:.c=.d)

fresh: clean all

-include $(src:.c=.d)