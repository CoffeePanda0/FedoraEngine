#include "net.h"
#include <string.h>
#include "include/internal.h"
#include "../include/game.h"
#include "../ui/include/chatbox.h"

#define _connection_timeout 5000
static ENetPeer *peer;
static ENetHost *client;

static bool connected = false;
static char *username;

// Game vars
static FE_Player *GamePlayer;
static FE_Camera *GameCamera;
static GPU_Image *world;

// ui elements
static FE_UI_Label *ping_label;
static FE_UI_Label *connecting_label;
static FE_UI_Chatbox *chatbox;

// struct for each connected client
static FE_List *players;

static bool Connect(int port, char *addr)
{
    // show connecting status
    GPU_Clear(PresentGame->Screen);
    connecting_label = FE_UI_CreateLabel(0, "Connecting...", 300, midx(256), 50, COLOR_WHITE);
    FE_UI_AddElement(FE_UI_LABEL, connecting_label);
    FE_UI_Render();
    GPU_Flip(PresentGame->Screen);

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
    switch (PacketType(event)) {
        case PACKET_TYPE_UPDATE:;
            // update player position
            char *user = JSONPacket_GetValue(event, "u");

            float x = atof(JSONPacket_GetValue(event, "x"));
            float y = atof(JSONPacket_GetValue(event, "y"));

            if (mstrcmp(user, username) == 0) {
                float velx = atof(JSONPacket_GetValue(event, "vx"));
                float vely = atof(JSONPacket_GetValue(event, "vy"));

                GamePlayer->PhysObj->position.x = x;
                GamePlayer->PhysObj->position.y = y;
                GamePlayer->PhysObj->velocity.x = velx;
                GamePlayer->PhysObj->velocity.y = vely;

                FE_UPDATE_RECT(GamePlayer->PhysObj->position, &GamePlayer->PhysObj->body);
            } else {
                // find player in list, update position
                for (FE_List *l = players; l; l = l->next) {
                    player *p = l->data;
                    if (mstrcmp(p->username, user) == 0) {
                        p->rect.x = x;
                        p->rect.y = y;
                        return;
                    }
                }
            }

            break;
        case PACKET_TYPE_SERVERMSG: ;
            char *servermsg = mstradd("SERVER MESSAGE: ", JSONPacket_GetValue(event, "msg"));
            info(servermsg);
            
            FE_UI_ChatboxMessage(chatbox, servermsg);

            free(servermsg);
            break;
        case PACKET_TYPE_SERVERCMD: ;
            char *cmd = JSONPacket_GetValue(event, "cmd");

            if (mstrcmp(cmd, "addplayer") == 0) {
                // add player to list
                player *p = xmalloc(sizeof(player));
                p->rect = (GPU_Rect) {
                    .x = atoi(JSONPacket_GetValue(event, "x")),
                    .y = atoi(JSONPacket_GetValue(event, "y")),
                    .w = 120,
                    .h = 100
                };
                p->texture = FE_LoadResource(FE_RESOURCE_TYPE_TEXTURE, "game/sprites/doge.png");
                mstrcpy(p->username, JSONPacket_GetValue(event, "username"));
                FE_List_Add(&players, p);

                // show player join message
                info("Player %s has joined", JSONPacket_GetValue(event, "username"));
                char *msg = mstradd(JSONPacket_GetValue(event, "username"), " has joined the game");
                FE_UI_ChatboxMessage(chatbox,  msg);
                free(msg);

            } else if (mstrcmp(cmd, "removeplayer") == 0) {
                // remove player from list and free memory
                char *user = JSONPacket_GetValue(event, "username");
                for (FE_List *p = players; p; p = p->next) {
                    player *pl = p->data;
                    if (mstrcmp(pl->username, user) == 0) {
                        FE_DestroyResource(pl->texture->path);
                        FE_List_Remove(&players, pl);
                        free(pl);

                        info("Player %s has left", user);
                        char *msg = mstradd(JSONPacket_GetValue(event, "username"), " has left the game");
                        FE_UI_ChatboxMessage(chatbox,  msg);
                        free(msg);

                        return;
                    }
                }
            }
            break;

        case PACKET_TYPE_KICK:
            // set the game disconnect info to the kick message
            PresentGame->DisconnectInfo = (FE_DisconnectInfo) {
                .set = true,
                .reason = JSONPacket_GetValue(event, "reason"),
            };
        break;

        case PACKET_TYPE_MESSAGE: ;
            // show chat message
            
            char *_username = JSONPacket_GetValue(event, "username");
            char *_msg = JSONPacket_GetValue(event, "msg");

            size_t len = mstrlen(_username) + mstrlen(_msg) + 7;
            char *msg = xmalloc(len);
            snprintf(msg, len, "[%s]: %s", _username, _msg);

            FE_UI_ChatboxMessage(chatbox, msg);
            free(msg);
            break;
        break;

        default:
            break;
    }
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
        GPU_FreeImage(world);
    if (world->target)
        GPU_FreeTarget(world->target);

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
	GamePlayer = FE_Player_Create(30, 50, 180, (GPU_Rect){PresentGame->MapConfig.PlayerSpawn.x, PresentGame->MapConfig.PlayerSpawn.y, 120, 100});
	GameCamera->follow = &GamePlayer->render_rect;

	// test particle system
	FE_ParticleSystem_Create(
		(GPU_Rect){0, -20, PresentGame->MapConfig.MapWidth, 20}, // Position for the whole screen, slightly above the top to create more random
		350, // Emission rate
		3000, // Max particles
		10000, // Max lifetime
		true, // Particles to respawn once they go off screen
		"rain.png", // Texture
		(vec2){15, 15}, // Max size of each particle
		(vec2){3, 3}, // Set initial velocity so particle doesn't float until they accelerate
		false
	);

	world = GPU_CreateImage(PresentGame->WindowWidth, PresentGame->WindowHeight, GPU_FORMAT_RGBA);
	FE_ResetDT();

    return true;
}

static void RenderPlayers()
{
    for (FE_List *l = players; l; l = l->next) {
        player *p = l->data;
        FE_RenderCopy(0, GameCamera, false, p->texture, NULL, &p->rect);
    }
}

void ClientRender()
{
    if (!connected) {
        GPU_Clear(PresentGame->Screen);
        FE_UI_Render();
        GPU_Flip(PresentGame->Screen);
    } else {
        PresentGame->Timing.RenderTime = SDL_GetPerformanceCounter();

        if (PresentGame->DebugConfig.LightingEnabled)
            GPU_LoadTarget(world);

        GPU_Clear(PresentGame->Screen);
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

        GPU_Flip(PresentGame->Screen);

	    PresentGame->Timing.RenderTime = ((SDL_GetPerformanceCounter() - PresentGame->Timing.RenderTime) / SDL_GetPerformanceFrequency()) * 1000;
    }
}

static void SendKeyDown(int key)
{
    JSONPacket_SendInt(peer, PACKET_TYPE_KEYDOWN, "key", key);
}

static void SendKeyUp(int key)
{
    JSONPacket_SendInt(peer, PACKET_TYPE_KEYUP, "key", key);
}

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
                        // todo change pause menu
                        break;
                    }
                    else if (keyboard_state[SDL_SCANCODE_GRAVE]) {
                        SDL_StartTextInput();
                        PresentGame->StartedInput = true;
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

    if (GamePlayer->PhysObj->velocity.y == 0) GamePlayer->on_ground = true;

	FE_DebugUI_Update(GamePlayer); 
	FE_Dialogue_Update();

	PresentGame->Timing.UpdateTime = SDL_GetPerformanceCounter();
	FE_Timers_Update();
	FE_Particles_Update();
	FE_Player_Update(GamePlayer);
	FE_Animations_Update();
	FE_Prefab_Update();
	FE_UpdateCamera(GameCamera);
	PresentGame->Timing.UpdateTime = ((SDL_GetPerformanceCounter() - PresentGame->Timing.UpdateTime) / SDL_GetPerformanceFrequency()) * 1000;
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
        return -1;
    }
    free(addr);

    // Sync state with server
    if (Client_Connect(_username, peer, client, &username, &players) == false) {
        warn("Unable to authenticate with server");
        Disconnect();
        return -1;
    }

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