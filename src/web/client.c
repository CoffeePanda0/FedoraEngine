#include "net.h"
#include "include/internal.h"
#include "../include/game.h"
#define MAP "test"

#define _connection_timeout 5000
static ENetPeer *peer;
static ENetHost *client;

static bool connected = false;
static char *username;

// Game vars
static FE_Player *GamePlayer;
static FE_Camera *GameCamera;
static SDL_Texture *world;
static FE_UI_Label *ping_label;

// struct for each connected client
static FE_List *players;
static bool Connect(int port, char *addr)
{
    // show connecting status
    SDL_RenderClear(PresentGame->Renderer);
    FE_UI_AddElement(FE_UI_LABEL, FE_UI_CreateLabel(0, "Connecting...", 300, midx(256), 50, COLOR_WHITE));
    FE_UI_Render();
    SDL_RenderPresent(PresentGame->Renderer);

    ENetAddress address;
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
                case ENET_EVENT_TYPE_CONNECT:
                    break;
                case ENET_EVENT_TYPE_RECEIVE:
                    enet_packet_destroy(event.packet);
                    break;
                case ENET_EVENT_TYPE_DISCONNECT:
                    info("Disconnected from server");
                    connected = false;
                    break;
                default: break;
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

                FE_DT_RECT(GamePlayer->PhysObj->position, &GamePlayer->PhysObj->body);
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
            FE_Console_SetText(servermsg);
            free(servermsg);
            break;
        case PACKET_TYPE_SERVERCMD: ;
            char *cmd = JSONPacket_GetValue(event, "cmd");

            if (mstrcmp(cmd, "username") == 0) {
                // the server setting the clients username
                username = strdup(JSONPacket_GetValue(event, "username"));
                info("Username set to %s", username);

            } else if (mstrcmp(cmd, "addplayer") == 0) {
                // add player to list
                player *p = xmalloc(sizeof(player));
                p->rect = (SDL_Rect) {
                    .x = atoi(JSONPacket_GetValue(event, "x")),
                    .y = atoi(JSONPacket_GetValue(event, "y")),
                    .w = 120,
                    .h = 100
                };
                p->texture = FE_LoadResource(FE_RESOURCE_TYPE_TEXTURE, "game/sprites/doge.png");
                mstrcpy(p->username, JSONPacket_GetValue(event, "username"));
                FE_List_Add(&players, p);

                info("Player %s has joined", JSONPacket_GetValue(event, "username"));
            } else if (mstrcmp(cmd, "removeplayer") == 0) {
                // remove player from list and free memory
                char *user = JSONPacket_GetValue(event, "username");
                for (FE_List *p = players; p; p = p->next) {
                    player *pl = p->data;
                    if (mstrcmp(pl->username, user) == 0) {
                        FE_DestroyResource(pl->texture->path);
                        FE_List_Remove(&players, p);
                        free(pl);
                        info("Player %s has left", user);
                        return;
                    }
                }
            }
            break;

        case PACKET_TYPE_SERVERSTATE: ;
             // parse as json
            LoadServerState(event, &players);
            break;

        case PACKET_TYPE_KICK:
            // set the game disconnect info to the kick message
            PresentGame->DisconnectInfo = (FE_DisconnectInfo) {
                .set = true,
                .reason = mstradd("Reason: ", JSONPacket_GetValue(event, "reason")),
            };
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
                .reason = "Disconnected from server (Timed out)",
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

    if (client)
        enet_host_destroy(client);
    enet_deinitialize();
    
    if (GamePlayer)
        FE_DestroyPlayer(GamePlayer);
    GamePlayer = 0;
    GameCamera = 0;
    if (world)
        SDL_DestroyTexture(world);
    world = 0;
}

bool LoadConfig()
{
    // Loads a game like usual
    FE_UI_ClearElements(PresentGame->UIConfig.ActiveElements);

    FE_LoadedMap *m = 0;
    if (!(m = FE_LoadMap(MAP))) {
        warn("Failed to load map %s", MAP);
        return false;
    }
    FE_Game_SetMap(m);

    // camera setup
	GameCamera = FE_CreateCamera();
	GameCamera->maxzoom = 2.0f;
	GameCamera->minzoom = 1.0f;

	// player setup
	GamePlayer = FE_CreatePlayer(40, 8, 18, (SDL_Rect){PresentGame->MapConfig.PlayerSpawn.x, PresentGame->MapConfig.PlayerSpawn.y, 120, 100});
	GameCamera->follow = &GamePlayer->render_rect;

	// test particle system
	FE_CreateParticleSystem(
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
        SDL_RenderClear(PresentGame->Renderer);
        FE_UI_Render();
        SDL_RenderPresent(PresentGame->Renderer);
    } else {
        PresentGame->Timing.RenderTime = SDL_GetPerformanceCounter();

        if (PresentGame->DebugConfig.LightingEnabled)
            SDL_SetRenderTarget(PresentGame->Renderer, world);

        SDL_RenderClear(PresentGame->Renderer);
        FE_RenderMapBackground(GameCamera);
        FE_RenderParticles(GameCamera);
        FE_RenderLoadedMap(GameCamera);
        FE_Trigger_Render(GameCamera);
        FE_GameObject_Render(GameCamera);
        FE_Trigger_Render(GameCamera);
        RenderPlayers();
        FE_RenderPlayer(GamePlayer, GameCamera);

        if (PresentGame->DebugConfig.LightingEnabled)
            FE_Light_Render(GameCamera, world);

        FE_UI_Render();
        SDL_RenderPresent(PresentGame->Renderer);

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
                        // todo change pause menu
                        break;
                    }
                    else if (keyboard_state[SDL_SCANCODE_GRAVE]) {
                        SDL_StartTextInput();
                        PresentGame->StartedInput = true;
                        FE_Console_Show();
                        break;
                    }

                    if ((int)event.key.keysym.scancode == FE_Key_Get("LEFT")) {
                        SendKeyDown(KEY_LEFT);
                        break;
                    } else if ((int)event.key.keysym.scancode == FE_Key_Get("RIGHT")) {
                        SendKeyDown(KEY_RIGHT);
                        break;
                    } else if ((int)event.key.keysym.scancode == FE_Key_Get("JUMP")) {
                        SendKeyDown(KEY_JUMP);
                        break;
                    }
                
                    if (keyboard_state[FE_Key_Get("ZOOM IN")])
                        FE_Camera_SmoothZoom(GameCamera, 0.5, 250);
                    else if (keyboard_state[FE_Key_Get("ZOOM OUT")])
                        FE_Camera_SmoothZoom(GameCamera, -0.5, 250);
                    
                    if (keyboard_state[SDL_SCANCODE_I]) {
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

                    if (keyboard_state[SDL_SCANCODE_L])
                        FE_Light_Toggle(GamePlayer->Light);
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

	PresentGame->Timing.UpdateTime = SDL_GetPerformanceCounter();
	FE_UpdateTimers();
	FE_UpdateParticles();
	FE_UpdatePlayer(GamePlayer);
	FE_UpdateAnimations();
	FE_Prefab_Update();
	FE_UpdateCamera(GameCamera);
	PresentGame->Timing.UpdateTime = ((SDL_GetPerformanceCounter() - PresentGame->Timing.UpdateTime) / SDL_GetPerformanceFrequency()) * 1000;
}

int InitClient(char *addr, int port, char *username)
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

    if (!LoadConfig()) {
        warn("Could not load game config");
        Disconnect();
        return -1;
    }

    json_packet *p = JSONPacket_Create();
    JSONPacket_Add(p, "msg", username);
    SendPacket(peer, PACKET_TYPE_MESSAGE, p);
    JSONPacket_Destroy(p);

    if (PresentGame->DebugConfig.ShowTiming) {
        ping_label = FE_UI_CreateLabel(0, "pong", 256, 0, 180, COLOR_BLACK);
        ping_label->showbackground = true;
        FE_UI_AddElement(FE_UI_LABEL, ping_label);
    }


    return 0;
}