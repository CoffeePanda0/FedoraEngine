#include "include/include.h"
#include "include/chatbox.h"
#include "../core/include/include.h"

#define CHATBOX_WIDTH 400
#define CHATBOX_HEIGHT 240

static FE_Font *chatbox_font;

static FE_UI_Chatbox *active_chatbox = 0;

static void (*callback)();
static void *_peer;

// todo: nicer input, scrolling, transparency

static void SendMessage(FE_UI_Chatbox *c)
{
    if (mstrlen(c->input->content) > 80) {
        warn("Message too long");
        return;
    }
    if (mstrempty(c->input->content)) {
        FE_UI_SetTextboxContent(c->input, "");
        return;
    }

    callback(c->input->content, _peer);

    FE_UI_SetTextboxContent(c->input, "");
}

FE_UI_Chatbox *FE_UI_CreateChatbox(void (*cb)(), void *peer)
{
    if (!chatbox_font)
        chatbox_font = FE_Font_Load("RobotoMono-Regular", 16);
    if (!cb) {
        warn("No callback provided for chatbox");
        return NULL;
    }
    if (active_chatbox) {
        warn("Chatbox is already active (FE_UI_CreateChatbox)");
        /* Destroy the existing chatbox */
        FE_UI_DestroyChatbox(active_chatbox);
    }

    FE_UI_Chatbox *chatbox = xmalloc(sizeof(FE_UI_Chatbox));
    chatbox->body = (SDL_Rect) {
        .x = 0,
        .y = PresentGame->WindowHeight - CHATBOX_HEIGHT,
        .w = CHATBOX_WIDTH,
        .h = CHATBOX_HEIGHT
    };

    chatbox->texture = FE_TextureFromFile("game/ui/chatbox.png");
    chatbox->visible = true;
    
    callback = cb;
    _peer = peer;

    chatbox->input = FE_UI_CreateTextbox(0, chatbox->body.y + chatbox->body.h - 40, chatbox->body.w, 0);
    
    chatbox->input->onenter = &SendMessage;
    chatbox->input->data = chatbox;

    FE_UI_AddElement(FE_UI_TEXTBOX, chatbox->input);

    chatbox->messages = 0;
    chatbox->message_count = 0;

    active_chatbox = chatbox;
    
    return chatbox;
}

void FE_UI_ChatboxMessage(char *message)
{
    active_chatbox->messages = xrealloc(active_chatbox->messages, sizeof(FE_UI_Label) * (active_chatbox->message_count + 1));

    active_chatbox->messages[active_chatbox->message_count] = FE_UI_CreateLabel(
        chatbox_font,
        message,
        CHATBOX_WIDTH - 5,
        0,
        active_chatbox->message_count == 0 ? active_chatbox->body.y + 2 : active_chatbox->messages[active_chatbox->message_count-1]->r.y + active_chatbox->messages[active_chatbox->message_count-1]->r.h,
        COLOR_WHITE
    );

    active_chatbox->message_count++;

    // check if the labels have started to overflow
    int last_y = active_chatbox->messages[active_chatbox->message_count-1]->r.y + active_chatbox->messages[active_chatbox->message_count-1]->r.h;

    if (last_y > active_chatbox->input->r.y) {
        int overflow = last_y - active_chatbox->input->r.y;
        for (size_t i = 0; i < active_chatbox->message_count; i++) {
            active_chatbox->messages[i]->r.y -= overflow; // todo scroll
        }
    }
}

void FE_UI_RenderChatbox()
{
    if (active_chatbox->visible) {
        SDL_RenderCopy(PresentGame->Client->Renderer, active_chatbox->texture, NULL, &active_chatbox->body);
        for (size_t i = 0; i < active_chatbox->message_count; i++)
            if (active_chatbox->messages[i]->r.y > active_chatbox->body.y)
                FE_UI_RenderLabel(active_chatbox->messages[i]);
    }
}

void FE_UI_DestroyChatbox()
{
    if (!active_chatbox) return;

    for (size_t i = 0; i < active_chatbox->message_count; i++)
        FE_UI_DestroyLabel(active_chatbox->messages[i], false);
    if (active_chatbox->messages)
        free(active_chatbox->messages);

    if (active_chatbox->visible)
        FE_UI_DestroyTextbox(active_chatbox->input, true);
    SDL_DestroyTexture(active_chatbox->texture);

    free(active_chatbox);
}

void FE_UI_ToggleChatbox()
{
    active_chatbox->visible = !active_chatbox->visible;
    if (!active_chatbox->visible)
        FE_UI_DestroyTextbox(active_chatbox->input, true);
    else {
        active_chatbox->input = FE_UI_CreateTextbox(0, active_chatbox->body.y + active_chatbox->body.h - 40, active_chatbox->body.w, 0);
        active_chatbox->input->onenter = &SendMessage;
        active_chatbox->input->data = active_chatbox;
        FE_UI_ForceActiveTextbox(active_chatbox->input);
        
        FE_UI_AddElement(FE_UI_TEXTBOX, active_chatbox->input);
    }
}