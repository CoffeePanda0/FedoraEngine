#ifndef _CHATBOX_H
#define _CHATBOX_H

#include <SDL.h>

typedef struct {
    SDL_Rect body;
    SDL_Texture *texture;
    
    FE_UI_Textbox *input;

    FE_UI_Label **messages;
    size_t message_count;

    bool visible;
} FE_UI_Chatbox;


/* Renders a given chatbox */
void FE_UI_RenderChatbox(FE_UI_Chatbox *chatbox);


/* Creates and returns a new chatbox */
FE_UI_Chatbox *FE_UI_CreateChatbox(void (*cb)(), void *peer);


/* Sends a message to the chatbox */
void FE_UI_ChatboxMessage(FE_UI_Chatbox *chatbox, char *message);


/* Destroys and frees a given chatbox */
void FE_UI_DestroyChatbox(FE_UI_Chatbox *chatbox);


/* Toggles the visibility of a given chatbox */
void FE_UI_ToggleChatbox(FE_UI_Chatbox *chatbox);


#endif