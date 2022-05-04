#include <stddef.h>
#include <stdbool.h>

typedef void (*free2fn)(char *key, void *val);

struct htentry {
	char *key;
	void *val;
};

typedef struct hashtable {
	size_t size, used;
	struct htentry *slots;
	free2fn release;
} hashtable;

void htinit(hashtable *tbl, free2fn release);
void *htget(hashtable *tbl, const char *key);
void htset(hashtable *tbl, char *key, void *val);
bool htunset(hashtable *tbl, const char *key);
void htclear(hashtable *tbl);
