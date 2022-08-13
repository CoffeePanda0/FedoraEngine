#include "editor.h"
#include "../ui/include/include.h"
#include "../core/include/file.h"
#include "../core/lib/string.h"

#define MAP_DIRECTORY "game/map/maps/"

static int TileCompare(const void *a, const void *b)
{
    const FE_Map_Tile *tilea = a;
    const FE_Map_Tile *tileb = b;
    if (tilea->position.x < tileb->position.x)
        return -1;
    if (tilea->position.x > tileb->position.x)
        return 1;
    if (tilea->position.x < tileb->position.x)
        return -1;
    if (tilea->position.x > tileb->position.x)
        return 1;
    return 0;
}

static void SortTiles(FE_Map *save)
{
    FE_Map_Tile *tiles = save->tiles;
    size_t tilecount = save->tilecount;

    /* use a quick sort algorithm to sort the tiles by x position.
    This means that when checking for collision, we can just check the
    tiles closest to the player/object's X coordinate*/

    // sort the tiles by x position
    qsort(tiles, tilecount, sizeof(FE_Map_Tile), TileCompare);
}

bool Editor_CallSave(FE_Map *save) // used to call the save function as a callback and display dialogues 
{
    save->name = 0;
	save->name = mstrdup(FE_Messagebox_GetText());
	if (save->name[0] == '\0') {
		warn("Editor: Map name cannot be empty! Aborting save.");
        FE_Messagebox_Show("Warning", "Map name cannot be empty!", MESSAGEBOX_TEXT);
		goto fail;
	}
	if (save->tilecount == 0) {
		warn("Editor: Map must have at least one tile! Aborting save.");
	    FE_Messagebox_Show("Warning", "Map must have at least one tile!", MESSAGEBOX_TEXT);
		goto fail;
	}
	if (save->PlayerSpawn.x == -1 && save->PlayerSpawn.y == -1) {
		warn("Editor: Map must have a spawn! Aborting save.");
	    FE_Messagebox_Show("Warning", "Map must have a spawn! (press Q)", MESSAGEBOX_TEXT);
		goto fail;
	}
    if (save->EndFlag.x == -1 && save->EndFlag.y == -1) {
        warn("Editor: Map must have an end flag! Aborting save.");
        FE_Messagebox_Show("Warning", "Map must have an end flag! (press E)", MESSAGEBOX_TEXT);
        goto fail;
    }
    SortTiles(save);
	Editor_Save(save);

    if (save->name && mstrlen(save->name) > 0) {
        free(save->name);
    }

	return true;

fail:
	free(save->name);
	return false;
}

void Editor_Save(FE_Map *mapsave)
{
    // Create new map file and write as binary
    char *map_path = xcalloc(mstrlen(MAP_DIRECTORY) + mstrlen(mapsave->name) + 2, 1);
    mstrcpy(map_path, MAP_DIRECTORY);
    mstrcat(map_path, mapsave->name);

    FILE *f = fopen(map_path, "wb");
    if (f == NULL) {
        warn("Editor: Failed to create map file %s", mapsave->name);
        return;
    }

    // Write map name
    if (!(FE_File_WriteStr(f, mapsave->name))) goto err;

    // write gravity
    if (fwrite(&mapsave->gravity, sizeof(float), 1, f) != 1) goto err;

    // write atlas path
    if (!FE_File_WriteStr(f, mapsave->atlaspath)) goto err;

    // write atlas tile size
    if (fwrite(&mapsave->atlas_tilesize, sizeof(Uint16), 1, f) != 1) goto err;

    // Write background texture path
    if (!(FE_File_WriteStr(f, mapsave->bg_texturepath))) goto err;

    // Write tilecount
    if (fwrite(&mapsave->tilecount, sizeof(Uint16), 1, f) != 1) goto err;

    // Write tilesize
    if (fwrite(&mapsave->tilesize, sizeof(Uint16), 1, f) != 1) goto err;
    
    // Write tiles
    for (int i = 0; i < mapsave->tilecount; i++) {
        if (fwrite(&mapsave->tiles[i].texture_x, sizeof(Uint16), 1, f) != 1) goto err;
        if (fwrite(&mapsave->tiles[i].texture_y, sizeof(Uint16), 1, f) != 1) goto err;
        if (fwrite(&mapsave->tiles[i].rotation, sizeof(Uint16), 1, f) != 1) goto err;
        if (fwrite(&mapsave->tiles[i].position, sizeof(vec2), 1, f) != 1) goto err;
    }

    // Write player spawnq
    if (fwrite(&mapsave->PlayerSpawn, sizeof(vec2), 1, f) != 1) goto err;

    // Write end flag
    if (fwrite(&mapsave->EndFlag, sizeof(vec2), 1, f) != 1) goto err;
    
    info("Editor: Saved map to %s", mapsave->name);
    
    free(map_path);
    fclose(f);
    FE_Messagebox_Show("Map Editor", "Map saved successfully", MESSAGEBOX_TEXT);
    return;

err:
    warn("Editor: Error writing map to file %s", map_path);
    FE_Messagebox_Show("Map Editor", "Error: Failed to save map", MESSAGEBOX_TEXT);
    fclose(f);
    free(map_path);
}