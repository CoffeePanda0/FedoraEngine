#include <FE_Client.h>

#include "../../common/net/include/packet.h"
#include "include/client.h"
#include "include/message.h"
#include "include/include.h"

#include "../ui/include/chatbox.h"
#include "../ui/include/menu.h"

#define _connection_timeout 5000
static ENetPeer *peer;
static ENetHost *client;

static bool connected = false;
static char *username;

// Game vars
static FE_Player *GamePlayer;
static FE_Camera *GameCamera;
static SDL_Texture *world;

// ui elements
static FE_UI_Label *ping_label;
static FE_UI_Label *connecting_label;
static FE_UI_Chatbox *chatbox;

// struct for each connected client
static FE_List *players;

static bool Connect(int port, char *addr)
{
    // show connecting status
    SDL_RenderClear(PresentGame->Client->Renderer);
    connecting_label = FE_UI_CreateLabel(0, "Connecting...", 300, midx(256), 50, COLOR_WHITE);
    FE_UI_AddElement(FE_UI_LABEL, connecting_label);
    FE_UI_Render();
    SDL_RenderPresent(PresentGame->Client->Renderer);

    ENetAddress address = {0};
    ENetEvent event;

    enet_address_set_host(&address, addr);
    address.port = port;

    peer = enet_host_connect(client, &address, 1, 0);
    if (!peer) {
        info("No available peers for initiating an ENet connection!");
        return false;
    }

    if (enet_host_service(client, &event, _connection_timeout) > 0 && event.type == ENET_EVENT_TYPE_CONNECT) {
        info("Connection to %s:%i succeeded", addr, port);
        return true;
    } else {
        enet_peer_reset(peer);
        info("Connection to %s:%i failed", addr, port);
    }
    return false;
}

static void Disconnect()
{
    // to be called when we want to manually disconnect from the server to let the server know we're done
    if (peer) {
        ENetEvent event;
        enet_peer_disconnect(peer, 0);
        while (enet_host_service(client, &event, 3000) > 0) {
            switch (event.type) {
                case ENET_EVENT_TYPE_RECEIVE:
                    enet_packet_destroy(event.packet);
                    break;
                case ENET_EVENT_TYPE_DISCONNECT:
                    info("Disconnected from server");
                    connected = false;
                    enet_packet_destroy(event.packet);
                    break;
                default:
                    enet_packet_destroy(event.packet);
                break;
            }
        }
    }
}

static void HandleRecieve(ENetEvent *event)
{
    FE_Net_RcvPacket *packet = FE_Net_GetPacket(event);
    if (!packet) {
        warn("Invalid packet from server");
        return;
    }

    switch (packet->type) {

        case PACKET_SERVER_UPDATE:;

            /* Check if the updated player is us */
            char *user = FE_Net_GetString(packet);

            if (mstrcmp(user, username) == 0) {
                /* Update our player */
                GamePlayer->player->PhysObj->position.x = FE_Net_GetInt(packet);
                GamePlayer->player->PhysObj->position.y = FE_Net_GetInt(packet);

                GamePlayer->player->PhysObj->body.x = GamePlayer->player->PhysObj->position.x;
                GamePlayer->player->PhysObj->body.y = GamePlayer->player->PhysObj->position.y;

                GamePlayer->player->PhysObj->velocity.x = FE_Net_GetFloat(packet);
                GamePlayer->player->PhysObj->velocity.y = FE_Net_GetFloat(packet);
            } else {
                /* Find player in list, update position */
                for (FE_List *l = players; l; l = l->next) {
                    player *p = l->data;
                    if (mstrcmp(p->username, user) == 0) {
                        p->rect.x = FE_Net_GetInt(packet);
                        p->rect.y = FE_Net_GetInt(packet);
                        return;
                    }
                }
            }
        break;

        case PACKET_SERVER_SERVERMSG: ;
            char *data = FE_Net_GetString(packet);
            char *servermsg = mstradd("SERVER MESSAGE: ", data);
            info(servermsg);
            
            FE_UI_ChatboxMessage(chatbox, servermsg);

            free(servermsg);
            free(data);
        
        break;

        case PACKET_SERVER_SPAWN: ;
            char *username = FE_Net_GetString(packet);
            int x = FE_Net_GetInt(packet);
            int y = FE_Net_GetInt(packet);

            // add player to list
            player *p = xmalloc(sizeof(player));
            p->rect = (SDL_Rect) {
                .x = x,
                .y = y,
                .w = 120,
                .h = 100
            };
            p->texture = FE_LoadResource(FE_RESOURCE_TYPE_TEXTURE, "game/sprites/doge.png");
            mstrcpy(p->username, username);
            FE_List_Add(&players, p);

            // show player join message
            char *msg = mstradd(username, " has joined the game");
            FE_UI_ChatboxMessage(chatbox,  msg);

            free(msg);
            free(username);
        break;

        case PACKET_SERVER_DESPAWN: ;
            /* Remove player from list and free memory */
            char *kicked_user = FE_Net_GetString(packet);
            for (FE_List *p = players; p; p = p->next) {
                player *pl = p->data;
                if (mstrcmp(pl->username, kicked_user) == 0) {
                    FE_DestroyResource(pl->texture->path);
                    FE_List_Remove(&players, pl);
                    free(pl);

                    info("Player %s has left", kicked_user);
                    char *msg = mstradd(kicked_user, " has left the game");
                    FE_UI_ChatboxMessage(chatbox,  msg);
                    free(msg);
                    free(kicked_user);

                    return;
                }
            }
        break;

        case PACKET_SERVER_KICK: ;
            // set the game disconnect info to the kick message
            PresentGame->DisconnectInfo = (FE_DisconnectInfo) {
                .set = true,
                .reason = FE_Net_GetString(packet),
            };
        break;

        case PACKET_SERVER_CHAT: ;
            // show chat message
            
            char *_username = FE_Net_GetString(packet);
            char *_msg = FE_Net_GetString(packet);

            size_t len = mstrlen(_username) + mstrlen(_msg) + 7;
            char *chat = xmalloc(len);
            snprintf(chat, len, "[%s]: %s", _username, _msg);

            FE_UI_ChatboxMessage(chatbox, chat);
            free(chat);
            free(_username);
            free(_msg);
    
            break;
        break;

        default:
            break;
    }
    if (packet) FE_Net_DestroyRcv(packet);

}

static void Exit()
{
    DestroyClient();
    FE_Menu_LoadMenu("Main");
}

static void HandleDisconnect(ENetEvent *event)
{
    connected = false;
    switch (event->data) {
        case DISC_SERVER:
            info("Disconnected from server (Timed out)");
            PresentGame->DisconnectInfo = (FE_DisconnectInfo) {
                .set = true,
                .type = DISC_SERVER,
                .reason = mstrdup("Disconnected from server (Timed out)"),
            };
            break;
        case DISC_KICK:
            info("You have been kicked from the server. (Reason: %s)", PresentGame->DisconnectInfo.reason);
            PresentGame->DisconnectInfo.type = DISC_KICK;
            break;
        case DISC_BAN:
            info("You have been banned from the server. (Reason: %s)", PresentGame->DisconnectInfo.reason);
            PresentGame->DisconnectInfo.type = DISC_BAN;
            break;
        case DISC_NOCON:
            PresentGame->DisconnectInfo.set = true;
            PresentGame->DisconnectInfo.reason = mstrdup("You have been banned from this server.");
            info("Failed to connect to server. (Reason: %s", PresentGame->DisconnectInfo.reason);
            PresentGame->DisconnectInfo.type = DISC_NOCON;
        break;
        default:
            info("Disconnected from server (Other)");
            break;
    }
}

static void HostClient()
{
    ENetEvent event;
    while (enet_host_service(client, &event, 0) > 0) {
        switch (event.type) {
            case ENET_EVENT_TYPE_RECEIVE: ;
                HandleRecieve(&event);
                enet_packet_destroy(event.packet);
            break;
            case ENET_EVENT_TYPE_DISCONNECT:
                HandleDisconnect(&event);
                enet_packet_destroy(event.packet);
            break;
            case ENET_EVENT_TYPE_DISCONNECT_TIMEOUT:
                HandleDisconnect(&event);
                enet_packet_destroy(event.packet);
            break;

            default: break;
        }
    }
}

static void UpdatePing()
{
    if (!PresentGame->DebugConfig.ShowTiming) return;
    char buff[16];
    sprintf(buff, "Ping: %ims", peer->lastRoundTripTime);
    FE_UI_UpdateLabel(ping_label, buff);
}

void DestroyClient()
{    
    if (players) {
        // destroy connected players
        for (FE_List *l = players; l; l = l->next) {
            player *p = l->data;
            FE_DestroyResource(p->texture->path);
            free(p);
        }
        FE_List_Destroy(&players);
        players = 0;
    }

    if (connected)
        Disconnect();
    if (username)
        free(username);
    username = 0;

    if (chatbox)
        FE_UI_DestroyChatbox(chatbox);
    chatbox = 0;

    if (client)
        enet_host_destroy(client);
    enet_deinitialize();
    
    if (GamePlayer)
        FE_Player_Destroy(GamePlayer);
    GamePlayer = 0;
    GameCamera = 0;
    if (world)
        SDL_DestroyTexture(world);
    world = 0;
}

static bool CreateGame()
{
    FE_UI_DestroyLabel(connecting_label, true);

    // camera setup
	GameCamera = FE_CreateCamera();
	GameCamera->maxzoom = 2.0f;
	GameCamera->minzoom = 1.0f;

	// player setup
	GamePlayer = FE_Player_Create(30, 50, 180, (SDL_Rect){PresentGame->MapConfig.PlayerSpawn.x, PresentGame->MapConfig.PlayerSpawn.y, 120, 100});
	GameCamera->follow = &GamePlayer->render_rect;

	// test particle system
	FE_ParticleSystem_Create(
		(SDL_Rect){0, -20, PresentGame->MapConfig.MapWidth, 20}, // Position for the whole screen, slightly above the top to create more random
		350, // Emission rate
		3000, // Max particles
		10000, // Max lifetime
		true, // Particles to respawn once they go off screen
		"rain.png", // Texture
		(vec2){15, 15}, // Max size of each particle
		(vec2){3, 3}, // Set initial velocity so particle doesn't float until they accelerate
		false
	);

	world = FE_CreateRenderTexture(PresentGame->WindowWidth, PresentGame->WindowHeight);
	FE_ResetDT();

    return true;
}

static void RenderPlayers()
{
    for (FE_List *l = players; l; l = l->next) {
        player *p = l->data;
        FE_RenderCopy(GameCamera, false, p->texture, NULL, &p->rect);
    }
}

void ClientRender()
{
    if (!connected) {
        SDL_RenderClear(PresentGame->Client->Renderer);
        FE_UI_Render();
        SDL_RenderPresent(PresentGame->Client->Renderer);
    } else {
        PresentGame->Timing.RenderTime = FE_QueryPerformanceCounter();

        if (PresentGame->DebugConfig.LightingEnabled)
            SDL_SetRenderTarget(PresentGame->Client->Renderer, world);

        SDL_RenderClear(PresentGame->Client->Renderer);
        FE_Map_RenderBackground(GameCamera);
        FE_Particles_Render(GameCamera);
        FE_Map_RenderLoaded(GameCamera);
        FE_Trigger_Render(GameCamera);
        FE_GameObject_Render(GameCamera);
        FE_Trigger_Render(GameCamera);
        RenderPlayers();
        FE_Player_Render(GamePlayer, GameCamera);

        if (PresentGame->DebugConfig.LightingEnabled)
            FE_Light_Render(GameCamera, world);


        FE_UI_RenderChatbox(chatbox);
        FE_UI_Render();

        SDL_RenderPresent(PresentGame->Client->Renderer);

	    PresentGame->Timing.RenderTime = ((FE_QueryPerformanceCounter() - PresentGame->Timing.RenderTime) / FE_QueryPerformanceFrequency()) * 1000;
    }
}

static void SendKeyDown(int key)
{
    FE_Net_Packet *p = FE_Net_Packet_Create(PACKET_CLIENT_KEYDOWN);
    FE_Net_Packet_AddInt(p, key);
    FE_Net_Packet_Send(peer, p, true);
}

static void SendKeyUp(int key)
{
    FE_Net_Packet *p = FE_Net_Packet_Create(PACKET_CLIENT_KEYUP);
    FE_Net_Packet_AddInt(p, key);
    FE_Net_Packet_Send(peer, p, true);}

void ClientEventHandle()
{
    if (!connected) return;

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
                        Disconnect();
                        break;
                    }
                    else if (keyboard_state[SDL_SCANCODE_GRAVE]) {
                        SDL_StartTextInput();
                        PresentGame->Client->StartedInput = true;
                        FE_Console_Show();
                        break;
                    }

                    else if ((int)event.key.keysym.scancode == FE_Key_Get("LEFT")) {
                        SendKeyDown(KEY_LEFT);
                        break;
                    } else if ((int)event.key.keysym.scancode == FE_Key_Get("RIGHT")) {
                        SendKeyDown(KEY_RIGHT);
                        break;
                    } else if ((int)event.key.keysym.scancode == FE_Key_Get("JUMP")) {
                        SendKeyDown(KEY_JUMP);
                        break;
                    }

                    else if (keyboard_state[FE_Key_Get("ZOOM IN")])
                        FE_Camera_SmoothZoom(GameCamera, 0.5, 250);
                    else if (keyboard_state[FE_Key_Get("ZOOM OUT")])
                        FE_Camera_SmoothZoom(GameCamera, -0.5, 250);
                    
                    else if (keyboard_state[SDL_SCANCODE_I]) {
                        PresentGame->DebugConfig.ShowTiming = !PresentGame->DebugConfig.ShowTiming;
                        if (!ping_label) {
                            ping_label = FE_UI_CreateLabel(0, "pong", 256, 0, 180, COLOR_BLACK);
                            ping_label->showbackground = true;
                            FE_UI_AddElement(FE_UI_LABEL, ping_label);
                        } else {
                            FE_UI_DestroyLabel(ping_label, true);
                            ping_label = 0;
                        }
                        break;
                    }

                    else if (keyboard_state[SDL_SCANCODE_L])
                        FE_Light_Toggle(GamePlayer->Light);
                    else if (keyboard_state[SDL_SCANCODE_C])
                        FE_UI_ToggleChatbox(chatbox);
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
                    SendKeyUp(KEY_LEFT);
                else if ((int)event.key.keysym.scancode == FE_Key_Get("RIGHT"))
                    SendKeyUp(KEY_RIGHT);
                else if ((int)event.key.keysym.scancode == FE_Key_Get("JUMP"))
                    SendKeyUp(KEY_JUMP);
            break;
        }
    }
}

void ClientUpdate()
{
    if (!connected && client) {
        Exit();
    }

	if (FE_FPS == 0 || !connected) {
		return;
	}
    HostClient();

    UpdatePing();

	FE_DebugUI_Update(GamePlayer); 
	FE_Dialogue_Update();

	PresentGame->Timing.UpdateTime = FE_QueryPerformanceCounter();
	FE_Timers_Update();
	FE_Particles_Update();

    GamePlayer->player->PhysObj->grounded = (GamePlayer->player->PhysObj->velocity.y == 0) ? true : false;
    
	FE_Player_Update(GamePlayer);
    
	FE_Animations_Update();
	FE_Prefab_Update();
	FE_UpdateCamera(GameCamera);
	PresentGame->Timing.UpdateTime = ((FE_QueryPerformanceCounter() - PresentGame->Timing.UpdateTime) / FE_QueryPerformanceFrequency()) * 1000;
}

int InitClient(char *addr, int port, char *_username)
{
    if (enet_initialize() != 0) {
        warn("An error occurred while initializing ENet!");
        return -1;
    }

    client = enet_host_create(NULL, 1, 1, 0, 0);
    if (!client) {
        warn("An error occurred while trying to create an ENet client host!");
        return -1;
    }

    info("Client initialised");

    connected = Connect(port, addr);
    if (!connected) {
        warn("Could not connect to server");
        if (client) enet_host_destroy(client);
        return -1;
    }

    /* Sync state with server */
    if (Client_Connect(_username, peer, client, &username, &players) == false) {
        warn("Unable to authenticate with server");
        Disconnect();
        return -1;
    }

    /* Write server info to last-connected file */
    if (FE_File_DirectoryExists("serverinfo.txt"))
        FE_File_Delete("serverinfo.txt");

    FILE *f = fopen("serverinfo.txt", "w");
    if (!f) {
        warn("Failed to write server info to file");
    } else {
        fprintf(f, "%s\n", addr);
        fprintf(f, "%d\n", port);
        fprintf(f, "%s\n", _username);
        fclose(f);
    }

    /* Create the game world */
    if (!CreateGame()) {
        warn("Could not create game");
        Disconnect();
        return -1;
    }

    if (PresentGame->DebugConfig.ShowTiming) {
        ping_label = FE_UI_CreateLabel(0, "pong", 256, 0, 180, COLOR_BLACK);
        ping_label->showbackground = true;
        FE_UI_AddElement(FE_UI_LABEL, ping_label);
    }

    chatbox = FE_UI_CreateChatbox(&Client_SendMesage, peer);
    FE_UI_ToggleChatbox(chatbox);

    return 0;
}