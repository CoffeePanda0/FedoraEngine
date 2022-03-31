#ifndef H_DIALOGUE
#define H_DIALOGUE
// pragma balls lmao gotem


/* Displays a one-time dialogue on screen
*\speaker The speaker of the dialogue
*\content The text to display
*/
int FE_DialogueFromStr(char *speaker, char *content);


/* Frees the memory used by the loaded dialogue */
int FE_FreeDialogue();


/* Loads multi-line dialogue from a file 
*\path The path to the file
*/
int FE_DialogueFromFile(char *path);


/* Handles event to move to the next line of dialogue, or close the dialogue */
int FE_DialogueInteract();

#endif