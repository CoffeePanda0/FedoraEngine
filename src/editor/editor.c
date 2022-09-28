#include <math.h>
#include <ctype.h>
#include "../ui/include/include.h"
#include "../ui/include/menu.h"
#include "../include/init.h"
#include "../core/include/include.h"
#include "../core/include/keymap.h"
#include "../core/include/file.h"
#include "../world/include/mapbg.h"
#include "../entity/include/prefab.h"
#include "editor.h"
#include "save.h"

typedef struct Editor_Save_Config {
    FE_UI_Textbox *name;
    FE_UI_Textbox *author;
    FE_UI_Textbox *gravity;
} Editor_Save_Config;

static bool Editor_Initialised = false;
static FE_LoadedMap *map = 0; // map for editing
static Editor_Save_Config *config = 0;
static GPU_Image *world = 0;

// Parallax
static FE_StrArr *parallaxes = 0;
static FE_UI_Container *parallax_container = 0;
static bool parallax_set = false;
static char *parallax_name = 0;

// Atlas
static FE_UI_Container *tile_container = 0;

// Coordinate system
static FE_UI_Label *coord_label = 0;
static bool render_grid = true;
static FE_Camera *camera = 0;

// Selected modes
typedef enum {
    TILE,
    PREFAB
} MODE;
static MODE mode = TILE;
static GPU_Image *thumbnail = 0;
static FE_Texture *spawntexture;
static int selected_tile = -1;
static char *selected_prefab = 0;
static int rotation = 0;

// for my drag queens
static bool drag_place = false;
static bool drag_delete = false;

// prefab
static FE_StrArr *prefabs = 0;
static FE_UI_Container *prefab_container = 0;
static size_t prefab_count = 0;
static FE_List *placed_prefabs = 0;

// light
static FE_UI_Container *light_container = 0;

/* Tiles */

static inline void Snap(int *x, int *y)
{
    // Snap to map tilesize
    *x = (*x / map->tilesize) * map->tilesize;
    *y = (*y / map->tilesize) * map->tilesize;
}

static inline int wc(int i, int s)
{
    return (i / camera->zoom) + s;
}

static void RotateTile(int x, int y)
{
	// snap x and y to a grid using tile size
	Snap(&x, &y);

	// check if tile already exists
	for (size_t i = 0; i < map->tilecount; i++) {
		if (map->tiles[i].position.x == x && map->tiles[i].position.y == y) {
			if (map->tiles[i].rotation == 0)
				rotation = 90;
			else if (map->tiles[i].rotation == 90)
				rotation = 180;
			else if (map->tiles[i].rotation == 180)
				rotation = 270;
			else if (map->tiles[i].rotation == 270)
				rotation = 0;
			map->tiles[i].rotation = rotation;
			return;
		}
	}
}

static void DeleteTile(int x, int y)
{
	// verify tile exists
	if (map->tilecount == 0)
		return;

	Snap(&x, &y);

	// find index of tile
	int index = -1;
	for (size_t i = 0; i < map->tilecount; i++) {
		if (map->tiles[i].position.x == x && map->tiles[i].position.y == y) {
			index = i;
			break;
		}
	}

	if (index == -1)
		return;

	// delete tile from map
	if (map->tilecount > 1) {
		FE_Map_Tile *temp = map->tiles;
		
		map->tiles = xmalloc(sizeof(FE_Map_Tile) * (--map->tilecount));
		mmemcpy(map->tiles, temp, sizeof(FE_Map_Tile) * index);
		mmemcpy(map->tiles + index, temp + index + 1, sizeof(FE_Map_Tile) * (map->tilecount - index));
		free(temp);
	} else {
		free(map->tiles);
		map->tiles = 0;
		map->tilecount = 0;
	}

}

static void SetSpawn(int x, int y)
{
	// snap x and y to a grid using tile size
	Snap(&x, &y);

	map->PlayerSpawn = vec(x, y);
	info("Editor: Spawn set to (%d, %d)", x, y);
}

static void SetEnd(int x, int y)
{
	Snap(&x, &y);

	map->EndFlag = vec(x, y);
	info("Editor: Spawn end flag to (%d, %d)", x, y);
}

static bool DeletePrefab(int x, int y)
{
    Snap(&x, &y);
    if (prefab_count == 0)
        return true;

    // Check if a prefab in that position already exists
    for (FE_List *l = placed_prefabs; l; l = l->next) {
		FE_Prefab *p = l->data;
        if ((p->last_location.x) == x && (p->last_location.y == y)) {
            FE_List_Remove(&placed_prefabs, p);
            FE_Prefab_Destroy(p);
            prefab_count--;
            return true;
        }
    }
    return false;
}

static void AddPrefab(int x, int y)
{
    if (selected_prefab == 0)
        return;

    Snap(&x, &y);

    // Avoid repeating
    static vec2 last_click = {-1, -1};
	static char *last_prefab = 0;
    if (last_click.x == x && last_click.y == y && last_prefab == selected_prefab)
        return;

    last_click = vec(x,y);
    last_prefab = selected_prefab;

    // Check if a prefab in that position already exists
    for (FE_List *l = placed_prefabs; l; l = l->next) {
		FE_Prefab *p = l->data;
        if (vec2_cmp(p->last_location, last_click))
            if (DeletePrefab(x, y)) break;
    }
    FE_Prefab *new = FE_Prefab_Create(selected_prefab, x, y);
    FE_List_Add(&placed_prefabs, new);
    prefab_count++;
}

static void AddTile(int x, int y)
{
    if (selected_tile == -1)
        return;

    // snap x and y to a grid using tile size
    Snap(&x, &y);

	static vec2 last_click = {-1, -1};
	static int last_texture = 0;

	// no point replacing same tile when mouse held
	if (last_click.x == x && last_click.y == y && last_texture == selected_tile)
		return;

	last_click = vec(x,y);
	last_texture = selected_tile;
    
	// check if tile in that location already exists, and it it does then deletes it
	for (size_t i = 0; i < map->tilecount; i++) {
		if (map->tiles[i].position.x == x && map->tiles[i].position.y == y) {
			DeleteTile(x,y);
		}
	}

	vec2 selectedtexture_position = FE_GetTexturePosition(map->atlas, (size_t)selected_tile);

	// Add tile to map
	if (map->tilecount == 0)
		map->tiles = xmalloc(sizeof(FE_Map_Tile));
	else
		map->tiles = xrealloc(map->tiles, sizeof(FE_Map_Tile) * (map->tilecount + 1));
	map->tiles[map->tilecount++] = (FE_Map_Tile){selectedtexture_position.x, selectedtexture_position.y, rotation, vec(x, y)};

}

static void Exit()
{
    FE_Editor_Destroy();
    FE_Menu_LoadMenu("Main");
}


/* UI Elements */

static void UpdateCoordLabel()
{
    // get current mouse position
    int x, y;
    SDL_GetMouseState(&x, &y);
    // convert to world coordinates
    x = (x / camera->zoom) + camera->x;
    y = (y / camera->zoom) + camera->y;
    Snap(&x, &y);
    
    // update label
    char buffer[64];
	sprintf(buffer, "X: %i    Y: %i    Zoom: %.2f", x, y, camera->zoom);
	FE_UI_UpdateLabel(coord_label, buffer);
}

static void SelectTile(int index)
{    
    mode = TILE;

    // Generate texture with border to save render time
    if (selected_tile == index) return;
    if (thumbnail) GPU_FreeImage(thumbnail);

    GPU_Image *t = FE_TextureFromAtlas(map->atlas, (size_t)index);
    thumbnail = GPU_CreateImage(72, 72, GPU_FORMAT_RGBA);

    GPU_Target *target = GPU_LoadTarget(thumbnail);
    FE_RenderBorder(target, 4, (GPU_Rect){0, 0, 72, 72}, COLOR_WHITE);
    GPU_Rect r = {4,4,64,64};
    GPU_BlitRect(t, NULL, target, &r);
    GPU_FreeTarget(target);
    GPU_FreeImage(t);

    FE_UI_DestroyContainer(tile_container, true, true);

    selected_tile = index;
    selected_prefab = 0;
}

static void SelectParallax(char *name)
{
    FE_Parallax_Load(name);
    parallax_set = true;
    parallax_name = name;
    FE_UI_DestroyContainer(parallax_container, true, true);
}

static void SelectPrefab(char *name)
{
    // Generate texture with border to save render time
    if (selected_prefab == name) return;
    if (thumbnail) GPU_FreeImage(thumbnail);

    GPU_Image *t = FE_Prefab_Thumbnail(name);
    thumbnail = GPU_CreateImage(72, 72, GPU_FORMAT_RGBA);

    GPU_Target *target = GPU_LoadTarget(thumbnail);
    FE_RenderBorder(target, 4, (GPU_Rect){0, 0, 72, 72}, COLOR_WHITE);
    GPU_Rect r = {4,4,64,64};
    GPU_BlitRect(t, NULL, target, &r);
    
    GPU_FreeTarget(target);
    GPU_FreeImage(t);

    mode = PREFAB;
    selected_tile = -1;
    selected_prefab = name;
    FE_UI_DestroyContainer(prefab_container, true, true);
}

static FE_Map_Prefab *Prefab_Convert()
{
    // converts the rendered FE_Prefab to the FE_Map_Prefab for exporting
    FE_Map_Prefab *pf = xmalloc(sizeof(FE_Map_Prefab) * prefab_count);

    size_t idx = 0;
    for (FE_List *l = placed_prefabs; l; l = l->next) {
		FE_Prefab *p = l->data;
        pf[idx].name = p->name;
        pf[idx].x = p->last_location.x;
        pf[idx++].y = p->last_location.y;
    }
    return pf;
}

static void Save_Action(Editor_Save_Config *config)
{
    /* Validating */
    char *name = FE_UI_GetTextboxContent(config->name);
    char *author = FE_UI_GetTextboxContent(config->author);
    char *gravity = FE_UI_GetTextboxContent(config->gravity);

    if (!name || !author || !gravity || mstrlen(name) == 0 || mstrlen(author) == 0 || mstrlen(gravity) == 0) {
        FE_Messagebox_Show("Error", "Please fill in all fields", MESSAGEBOX_TEXT);
        info("Failed to save map: fields not filled in");
        return;
    }

    // Validate gravity
    float gravity_f = 0;
    if(sscanf(gravity, "%f", &gravity_f) != 1) {
        FE_Messagebox_Show("Error", "Invalid gravity value", MESSAGEBOX_TEXT);
        warn("Failed to save map: invalid gravity");
        return;
    }

    FE_Map save = {
        .name = name,
        .author = author,
        .gravity = gravity_f,
        .atlaspath = map->atlas->path,
        .atlas_tilesize = map->atlas->texturesize,
        .static_bg = !parallax_set,
        .bg_texturepath = "game/map/backgrounds/bg.jpg",
        .parallax = parallax_name,
        .ambientlight = map->ambientlight,
        .tilecount = map->tilecount,
        .tilesize = map->tilesize,
        .tiles = map->tiles,
        .prefabcount = prefab_count,
        .prefabs = Prefab_Convert(),
        .PlayerSpawn = map->PlayerSpawn,
        .EndFlag = map->EndFlag
    };

    Editor_CallSave(&save);
    free(save.prefabs);
}

void UI_Save()
{
    FE_UI_Container *c = FE_UI_CreateContainer(midx(500), midy(400), 500, 400, "Save Map", true);

    FE_UI_AddChild(c, FE_UI_LABEL, FE_UI_CreateLabel(0, "Map Name", 256, 0, 0, COLOR_WHITE), FE_LOCATION_CENTRE);
    FE_UI_Textbox *name = FE_UI_CreateTextbox(0,0,256, map->name);
    FE_UI_AddChild(c, FE_UI_TEXTBOX, name, FE_LOCATION_CENTRE);

    FE_UI_AddChild(c, FE_UI_LABEL, FE_UI_CreateLabel(0, "Map Creator", 256, 0, 0, COLOR_WHITE), FE_LOCATION_CENTRE);
    FE_UI_Textbox *author = FE_UI_CreateTextbox(0,0,256, map->author);
    FE_UI_AddChild(c, FE_UI_TEXTBOX, author, FE_LOCATION_CENTRE);

    FE_UI_AddChild(c, FE_UI_LABEL, FE_UI_CreateLabel(0, "Map Gravity", 256, 0, 0, COLOR_WHITE), FE_LOCATION_CENTRE);

    // read gravity from map
    char buffer[32];
    sprintf(buffer, "%.1f", map->gravity);

    FE_UI_Textbox *gravity = FE_UI_CreateTextbox(0,0,128,buffer);
    FE_UI_AddChild(c, FE_UI_TEXTBOX, gravity, FE_LOCATION_CENTRE);
    
    config = xmalloc(sizeof(Editor_Save_Config));
    config->author = author; config->name = name; config->gravity = gravity; 

    FE_UI_AddContainerSpacer(c, 64);
    FE_UI_AddChild(c, FE_UI_BUTTON, FE_UI_CreateButton("Save", 0,0, BUTTON_LARGE, &Save_Action, config), FE_LOCATION_CENTRE);
   
    FE_UI_AddContainerClose(c);
    FE_UI_AddElement(FE_UI_CONTAINER, c);
}

static void UI_Prefab_Selector()
{
    if (!prefabs)
        prefabs = FE_File_GetFiles("game/prefabs/");

    int height = clamp(((int)prefabs->items * 64), 400, PresentGame->WindowHeight);
    prefab_container = FE_UI_CreateContainer(midx(400), midy(height), 400, height, "Select Prefab", true);
    FE_UI_AddContainerClose(prefab_container);
    
    // create buttons for each parallax
    for (size_t i = 0; i < prefabs->items; i++) {
        char *name = prefabs->data[i];
        FE_UI_Button *b = FE_UI_CreateButton(name, 128, i * 64, BUTTON_MEDIUM, &SelectPrefab, prefabs->data[i]);
        FE_UI_AddChild(prefab_container, FE_UI_BUTTON, b, FE_LOCATION_CENTRE);
    }

    FE_UI_AddElement(FE_UI_CONTAINER, prefab_container);

}

static void UI_Parallax_Selector()
{
    if (!parallaxes)
        parallaxes = FE_Parallax_Count();

    // get list of parallaxes
    int height = clamp(((int)parallaxes->items * 64), 400, PresentGame->WindowHeight);
    parallax_container = FE_UI_CreateContainer(midx(400), midy(height), 400, height, "Select Parallax", true);
    FE_UI_AddContainerClose(parallax_container);
    
    // create buttons for each parallax
    for (size_t i = 0; i < parallaxes->items; i++) {
        char *name = parallaxes->data[i];
        FE_UI_Button *b = FE_UI_CreateButton(name, 128, i * 64, BUTTON_MEDIUM, &SelectParallax, parallaxes->data[i]);
        FE_UI_AddChild(parallax_container, FE_UI_BUTTON, b, FE_LOCATION_CENTRE);
    }

    FE_UI_AddElement(FE_UI_CONTAINER, parallax_container);
}

static void UI_Tile_Selector()
{
    const int grid_size = 640;

    // Get the amount of tiles
    size_t tilecount = (map->atlas->width / map->atlas->texturesize) * (map->atlas->height / map->atlas->texturesize);

    int height = 0;
    int width = 0;

    // Calculate rows and columns
    if ((int)tilecount * map->tilesize <= grid_size) {
        height = map->tilesize;
        width = tilecount * map->tilesize;
    } else {
        width = grid_size / map->tilesize;
        height = (tilecount / width) + ((tilecount % width) > 0 ? 1 : 0);
    }

    int container_width = width + 128;
    int container_height = height + 128;

    tile_container = FE_UI_CreateContainer(midx(container_width), midy(container_height), container_width, container_height, "Select Tile", true);
    FE_UI_AddContainerClose(tile_container);
    FE_UI_Grid *g = FE_UI_CreateGrid(midx(width), midy(height), width, height, (height / map->tilesize) * (width / map->tilesize), map->tilesize, map->tilesize, &SelectTile);

    // Fill the grid with tiles
    for (size_t i = 0; i < tilecount; i++) {
        GPU_Image *t = FE_TextureFromAtlas(map->atlas, i);
        FE_UI_AddTile(g, t);
    }

    FE_UI_UpdateGridBorder(g, COLOR_WHITE, 5);

    FE_UI_AddChild(tile_container, FE_UI_GRID, g, FE_LOCATION_CENTRE);
    FE_UI_AddElement(FE_UI_CONTAINER, tile_container);
}

static void SaveAmbientLight(FE_UI_Textbox *t)
{
    char *content = t->content;

    // check value only contains numerical values
    for (size_t i = 0; i < strlen(content); i++) {
        if (!isdigit(content[i])) {
            FE_Messagebox_Show("Invalid Value", "Ambient light must be a numerical value", MESSAGEBOX_TEXT);
            return;
        }
    }
    int value = atoi(content);    
    if (value < 0 || value > 255) {
        FE_Messagebox_Show("Invalid value", "The value must be between 0 and 255", MESSAGEBOX_TEXT);
        return;
    }
    map->ambientlight = value;
    PresentGame->MapConfig.AmbientLight = value;
    FE_UI_DestroyContainer(light_container, true, true);
    light_container = 0;
}

static void UI_AmbientLight()
{
    if (light_container) {
        light_container = 0;
        FE_UI_DestroyContainer(light_container, true, true);
    } else {
        light_container = FE_UI_CreateContainer(midx(400), midy(200), 400, 200, "Ambient Light", true);
        
        char buffer[32];
        sprintf(buffer, "%d", map->ambientlight);

        FE_UI_Textbox *t = FE_UI_CreateTextbox(0,0,256, buffer);
        FE_UI_Button *b = FE_UI_CreateButton("Save", 0, 0, BUTTON_LARGE, &SaveAmbientLight, t);
        FE_UI_AddChild(light_container, FE_UI_TEXTBOX, t, FE_LOCATION_CENTRE);
        FE_UI_AddContainerSpacer(light_container, 32);
        FE_UI_AddChild(light_container, FE_UI_BUTTON, b, FE_LOCATION_CENTRE);
        FE_UI_AddElement(FE_UI_CONTAINER, light_container);
    }
}

static void ShowHelp()
{
    FE_UI_Container *c = FE_UI_CreateContainer(midx(400), midy(400), 400, 400, "Help", true);
    FE_UI_AddChild(c, FE_UI_LABEL, FE_UI_CreateLabel(0, "WASD - Movement", 350, 0,0,COLOR_WHITE), FE_LOCATION_CENTRE);
    FE_UI_AddChild(c, FE_UI_LABEL, FE_UI_CreateLabel(0, "SHIFT - Increase Speed", 350, 0,0,COLOR_WHITE), FE_LOCATION_CENTRE);
    FE_UI_AddChild(c, FE_UI_LABEL, FE_UI_CreateLabel(0, "Z - Zoom in", 350, 0,0,COLOR_WHITE), FE_LOCATION_CENTRE);
    FE_UI_AddChild(c, FE_UI_LABEL, FE_UI_CreateLabel(0, "X - Zoom out", 350, 0,0,COLOR_WHITE), FE_LOCATION_CENTRE);
    FE_UI_AddChild(c, FE_UI_LABEL, FE_UI_CreateLabel(0, "L - Change light level", 350, 0,0,COLOR_WHITE), FE_LOCATION_CENTRE);
    FE_UI_AddContainerClose(c);
    FE_UI_AddElement(FE_UI_CONTAINER, c);
}

static void UI_ToolBar_Create()
{
    FE_UI_Object *o = FE_UI_CreateObject(0, 0, PresentGame->WindowWidth, 48, "toolbar.png");
    FE_UI_AddElement(FE_UI_OBJECT, o);

    FE_UI_AddElement(FE_UI_BUTTON, FE_UI_CreateButton("Exit", 10, 8, BUTTON_TINY, &Exit, 0));
    FE_UI_AddElement(FE_UI_BUTTON, FE_UI_CreateButton("Reset", 92, 8, BUTTON_TINY, &FE_Editor_Init, 0));
    FE_UI_AddElement(FE_UI_BUTTON, FE_UI_CreateButton("Save", 192, 8, BUTTON_TINY, &UI_Save, 0));
    
    coord_label = FE_UI_CreateLabel(PresentGame->UIConfig.UIFont, "X: 0000  Y: 0000", 10700, 300, 8, COLOR_WHITE);
    
    FE_UI_AddElement(FE_UI_LABEL, coord_label);
    FE_UI_AddElement(FE_UI_BUTTON, FE_UI_CreateButton("Select Prefab", PresentGame->WindowWidth - 585, 8, BUTTON_TINY, &UI_Prefab_Selector, 0));
    FE_UI_AddElement(FE_UI_BUTTON, FE_UI_CreateButton("Select Tile", PresentGame->WindowWidth - 400, 8, BUTTON_TINY, &UI_Tile_Selector, 0));
    FE_UI_AddElement(FE_UI_BUTTON, FE_UI_CreateButton("Select Parallax", PresentGame->WindowWidth - 250, 8, BUTTON_TINY, &UI_Parallax_Selector, 0));
    FE_UI_AddElement(FE_UI_BUTTON, FE_UI_CreateButton("?", PresentGame->WindowWidth - 48, 8, BUTTON_CLOSE, &ShowHelp, 0));
}

/****/

/* Loops */

void FE_Editor_EventHandler()
{
    const Uint8* keyboard_state = SDL_GetKeyboardState(NULL);
	SDL_PumpEvents();
	SDL_Event event;

	bool ui_handled = false;

    while (SDL_PollEvent(&event)) {
		if (FE_UI_HandleEvent(&event, keyboard_state)) {
			ui_handled = true;
            drag_delete = false;
            drag_place = false;
			break;
		}

        switch (event.type) {

            // single-press key events
            case SDL_KEYDOWN:
                if (event.key.repeat == 0) {
                    if (keyboard_state[FE_Key_Get("ZOOM IN")])
                        FE_Camera_SmoothZoom(camera, 0.25, 150);
                    else if (keyboard_state[FE_Key_Get("ZOOM OUT")])
                        FE_Camera_SmoothZoom(camera, -0.25, 150);
                    else if (keyboard_state[SDL_SCANCODE_G])
                        render_grid = !render_grid;
                    else if (keyboard_state[SDL_SCANCODE_Q]) {
					    int mouse_x, mouse_y;
					    SDL_GetMouseState(&mouse_x, &mouse_y);
                        mouse_x = wc(mouse_x, camera->x);
                        mouse_y = wc(mouse_y, camera->y);
                        SetSpawn(mouse_x, mouse_y);
                    }
                    else if (keyboard_state[SDL_SCANCODE_E]) {
                        int mouse_x, mouse_y;
                        SDL_GetMouseState(&mouse_x, &mouse_y);
                        mouse_x = wc(mouse_x, camera->x);
                        mouse_y = wc(mouse_y, camera->y);
                        SetEnd(mouse_x, mouse_y);
                    } else if (keyboard_state[SDL_SCANCODE_R]) {
                        int mouse_x, mouse_y;
                        SDL_GetMouseState(&mouse_x, &mouse_y);
                        RotateTile(mouse_x + camera->x, mouse_y + camera->y);
                    } else if (keyboard_state[SDL_SCANCODE_L]) {
                        UI_AmbientLight();
                    }
                }
            break;

            // click events for placing or deleting
            case SDL_MOUSEBUTTONDOWN:
                    if (event.button.button == SDL_BUTTON_LEFT) {
                        drag_place = true;
                        drag_delete = false;
                    }
                    else if (event.button.button == SDL_BUTTON_RIGHT) {
                        drag_delete = true;
                        drag_place = false;
                    }
                break;

                case SDL_MOUSEBUTTONUP:
                    if (event.button.button == SDL_BUTTON_LEFT) {
                        drag_place = false;
                    } else if (event.button.button == SDL_BUTTON_RIGHT) {
                        drag_delete = false;
                    }
                break;
        }
    }

    if (!ui_handled && !FE_UI_ControlContainerLocked) {
        int movspeed = 10;
        // make camera move faster if shift is held for longer
        static float hold_time = 0;
        if (keyboard_state[SDL_SCANCODE_LSHIFT]) {
            hold_time += FE_DT;
            movspeed = hold_time > 0.5 ? 30 : 20;
        } else {
            hold_time = 0;
        }

        if (keyboard_state[SDL_SCANCODE_W]) {
			FE_MoveCamera(0, -movspeed * FE_DT_MULTIPLIER, camera);
            UpdateCoordLabel();
        }
	    if (keyboard_state[SDL_SCANCODE_A]) {
			FE_MoveCamera(-movspeed * FE_DT_MULTIPLIER, 0, camera);
            UpdateCoordLabel();
        }
		if (keyboard_state[SDL_SCANCODE_S]) {
			FE_MoveCamera(0, movspeed * FE_DT_MULTIPLIER, camera);
            UpdateCoordLabel();
        }
		if (keyboard_state[SDL_SCANCODE_D]) {
			FE_MoveCamera(movspeed * FE_DT_MULTIPLIER, 0, camera);
            UpdateCoordLabel();
		}

        // Check if mouse has moved
        static int last_x = 0; static int last_y = 0;
        static float last_zoom = 1.0f;
        int mx, my;
        SDL_GetMouseState(&mx, &my);
        if (mx != last_x || my != last_y || camera->zoom != last_zoom) {
            last_x = mx;
            last_y = my;
            last_zoom = camera->zoom;
            UpdateCoordLabel();
        }

        /* Allow user to drag to place tiles */
		if (drag_place) {
            if (mode == TILE)
			    AddTile(wc(mx, camera->x), wc(my, camera->y));
            else
                AddPrefab(wc(mx, camera->x), wc(my, camera->y));
		} else if (drag_delete) {
            if (mode == TILE)
			    DeleteTile(wc(mx, camera->x), wc(my, camera->y));
            else
                DeletePrefab(wc(mx, camera->x), wc(my, camera->y));
		}
    }
}

static void RenderGrid()
{
    if (!render_grid) return;

	// render grid on map with camera - note this is inefficient and renders the whole screen regardless of zoom
	for (int i = 0; i < 20960 + PresentGame->WindowWidth; i += map->tilesize) {
        int x = (i - camera->x) * camera->zoom;
        int y = ((PresentGame->WindowWidth + 10240) - camera->y) * camera->zoom;
        GPU_Line(PresentGame->Screen, x, 0, x, y, COLOR_WHITE);
    }
	for (int i = 0; i < 20960 + PresentGame->WindowHeight; i += map->tilesize) {
        int y = (i - camera->y) * camera->zoom;
        int x = ((10240 + PresentGame->WindowWidth) - camera->x) * camera->zoom;
		GPU_Line(PresentGame->Screen, 0, y, x, y, COLOR_WHITE);
    }
}

static void RenderThumbnail()
{
    GPU_Rect r = {0, PresentGame->WindowHeight - 72, 72,72};
    GPU_BlitRectX(thumbnail, NULL, PresentGame->Screen, &r, rotation, 0, 0, GPU_FLIP_NONE);
}

void FE_Editor_Render()
{
    if (!Editor_Initialised || (PresentGame->GameState != GAME_STATE_EDITOR))
        error("Configuration error: Editor state not created");

    FE_UpdateCamera(camera);
    FE_Prefab_Update();

    GPU_Target *target = GPU_LoadTarget(world);

    GPU_Clear(PresentGame->Screen);

    if (parallax_set)
        FE_Map_RenderBG(camera, 0);

    FE_Particles_Render(camera);
    FE_Map_Render(map, camera);
    FE_GameObject_Render(camera);

    // render spawn
	if (!vec2_null(map->PlayerSpawn)) {
		GPU_Rect spawnrect = (GPU_Rect){map->PlayerSpawn.x, map->PlayerSpawn.y, map->tilesize, map->tilesize};
		FE_RenderCopy(target, camera, false, spawntexture, NULL, &spawnrect);
	}

    FE_Light_Render(camera, world);

    RenderGrid();
    RenderThumbnail();
    FE_UI_Render();
    
    GPU_Flip(PresentGame->Screen);
}

/****/

void FE_Editor_Init(char *path)
{
    FE_CleanAll();

    if (Editor_Initialised)
        FE_Editor_Destroy();

    PresentGame->GameState = GAME_STATE_EDITOR;

    UI_ToolBar_Create();
	world = GPU_CreateImage(PresentGame->WindowWidth, PresentGame->WindowHeight, GPU_FORMAT_RGBA);

    camera = FE_CreateCamera();
    camera->x = 0;
    camera->y = 0;
    camera->minzoom = 0.25;

    if (!path) {
        // Create a new map
        map = xmalloc(sizeof(FE_LoadedMap));
        map->atlas = FE_LoadResource(FE_RESOURCE_TYPE_ATLAS, "tileset.png");
        map->bg = 0;
        map->EndFlag = VEC_NULL;
        map->gravity = 70;
        map->name = 0;
        map->PlayerSpawn = VEC_NULL;
        map->static_bg = false;
        map->author = 0;
        map->tilecount = 0;
        map->tiles = 0;
        map->tilesize = 64;
        map->ambientlight = 255;
        PresentGame->MapConfig.AmbientLight = 255;
    } else {
        map = FE_Map_Load(path);
        if (!map)
            return;
        // set pre-loaded assets
        parallax_set = !map->static_bg;
        parallax_name = FE_Parallax_GetName();

        FE_List *pf = FE_Prefab_Get();
        for (FE_List *l = pf; l; l = l->next) {
            FE_Prefab *p = l->data;
            prefab_count++;
            FE_List_Add(&placed_prefabs, p);
        }
    }

    spawntexture = FE_LoadResource(FE_RESOURCE_TYPE_TEXTURE, "game/map/spawn.png");

    Editor_Initialised = true;
    info("Editor: Started Editor");
}

void FE_Editor_Destroy()
{
    // destroys resources and resets static values
    if (parallaxes)
        FE_StrArr_Destroy(parallaxes);
    if (prefabs)
        FE_StrArr_Destroy(prefabs);
    if (placed_prefabs)
        FE_List_Destroy(&placed_prefabs);
    if (config)
        free(config);
    if (world)
        GPU_FreeImage(world);
        
    world = 0;
    config = 0;
    placed_prefabs = 0;
    prefabs = 0;
    parallaxes = 0;
    parallax_name = 0;

    if (map)
        FE_Map_Close(map);
    map = 0;
    if (spawntexture)
        FE_DestroyResource(spawntexture->path);
    spawntexture = 0;
    if (thumbnail)
        GPU_FreeImage(thumbnail);
    thumbnail = 0;

    parallax_set = false;

    Editor_Initialised = false;
    render_grid = true;

    mode = TILE;

    selected_tile = -1;
    selected_prefab = 0;
    rotation = 0;
    prefab_count = 0;
}
