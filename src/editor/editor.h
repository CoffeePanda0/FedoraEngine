#ifndef H_EDITOR
#define H_EDITOR

#include "../core/include/utils.h"
#include "../world/include/camera.h"
#include "../world/include/map.h"
#include "../core/include/texture.h"
#include "../core/include/resourcemanager.h"
#include <stdlib.h>

/* Renders all map editor elements */
void FE_RenderEditor();

/* Initialises and displays the map editor */
void FE_StartEditor();

/* Handles all events for the map editor */
void FE_EventEditorHandler();

/* Destroys the Map editor and frees all used resources */
void FE_CleanEditor();

/* Editor functions - not to be used other than locally in editor.c */
void Editor_Save(FE_Map *mapsave);
size_t Editor_LoadBackgrounds();
size_t Editor_LoadTextures();
bool Editor_CallSave(FE_Map *save);

/* Editor variables - not to be used other than locally in editor.c */
extern FE_Texture **editor_backgrounds; // The background atlas (stores 10)

#endif