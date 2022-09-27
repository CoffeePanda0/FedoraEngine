#include "../core/include/include.h"
#include "include/checkbox.h"
#include "include/container.h"

#define CHECKBOX_SIZE 32

#define CHECKBOX_TEXTURE "game/ui/checkbox.png"
#define CHECKBOX_ACTIVE_TEXTURE "game/ui/checkbox_active.png"

static GPU_Image *checkbox_texture = 0;
static GPU_Image *checkbox_active_texture = 0;

FE_UI_Checkbox *FE_UI_CreateCheckbox(const char *label, int x, int y, bool checked, void (*onclick)(), void *onclick_data)
{
    // check if textures are loaded
    if (!checkbox_texture) {
        checkbox_texture = FE_TextureFromFile(CHECKBOX_TEXTURE);
        checkbox_active_texture = FE_TextureFromFile(CHECKBOX_ACTIVE_TEXTURE);
    }

    FE_UI_Checkbox *c = xmalloc(sizeof(FE_UI_Checkbox));
    c->r = (GPU_Rect){x, y, CHECKBOX_SIZE, CHECKBOX_SIZE};

    if (!onclick) {
        warn("Passing NULL onclick callback (CreateCheckbox)");
        return NULL;
    }

    c->checked = checked;
    c->onclick = onclick;
    c->onclick_data = onclick_data;

    // create label from text
    SDL_Surface *text_surface = FE_RenderText(PresentGame->font, label, COLOR_BLACK); 
    GPU_Image *button_label = GPU_CopyImageFromSurface(text_surface);

    // calculate label rect from texture size
    GPU_Rect label_rect;
    label_rect.w = button_label->w;
    label_rect.h = button_label->h;
    label_rect.x = x + CHECKBOX_SIZE + 8;
    label_rect.y = y + (CHECKBOX_SIZE - label_rect.h) / 2;

    c->label_rect = label_rect;
    c->label = button_label;

    SDL_FreeSurface(text_surface);

    if (!button_label) {
        warn("Unable to create button label (CreateButton)");
        return NULL;
    }

    return c;
}

bool FE_UI_CheckboxClick(int x, int y)
{
    // Check all present game buttons first
    if (!FE_UI_ControlContainerLocked) {
        for (FE_List *l = PresentGame->UIConfig.ActiveElements->Checkboxes; l; l = l->next) {
            FE_UI_Checkbox *c = l->data;
            if (c->r.x < x && c->r.x + c->r.w > x &&
            c->r.y < y && c->r.y + c->r.h > y) {
                c->checked = !c->checked;
                c->onclick(c->onclick_data);
                return true;
            }
        }
    }

    // Check through container to see if any buttons are hovered
    for (FE_List *l = PresentGame->UIConfig.ActiveElements->Containers; l; l = l->next) {
        FE_UI_Container *c = l->data;
        for (size_t i = 0; i < c->children_count; i++) {
            if (c->children[i].type == FE_UI_BUTTON) {
                FE_UI_Checkbox *b = c->children[i].element;
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

void FE_UI_RenderCheckbox(FE_UI_Checkbox *c)
{
    if (c->checked) {
        GPU_BlitRect(checkbox_active_texture, NULL, PresentGame->Screen, &c->r);
    } else {
        GPU_BlitRect(checkbox_texture, NULL, PresentGame->Screen, &c->r);
    }

    GPU_BlitRect(c->label, NULL, PresentGame->Screen, &c->label_rect);
}

void FE_UI_DestroyCheckbox(FE_UI_Checkbox *c, bool global)
{
    if (!c) {
        warn("Passing NULL (DestroyCheckbox)");
        return;
    }

    GPU_FreeImage(c->label);
    
    if (global) {
        FE_List_Remove(&PresentGame->UIConfig.ActiveElements->Checkboxes , c);
        PresentGame->UIConfig.ActiveElements->Count--;
    }

    free(c);
} 