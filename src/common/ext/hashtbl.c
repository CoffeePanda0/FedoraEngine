/*
 * Derived from yclib.
 *
 * Copyright (c) 2022 Elaine Gibson, et al.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "hashtbl.h"

static uint32_t hash(const char *str) {
	uint32_t h = 0x811C9DC5UL;
	unsigned char *s = (unsigned char*)str;
	while (*s) {
		h ^= *s++;
		h *= 0x01000193UL;
	}
	return h;
}

void htinit(hashtable *tbl, free2fn release) {
	tbl->size = 16;
	tbl->used = 0;
	tbl->slots = calloc(tbl->size, sizeof *tbl->slots);
	tbl->release = release;
}

static size_t getidx(hashtable *tbl, const char *key) {
	size_t idx = hash(key) & (tbl->size - 1);
	while (tbl->slots[idx].key && strcmp(tbl->slots[idx].key, key) != 0)
		if (++idx == tbl->size)
			idx = 0;
	return idx;
}

#define getentry(tbl,key) (&tbl->slots[getidx(tbl, key)])

void *htget(hashtable *tbl, const char *key) {
	struct htentry *entry = getentry(tbl, key);
	return entry->key ? entry->val : NULL;
}

void htset(hashtable *tbl, char *key, void *val) {
	struct htentry *entry, *oldslots;
	size_t oldused, pos;

	entry = getentry(tbl, key);
	if (entry->key) {
		if (tbl->release)
			tbl->release(entry->key, entry->val);
	} else {
		if (tbl->used >= tbl->size / 2) {
			oldslots = tbl->slots;
			oldused = tbl->used;

			tbl->used = 0;
			tbl->size *= 2;
			tbl->slots = calloc(tbl->size, sizeof *tbl->slots);

			for (pos = 0; oldused; pos++) {
				if (oldslots[pos].key) {
					htset(tbl, oldslots[pos].key, oldslots[pos].val);
					oldused--;
				}
			}

			free(oldslots);
			entry = getentry(tbl, key);
		}
		tbl->used++;
	}

	entry->key = key;
	entry->val = val;
}

bool htunset(hashtable *tbl, const char *key) {
	size_t idx, cur, cidx;

	idx = getidx(tbl, key);

	if (!tbl->slots[idx].key)
		return false;

	if (tbl->release)
		tbl->release(tbl->slots[idx].key, tbl->slots[idx].val);
	tbl->slots[idx].key = NULL;
	tbl->used--;

	cur = idx;
	while (1) {
		if (++cur == tbl->size)
			cur = 0;
		if (!tbl->slots[cur].key)
			break;
		cidx = hash(tbl->slots[cur].key) & (tbl->size - 1);
		if ((cur > idx && (cidx <= idx || cidx > cur)) || (cur < idx && cidx <= idx && cidx > cur)) {
			tbl->slots[idx] = tbl->slots[cur];
			tbl->slots[cur].key = NULL;
			idx = cur;
		}
	}

	return true;
}

void htclear(hashtable *tbl) {
	size_t idx;
	for (idx = 0; tbl->used; idx++) {
		if (tbl->slots[idx].key) {
			if (tbl->release)
				tbl->release(tbl->slots[idx].key, tbl->slots[idx].val);
			tbl->slots[idx].key = NULL;
			tbl->used--;
		}
	}
}
