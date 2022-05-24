#include "../core/include/include.h"
#include "include/button.h"

#define BUTTON_TEXTURE "game/ui/button.png"
#define ACTIVE_TEXTURE "game/ui/button_active.png"

#define BUTTON_SMALL_W 128
#define BUTTON_SMALL_H 32

static FE_List *FE_Buttons = 0;

static int mouse_prev_x;
static int mouse_prev_y;

FE_Button *FE_CreateButton(const char *text, int x, int y, enum FE_BUTTON_TYPE t, void (*onclick)(), void *onclick_data) // Creates a button with the given text and position
{
    FE_Button *b = xmalloc(sizeof(FE_Button));
    b->r.x = x;
    b->r.y = y;

    switch (t) {
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
    SDL_Surface *text_surface = FE_RenderText(PresentGame->font, text, COLOR_BLUE); 
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
    }

    FE_List_Add(&FE_Buttons, b);

    return b;
}

int FE_DestroyButton(FE_Button *b) // Destroys the button and removes from linked list
{
    if (!b) {
        warn("Can't destroy a null button (DestroyButton)");
        return -1;
    }

    SDL_DestroyTexture(b->text);
    SDL_DestroyTexture(b->hover_text);
    SDL_DestroyTexture(b->label);
    FE_List_Remove(&FE_Buttons, b);
    free(b);

    return 1;
}

void FE_CheckHover() // Checks if the mouse is hovering over a button
{
    int mouse_x, mouse_y;
    SDL_GetMouseState(&mouse_x, &mouse_y);

    // only bother checking if the mouse moved
    if (mouse_x == mouse_prev_x && mouse_y == mouse_prev_y) {
        return;
    } else {
        mouse_prev_x = mouse_x;
        mouse_prev_y = mouse_y;
    }


    for (FE_List *f = FE_Buttons; f; f = f->next) {
        struct FE_Button *t = f->data;
        if (t->r.x < mouse_x && t->r.x + t->r.w > mouse_x &&
            t->r.y < mouse_y && t->r.y + t->r.h > mouse_y) {
            t->hover = true;
        } else {
            t->hover = false;
        }
    }
}

bool FE_ButtonClick(int x, int y) // Checks if the mouse is clicking on a button. Returns true if button found
{
    for (struct FE_List *l = FE_Buttons; l; l = l->next) {
        struct FE_Button *t = l->data;
        if (t->r.x < x && t->r.x + t->r.w > x &&
            t->r.y < y && t->r.y + t->r.h > y) {
            t->onclick(t->onclick_data);
            return true;
        }
    }
    return false;
}

void FE_RenderButtons() // Renders all buttons and their labels
{
    if (!FE_Buttons)
        return;
    else
        FE_CheckHover();

    for (struct FE_List *l = FE_Buttons; l; l = l->next) {
        struct FE_Button *t = l->data;
        if (t->hover) {
            SDL_RenderCopy(PresentGame->Renderer, t->hover_text, NULL, &t->r);
        } else {
            SDL_RenderCopy(PresentGame->Renderer, t->text, NULL, &t->r);
        }
        SDL_RenderCopy(PresentGame->Renderer, t->label, NULL, &t->label_rect);
    }
}

int FE_CleanButtons() // Frees all buttons and removes from linked list
{
    // free resources inside button
    for (struct FE_List *l = FE_Buttons; l; l = l->next) {
        struct FE_Button *b = l->data;
        SDL_DestroyTexture(b->text);
        SDL_DestroyTexture(b->hover_text);
        SDL_DestroyTexture(b->label);
        free(b);
    }

    // destroy linked list
    FE_List_Destroy(&FE_Buttons);

    return 1;
}