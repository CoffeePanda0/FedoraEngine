#include <FE_Client.h>
#include "../../common/net/include/packet.h"
#include "include/include.h"
#include "../ui/include/chatbox.h"

static void SendKeyDown(int key, ENetPeer *peer)
{
    FE_Net_Packet *p = FE_Net_Packet_Create(PACKET_CLIENT_KEYDOWN);
    FE_Net_Packet_AddInt(p, key);
    FE_Net_Packet_Send(peer, p, true);
}

static void SendKeyUp(int key, ENetPeer *peer)
{
    FE_Net_Packet *p = FE_Net_Packet_Create(PACKET_CLIENT_KEYUP);
    FE_Net_Packet_AddInt(p, key);
    FE_Net_Packet_Send(peer, p, true);
}

void ClientEventHandle(FE_Camera *GameCamera, FE_Player *GamePlayer, FE_Net_Client *Client)
{
    SDL_PumpEvents();
	SDL_Event event;
    const Uint8* keyboard_state = SDL_GetKeyboardState(NULL);

    while (SDL_PollEvent(&event)) {
        if (FE_UI_HandleEvent(&event, keyboard_state))
                break;

        switch (event.type) {
            case SDL_KEYDOWN:
                if (event.key.repeat == 0) {
                    if (keyboard_state[SDL_SCANCODE_ESCAPE]) {
                        DisconnectClient(Client);
                        break;
                    }
                    else if (keyboard_state[SDL_SCANCODE_GRAVE]) {
                        SDL_StartTextInput();
                        PresentGame->Client->StartedInput = true;
                        FE_Console_Show();
                        break;
                    }

                    else if ((int)event.key.keysym.scancode == FE_Key_Get("LEFT")) {
                        SendKeyDown(KEY_LEFT, Client->Peer);
                        break;
                    } else if ((int)event.key.keysym.scancode == FE_Key_Get("RIGHT")) {
                        SendKeyDown(KEY_RIGHT, Client->Peer);
                        break;
                    } else if ((int)event.key.keysym.scancode == FE_Key_Get("JUMP")) {
                        SendKeyDown(KEY_JUMP, Client->Peer);
                        break;
                    }

                    else if (keyboard_state[FE_Key_Get("ZOOM IN")])
                        FE_Camera_SmoothZoom(GameCamera, 0.5, 250);
                    else if (keyboard_state[FE_Key_Get("ZOOM OUT")])
                        FE_Camera_SmoothZoom(GameCamera, -0.5, 250);
                    
                    else if (keyboard_state[SDL_SCANCODE_I]) {
                        PresentGame->DebugConfig.ShowTiming = !PresentGame->DebugConfig.ShowTiming;
                        break;
                    }

                    else if (keyboard_state[SDL_SCANCODE_L])
                        FE_Light_Toggle(GamePlayer->Light);
                    else if (keyboard_state[SDL_SCANCODE_C])
                        FE_UI_ToggleChatbox();
                }

                if (keyboard_state[SDL_SCANCODE_M]) {
                    if (PresentGame->MapConfig.AmbientLight <= 252) {
                        PresentGame->MapConfig.AmbientLight += 3;
                    }
                    break;
                }
                else if (keyboard_state[SDL_SCANCODE_N]) {
                    if (PresentGame->MapConfig.AmbientLight >= 3) {
                        PresentGame->MapConfig.AmbientLight -= 3;
                    }
                    break;
                }

            break;

            case SDL_KEYUP:
                if ((int)event.key.keysym.scancode == FE_Key_Get("LEFT"))
                    SendKeyUp(KEY_LEFT, Client->Peer);
                else if ((int)event.key.keysym.scancode == FE_Key_Get("RIGHT"))
                    SendKeyUp(KEY_RIGHT, Client->Peer);
                else if ((int)event.key.keysym.scancode == FE_Key_Get("JUMP"))
                    SendKeyUp(KEY_JUMP, Client->Peer);
            break;
        }
    }
}