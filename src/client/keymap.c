#include <stdlib.h>
#include <string.h>
#include <../lib/string.h>
#include "../common/core/include/utils.h"
#include "../common/ext/hashtbl.h"
#include "ui/include/messagebox.h"

#define KEYMAP_FILE "game/keymap.txt"

static hashtable keymap;
static bool keymap_initialsed = false;

typedef struct {
    uint8_t keycode;
} entry;

// Saves the current keymap to file
void FE_Key_Save()
{
    if (!keymap_initialsed)
        error("Keymap not initialsed");
    
    FILE *f = fopen(KEYMAP_FILE, "w");

    if (!f) {
        warn("Could not open keymap file for writing");
        return;
    }

    // Write each item in the hashtable
    for (size_t idx = 0; idx < keymap.size; idx++) {
        if (!keymap.slots[idx].key) continue;
        fprintf(f, "%s=%i\n", keymap.slots[idx].key, ((entry *)keymap.slots[idx].val)->keycode);
    }

    info("Saved keymap");
    fclose(f);
}

int FE_Key_Get(const char *input)
{
    if (!keymap_initialsed)
        error("Keymap not initialsed");

    void *res = htget(&keymap, input);
    if (!res) {
        warn("Key %s not mapped", input);
        return 0;
    }

    return ((entry *)res)->keycode;
}

static void DestroyKey(char *key, void *val)
{
    free(key);
    free(val);
}

// Assigns a key to an input
int FE_Key_Assign(char *input, uint8_t keycode)
{
    // Check if keycode is already assigned
    for (size_t i = 0; i < keymap.size; i++) {
        if (!keymap.slots[i].key) continue;
        if (((entry *)keymap.slots[i].val)->keycode == keycode && strcmp(keymap.slots[i].key, input) != 0) {
            char msg[128];
            sprintf(msg, "Key '%s' is already assigned to %s", (char*)SDL_GetKeyName(SDL_GetKeyFromScancode(keycode)), keymap.slots[i].key);
            FE_Messagebox_Show("Key Already Assigned", msg, MESSAGEBOX_TEXT);
            return 0; // enter or space also overlap with other keys
        }
    }

    entry *e = xmalloc(sizeof(entry));
    e->keycode = keycode;
    htset(&keymap, mstrdup(input), e);
    return 1;
}

void FE_Key_Clean()
{
    if (!keymap_initialsed)
        return;
    htclear(&keymap);
}

void FE_Key_Read()
{
    FILE *f = fopen(KEYMAP_FILE, "r");
    if (!f)
        error("Could not open keymap file");

    // Read line by line
    char line[128];
    while (fgets(line, 128, f)) {
        // Split line into keycode and key
        char *key = strtok(line, "=");
        char *keycode = strtok(NULL, "=");
        // Add to keymap
        FE_Key_Assign(key, atoi(keycode));
    }

    fclose(f);
    info("Loaded keymap");
}

void FE_Key_Init()
{
    if (keymap_initialsed)
        return;

    keymap_initialsed = true;
    htinit(&keymap, DestroyKey);

    FE_Key_Read();
}