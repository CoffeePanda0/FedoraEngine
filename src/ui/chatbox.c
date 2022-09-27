#include "include/include.h"
#include "include/chatbox.h"
#include "../core/include/include.h"

#define CHATBOX_WIDTH 400
#define CHATBOX_HEIGHT 240

static FE_Font *chatbox_font;

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
        chatbox_font = FE_LoadFont("RobotoMono-Regular", 16);
    if (!cb) {
        warn("No callback provided for chatbox");
        return NULL;
    }

    FE_UI_Chatbox *chatbox = xmalloc(sizeof(FE_UI_Chatbox));
    chatbox->body = (GPU_Rect) {
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
    
    return chatbox;
}

void FE_UI_ChatboxMessage(FE_UI_Chatbox *chatbox, char *message)
{
    chatbox->messages = xrealloc(chatbox->messages, sizeof(FE_UI_Label) * (chatbox->message_count + 1));

    chatbox->messages[chatbox->message_count] = FE_UI_CreateLabel(
        chatbox_font,
        message,
        CHATBOX_WIDTH - 5,
        0,
        chatbox->message_count == 0 ? chatbox->body.y + 2 : chatbox->messages[chatbox->message_count-1]->r.y + chatbox->messages[chatbox->message_count-1]->r.h,
        COLOR_WHITE
    );

    chatbox->message_count++;

    // check if the labels have started to overflow
    int last_y = chatbox->messages[chatbox->message_count-1]->r.y + chatbox->messages[chatbox->message_count-1]->r.h;

    if (last_y > chatbox->input->r.y) {
        int overflow = last_y - chatbox->input->r.y;
        for (size_t i = 0; i < chatbox->message_count; i++) {
            chatbox->messages[i]->r.y -= overflow; // todo scroll
        }
    }
}

void FE_UI_RenderChatbox(FE_UI_Chatbox *chatbox)
{
    if (chatbox->visible) {
        GPU_BlitRect(chatbox->texture, NULL, PresentGame->Screen, &chatbox->body);
        for (size_t i = 0; i < chatbox->message_count; i++)
            if (chatbox->messages[i]->r.y > chatbox->body.y)
                FE_UI_RenderLabel(chatbox->messages[i]);
    }
}

void FE_UI_DestroyChatbox(FE_UI_Chatbox *chatbox)
{
    if (!chatbox)
        warn("Attempted to destroy null chatbox");

    for (size_t i = 0; i < chatbox->message_count; i++)
        FE_UI_DestroyLabel(chatbox->messages[i], false);
    if (chatbox->messages)
        free(chatbox->messages);

    if (chatbox->visible)
        FE_UI_DestroyTextbox(chatbox->input, true);
    GPU_FreeImage(chatbox->texture);

    free(chatbox);
}

void FE_UI_ToggleChatbox(FE_UI_Chatbox *chatbox)
{
    chatbox->visible = !chatbox->visible;
    if (!chatbox->visible)
        FE_UI_DestroyTextbox(chatbox->input, true);
    else {
        chatbox->input = FE_UI_CreateTextbox(0, chatbox->body.y + chatbox->body.h - 40, chatbox->body.w, 0);
        chatbox->input->onenter = &SendMessage;
        chatbox->input->data = chatbox;
        FE_UI_ForceActiveTextbox(chatbox->input);
        
        FE_UI_AddElement(FE_UI_TEXTBOX, chatbox->input);
    }
}