#include "../core/include/include.h"
#include "include/include.h"

#define BUTTON_TEXTURE "game/ui/button.png"
#define ACTIVE_TEXTURE "game/ui/button_active.png"

#define BUTTON_SMALL_W 128
#define BUTTON_SMALL_H 32

FE_UI_Button *FE_UI_CreateButton(const char *text, int x, int y, FE_BUTTON_TYPE t, void (*onclick)(), void *onclick_data) // Creates a button with the given text and position
{
    FE_UI_Button *b = xmalloc(sizeof(FE_UI_Button));
    b->r.x = x;
    b->r.y = y;

    switch (t) {
        case BUTTON_CLOSE:
            b->r.w = 32;
            b->r.h = 32;
            break;
        case BUTTON_TINY:
            b->r.w = BUTTON_SMALL_W / 3;
            b->r.h = BUTTON_SMALL_H;
            break;
        case BUTTON_SMALL:
            b->r.w = BUTTON_SMALL_W;
            b->r.h = BUTTON_SMALL_H;
            break;
        case BUTTON_MEDIUM:
            b->r.w = BUTTON_SMALL_W * 1.5;
            b->r.h = BUTTON_SMALL_H * 1.5;
            break;
        case BUTTON_LARGE:
            b->r.w = BUTTON_SMALL_W * 2;
            b->r.h = BUTTON_SMALL_H * 2;
            break;
    }

    // make sure onclick is valid
    if (!onclick) {
        warn("Button onclick is null (CreateButton)");
        return NULL;
    }

    b->onclick = onclick;
    b->onclick_data = onclick_data;
    b->text = FE_TextureFromFile(BUTTON_TEXTURE);
    b->hover_text = FE_TextureFromFile(ACTIVE_TEXTURE);
    b->hover = false;

    if (!b) {
        warn("Failed to create button (CreateButton)");
        return 0;
    }

    // create label from text
    SDL_Surface *text_surface = FE_RenderText(PresentGame->font, text, (SDL_Color){106,113,111, 255});
    b->label = SDL_CreateTextureFromSurface(PresentGame->Renderer, text_surface);
    SDL_FreeSurface(text_surface);


    // calculate label rect from texture size
    SDL_QueryTexture(b->label, NULL, NULL, &b->label_rect.w, &b->label_rect.h); 

    // centre label inside button
    b->label_rect.y = y + (b->r.h - b->label_rect.h) / 2;
    b->label_rect.x = x + (b->r.w - b->label_rect.w) / 2;

    // resize button length if label is too long (and add padding)
    if (b->label_rect.w > b->r.w) {
        b->r.w = b->label_rect.w + 30;
        // recentre label
        b->label_rect.x = x + (b->r.w - b->label_rect.w) / 2;
    } else {
        if (t != BUTTON_CLOSE)
            b->r.w += 30;
        b->label_rect.x = x + (b->r.w - b->label_rect.w) / 2;
    }


    return b;
}

void FE_UI_DestroyButton(FE_UI_Button *b, bool global)
{
    if (!b) {
        warn("Can't destroy a null button (DestroyButton)");
        return;
    }

    SDL_DestroyTexture(b->text);
    SDL_DestroyTexture(b->hover_text);
    SDL_DestroyTexture(b->label);

    // Check if this label exists in the global list, if so remove it
    if (global) {
        int r = FE_List_Remove(&PresentGame->UIConfig.ActiveElements->Buttons, b);
        if (r == 1) PresentGame->UIConfig.ActiveElements->Count--;
    }

    free(b);
}

static void CheckHover(FE_UI_Button *b)
{
    if (!b) return;
    int mouse_x, mouse_y;
    SDL_GetMouseState(&mouse_x, &mouse_y);

    if (b->r.x < mouse_x && b->r.x + b->r.w > mouse_x &&
        b->r.y < mouse_y && b->r.y + b->r.h > mouse_y) {
        b->hover = true;
    } else {
        b->hover = false;
    }
}

void FE_UI_CheckButtonHover() // checks all buttons to see which (if any) are being hovered over
{
    // Check all present game buttons first
    if (!FE_UI_ControlContainerLocked) {
        for (FE_List *l = PresentGame->UIConfig.ActiveElements->Buttons; l; l = l->next)
            CheckHover(l->data);
    }

    // Check through container to see if any buttons are hovered
    for (FE_List *l = PresentGame->UIConfig.ActiveElements->Containers; l; l = l->next) {
        FE_UI_Container *c = l->data;
        for (size_t i = 0; i < c->children_count; i++) {
            if (c->children[i].type == FE_UI_BUTTON)
                CheckHover(c->children[i].element);
        }
    }
}

bool FE_UI_ButtonClick(int x, int y)
{
    // Check all present game buttons first
    if (!FE_UI_ControlContainerLocked) {
        for (FE_List *l = PresentGame->UIConfig.ActiveElements->Buttons; l; l = l->next) {
            FE_UI_Button *b = l->data;
            if (b->r.x < x && b->r.x + b->r.w > x &&
                b->r.y < y && b->r.y + b->r.h > y) {
                b->onclick(b->onclick_data);
                return true;
            }
        }
    }

    // Check through container to see if any buttons arehovered
    for (FE_List *l = PresentGame->UIConfig.ActiveElements->Containers; l; l = l->next) {
        FE_UI_Container *c = l->data;
        for (size_t i = 0; i < c->children_count; i++) {
            if (c->children[i].type == FE_UI_BUTTON) {
                FE_UI_Button *b = c->children[i].element;
                if (b->r.x < x && b->r.x + b->r.w > x &&
                    b->r.y < y && b->r.y + b->r.h > y) {
                    b->onclick(b->onclick_data);
                    return true;
                }
            }
        }
    }

    return false;
}

void FE_UI_MoveButton(FE_UI_Button *b, int x, int y)
{
    if (!b) {
        warn("Can't move a null button (MoveButton)");
        return;
    }

    b->r.x = x;
    b->r.y = y;

    // recentre label
    b->label_rect.x = x + (b->r.w - b->label_rect.w) / 2;
    b->label_rect.y = y + (b->r.h - b->label_rect.h) / 2;
}

void FE_UI_RenderButton(FE_UI_Button *b) // Renders all buttons and their labels
{
    if (b->hover)
        SDL_RenderCopy(PresentGame->Renderer, b->hover_text, NULL, &b->r);
    else
        SDL_RenderCopy(PresentGame->Renderer, b->text, NULL, &b->r);
    SDL_RenderCopy(PresentGame->Renderer, b->label, NULL, &b->label_rect);
}