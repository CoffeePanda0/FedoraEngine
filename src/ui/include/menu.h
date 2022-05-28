#ifndef _MENU_H
#define _MENU_H

/* Renders the components only needed for the menu */
void FE_Menu_Render();

/* Handles events for the menu pages */
void FE_Menu_EventHandler();

/** Loads one of the preset menu pages
 * \param page The name of the page to load
 */
void FE_Menu_LoadMenu(const char *page);


#endif