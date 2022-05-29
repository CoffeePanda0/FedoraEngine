#ifndef _KEYMAP_H
#define _KEYMAP_H


/** Gets the keycode for a given command.
 * \param input The input to get the keycode for.
 * \return The keycode for the command.
 */
int FE_Key_Get(const char *input);


/* Initialises the fedoraengine keymap system. */
void FE_Key_Init();


/* Cleans up the fedoraengine keymap system. */
void FE_Key_Clean();


/** Assigns a key to an input.
 * \param input The input to assign the key to.
 * \param keycode The keycode to assign.
 */
void FE_Key_Assign(char *input, uint8_t keycode);


/* Saves the current keymap to file. */
void FE_Key_Save();


#endif