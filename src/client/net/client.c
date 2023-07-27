#include <FE_Client.h>

#include "../../common/net/include/packet.h"
#include "include/client.h"
#include "include/message.h"
#include "include/include.h"

#include "../ui/include/chatbox.h"
#include "../ui/include/menu.h"

static FE_Net_Client Client = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

#define _connection_timeout 5000

static uint64_t LastUpdate = 0;

// Game vars
static FE_Player *GamePlayer;
static FE_Camera *GameCamera;
static SDL_Texture *world;

// ui elements
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

    Client.Peer = enet_host_connect(Client.Client, &address, 1, 0);
    if (!Client.Peer) {
        info("No available peers for initiating an ENet connection!");
        return false;
    }

    if (enet_host_service(Client.Client, &event, _connection_timeout) > 0 && event.type == ENET_EVENT_TYPE_CONNECT) {
        info("Connection to %s:%i succeeded", addr, port);
        return true;
    } else {
        enet_peer_reset(Client.Peer);
        info("Connection to %s:%i failed", addr, port);
    }
    return false;
}

static void CalculateJitter()
{
    /* Keep a buffer of the last five differences between packet timings to calculate jitter */
    static long diffs[5] = {0};
    static size_t index = 0;

    /* Calculate difference */
    long diff = Client.LatestPacket - Client.LastPacket;

    diffs[index] = diff;
    index = (index + 1) % 5;

    /* Calculate jitter between differences */
    int jitter = 0;
    for (size_t i = 0; i < 4; i++) {
        jitter += diffs[i] - diffs[i + 1];
    }
    jitter /= 4;

    /* Normalise */
    Client.Jitter = abs(jitter);
    
    /* Log data every 0.5s FOR TESTING  */
    if (true) {
        static float timer = 0;
        timer += FE_DT;
        if (timer < 0.5f)
            return;

        timer -= 0.5f;
      // debugging:  printf("PING: %f | JITTER: %i\n",  Client.Ping, Client.Jitter);
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

            /* Update timing information*/
            Client.LastPacket = Client.LatestPacket;
            Client.LatestPacket = packet->timestamp;

            /* Expected types for player: str, int, int, float, float */
            FE_Net_Array *player_array = FE_Net_GetArray(packet, (uint8_t[]) {KEY_STRING, KEY_INT, KEY_INT, KEY_FLOAT, KEY_FLOAT}, 5, true);
            if (!player_array) {
                warn("Invalid player array");
                return;
            }

            if (player_array->size > 0) {

                /* Check if multiple player updates have occured */
                size_t update_count = (size_t)(player_array->size / 5);
                for (size_t i = 0; i < update_count; i++) {

                    /* Check if the updated player is us */
                    char *user = FE_Net_Array_GetString(player_array);

                    if (mstrcmp(user, Client.Username) == 0) {
                        /* Update our player */

                        /* Save last position for interpolation */
                        GamePlayer->player->PhysObj->last_position = GamePlayer->player->PhysObj->position;

                        /* New position */
                        GamePlayer->player->PhysObj->position.x = FE_Net_Array_GetInt(player_array);
                        GamePlayer->player->PhysObj->position.y = FE_Net_Array_GetInt(player_array);
                        /* New velocity */
                        GamePlayer->player->PhysObj->velocity.x = FE_Net_Array_GetFloat(player_array);
                        GamePlayer->player->PhysObj->velocity.y = FE_Net_Array_GetFloat(player_array);

                        LastUpdate = FE_GetTicks64();

                    } else {
                        /* Find player in list, update position */
                        for (FE_List *l = players; l; l = l->next) {
                            player *p = l->data;
                            if (mstrcmp(p->username, user) == 0) {
                                p->last_position = vec(p->rect.x, p->rect.y);

                                p->s.time_rcv = FE_GetTicks64();
                                p->s.new_position = vec(FE_Net_Array_GetInt(player_array), FE_Net_Array_GetInt(player_array));

                                /* Skip pointer to end of this character */
                                player_array->ptr += 2;
                                break;
                            }
                        }
                    }
                }
            }

            FE_Net_Array *object_array = FE_Net_GetArray(packet, (uint8_t[]) {KEY_INT, KEY_INT, KEY_INT}, 3, true);
            if (!object_array) {
                warn("Invalid object array");
                return;
            }

            if (object_array->size > 0) {

                /* Check if multiple objects in the same update have moved */
                size_t update_count = (size_t)(object_array->size / 3);

                for (size_t i = 0; i < update_count; i++) {
                    /* Update object */
                    size_t id = FE_Net_Array_GetInt(object_array);
                    int x = FE_Net_Array_GetInt(object_array);
                    int y = FE_Net_Array_GetInt(object_array);

                    /* Find object in list, update position */
                    for (FE_List *l = FE_GameObjects; l; l = l->next) {
                        FE_GameObject *obj = l->data;
                        if (obj->id == id) {
                            obj->phys->position.x = x;
                            obj->phys->position.y = y;
                            FE_UPDATE_RECT(obj->phys->position, &obj->phys->body);
                            break;
                        }
                    }
                }
            }

            /* Clean up */
            FE_Net_Array_Destroy(player_array);
            FE_Net_Array_Destroy(object_array);
            
        break;

        case PACKET_SERVER_SERVERMSG: ;
            char *data = FE_Net_GetString(packet);
            char *servermsg = mstradd("SERVER MESSAGE: ", data);
            info(servermsg);
            
            FE_UI_ChatboxMessage(servermsg);

            free(servermsg);
            free(data);
        
        break;

        case PACKET_SERVER_SPAWN: ;
            char *username = FE_Net_GetString(packet);
            int x_ = FE_Net_GetInt(packet);
            int y_ = FE_Net_GetInt(packet);

            // add player to list
            player *p = xmalloc(sizeof(player));
            p->rect = (SDL_Rect) {
                .x = x_,
                .y = y_,
                .w = 120,
                .h = 100
            };
            p->last_position = vec(x_, y_);
            p->s.new_position = vec(x_, y_);
            p->s.time_rcv = packet->timestamp;

            p->texture = FE_LoadResource(FE_RESOURCE_TYPE_TEXTURE, "game/sprites/doge.png");

            mstrcpy(p->username, username);
            FE_List_Add(&players, p);

            // show player join message
            char *msg = mstradd(username, " has joined the game");
            FE_UI_ChatboxMessage(msg);

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
                    FE_UI_ChatboxMessage(msg);
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

            FE_UI_ChatboxMessage(chat);
            free(chat);
            free(_username);
            free(_msg);
    
            break;
        break;

        case PACKET_SERVER_SNAPSHOTRATE: ;
            // set the snapshot rate
            Client.SnapshotRate = FE_Net_GetInt(packet);
        break;

        case PACKET_SERVER_TIME: ;
            ParseTimeResponse(&Client, packet);
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

static void HostClient()
{
    ENetEvent event;
    while (enet_host_service(Client.Client, &event, 0) > 0) {
        switch (event.type) {
            case ENET_EVENT_TYPE_RECEIVE: ;
                HandleRecieve(&event);
            break;
            case ENET_EVENT_TYPE_DISCONNECT:
                HandleDisconnect(&event, &Client);
            break;
            case ENET_EVENT_TYPE_DISCONNECT_TIMEOUT:
                HandleDisconnect(&event, &Client);
            break;

            default: break;
        }
        enet_packet_destroy(event.packet);
    }
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

    if (Client.Connected)
        DisconnectClient(&Client);

    if (Client.Username)
        free(Client.Username);

    if (chatbox)
        FE_UI_DestroyChatbox();
    chatbox = 0;

    if (Client.Client)
        enet_host_destroy(Client.Client);
    enet_deinitialize();
    
    if (GamePlayer)
        FE_Player_Destroy(GamePlayer);
    GamePlayer = 0;
    
    if (world)
        SDL_DestroyTexture(world);
    world = 0;

    GameCamera = 0;
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

	/* Snow particles */
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
    if (!Client.Connected) {
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


        FE_UI_RenderChatbox();
        FE_UI_Render();

        SDL_RenderPresent(PresentGame->Client->Renderer);

	    PresentGame->Timing.RenderTime = ((FE_QueryPerformanceCounter() - PresentGame->Timing.RenderTime) / FE_QueryPerformanceFrequency()) * 1000;
    }
}

void InterpolateState()
{
    if (FE_FPS > Client.SnapshotRate) {
        float snapshot_ms = 1.0f / Client.SnapshotRate;

        uint64_t time = FE_GetTicks64();

        /* Interpolate our player state */
        if (LastUpdate != 0) {
            /* Calculate the difference since the last snapshot was recieved */
            float time_taken = (time - LastUpdate) / 1000.0f;
            
            /* If there have been no updates to process */
            if (time_taken > snapshot_ms) {
                LastUpdate = 0;
                return;
            }

            float alpha = time_taken / snapshot_ms;

            vec2 pos = vec2_lerp(GamePlayer->player->PhysObj->last_position, GamePlayer->player->PhysObj->position, alpha);
            FE_UPDATE_RECT(pos, &GamePlayer->player->PhysObj->body);
        }

        /* Interoplate positions of other players */
        for (FE_List *l = players; l; l = l->next) {
            player *p = l->data;
            if (p->s.time_rcv != 0) {
                float time_taken = (time - p->s.time_rcv) / 1000.0f;

                if (time_taken > snapshot_ms) {
                    p->s.time_rcv = 0;
                    continue;
                }

                float alpha = time_taken / snapshot_ms;
                vec2 pos = vec2_lerp(p->last_position, p->s.new_position, alpha);
                FE_UPDATE_RECT(pos, &p->rect);
            }
        }
    }

}

void ClientUpdate()
{
    if (!Client.Connected && Client.Client) {
        Exit();
    }
    if (FE_FPS == 0 || !Client.Connected) {
		return;
	}


    float event_time = FE_QueryPerformanceCounter();
    /* Take inputs from the client */
    ClientEventHandle(GameCamera, GamePlayer, &Client);
    PresentGame->Timing.EventTime = ((FE_QueryPerformanceCounter() - event_time) / (FE_QueryPerformanceFrequency()) * 1000);

	float update_time = FE_QueryPerformanceCounter();

    /* Check if we need to poll for time */
    KeepServerTime(&Client);

    /* Host the client (listen for packets from server) */
    HostClient();

    /* Calculate jitter to use in later calculations */
    CalculateJitter();

	FE_Dialogue_Update();
	FE_Timers_Update();
	FE_Particles_Update(GameCamera);

    GamePlayer->player->PhysObj->grounded = (GamePlayer->player->PhysObj->velocity.y == 0) ? true : false;
    
	FE_Player_Update(GamePlayer);

    /* Use linear interpolation on the player positions */
    InterpolateState();
    
	FE_Animations_Update();
	FE_Prefab_Update();
	FE_UpdateCamera(GameCamera);
    
	PresentGame->Timing.UpdateTime = ((FE_QueryPerformanceCounter() - update_time) / (FE_QueryPerformanceFrequency()) * 1000);
}


int InitClient(char *addr, int port, char *_username)
{
    /* Initialise networking library */
    if (enet_initialize() != 0) {
        warn("An error occurred while initializing ENet!");
        return -1;
    }

    /* Create the client host */
    Client.Client = enet_host_create(NULL, 1, 1, 0, 0);
    if (!Client.Client) {
        warn("An error occurred while trying to create an ENet client host!");
        return -1;
    }
    info("Client initialised");

    /* Attempt to connect the client to the server (establishes the peer for the connection)*/
    Client.Connected = Connect(port, addr);
    if (!Client.Connected) {
        warn("Could not connect to server");
        return -1;
    }

    /* Sync state with server */
    if (Client_Connect(_username, &Client, &players) == false) {
        warn("Unable to authenticate with server");
        DisconnectClient(&Client);
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
        DisconnectClient(&Client);
        return -1;
    }

    chatbox = FE_UI_CreateChatbox(&Client_SendMesage, Client.Peer);
    FE_UI_ToggleChatbox();

    return 0;
}