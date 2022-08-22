
#include <SDL.h>
#include "../core/include/include.h"
#include "include/include.h"

static FE_Font *TitleFont;
static const int BorderWidth = 12;
static const int BorderHeight = 64;
static const int Padding = 10;

static FE_UI_Container *close_container = 0; // container with close button for key close

vec2 FE_GetCentre(SDL_Rect r, SDL_Rect container)
{
    return vec(container.x + (container.w - r.w) / 2, container.y + (container.h - r.h) / 2);
}


static void CloseContainer(void *data)
{
    FE_UI_Container *c = data;
    FE_UI_ControlContainerLocked = false;
    FE_UI_DestroyContainer(c, true, true);
}

bool FE_UI_CloseOpenContainer()
{
    if (!close_container)
        return false;
    CloseContainer(close_container);
    return true;
}

void FE_UI_AddContainerClose(FE_UI_Container *c)
{
    // Add close so it doesn't affect the padding of the children
    FE_UI_Element close_elem;
    close_elem.type = FE_UI_BUTTON;
    close_elem.element = FE_UI_CreateButton("X", c->body.x + c->body.w - 32, c->body.y, BUTTON_CLOSE, &CloseContainer, c);

    c->children = xrealloc(c->children, sizeof(FE_UI_Element) * (c->children_count + 1));
    c->children[c->children_count++] = close_elem;
    close_container = c;
}

FE_UI_Container *FE_UI_CreateContainer(int x, int y, int w, int h, char *title, bool lockcontrols)
{
    FE_UI_Container *c = xmalloc(sizeof(FE_UI_Container));

    TitleFont = PresentGame->font;

    c->children = 0;
    c->children_count = 0;

    c->body = (SDL_Rect){x,y,w,h};
    c->texture = FE_LoadResource(FE_RESOURCE_TYPE_TEXTURE, "game/ui/container_outer.png");
    c->title = FE_UI_CreateLabel(TitleFont, title, w, VEC_EMPTY, PresentGame->UIConfig.UIFontColor);
    c->title->r.x = FE_GetCentre(c->title->r, c->body).x;
    c->title->r.y = FE_GetCentre(c->title->r, (SDL_Rect){x, y, w - BorderWidth, BorderHeight}).y;

    c->inner_rect = (SDL_Rect){
        .x = c->body.x + BorderWidth,
        .y = c->body.y + BorderHeight,
        .w = w - (BorderWidth * 2),
        .h = h - (BorderHeight * 2)
    };

    c->last_child_bottom = c->inner_rect.y + Padding;

    FE_UI_ControlContainerLocked = lockcontrols;    

    return c;
}

static inline int calc_location(FE_UI_LOCATION location, SDL_Rect *r, SDL_Rect *container)
{
    if (location == FE_LOCATION_CENTRE)
        return FE_GetCentre(*r, *container).x;
    else if (location == FE_LOCATION_LEFT)
        return container->x + Padding;
    else if (location == FE_LOCATION_RIGHT)
        return container->x + container->w - r->w - Padding;
    else if (location == FE_LOCATION_NONE)
        return r->x;
    return 0;
}

void FE_UI_AddChild(FE_UI_Container *container, FE_UI_Type type, void *element, FE_UI_LOCATION location)
{
    if (!container || !element) {
        warn("Attempted to add null child or container (UI_AddChild)");
        return;
    }

    SDL_Rect *child_r = 0;

    /* Calculate the position of the child relative to the previous child */
    int newy = container->last_child_bottom + Padding;
    int newx = 0;
    
    switch (type) {
        case FE_UI_LABEL:
            child_r = &((FE_UI_Label *)element)->r;
            child_r->x = calc_location(location, child_r, &container->inner_rect);
            child_r->y = newy;
            break;
        case FE_UI_BUTTON:
            child_r = &((FE_UI_Button *)element)->r;
            newx = calc_location(location, child_r, &container->inner_rect);
            FE_UI_MoveButton(element, newx, newy);
            break;
        case FE_UI_OBJECT:
            child_r = &((FE_UI_Object *)element)->r;
            child_r->x = calc_location(location, child_r, &container->inner_rect);
            break;
        case FE_UI_CHECKBOX:
            child_r = &((FE_UI_Checkbox *)element)->r;
            child_r->x = calc_location(location, child_r, &container->inner_rect);
            break;
        case FE_UI_TEXTBOX:
            child_r = &((FE_UI_Textbox *)element)->r;
            newx = calc_location(location, child_r, &container->inner_rect);
            FE_UI_MoveTextbox(element, newx, newy);
            break;
        case FE_UI_GRID:
            child_r = &((FE_UI_Grid *)element)->r;
            newx = calc_location(location, child_r, &container->inner_rect);
            FE_UI_MoveGrid(element, newx, newy);
            break;
        default:
            warn("Unknown UI element type (UI_AddChild)");
            return;
    }

    // Expand the container to fit the child
    container->children = xrealloc(container->children, sizeof(FE_UI_Element) * (container->children_count + 1));
    container->children[container->children_count++] = (FE_UI_Element){type, element};

    /* Set the position of the child */

    /* Grow container to fit child */
    if (child_r->y + child_r->h > container->inner_rect.y + container->inner_rect.h - Padding) {
        container->inner_rect.h += child_r->h;
        container->body.h += child_r->h;
    }
    container->last_child_bottom = child_r->y + child_r->h;
}

void FE_UI_DestroyContainer(FE_UI_Container *c, bool free_children, bool global)
{
    if (!c) {
        warn("Attempted to destroy null container (UI_DestroyContainer)");
        return;
    }

    // Destroy the children
    if (free_children) {
        for (size_t i = 0; i < c->children_count; i++) {
            switch (c->children[i].type) {
                case FE_UI_LABEL:
                    FE_UI_DestroyLabel((FE_UI_Label *)c->children[i].element, false);
                    break;
                case FE_UI_BUTTON:
                    FE_UI_DestroyButton((FE_UI_Button *)c->children[i].element, false);
                    break;
                case FE_UI_OBJECT:
                    FE_UI_DestroyObject((FE_UI_Object *)c->children[i].element, false);
                    break;
                case FE_UI_CHECKBOX:
                    FE_UI_DestroyCheckbox((FE_UI_Checkbox *)c->children[i].element, false);
                    break;
                case FE_UI_TEXTBOX:
                    FE_UI_DestroyTextbox((FE_UI_Textbox *)c->children[i].element, false);
                    break;
                case FE_UI_GRID:
                    FE_UI_DestroyGrid((FE_UI_Grid *)c->children[i].element, false);
                    break;
                default:
                    warn("Unknown UI element type (UI_DestroyContainer)");
                    break;
            }
        }
    }

    // Free the children array
    free(c->children);

    // Free UI
    FE_UI_DestroyLabel(c->title, false);
    FE_DestroyResource(c->texture->path);

    // Check if this label exists in the global list, if so remove it
    if (global) {
        int r = FE_List_Remove(&PresentGame->UIConfig.ActiveElements->Containers, c);
        if (r == 1) PresentGame->UIConfig.ActiveElements->Count--;
    }

    // Free the container
    free(c);
    FE_UI_ControlContainerLocked = false;
    close_container = 0;
}

void FE_UI_AddContainerSpacer(FE_UI_Container *container, int h)
{
    if (h > container->inner_rect.y + container->inner_rect.h - Padding) {
        container->inner_rect.h += h;
        container->body.h += h;
    }
    container->last_child_bottom += h;
}

void FE_UI_RenderContainer(FE_UI_Container *c)
{
    if (!c) {
        warn("Attempted to render null container (UI_RenderContainer)");
        return;
    }

    // Render the container
    SDL_RenderCopy(PresentGame->Renderer, c->texture->Texture, NULL, &c->body);
    FE_UI_RenderLabel(c->title);

    // Render the children
    for (size_t i = 0; i < c->children_count; i++) {
        switch (c->children[i].type) {
            case FE_UI_LABEL:
                FE_UI_RenderLabel((FE_UI_Label *)c->children[i].element);
                break;
            case FE_UI_BUTTON:
                FE_UI_RenderButton((FE_UI_Button *)c->children[i].element);
                break;
            case FE_UI_CHECKBOX:
                FE_UI_RenderCheckbox((FE_UI_Checkbox *)c->children[i].element);
                break;
            case FE_UI_OBJECT:
                FE_UI_RenderObject((FE_UI_Object *)c->children[i].element);
                break;
            case FE_UI_TEXTBOX:
                FE_UI_RenderTextbox((FE_UI_Textbox *)c->children[i].element);
                break;
            case FE_UI_GRID:
                FE_UI_RenderGrid((FE_UI_Grid *)c->children[i].element);
                break;
            default:
                warn("Unknown UI element type (UI_RenderContainer)");
                break;
        }
    }
}