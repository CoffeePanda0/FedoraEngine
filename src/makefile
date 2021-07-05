src = game.c save.c lib.c main.c player.c event.c ui/ui.c map.c ui/menu.c ext/json/tiny-json.c ui/dialogue.c entity/gameobject.c entity/enemy.c client.c entity/particle.c ui/console.c lua.c entity/animation.c
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