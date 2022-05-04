#include "../include/game.h"

#define TEXTURE_PATH "game/map/textures/"
#define BG_PATH "game/map/backgrounds/"

#ifdef _WIN32
	#include "../ext/dirent.h" // for finding textures in dir
#else
	#include <dirent.h> // for finding textures in dir
#endif

static size_t texturecount;
static size_t bgcount;

size_t Editor_LoadTextures() // Load up to 10 textures for the editor from the map directory
{
	texturecount = 0;
	// Count amount of textures in folder, try opening to check if they are textures
	DIR *dir = 0;
	struct dirent *ent = 0;
	if ((dir = opendir(TEXTURE_PATH)) != NULL) {
		while ((ent = readdir(dir)) != NULL) {
			if (ent->d_type == DT_REG) {
				// validate file is a texure by trying to load it
				SDL_Surface *tmp;
				char *path = xmalloc(strlen(TEXTURE_PATH) + strlen(ent->d_name) + 1);
				strcpy(path, TEXTURE_PATH);
				strcat(path, ent->d_name);
				tmp = IMG_Load(path);
				if (tmp != NULL) {
					if (texturecount == 10) {
						info("Editor: Too many textures in map directory, only loading 10");
						break;
					}

					// load to texture array
					if (texturecount == 0 || !editor_textures)
						editor_textures = xmalloc(sizeof(FE_Texture*));
					else
						editor_textures = xrealloc(editor_textures, sizeof(FE_Texture*) * (texturecount + 1));
					editor_textures[texturecount++] = FE_LoadResource(FE_RESOURCE_TYPE_TEXTURE, path);

				}
				SDL_FreeSurface(tmp);
				free(path);
			}
		}
		closedir(dir);
	}
	
	info("Editor: Loaded %zu textures in %s", texturecount, TEXTURE_PATH);
	return texturecount;
}

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
				char *path = xmalloc(strlen(BG_PATH) + strlen(ent->d_name) + 1);
				strcpy(path, BG_PATH);
				strcat(path, ent->d_name);
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
