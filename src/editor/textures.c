#include <SDL.h>
#include <SDL_image.h>
#include "../core/lib/bool.h"
#include "../core/lib/string.h"
#include "editor.h"

#define BG_PATH "game/map/backgrounds/"

#ifdef _WIN32
	#include "../ext/dirent.h" // for finding textures in dir
#else
	#include <dirent.h> // for finding textures in dir
#endif

static size_t bgcount;

size_t Editor_LoadBackgrounds()
{
	bgcount = 0;
	// Load backgrounds
	DIR *dir = 0;
	struct dirent *ent = 0;
	if ((dir = opendir(BG_PATH)) != NULL) {
		while ((ent = readdir(dir)) != NULL) {
			if (ent->d_type == DT_REG) {
				// validate file is a texure by trying to load it
				SDL_Surface *tmp;
				char *path = xmalloc(mstrlen(BG_PATH) + mstrlen(ent->d_name) + 1);
				mstrcpy(path, BG_PATH);
				mstrcat(path, ent->d_name);
				tmp = IMG_Load(path);
				if (tmp != NULL) {
					if (bgcount == 10) {
						info("Too many backgrounds in map directory, only loading 10");
						break;
					}
					// load to texture array
					if (bgcount == 0)
						editor_backgrounds = xmalloc(sizeof(FE_Texture*));
					else
						editor_backgrounds = xrealloc(editor_backgrounds, sizeof(FE_Texture*) * (bgcount + 1));
					editor_backgrounds[bgcount++] = FE_LoadResource(FE_RESOURCE_TYPE_TEXTURE, path);
				}
				SDL_FreeSurface(tmp);
				free(path);
			}
		}
		closedir(dir);
	}
	return bgcount;
}
