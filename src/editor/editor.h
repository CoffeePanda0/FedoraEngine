#ifndef _EDITOR_H_
#define _EDITOR_H_


/** Initialises the editor
 * \param path The path to the map to edit (pass NULL to create a new map)
 */
void FE_Editor_Init(char *path);


/* Renders the editor */
void FE_Editor_Render();


/* Editor Event Handler */
void FE_Editor_EventHandler();


/* Closes the editor cleanly */
void FE_Editor_Destroy();


#endif