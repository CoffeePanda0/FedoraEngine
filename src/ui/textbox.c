#include "../include/game.h"

#define TEXTURE "game/ui/textbox.png"

bool FE_intext = false;
static FE_List *FE_Textboxes = 0;

FE_TextBox *FE_GetActiveTextBox()
{  
    if (!FE_Textboxes)
        return NULL;
    
    FE_TextBox *tmp = FE_Textboxes->data;
    if (tmp && tmp->active)
        return tmp;
    
    for (FE_List *t = FE_Textboxes; t && t->next; t = t->next) {
        tmp = t->data;
        if (tmp->active)
            return tmp;
    }
    return NULL;
}

int FE_ForceActiveTextBox(FE_TextBox *t)
{
    if (!t) {
        warn("Passing NULL textbox (FE_ForceActiveTextBox)");
        return -1;
    }

    FE_TextBox *current = FE_GetActiveTextBox();
    if (current)
        current->active = false;
    
    t->active = true;
    return 1;
}

bool FE_TextBoxClick(int x, int y)
{
    // check if user clicked in textbox rect
    for (FE_List *t = FE_Textboxes; t && t->next; t = t->next) {
        FE_TextBox *c = t->data;
        if (c->r.x < x && c->r.x + c->r.w > x &&
            c->r.y < y && c->r.y + c->r.h > y) {
            FE_ForceActiveTextBox(c);
            return true;
        }
    }
    return false;
}

static void FreeTextbox(FE_TextBox *t) // frees memory held by textbox
{
    if (!t) {
        warn("Passing NULL textbox (FreeTextbox)");
        return;
    }

    SDL_DestroyTexture(t->text);
    SDL_DestroyTexture(t->label->text);
    free(t->label);
    free(t->content);
}

FE_TextBox *FE_CreateTextBox(int x, int y, int w, int h, char *value) // Makes a new textbox and returns pointer to the node
{  
    // make box
    FE_TextBox *temp = xmalloc(sizeof(FE_TextBox));
    temp->text = FE_TextureFromFile(TEXTURE);
    temp->r = (SDL_Rect){x,y,w,h};
    
    // make box label
    SDL_Surface *text_surface = TTF_RenderText_Solid(Sans, value, COLOR_WHITE); 
    temp->label = xmalloc(sizeof(FE_Label));
    temp->label->text = SDL_CreateTextureFromSurface(renderer, text_surface);

    SDL_QueryTexture(temp->label->text, NULL, NULL, &temp->label->r.w, &temp->label->r.h); // Get w and h for rect
    SDL_FreeSurface(text_surface);
    temp->label->r.x = x;
    temp->label->r.y = y;


    temp->content = strdup(value);
    FE_List_Add(&FE_Textboxes, temp);

    temp->active = false;

    return temp;
}

char *FE_GetContent(FE_TextBox *t) // Returns the content of a textbox
{
    if (!t) {
        warn("Trying to get content of null textbox (FE_GetContent)");
        return NULL;
    }
    return t->content;
}

int FE_SetContent(FE_TextBox *t, char *value) // Sets the content of a textbox
{
    if (t->content)
        free(t->content);
    t->content = strdup(value);

    // redo label texture
    SDL_DestroyTexture(t->label->text);
    
    SDL_Surface *text_surface = TTF_RenderText_Solid(Sans, t->content, COLOR_WHITE); 
    t->label->text = SDL_CreateTextureFromSurface(renderer, text_surface);
    SDL_FreeSurface(text_surface);

    SDL_QueryTexture(t->label->text, NULL, NULL, &t->label->r.w, &t->label->r.h); // Get w and h for rect

    return 1;
}

int FE_UpdateTextBox(char c) // Adds or subtracts a character from the text of active box
{
    // get active textbox TODO
    FE_TextBox *t = FE_GetActiveTextBox();
    if (!t) {
        warn("No active textbox to update");
        return -1;
    }

    // update content with s
    if (c == '\b') { // if we are backspacing
        if (!t->content || strlen(t->content) == 0) {
            return 1;
        } else {
            t->content[strlen(t->content)-1] = '\0';
            t->content = xrealloc(t->content, strlen(t->content) +1);
        }
    } else { // for adding text
        if (t->content && strlen(t->content) > 0) { // if content already exists, realloc
            // check if textbox is full, and work out average char width to prevent overflow
            int w, h;
            SDL_QueryTexture(t->label->text, NULL, NULL, &w, &h);
            float avgchar = (w / strlen(t->content));
            if (w >= t->r.w - avgchar - 5) {
                return 0;
            }

            size_t len = strlen(t->content);
            t->content = xrealloc(t->content, len+2);
            t->content[len] = c;
            t->content[len+1] = '\0';
        } else { // else calloc and add new char
            t->content = calloc(2,1);
            t->content[0] = c;
        }
    }
    
    // redo label textures
    SDL_DestroyTexture(t->label->text);
    
    SDL_Surface *text_surface = TTF_RenderText_Solid(Sans, t->content, COLOR_WHITE); 
    t->label->text = SDL_CreateTextureFromSurface(renderer, text_surface);
    SDL_FreeSurface(text_surface);

    SDL_QueryTexture(t->label->text, NULL, NULL, &t->label->r.w, &t->label->r.h); // Get w and h for rect
    return 1;
}

int FE_CleanTextBoxes()
{
    if (!FE_Textboxes)
        return 0;

    /// destroys label and frees resources
    for (struct FE_List *l = FE_Textboxes; l; l = l->next) {
        FE_TextBox *tmp = l->data;
        FreeTextbox(tmp);
        free(tmp);
    }

    FE_List_Destroy(&FE_Textboxes);

    return 1;
}

int FE_DestroyTextBox(FE_TextBox *l)
{
    if (!l) {
        warn("Passing NULL (DestroyTextBox)");
        return -1;
    }

    FreeTextbox(l); // frees data from textbox
    FE_List_Remove(&FE_Textboxes, l); // remove from list
    free(l);
    
    return 1;
}

void FE_RenderTextBox()
{
    for (FE_List *l = FE_Textboxes; l; l = l->next) {
        FE_TextBox *t = l->data;
        SDL_RenderCopy(renderer, t->text, NULL, &t->r);
        SDL_RenderCopy(renderer, t->label->text, NULL, &t->label->r);
    }
}