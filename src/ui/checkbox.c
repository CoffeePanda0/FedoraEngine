#include "../core/include/include.h"
#include "include/checkbox.h"

#define CHECKBOX_SIZE 32

#define CHECKBOX_TEXTURE "game/ui/checkbox.png"
#define CHECKBOX_ACTIVE_TEXTURE "game/ui/checkbox_active.png"

static FE_List *FE_Checkboxes = 0;
static SDL_Texture *checkbox_texture = 0;
static SDL_Texture *checkbox_active_texture = 0;

FE_CheckBox *FE_CreateCheckbox(const char *label, int x, int y, bool checked, void (*onclick)(), void *onclick_data)
{
    // check if textures are loaded
    if (!checkbox_texture) {
        checkbox_texture = FE_TextureFromFile(CHECKBOX_TEXTURE);
        checkbox_active_texture = FE_TextureFromFile(CHECKBOX_ACTIVE_TEXTURE);
    }

    FE_CheckBox *c = xmalloc(sizeof(FE_CheckBox));
    c->r = (SDL_Rect){x, y, CHECKBOX_SIZE, CHECKBOX_SIZE};

    if (!onclick) {
        warn("Passing NULL onclick callback (CreateCheckbox)");
        return NULL;
    }

    c->checked = checked;
    c->onclick = onclick;
    c->onclick_data = onclick_data;

    // create label from text
    SDL_Surface *text_surface = FE_RenderText(PresentGame->font, label, COLOR_BLACK); 
    SDL_Texture *button_label = SDL_CreateTextureFromSurface(PresentGame->Renderer, text_surface);

    // calculate label rect from texture size
    SDL_Rect label_rect;
    SDL_QueryTexture(button_label, NULL, NULL, &label_rect.w, &label_rect.h); 
    label_rect.x = x + CHECKBOX_SIZE + 8;
    label_rect.y = y + (CHECKBOX_SIZE - label_rect.h) / 2;

    c->label_rect = label_rect;
    c->label = button_label;

    SDL_FreeSurface(text_surface);

    if (!button_label) {
        warn("Unable to create button label (CreateButton)");
        return NULL;
    }

    // add to list
    FE_List_Add(&FE_Checkboxes, c);

    return c;
}

bool FE_CheckboxClick(int x, int y) // Checks if the mouse is clicking on a checkbox. Returns true if button found
{
    for (struct FE_List *l = FE_Checkboxes; l; l = l->next) {
        struct FE_Checkbox *c = l->data;
        if (c->r.x < x && c->r.x + c->r.w > x &&
            c->r.y < y && c->r.y + c->r.h > y) {

            c->checked = !c->checked;
            c->onclick(c->onclick_data);
            return true;
        }
    }
    return false;
}

void FE_RenderCheckBoxes()
{
    for (struct FE_List *l = FE_Checkboxes; l; l = l->next) {
        struct FE_Checkbox *c = l->data;
        if (c->checked) {
            SDL_RenderCopy(PresentGame->Renderer, checkbox_active_texture, NULL, &c->r);
        } else {
            SDL_RenderCopy(PresentGame->Renderer, checkbox_texture, NULL, &c->r);
        }

        SDL_RenderCopy(PresentGame->Renderer, c->label, NULL, &c->label_rect); // render label
    }
}

int FE_DestroyCheckbox(FE_CheckBox *c) // Destroys the checkbox and removes from linked list
{
    if (!c) {
        warn("Passing NULL (DestroyCheckbox)");
        return -1;
    }

    SDL_DestroyTexture(c->label);
    int a = FE_List_Remove(&FE_Checkboxes, c);
    free(c);
    return a;
} 

int FE_CleanCheckBoxes() // Destroys all checkboxes and the loaded texture
{
    if (!FE_Checkboxes)
        return 1;

    // check if textures are loaded, if so free them
    if (checkbox_texture) {
        SDL_DestroyTexture(checkbox_texture);
        SDL_DestroyTexture(checkbox_active_texture);
        checkbox_texture = 0;
        checkbox_active_texture = 0;
    }

    /// destroys label and frees resources
    for (struct FE_List *l = FE_Checkboxes; l; l = l->next) {
        struct FE_Checkbox *tmp = l->data;
        SDL_DestroyTexture(tmp->label);
        free(tmp);
    }

    FE_List_Destroy(&FE_Checkboxes);
    

    return 1;
}