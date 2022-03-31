#ifndef H_DIALOGUE
#define H_DIALOGUE
// pragma balls lmao gotem

int FE_DialogueFromStr(char *speaker, char *content);
int FE_FreeDialogue();
int FE_DialogueFromFile(char *path);
int FE_DialogueInteract();

#endif