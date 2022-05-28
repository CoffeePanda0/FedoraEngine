#include "../core/include/include.h"
#include "include/messagebox.h"
#include "include/textbox.h"
#include "include/container.h"

#define MB_WIDTH 400
#define MB_HEIGHT 200

static FE_UI_Messagebox *MB;

static void (*callback)();
static void *callback_data;

static char *input_content;

static FE_UI_Textbox *previous_active;

static void HandleCallBack()
{
    FE_Messagebox_Destroy();
    PresentGame->MBShown = false;
    if (callback)
        callback(callback_data);
}

void FE_Messagebox_AddCallback(void (*func)(), void *data)
{
    if (PresentGame->MBShown) {
        callback = func;
        callback_data = data;
    }
}

void FE_Messagebox_Show(char *title, char *body, FE_UI_MBType type)
{
    if (PresentGame->MBShown) {
        warn("Trying to create a message box when one is already active");
        return;
    }
    callback = NULL;
    callback_data = NULL;

    if (input_content)
        free(input_content);
    input_content = 0;

    MB = xmalloc(sizeof(FE_UI_Messagebox));
    MB->texture = FE_LoadResource(FE_RESOURCE_TYPE_TEXTURE, "game/ui/messagebox.png");
    MB->type = type;

    // calculate display rect
    MB->displayrect = (SDL_Rect){
        (PresentGame->Window_width / 2) - (MB_WIDTH / 2),
        (PresentGame->Window_height / 2) - (MB_HEIGHT / 2),
        MB_WIDTH,
        MB_HEIGHT
    };

    // title
    MB->title = FE_UI_CreateLabel(PresentGame->UIConfig.UIFont, title, MB_WIDTH, VEC_EMPTY, PresentGame->UIConfig.UIFontColor);
    MB->title->r.x = FE_GetCentre(MB->title->r, MB->displayrect).x;
    MB->title->r.y = MB->displayrect.y + 10;

    // content
    MB->content = FE_UI_CreateLabel(PresentGame->UIConfig.UIFont, body, (0.8 * MB_WIDTH), VEC_EMPTY, PresentGame->UIConfig.UIFontColor);
    MB->content->r.x = FE_GetCentre(MB->content->r, MB->displayrect).x;
    MB->content->r.y = MB->title->r.y + 48;

    // create button
    MB->button = FE_UI_CreateButton("OK", 0, 0, BUTTON_SMALL, &HandleCallBack, NULL);
    FE_UI_MoveButton(MB->button, FE_GetCentre(MB->button->r, MB->displayrect).x, MB->displayrect.y + MB_HEIGHT - MB->button->r.h - 20);
    FE_UI_AddElement(FE_UI_BUTTON, MB->button);


    if (MB->type == MESSAGEBOX_TEXTBOX) {
        MB->textbox = FE_UI_CreateTextbox(0, 0, MB_WIDTH * 0.8, 40, "");
        FE_UI_ForceActiveTextbox(MB->textbox);
        FE_UI_MoveTextbox(MB->textbox, FE_GetCentre(MB->textbox->r, MB->displayrect).x, MB->content->r.y + MB->content->r.h + 10);
        FE_UI_AddElement(FE_UI_TEXTBOX, MB->textbox);
    }

    PresentGame->MBShown = true;
}

char *FE_Messagebox_GetText()
{
    if (input_content)
        return input_content;
    else
        return NULL;
}

void FE_Messagebox_Render()
{
    if (!PresentGame->MBShown)
        return;

    SDL_RenderCopy(PresentGame->Renderer, MB->texture->Texture, NULL, &MB->displayrect);
    FE_UI_RenderLabel(MB->title);
    FE_UI_RenderLabel(MB->content);
    
    if (MB->type == MESSAGEBOX_TEXTBOX)
        FE_UI_RenderTextbox(MB->textbox);

    FE_UI_RenderButton(MB->button);
}   

void FE_Messagebox_Destroy()
{
    if (!PresentGame->MBShown)
        return;

    if (MB->type == MESSAGEBOX_TEXTBOX) {
        if (previous_active)
            FE_UI_ForceActiveTextbox(previous_active);
        input_content = mstrdup(MB->textbox->content);
        FE_UI_DestroyTextbox(MB->textbox, true);
    }

    FE_UI_DestroyButton(MB->button, true);
    FE_DestroyResource(MB->texture->path);
    FE_UI_DestroyLabel(MB->content, false);
    FE_UI_DestroyLabel(MB->title, false);
    free(MB);

    MB = 0;
    PresentGame->MBShown = false;
    previous_active = NULL;
}