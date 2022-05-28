#ifndef _UI_H
#define _UI_H

#include "../../core/include/linkedlist.h"
#include "../../core/include/fedoraengine.h"
#include "../../entity/include/player.h"

typedef enum {
    FE_UI_LABEL,
    FE_UI_BUTTON,
    FE_UI_CONTAINER,
    FE_UI_OBJECT,
    FE_UI_CHECKBOX,
    FE_UI_TEXTBOX
} FE_UI_Type;

typedef struct {
    FE_UI_Type type;
    void *element;
} FE_UI_Element;

typedef enum {
    FE_LOCATION_CENTRE,
    FE_LOCATION_LEFT,
    FE_LOCATION_RIGHT,
    FE_LOCATION_NONE
} FE_UI_LOCATION;

extern bool FE_UI_ControlContainerLocked;

/* Renders the active list of elements of any UI type to the screen */
void FE_UI_Render();


/** Adds an element to the active world to be rendered 
*\param type The type of element to be added
*\param element The element to be added
*/
void FE_UI_AddElement(FE_UI_Type type, void *element);


/** Removes an element from the active list to be rendered, freeing the resource
 * \param type The type of element to be removed
 * \param element The element to be removed
 */
void FE_UI_Destroy(FE_UI_Element *element);


/**
*\param Clears a list of UI elements, destroying all elements in the list
*\param Elements The list of elements to be cleared
*/
void FE_UI_ClearElements(FE_UIList *Elements);


/* Initialises the UI system */
void FE_UI_InitUI();


/* Updates the UI elements */
void FE_UI_Update();


/* Updates the DebugUI */
void FE_DebugUI_Update(FE_Player *player);


/* Handles a click event on screen, checking if it is on an element */
bool FE_UI_HandleClick(SDL_Event *event);


/** Handles events for moving / resizing windows and taking input keys used by FedoraEngine
 * \param event The SDL_Event to be handled
 * \param keyboard The SDL_KeyboardState to be handled
 * \returns true if the event was handled, false if not. If false, continue to handle the event in the game loop
*/
bool FE_UI_HandleEvent(SDL_Event *event, const Uint8* keyboard_state);


#endif