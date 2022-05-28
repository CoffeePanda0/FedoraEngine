#include "../core/include/include.h"
#include "include/include.h"
#include "../ext/tiny-json.h"

#define DIALOGUEPATH "game/dialogue/"
#define DIALOGUETEXT "dialogue.png"

static FE_UI_Label *title;
static FE_UI_Label *content;
static FE_UI_Object *box;

static size_t cur_index; // cur_index and max_index start at 1 so we use 0 for no text loaded
static size_t max_index; // last index of the text

static char **speakers; // array of speakers
static char **contents; // array containing all content

static size_t current_char = 1; // the current char that we have drawn
static bool drawing_text = false; // whether or not we are slowly drawing text

void FE_Dialogue_Update()
{
    static float last_time = 0;
    if (!drawing_text || !PresentGame->UIConfig.DialogueActive) {
        last_time = 0;
        return;
    }
    
    size_t len = mstrlen(contents[cur_index-1]);
    if (len == current_char) {
        drawing_text = false;
        return;
    }

    float dialoguespeed = ((100 - PresentGame->DialogueSpeed) * 2) / 1000.0f;
    if (last_time > dialoguespeed) {
        last_time = 0;
        char *newtext = mstrndup(contents[cur_index-1], ++current_char);
        FE_UI_UpdateLabel(content, newtext);
        free(newtext);
    }
    last_time += FE_DT;
}

int FE_Dialogue_Free() // frees both speakers and content array
{
    PresentGame->UIConfig.DialogueActive = false;

    if (max_index > 0) {
        if (box)
            FE_UI_DestroyObject(box, true);
        if (title)
            FE_UI_DestroyLabel(title, true);
        if (content)
            FE_UI_DestroyLabel(content, true);
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

int FE_Dialogue_Play() // plays current dialogue
{
    if (max_index == 0) {
        warn("No dialogue loaded (PlayDialogue)");
        return -1;
    }
    
    char *first = mstrndup(contents[cur_index-1], 1);

    if (!PresentGame->UIConfig.DialogueActive) { // if elements have not been made yet
        box = FE_UI_CreateObject(0, 0, PresentGame->WindowWidth, PresentGame->WindowHeight / 6, DIALOGUETEXT);
        title = FE_UI_CreateLabel(NULL, speakers[cur_index-1], rel_w(90), vec2(30, 10), COLOR_WHITE);
        content = FE_UI_CreateLabel(NULL, first, rel_w(90), vec2(30, 50), COLOR_WHITE);
        FE_UI_AddElement(FE_UI_OBJECT, box);
        FE_UI_AddElement(FE_UI_LABEL, title);
        FE_UI_AddElement(FE_UI_LABEL, content);
    } else {
        FE_UI_UpdateLabel(title, speakers[cur_index-1]);
        FE_UI_UpdateLabel(content, first);
    }

    free(first);
    current_char = 1;
    drawing_text = true;
    PresentGame->UIConfig.DialogueActive = true;

    return 1;
}

int FE_Dialogue_FromStr(char *speaker, char *content) // plays single line of dialogue at once. Also empties current dialogue!
{
    if (!speaker || !content) {
        warn("NullPTR Speaker or content! (PlayDialogueFromStr)");
        return -1;
    }
    if (mstrlen(speaker) == 0 || mstrlen(content) == 0) {
        warn("Speaker or content cannot be empty (PlayDialogueFromStr)");
        return -1;
    }

    // create UI elements
    speakers = xmalloc(sizeof(char*));
    speakers[0] = mstrdup(speaker);
    contents = xmalloc(sizeof(char*));
    contents[0] = mstrdup(content);
    
    cur_index = 1;
    max_index = 1;

    FE_Dialogue_Play();

    return 1;
}

int FE_Dialogue_FromFile(char *path)
{
    char *newpath = xmalloc(mstrlen(path) + mstrlen(DIALOGUEPATH) + 1); // adds full file path
    mstrcpy(newpath, DIALOGUEPATH);
    mstrcat(newpath, path);

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
                speakers[dialogue_count-1] = mstrdup(child->u.value);
                n++;
            } else { // if dialogue
                n = 0;
                contents = xrealloc(contents, sizeof(char*) * dialogue_count + 1);
                contents[dialogue_count-1] = mstrdup(child->u.value);
            }
        }
    }
    
    cur_index = 1;
    max_index = dialogue_count;

    info("Loaded dialogue script %s", path);
    fclose(f);
    free(fs);
    free(newpath);
    FE_Dialogue_Play();

    return 0;
}

int FE_Dialogue_Interact()
{
    // If text is slowly being typed, complete text first
    if (drawing_text) {
        drawing_text = false;
        FE_UI_UpdateLabel(content, contents[cur_index-1]);
        return 1;
    }

    if (cur_index == max_index) {
        FE_Dialogue_Free();
        return 1;
    }
    
    cur_index++;
    FE_Dialogue_Play();

    return 1;
}