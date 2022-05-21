#include <string.h>
#include "../core/include/include.h"
#include "include/uiobject.h"
#include "include/label.h"
#include "../ext/tiny-json.h"

#define DIALOGUEPATH "game/dialogue/"
#define DIALOGUETEXT "dialogue.png"

static FE_Label *title;
static FE_Label *content;
static FE_UIObject *box;

static size_t cur_index; // cur_index and max_index start at 1 so we use 0 for no text loaded
static size_t max_index; // last index of the text

static char **speakers; // array of speakers
static char **contents; // array containing all content

static size_t current_char = 1; // the current char that we have drawn
static bool drawing_text = false; // whether or not we are slowly drawing text

void FE_Dialogue_Update()
{
    static float last_time = 0;
    if (!drawing_text || !PresentGame->DialogueActive) {
        last_time = 0;
        return;
    }
    
    size_t len = strlen(contents[cur_index-1]);
    if (len == current_char) {
        drawing_text = false;
        return;
    }

    float dialoguespeed = ((100 - PresentGame->DialogueSpeed) * 2) / 1000.0f;
    if (last_time > dialoguespeed) {
        last_time = 0;
        char *newtext = substr(contents[cur_index-1], ++current_char);
        FE_UpdateLabel(content, newtext);
        free(newtext);
    }
    last_time += FE_DT;
}

int FE_FreeDialogue() // frees both speakers and content array
{
    PresentGame->DialogueActive = false;

    if (max_index > 0) {
        if (box)
            FE_DestroyUIObject(box);
        if (title)
            FE_DestroyLabel(title);
        if (content)
            FE_DestroyLabel(content);
    }

    if (speakers && contents && max_index != 0) {
        for (size_t i = 0; i < max_index; i++) {
            free(speakers[i]);
            free(contents[i]);
        }
        free(speakers);
        free(contents);
        cur_index = 0, max_index = 0;
    }
    
    return 1;
}

int FE_PlayDialogue() // plays current dialogue
{
    if (max_index == 0) {
        warn("No dialogue loaded (PlayDialogue)");
        return -1;
    }
    
    char *first = substr(contents[cur_index-1], 1);

    if (!PresentGame->DialogueActive) { // if elements have not been made yet
        box = FE_CreateUIObject(0, 0, PresentGame->Window_width, PresentGame->Window_height / 6, DIALOGUETEXT);
        title = FE_CreateLabel(NULL, speakers[cur_index-1], FE_NewVector(30, 10), COLOR_WHITE);
        content = FE_CreateLabel(NULL, first, FE_NewVector(30, 50), COLOR_WHITE);
    } else {
        FE_UpdateLabel(title, speakers[cur_index-1]);
        FE_UpdateLabel(content, first);
    }

    free(first);
    current_char = 1;
    drawing_text = true;
    PresentGame->DialogueActive = true;

    return 1;
}

int FE_DialogueFromStr(char *speaker, char *content) // plays single line of dialogue at once. Also empties current dialogue!
{
    if (!speaker || !content) {
        warn("NullPTR Speaker or content! (PlayDialogueFromStr)");
        return -1;
    }
    if (strlen(speaker) == 0 || strlen(content) == 0) {
        warn("Speaker or content cannot be empty (PlayDialogueFromStr)");
        return -1;
    }

    // create UI elements
    speakers = xmalloc(sizeof(char*));
    speakers[0] = strdup(speaker);
    contents = xmalloc(sizeof(char*));
    contents[0] = strdup(content);
    
    cur_index = 1;
    max_index = 1;

    FE_PlayDialogue();

    return 1;
}

int FE_DialogueFromFile(char *path)
{
    char *newpath = xmalloc(strlen(path) + strlen(DIALOGUEPATH) + 1); // adds full file path
    strcpy(newpath, DIALOGUEPATH);
    strcat(newpath, path);

    FILE *f = fopen(newpath, "r");
    if (!f) {
        warn("Failed to open dialogue file: %s", newpath);
        return -1;
    }

    // see how big file is, then malloc for it, load into fs
    fseek(f, 0, SEEK_END);
    size_t s = ftell(f);
    char *fs = xmalloc(s);
    rewind(f);
    fread(fs, s, 1, f);

    // parse as json
    json_t mem[32];
    json_t const *json = json_create(fs, mem, sizeof mem / sizeof *mem);
    if (!json) {
        warn("Could not create json for dialogue %s", path);
        return -1;
    }

    json_t const *lines = json_getProperty(json, "lines");
    if (!lines || JSON_ARRAY != json_getType(lines)) {
        warn("Invalid dialogue format %s", path);
        return -1;
    }

    json_t const *line = json_getChild(lines);
    if (!line) {
        warn("Dialogue file empty %s", path);
        return -1;
    }

    // load lines into array of each line
    size_t dialogue_count = 0;
    json_t const *i_line;
    for (i_line = line; i_line != 0; i_line = json_getSibling(i_line)) {

        json_t const *child;
        int n = 0;

        for (child = json_getChild(i_line); child != 0; child = json_getSibling(child)) { // parse the individual line
            if (n == 0) { // if speaker
                dialogue_count++;
                speakers = xrealloc(speakers, sizeof(char*) * dialogue_count + 1);
                speakers[dialogue_count-1] = strdup(child->u.value);
                n++;
            } else { // if dialogue
                n = 0;
                contents = xrealloc(contents, sizeof(char*) * dialogue_count + 1);
                contents[dialogue_count-1] = strdup(child->u.value);
            }
        }
    }
    
    cur_index = 1;
    max_index = dialogue_count;

    info("Loaded dialogue script %s", path);
    fclose(f);
    free(fs);
    free(newpath);
    FE_PlayDialogue();

    return 0;
}

int FE_DialogueInteract()
{
    // If text is slowly being typed, complete text first
    if (drawing_text) {
        drawing_text = false;
        FE_UpdateLabel(content, contents[cur_index-1]);
        return 1;
    }

    if (cur_index == max_index) {
        FE_FreeDialogue();
        return 1;
    }
    
    cur_index++;
    FE_PlayDialogue();

    return 1;
}