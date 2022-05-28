#ifndef _CONSOLE_H
#define _CONSOLE_H


/* Initialises the console */
int FE_Console_Init();


/* Shows the console */
void FE_Console_Show();


/* Hides the console */
void FE_Console_Hide();


/* Destroys and uninitialises the console */
void FE_Console_Destroy();


/* Sets the value of the console input
*\ param in The value to set the console input to
*/
void FE_Console_UpdateInput(char *in);


/* Renders the console */
void FE_Console_Render();


/** Sets the output text of the console
 * \param text The text to be set
 */
void FE_Console_SetText(const char *text);

#endif
