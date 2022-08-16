#include "../core/include/include.h"
#include "include/grid.h"
#include "include/container.h"

bool FE_UI_GridFull(FE_UI_Grid *grid)
{
    if (!grid) {
        warn("NULL grid passed");
        return -1;
    }
    if (grid->tile_count + 1 == grid->tiles_set) return true;
    return false;
}

int FE_UI_GetFreeTile(FE_UI_Grid *grid)
{
    if (!grid) {
        warn("NULL grid passed");
        return -1;
    }
    for (size_t i = 0; i < grid->tile_count; i++) {
        if (grid->tiles[i].empty) return (int)i;
    }
    return -1;
}

void FE_UI_UpdateTile(FE_UI_Grid *grid, FE_Texture *texture, size_t index)
{
    if (!grid) {
        warn("NULL passed to grid");
        return;
    }
    if (index > grid->tile_count) {
        info("Tile index greater than tile count");
        return;
    }
    FE_UI_Tile *tile = &grid->tiles[index];
    if (tile->empty) {
        tile->empty = false;
        grid->tiles_set++;
    }

    tile->texture = texture;
    grid->buffer_dirty = true;
}

void FE_UI_UpdateGridBorder(FE_UI_Grid *grid, SDL_Color color, int thickness)
{
    if (!grid) {
        warn("NULL passed to grid");
        return;
    }
    grid->color = color;
    grid->border_width = thickness;
    grid->buffer_dirty = true;
}

void FE_UI_AddTile(FE_UI_Grid *grid, FE_Texture *texture)
{
    if (FE_UI_GridFull(grid)) {
        info("Unable to add tile - Grid is full");
        return;
    }
    FE_UI_UpdateTile(grid, texture, FE_UI_GetFreeTile(grid));
    grid->buffer_dirty = true;
}

void FE_UI_ClearTile(FE_UI_Grid *grid, size_t index)
{
    if (!grid || index > grid->tile_count) return;
    FE_UI_Tile *t = &grid->tiles[index];
    t->empty = true;
    grid->tiles_set--;
    grid->buffer_dirty = true;
}

static void CheckHover(FE_UI_Grid *grid)
{
    if (!grid) return;
    int mouse_x, mouse_y;
    SDL_GetMouseState(&mouse_x, &mouse_y);

    if (grid->r.x < mouse_x && grid->r.x + grid->r.w > mouse_x &&
        grid->r.y < mouse_y && grid->r.y + grid->r.h > mouse_y) {
        // Calculate which tile the mouse is hovering over
        grid->hovered.x = (mouse_x - grid->r.x) / grid->tiles[0].r.w;
        grid->hovered.y = (mouse_y - grid->r.y) / grid->tiles[0].r.h;
        if (grid->hovered.x > grid->cols -1) grid->hovered.x = grid->cols -1;
        if (grid->hovered.y > grid->rows -1) grid->hovered.y = grid->rows -1;
    } else {
        grid->hovered = VEC_NULL;
    }
}

void FE_UI_CheckGridHover()
{
    // Check all present game buttons first
    if (!FE_UI_ControlContainerLocked && !PresentGame->UIConfig.MBShown) {
        for (FE_List *l = PresentGame->UIConfig.ActiveElements->Grids; l; l = l->next)
            CheckHover(l->data);
    }

    // Check through container to see if any buttons are hovered
    for (FE_List *l = PresentGame->UIConfig.ActiveElements->Containers; l; l = l->next) {
        FE_UI_Container *c = l->data;
        for (size_t i = 0; i < c->children_count; i++) {
            if (c->children[i].type == FE_UI_GRID)
                CheckHover(c->children[i].element);
        }
    }
}

static void HandleCallback(FE_UI_Grid *grid)
{
    // Calculate the index of the tile that was clicked
    int index = grid->hovered.x + grid->hovered.y * grid->cols;
    if (grid->onclick)
        grid->onclick(grid->tiles[index]);
}

bool FE_UI_GridClick()
{
    // Check global
    if (!FE_UI_ControlContainerLocked) {
        for (FE_List *l = PresentGame->UIConfig.ActiveElements->Grids; l; l = l->next) {
            FE_UI_Grid *grid = l->data;
            if (!vec2_null(grid->hovered)) {
                HandleCallback(grid);
                return true;
            }
        }
    }

    // Check containers
    for (FE_List *l = PresentGame->UIConfig.ActiveElements->Containers; l; l = l->next) {
        FE_UI_Container *container = l->data;
        for (size_t i = 0; i < container->children_count; i++) {
            if (container->children[i].type == FE_UI_GRID) {
                FE_UI_Grid *grid = container->children[i].element;
                if (!vec2_null(grid->hovered)) {
                    HandleCallback(grid);
                    return true;
                }
            }
        }
    }

    return false;
}

void FE_UI_RenderGrid(FE_UI_Grid *grid)
{
    if (!grid) {
        warn("NULL grid passed");
        return;
    }

    if (grid->buffer_dirty) { // only redraw if buffer is dirty (grid has changed)
        
        SDL_Texture *prev_target = SDL_GetRenderTarget(PresentGame->Renderer);
        if (grid->buffer_texture) {
            SDL_DestroyTexture(grid->buffer_texture);
        }
        grid->buffer_texture = FE_CreateRenderTexture(grid->r.w, grid->r.h);
        SDL_SetRenderTarget(PresentGame->Renderer, grid->buffer_texture);
        SDL_SetTextureBlendMode(grid->buffer_texture, SDL_BLENDMODE_BLEND);

        // Render tiles
        for (size_t i = 0; i < grid->tile_count; i++) {
            FE_UI_Tile *t = &grid->tiles[i];
            if (t->empty) continue;
            SDL_RenderCopy(PresentGame->Renderer, t->texture->Texture, NULL, &t->r);
        }

        // Render outside border
        SDL_Rect border_r = {0, 0, grid->r.w, grid->r.h};
        FE_RenderBorder(grid->border_width, border_r, grid->color);

        Uint8 r, g, b, a;
        SDL_GetRenderDrawColor(PresentGame->Renderer, &r, &g, &b, &a);
        SDL_SetRenderDrawColor(PresentGame->Renderer, grid->color.r, grid->color.g, grid->color.b, grid->color.a);

        // Render Border for cols
        for (size_t i = 0; i < grid->cols; i++) {
            if (i == 0 || i == grid->cols) continue;
            SDL_Rect r = {((grid->tile_w + grid->border_width)* i), 0, grid->border_width, grid->r.h};
            SDL_RenderFillRect(PresentGame->Renderer, &r);
        }
        // Render Border for rows
        for (size_t i = 0; i < grid->rows; i++) {
            if (i == 0 || i == grid->rows) continue;
            SDL_Rect r = {0, ((grid->tile_h + grid->border_width)* i), grid->r.w, grid->border_width};
            SDL_RenderFillRect(PresentGame->Renderer, &r);
        }

        SDL_SetRenderTarget(PresentGame->Renderer, prev_target);
        SDL_SetRenderDrawColor(PresentGame->Renderer, r, g, b, a);
        grid->buffer_dirty = false;
    }

    SDL_RenderCopy(PresentGame->Renderer, grid->buffer_texture, NULL, &grid->r);

    // Render transulcent square behind hovered tile
    if (!vec2_null(grid->hovered)) {
        SDL_SetRenderDrawBlendMode(PresentGame->Renderer, SDL_BLENDMODE_BLEND);
        Uint8 prev_r, prev_g, prev_b, prev_a;
        SDL_GetRenderDrawColor(PresentGame->Renderer, &prev_r, &prev_g, &prev_b, &prev_a);
        SDL_SetRenderDrawColor(PresentGame->Renderer, grid->hover_color.r, grid->hover_color.g, grid->hover_color.b, grid->hover_color.a);

        SDL_Rect r = {grid->r.x + (grid->hovered.x * grid->tile_w) + ((grid->hovered.x +1 )* grid->border_width), grid->r.y + (grid->hovered.y * grid->tile_h) + ((grid->hovered.y +1 )* grid->border_width), grid->tile_w , grid->tile_h};
        SDL_RenderFillRect(PresentGame->Renderer, &r);

        SDL_SetRenderDrawColor(PresentGame->Renderer, prev_r, prev_g, prev_b, prev_a);
    }
    
    FE_UI_CheckGridHover();
}

void FE_UI_MoveGrid(FE_UI_Grid *grid, int x, int y)
{
    if (!grid) {
        warn("NULL grid passed to MoveGrid");
        return;
    }

    grid->buffer_dirty = true;
}

FE_UI_Grid *FE_UI_CreateGrid(int x, int y, int w, int h, size_t tile_count, int tile_w, int tile_h, void (*onclick)())
{
    // Create new grid
    FE_UI_Grid *grid = xmalloc(sizeof(FE_UI_Grid));
    grid->r = (SDL_Rect){x,y,w,h};
    grid->tile_count = tile_count;
    grid->onclick = onclick;
    grid->color = COLOR_WHITE;

    grid->hovered = VEC_NULL;
    grid->hover_color = (SDL_Color){255, 255, 255, 150};

    grid->border_width = 6;
    grid->buffer_dirty = true;
    grid->buffer_texture = 0;
    grid->tile_w = tile_w - grid->border_width;
    grid->tile_h = tile_h - grid->border_width;

    // Calculate rows and cols
    grid->cols = ceil(w / tile_w);
    grid->rows = 0;
    if (grid->cols > 0) grid->rows = ceil(h / tile_h);

    // Create empty tiles
    grid->tiles = xmalloc(sizeof(FE_UI_Tile) * tile_count);
    grid->tiles_set = 0;
    size_t cur_row = 0;
    size_t cur_col = 0;
    for (size_t idx = 0; idx < tile_count; idx++) {
        if (cur_row >= grid->rows) {
            warn("Grid is too small to fit all tiles");
            break;
        }
        grid->tiles[idx].empty = true;
        grid->tiles[idx].texture = 0;
        grid->tiles[idx].r = (SDL_Rect){(grid->tile_w * cur_col) + ((cur_col + 1) * grid->border_width), (grid->tile_h * cur_row) + ((cur_row + 1) * grid->border_width), grid->tile_w, grid->tile_h};
        if (++cur_col >= grid->cols) {
            cur_col = 0;
            cur_row++;
        }
    }
    return grid;
}

void FE_UI_DestroyGrid(FE_UI_Grid *grid, bool global)
{
    // Destroy tiles
    for (size_t i = 0; i < grid->tile_count; i++) {
        FE_UI_Tile *t = &grid->tiles[i];
        FE_DestroyResource(t->texture->path);
    }
    grid->tile_count = 0;
    free(grid->tiles);
    grid->tiles = 0;

    if (grid->buffer_texture) {
        SDL_DestroyTexture(grid->buffer_texture);
        grid->buffer_texture = 0;
    }

    if (global) {
        FE_List_Remove(&PresentGame->UIConfig.ActiveElements->Grids, grid);
        PresentGame->UIConfig.ActiveElements->Count--;
    }
    free(grid);
}