#ifndef _EDITORSAVE_H
#define _EDITORSAVE_H

#include "../../world/include/map.h"

// only to be used internally by editor
bool Editor_CallSave(FE_Map *save); // used to call the save function as a callback and display dialogues 
void Editor_Save(FE_Map *mapsave);


#endif