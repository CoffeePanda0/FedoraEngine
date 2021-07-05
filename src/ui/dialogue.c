#include "../game.h"
#include <string.h>

static char ***dialoguearray;
static size_t maxindex;
static size_t currentnode;
struct GameObject TextBox;
struct TextObject TextName;
struct TextObject TextText;
static size_t lines = 0;

/*
DIALOGUE SCRIPT FORMAT:
speaker:text,

(comma on the end of each line, including the last line)
Example:
narrator:hello world,
narrator:triple pointers make me cry at night,
*/

void DialogueUpdate(int option, char* text)
{ // used to update existing text
	if (option == 0)
		UpdateText(&TextName, text, White);
	else if (option == 1)
		UpdateText(&TextText, text, White);
	else
		warn("Unkown Option for UI Text update. (Choose 0-1)");
}

void CreateDialogue(char *speaker, char *text)
{
	CreateObject(0, screen_height - screen_height / 3, screen_height / 3, screen_width, "../game/ui/dialogue.png", &TextBox, "DialogueBox");
	NewText(&TextName, Sans, speaker, White, 20, TextBox.objRect.y + 20);
	NewText(&TextText, Sans, text, White, 20, TextBox.objRect.y + 50);
	TextPaused = true;
}

void DialogueInteract(int option) // add options for how to interact with text. 
{
	if (TextPaused) {
		switch(option) {
			case 0:
				if (dialoguearray) {
					if (currentnode < maxindex -1) { // if there is still text we want to show
						currentnode++;
						UpdateText(&TextName, dialoguearray[currentnode][0], White);
						UpdateText(&TextText, dialoguearray[currentnode][1], White);
					} else { //destroy text fbox and unload dialogue script
						DestroyObject(&TextBox);
						FreeText(&TextText);
						FreeText(&TextName);
						TextPaused = false;
						for (size_t x = 0; x < lines; x++)
							free(dialoguearray[x]);

						free(dialoguearray);
						currentnode = 0;
						maxindex = 0;
					}
				}
				break;
			default:
				warn("Dialogue: No option exists");
				break;
		}
	}
}

int LoadDialogueScript(char *fp) // Loads dialogue to 2d array. Returns 0 on success, anything lower on failure.
{
	// Max index: The node that it will read up to e.g max index 2 would read element 0,1
	if ((currentnode + 1) < 1 || maxindex < 1) {
		warn("Dialogue: First and last index must be greater than one");
		return -1;
	}

	FILE *f = fopen(fp, "r"); 

	if (!f) {
		warn("Dialogue: Script %s does not exist", fp);
		return -1;
	}

	char cc = 0;
	
	while ((cc=fgetc(f)) != EOF) {		
		if (cc == ',')
			lines++;
	}

	dialoguearray = calloc(lines * sizeof(*dialoguearray), 1);
	for (size_t x = 0; x < lines; x++)
		dialoguearray[x] = calloc(lines * sizeof(**dialoguearray), 1);

	if (!dialoguearray)
		error("Could not allocate memory for dialogue script");

	rewind(f);
	char aa;
	char file[5000];
	int cur = 0;
	while ((aa = fgetc(f)) != EOF) {
		if (aa != '\n' && aa != '\0') {
			file[cur] = aa;
			cur++;
		}
	}

	char *pt;

	size_t t_line = 0;
	size_t t_index = 0;

    pt = strtok(file,",");

    while (pt != NULL) { // first separate file into strings

		char *tok;
		while ((tok = strseps(&pt,":")) != NULL) {

		dialoguearray[t_line][t_index] = strdup(tok); // load split screen to memory
        	if (t_index == 0)
				t_index++;
			else {
				if (t_line < lines -1) // when the segfault is sus 
					t_line++;
				t_index = 0;
			}

		}

        pt = strtok(NULL, ","); // move onto next object
    }
	
	fclose(f);

	if (maxindex -1 > t_line) {
		warn("Dialogue: Max index is too high!");
		for (size_t x = 0; x < lines; x++)
			free(dialoguearray[x]);
		free(dialoguearray);	
		return -1;
	}
	
	return 0;

} 

void PlayDialogue(int startindex, int max, char *fp) // defines which dialogue node to start and end on (CALL THIS FIRST)
{
	if (!dialoguearray) {
		maxindex = max;

		currentnode = startindex -1; // make it 1 indexed as it makes more sense
		if (LoadDialogueScript(fp) == 0)
			CreateDialogue(dialoguearray[currentnode][0], dialoguearray[currentnode][1]); // load startindex in to begin text
		else
			warn("Unable to load or parse dialogue script %s",fp);
	} else
		warn("Dialogue: Tried to load to dialogue files at once");
}
