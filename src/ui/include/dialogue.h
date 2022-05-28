#ifndef _DIALOGUE_H
#define _DIALOGUE_H
// pragma balls lmao gotem


/* Displays a one-time dialogue on screen
*\speaker The speaker of the dialogue
*\content The text to display
*/
int FE_Dialogue_FromStr(char *speaker, char *content);


/* Frees the memory used by the loaded dialogue */
int FE_Dialogue_Free();


/* Loads multi-line dialogue from a file 
*\path The path to the file
*/
int FE_Dialogue_FromFile(char *path);


/* Handles event to move to the next line of dialogue, or close the dialogue */
int FE_Dialogue_Interact();


/* Updates the text being displayed with time */
void FE_Dialogue_Update();


/** Sets the speed for the dialogue to be written at
 * \param speed The speed to write the dialogue at (1 being slowest, 100 fastest)
*/
void FE_Dialogue_SetSpeed(int speed);


#endif