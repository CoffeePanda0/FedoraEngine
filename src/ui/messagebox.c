#include "../core/include/include.h"
#include "include/messagebox.h"

#define MB_WIDTH 400
#define MB_HEIGHT 200

static FE_MessageBox *MB;

static void (*callback)();
static void *callback_data;

static char *input_content;

static FE_TextBox *previous_active;

static void HandleCallBack()
{
    FE_DestroyMessageBox();
    PresentGame->MBShown = false;
    if (callback)
        callback(callback_data);
}

int FE_ShowInputMessageBox(char *title, char *dialogue, void (*onclick)(), void *onclick_data)
{
    if (PresentGame->MBShown) {
        warn("Trying to create a message box when one is already active");
        return -1;
    }
    MB = xmalloc(sizeof(FE_MessageBox));
    MB->type = MESSAGEBOX_TEXTBOX;
    MB->texture = FE_LoadResource(FE_RESOURCE_TYPE_TEXTURE, "game/ui/messagebox.png");

    // calculate display rect
    MB->displayrect = (SDL_Rect){
        (PresentGame->Window_width / 2) - (MB_WIDTH / 2),
        (PresentGame->Window_height / 2) - (MB_HEIGHT / 2),
        MB_WIDTH,
        MB_HEIGHT
    };

    // title texture
    MB->title_texture = FE_TextureFromText(title, COLOR_WHITE);
    SDL_QueryTexture(MB->title_texture, NULL, NULL, &MB->title_rect.w, &MB->title_rect.h);
    MB->title_rect.x = (PresentGame->Window_width - MB->title_rect.w) / 2;
    MB->title_rect.y = MB->displayrect.y + 10;

    // dialogue texture (allow multi line)
    MB->dialogue_texture = FE_TextureFromText(dialogue, COLOR_WHITE);
    SDL_QueryTexture(MB->dialogue_texture, NULL, NULL, &MB->dialogue_rect.w, &MB->dialogue_rect.h);
    MB->dialogue_rect.x = ((PresentGame->Window_width - MB->dialogue_rect.w) / 2);
    MB->dialogue_rect.y = ((PresentGame->Window_height - MB->dialogue_rect.h) / 2) - 30;

    // create textbox
    MB->textbox = FE_CreateTextBox(MB->displayrect.x + 40, MB->displayrect.y + MB->displayrect.h - 100, MB->displayrect.w - 80, 30, "");

    if (onclick) {
        callback = onclick;
        callback_data = onclick_data;
    } else {
        callback = NULL;
        callback_data = NULL;
    }

    if (input_content)
        free(input_content);
    input_content = NULL;

    // create button
    MB->button = FE_CreateButton("OK", MB->displayrect.x + (MB->dialogue_rect.w / 2) + 64, MB->displayrect.y + MB->displayrect.h - 60, BUTTON_SMALL, &HandleCallBack, NULL);
    PresentGame->MBShown = true;

    // get the current active textbox so that we can set it back to being active once message is complete
    FE_TextBox *prev = FE_GetActiveTextBox();
    if (prev)
        previous_active = prev;
    FE_ForceActiveTextBox(MB->textbox);

    return 1;
}

int FE_ShowMessageBox(char *title, char *body)
{
    if (PresentGame->MBShown) {
        warn("Trying to create a message box when one is already active");
        return -1;
    }

    callback = NULL;
    callback_data = NULL;

    MB = xmalloc(sizeof(FE_MessageBox));
    MB->type = MESSAGEBOX_TEXT;
    MB->texture = FE_LoadResource(FE_RESOURCE_TYPE_TEXTURE, "game/ui/messagebox.png");

    // calculate display rect
    MB->displayrect = (SDL_Rect){
        (PresentGame->Window_width / 2) - (MB_WIDTH / 2),
        (PresentGame->Window_height / 2) - (MB_HEIGHT / 2),
        MB_WIDTH,
        MB_HEIGHT
    };

    // title texture
    MB->title_texture = FE_TextureFromText(title, COLOR_WHITE);
    SDL_QueryTexture(MB->title_texture, NULL, NULL, &MB->title_rect.w, &MB->title_rect.h);
    MB->title_rect.x = (PresentGame->Window_width - MB->title_rect.w) / 2;
    MB->title_rect.y = MB->displayrect.y + 10;

    // dialogue texture (allow multi line)
    MB->dialogue_texture = FE_TextureFromText(body, COLOR_WHITE);
    SDL_QueryTexture(MB->dialogue_texture, NULL, NULL, &MB->dialogue_rect.w, &MB->dialogue_rect.h);
    MB->dialogue_rect.x = ((PresentGame->Window_width - MB->dialogue_rect.w) / 2);
    MB->dialogue_rect.y = ((PresentGame->Window_height - MB->dialogue_rect.h) / 2) - 30;

    // create button
    MB->button = FE_CreateButton("OK", MB->displayrect.x + (MB->dialogue_rect.w / 2) + 16, MB->displayrect.y + MB->displayrect.h - 60, BUTTON_SMALL, &HandleCallBack, NULL);

    // center the button
    MB->button->r.x = (PresentGame->Window_width - MB->button->r.w) / 2;
    MB->button->label_rect.x = MB->button->r.x + (MB->button->r.w / 2) - (MB->button->label_rect.w / 2);
    PresentGame->MBShown = true;
    return 1;
}

char *FE_GetMessageBoxInput()
{
    if (input_content)
        return input_content;
    else
        return NULL;
}

void FE_RenderMessageBox()
{
    if (!PresentGame->MBShown)
        return;

    SDL_RenderCopy(PresentGame->Renderer, MB->texture->Texture, NULL, &MB->displayrect);
    SDL_RenderCopy(PresentGame->Renderer, MB->title_texture, NULL, &MB->title_rect);
    SDL_RenderCopy(PresentGame->Renderer, MB->dialogue_texture, NULL, &MB->dialogue_rect);
    
    if (MB->type == MESSAGEBOX_TEXTBOX) { // render again to bring textbox to front
        SDL_RenderCopy(PresentGame->Renderer, MB->textbox->text, NULL, &MB->textbox->r); 
        SDL_RenderCopy(PresentGame->Renderer, MB->textbox->label->texture, NULL, &MB->textbox->label->r); 
    }

    // render button again
    if (MB->button->hover) {
            SDL_RenderCopy(PresentGame->Renderer, MB->button->hover_text, NULL, &MB->button->r);
    } else {
        SDL_RenderCopy(PresentGame->Renderer, MB->button->text, NULL, &MB->button->r);
    }
     SDL_RenderCopy(PresentGame->Renderer, MB->button->label, NULL, &MB->button->label_rect);
}   

void FE_DestroyMessageBox()
{
    if (!PresentGame->MBShown)
        return;
    if (MB->type == MESSAGEBOX_TEXTBOX) {
        if (previous_active)
            FE_ForceActiveTextBox(previous_active);
        input_content = mstrdup(MB->textbox->content);
        FE_DestroyTextBox(MB->textbox);
    }
    FE_DestroyButton(MB->button);
    FE_DestroyResource(MB->texture->path);
    SDL_DestroyTexture(MB->title_texture);
    SDL_DestroyTexture(MB->dialogue_texture);
    free(MB);
    MB = 0;
    PresentGame->MBShown = false;
    previous_active = NULL;
    
}