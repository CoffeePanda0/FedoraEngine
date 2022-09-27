#ifndef _GRID_H
#define _GRID_H

typedef struct {
    bool empty;
    GPU_Rect r;
    GPU_Image *texture;
} FE_UI_Tile;

typedef struct {
    GPU_Rect r;

    size_t tile_count;
    size_t tiles_set;
    
    FE_UI_Tile *tiles;
    void (*onclick)();

    SDL_Color color;
    int border_width;
    int tile_w;
    int tile_h;

    GPU_Image *buffer_texture;
    bool buffer_dirty;

    vec2 hovered;
    SDL_Color hover_color;

    size_t cols;
    size_t rows;
} FE_UI_Grid;


/** Checks whether a grid is full or not
 * \param grid The grid to check
 * \return true if the grid is full, false otherwise
 */
bool FE_UI_GridFull(FE_UI_Grid *grid);

/** Gets the index of the first free tile in the grid
 * \param grid The grid to check
 * \return The index of the first free tile, or -1 if the grid is full
 */
int FE_UI_GetFreeTile(FE_UI_Grid *grid);


/** Updates a tile in the grid
 * \param grid The grid to update
 * \param texture The texture to set the tile to
 * \param index The index of the tile to update
 */
void FE_UI_UpdateTile(FE_UI_Grid *grid, GPU_Image *texture, size_t index);


/** Adds a tile to the first free slot in the grid
 * \param grid The grid to add to
 * \param texture The texture to set the tile to
 */
void FE_UI_AddTile(FE_UI_Grid *grid, GPU_Image *texture);


/** Clears a tile in the grid, setting it to empty
 * \param grid The grid to clear
 * \param index The index of the tile to clear
 */
void FE_UI_ClearTile(FE_UI_Grid *grid, size_t index);


/** Renders a grid
 * \param grid The grid to render
 */
void FE_UI_RenderGrid(FE_UI_Grid *grid);


/** Moves a grid to a new position
 * \param grid The grid to move
 * \param x The new x position
 * \param y The new y position
 */
void FE_UI_MoveGrid(FE_UI_Grid *grid, int x, int y);


/** Creates a new grid
 * \param x The x position of the grid
 * \param y The y position of the grid
 * \param w The width of the grid
 * \param h The height of the grid
 * \param tile_count The number of tiles in the grid
 * \param tile_w The width of each tile
 * \param tile_h The height of each tile
 * \param onclick The function to call when a tile is clicked
 * \return The new grid
 */
FE_UI_Grid *FE_UI_CreateGrid(int x, int y ,int w, int h, size_t tile_count, int tile_w, int tile_h, void (*onclick)());


/** Destroys a grid
 * \param grid The grid to destroy
 * \param global Whether to destroy the grid in the global list of grids
 */
void FE_UI_DestroyGrid(FE_UI_Grid *grid, bool global);


/** Updates the border of a grid
 * \param grid The grid to update
 * \param color The color of the border
 * \param thickness The thickness of the border
 */
void FE_UI_UpdateGridBorder(FE_UI_Grid *grid, SDL_Color color, int thickness);


/** Checks if a click lies within a grid, and if so calls the onclick function
 * \return true if the click was within the grid and event handled, false otherwise
 */
bool FE_UI_GridClick();


/* Checks if the user is hovering over a grid tile */
void FE_UI_CheckGridHover();


#endif