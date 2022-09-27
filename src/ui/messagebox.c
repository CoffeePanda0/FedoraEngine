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

void FE_Messagebox_HandleCallBack()
{
    FE_Messagebox_Destroy();
    PresentGame->UIConfig.MBShown = false;
    if (callback)
        callback(callback_data);
}

void FE_Messagebox_AddCallback(void (*func)(), void *data)
{
    if (PresentGame->UIConfig.MBShown) {
        callback = func;
        callback_data = data;
    }
}

bool FE_Messagebox_Click(int x, int y)
{
    // Check if the click is the button
    if (MB->button->r.x < x && MB->button->r.x + MB->button->r.w > x &&
        MB->button->r.y < y && MB->button->r.y + MB->button->r.h > y) {
        MB->button->onclick(MB->button->onclick_data);
        return true;
    }

    // Check if the click is the textbox
    if (MB->type == MESSAGEBOX_TEXTBOX && MB->textbox) {
        if (MB->textbox->r.x < x && MB->textbox->r.x + MB->textbox->r.w > x &&
            MB->textbox->r.y < y && MB->textbox->r.y + MB->textbox->r.h > y) {
            FE_UI_ForceActiveTextbox(MB->textbox);
            return true;
        }
    }

    return false;
}

void FE_Messagebox_Show(char *title, char *body, FE_UI_MBType type)
{
    if (PresentGame->UIConfig.MBShown) {
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
    MB->displayrect = (GPU_Rect){
        (PresentGame->WindowWidth / 2) - (MB_WIDTH / 2),
        (PresentGame->WindowHeight / 2) - (MB_HEIGHT / 2),
        MB_WIDTH,
        MB_HEIGHT
    };

    // title
    MB->title = FE_UI_CreateLabel(PresentGame->UIConfig.UIFont, title, 1000, 0,0 , PresentGame->UIConfig.UIFontColor);
    MB->title->r.x = FE_GetCentre(MB->title->r, MB->displayrect).x;
    MB->title->r.y = MB->displayrect.y + 10;

    // content
    MB->content = FE_UI_CreateLabel(PresentGame->UIConfig.UIFont, body, MB_WIDTH, 0,0 , PresentGame->UIConfig.UIFontColor);
    MB->content->r.x = FE_GetCentre(MB->content->r, MB->displayrect).x;
    MB->content->r.y = MB->title->r.y + 48;

    // create button
    MB->button = FE_UI_CreateButton("OK", 0, 0, BUTTON_SMALL, &FE_Messagebox_HandleCallBack, NULL);
    FE_UI_MoveButton(MB->button, FE_GetCentre(MB->button->r, MB->displayrect).x, MB->displayrect.y + MB_HEIGHT - MB->button->r.h - 20);
    FE_UI_AddElement(FE_UI_BUTTON, MB->button);


    if (MB->type == MESSAGEBOX_TEXTBOX) {
        MB->textbox = FE_UI_CreateTextbox(0, 0, MB_WIDTH * 0.8, "");
        FE_UI_ForceActiveTextbox(MB->textbox);
        FE_UI_MoveTextbox(MB->textbox, FE_GetCentre(MB->textbox->r, MB->displayrect).x, MB->content->r.y + MB->content->r.h + 10);
        FE_UI_AddElement(FE_UI_TEXTBOX, MB->textbox);
        FE_UI_AddTextboxCallback(MB->textbox, &FE_Messagebox_HandleCallBack, NULL);
    }

    PresentGame->UIConfig.MBShown = true;
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
    if (!PresentGame->UIConfig.MBShown)
        return;

    GPU_BlitRect(MB->texture->Texture, NULL, PresentGame->Screen, &MB->displayrect);
    FE_UI_RenderLabel(MB->title);
    FE_UI_RenderLabel(MB->content);
    
    if (MB->type == MESSAGEBOX_TEXTBOX)
        FE_UI_RenderTextbox(MB->textbox);

    FE_UI_RenderButton(MB->button);
}   

void FE_Messagebox_Destroy()
{
    if (!PresentGame->UIConfig.MBShown)
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
    PresentGame->UIConfig.MBShown = false;
    previous_active = NULL;
}