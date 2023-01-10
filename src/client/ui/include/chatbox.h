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


/* Renders the active chatbox */
void FE_UI_RenderChatbox();


/* Creates and returns a new chatbox */
FE_UI_Chatbox *FE_UI_CreateChatbox(void (*cb)(), void *peer);


/* Sends a message to the active chatbox */
void FE_UI_ChatboxMessage(char *message);


/* Destroys and frees the active chatbox */
void FE_UI_DestroyChatbox();


/* Toggles the visibility of the active chatbox */
void FE_UI_ToggleChatbox();


#endif