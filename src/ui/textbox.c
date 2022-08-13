#include "../core/include/include.h"
#include "include/textbox.h"
#include "include/container.h"

#define TEXTURE "game/ui/textbox.png"
#define TEXTURE_ACTIVE "game/ui/textbox_active.png"

FE_UI_Textbox *FE_UI_GetActiveTextbox()
{  
    // Check if there is an active textbox in Textboxes
    for (FE_List *l = PresentGame->UIConfig.ActiveElements->Textboxes; l; l = l->next) {
        FE_UI_Textbox *t = (FE_UI_Textbox *)l->data;
        if (t->active)
            return t;
    }

    // Check if there is an active textbox in Containers
    for (FE_List *l = PresentGame->UIConfig.ActiveElements->Containers; l; l = l->next) {
        FE_UI_Container *c = l->data;
        for (size_t i = 0; i < c->children_count; i++) {
            if (c->children[i].type == FE_UI_TEXTBOX) {
                FE_UI_Textbox *t = (FE_UI_Textbox *)c->children[i].element;
                if (t->active)
                    return t;
            }
        }
    }
    return NULL;
}

void FE_UI_ForceActiveTextbox(FE_UI_Textbox *tb)
{

    FE_UI_Textbox *current = FE_UI_GetActiveTextbox();
    if (current)
        current->active = false;
    
    if (tb) {
        PresentGame->UIConfig.InText = true;
        tb->active = true;
    } else {
        PresentGame->UIConfig.InText = false;
    }
}

bool FE_UI_TextboxClick(int x, int y)
{
    // Check all present game textboxes first
    if (!FE_UI_ControlContainerLocked) {
        for (FE_List *l = PresentGame->UIConfig.ActiveElements->Textboxes; l; l = l->next) {
            FE_UI_Textbox *b = l->data;
            if (b->r.x < x && b->r.x + b->r.w > x &&
                b->r.y < y && b->r.y + b->r.h > y) {
                FE_UI_ForceActiveTextbox(b);
                return true;
            }
        }
    }

    // Check through container to see if any textboxes are clicked
    for (FE_List *l = PresentGame->UIConfig.ActiveElements->Containers; l; l = l->next) {
        FE_UI_Container *c = l->data;
        for (size_t i = 0; i < c->children_count; i++) {
            if (c->children[i].type == FE_UI_TEXTBOX) {
                FE_UI_Textbox *b = c->children[i].element;
                if (b->r.x < x && b->r.x + b->r.w > x &&
                    b->r.y < y && b->r.y + b->r.h > y) {
                    FE_UI_ForceActiveTextbox(b);
                    return true;
                }
            }
        }
    }

    return false;
}

FE_UI_Textbox *FE_UI_CreateTextbox(int x, int y, int w, int h, char *value) // Makes a new textbox and returns pointer to the node
{  
    // make box
    FE_UI_Textbox *temp = xmalloc(sizeof(FE_UI_Textbox));

    temp->text = FE_LoadResource(FE_RESOURCE_TYPE_TEXTURE, TEXTURE);
    temp->active_text = FE_LoadResource(FE_RESOURCE_TYPE_TEXTURE, TEXTURE_ACTIVE);

    temp->r = (SDL_Rect){x,y,w,h};
    
    // make box label
    temp->label = FE_UI_CreateLabel(PresentGame->UIConfig.UIFont, value, w, vec(x + 5,y + 5), COLOR_WHITE);

    temp->content = mstrdup(value);

    temp->active = false;
    temp->onenter = NULL;
    temp->data = NULL;
    return temp;
}

char *FE_UI_GetTextboxContent(FE_UI_Textbox *tb) // Returns the content of a textbox
{
    if (!tb) {
        warn("Trying to get content of null textbox (FE_GetContent)");
        return NULL;
    }
    return tb->content;
}

void FE_UI_AddTextboxCallback(FE_UI_Textbox *tb, void (*callback)(), void *data) // Adds a callback to a textbox
{
    if (!tb) {
        warn("Trying to add callback to null textbox (FE_AddCallback)");
        return;
    }
    tb->onenter = callback;
    tb->data = data;
}

void FE_UI_SetTextboxContent(FE_UI_Textbox *tb, char *value) // Sets the content of a textbox
{
    if (tb->content)
        free(tb->content);
    tb->content = mstrdup(value);

    // redo label texture
    SDL_DestroyTexture(tb->label->texture);
    
    SDL_Surface *text_surface = FE_RenderText(PresentGame->font, tb->content, COLOR_WHITE); 
    tb->label->texture = SDL_CreateTextureFromSurface(PresentGame->Renderer, text_surface);
    SDL_FreeSurface(text_surface);

    SDL_QueryTexture(tb->label->texture, NULL, NULL, &tb->label->r.w, &tb->label->r.h); // Get w and h for rect
}

void FE_UI_UpdateTextbox(char c) // Adds or subtracts a character from the text of active box
{
    FE_UI_Textbox *t = FE_UI_GetActiveTextbox();
    if (!t) {
        warn("No active textbox to update");
        return;
    }

    // Call callback if enter key is pressed
    if (c == '\r') {
        if (t->onenter)
            t->onenter(t->data);
        return;
    }
    
    // update content with s
    if (c == '\b') { // if we are backspacing
        if (!t->content || mstrlen(t->content) == 0) {
            return;
        } else {
            t->content[mstrlen(t->content)-1] = '\0';
            t->content = xrealloc(t->content, mstrlen(t->content) +1);
        }
    } else { // for adding text
        if (t->content && mstrlen(t->content) > 0) { // if content already exists, realloc
            // check if textbox is full, and work out average char width to prevent overflow
            int w, h;
            SDL_QueryTexture(t->label->texture, NULL, NULL, &w, &h);
            float avgchar = (w / mstrlen(t->content));
            if (w >= t->r.w - avgchar - 5)
                return;

            size_t len = mstrlen(t->content);
            t->content = xrealloc(t->content, len+2);
            t->content[len] = c;
            t->content[len+1] = '\0';
        } else { // else calloc and add new char
            if (t->content)
                free(t->content);
            t->content = xcalloc(2,1);
            t->content[0] = c;
        }
    }
    
    // redo label textures
    SDL_DestroyTexture(t->label->texture);
    
    SDL_Surface *text_surface = FE_RenderText(t->label->font, t->content, COLOR_WHITE); 
    t->label->texture = SDL_CreateTextureFromSurface(PresentGame->Renderer, text_surface);
    SDL_FreeSurface(text_surface);

    SDL_QueryTexture(t->label->texture, NULL, NULL, &t->label->r.w, &t->label->r.h); // Get w and h for rect
}

void FE_UI_DestroyTextbox(FE_UI_Textbox *tb, bool global)
{
    if (!tb) {
        warn("Passing NULL (DestroyTextBox)");
        return;
    }

    if (tb->content) free(tb->content);
    FE_UI_DestroyLabel(tb->label, false);

    FE_DestroyResource(tb->text->path);
    FE_DestroyResource(tb->active_text->path);

    if (global) {
        FE_List_Remove(&PresentGame->UIConfig.ActiveElements->Textboxes, tb);
        PresentGame->UIConfig.ActiveElements->Count--;
    }

    if (tb->active)
        PresentGame->UIConfig.InText = false;
        
    free(tb);
}

void FE_UI_MoveTextbox(FE_UI_Textbox *tb, int x, int y)
{
    if (!tb) {
        warn("Passing NULL (MoveTextBox)");
        return;
    }

    tb->r.x = x;
    tb->r.y = y;
    tb->label->r.x = x + 5;
    tb->label->r.y = y + 5;
}

void FE_UI_RenderTextbox(FE_UI_Textbox *tb)
{
    if (!tb) {
        warn("Passing NULL (RenderTextBox)");
        return;
    }

    if (tb->active) {
        SDL_RenderCopy(PresentGame->Renderer, tb->active_text->Texture, NULL, &tb->r);

        // draw a cursor at end of text
        if (mstrlen(tb->content) > 0) {
            static float time = 0;
            if (time > 0.5f) {
                time = -0.5f;
            } else if (time > 0.0f) {
                uint8_t r, g, b, a;
                SDL_GetRenderDrawColor(PresentGame->Renderer, &r, &g, &b, &a);
                SDL_SetRenderDrawColor(PresentGame->Renderer, 255, 255, 255, 255);
                SDL_RenderDrawLine(PresentGame->Renderer, tb->label->r.x + tb->label->r.w + 2, tb->label->r.y, tb->label->r.x + tb->label->r.w + 2, tb->label->r.y + tb->label->r.h);
                SDL_SetRenderDrawColor(PresentGame->Renderer, r, g, b, a);
            }
            time += FE_DT;
        }

    } else
        SDL_RenderCopy(PresentGame->Renderer, tb->text->Texture, NULL, &tb->r);
    FE_UI_RenderLabel(tb->label);
}