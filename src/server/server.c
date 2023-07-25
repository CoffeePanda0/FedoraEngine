#include <FE_Common.h>

#include "../common/net/include/packet.h"
#include "../common/net/include/packet.h"

#include "include/rcon.h"
#include "include/message.h"
#include "include/include.h"

#include "../common/entity/include/timer.h"
#include "../common/entity/include/gameobject.h"
#include "../common/world/include/map.h"

static const size_t key_count = 3;

static ENetHost *server;

static FE_List *clients;
static size_t client_count;

static FE_StrArr *ip_spam;

/* sends a packet to all clients other than the one who sent it
	(set to 0 to send to all clients) */
void BroadcastPacket(ENetPeer *peer, FE_Net_Packet *p)
{
	for (FE_List *l = clients; l; l = l->next) {
		client_t *c = l->data;
		if (!peer || c->peer != peer) {
			FE_Net_Packet_Send(c->peer, p, false);
		}
	}
}

static size_t CheckClientCount(char *ip)
{
	size_t count = 0;
	for (FE_List *l = clients; l; l = l->next) {
		client_t *c = l->data;
		if (mstrcmp(c->ip, ip) == 0) {
			count++;
		}
	}
	return count;
}

// gets a client info by their peer
static client_t *GetClient(ENetPeer *peer)
{
	for (FE_List *l = clients; l; l = l->next) {
		client_t *c = l->data;
		if (c->peer == peer) {
			return c;
		}
	}
	return 0;
}

// handles a client connecting
static void HandleConnect(ENetEvent *event)
{
	// set default values
	client_t *c = xmalloc(sizeof(client_t));
	enet_peer_get_ip(event->peer, c->ip, 32);

	c->peer = event->peer;

	c->authenticated = false;
	mmemset(c->username, 0, sizeof(c->username));

	c->has_rcon = false;
	c->rcon_attempts = 0;

	c->packets_sent = 0;

	c->limited_count = 0;
	c->messages_sent = 0;
	c->muted = RCON_CheckMute(c->ip);

	c->last_location = vec(PresentGame->MapConfig.PlayerSpawn.x, PresentGame->MapConfig.PlayerSpawn.y);
	c->last_velocity = vec(-1, -1);
	
	// Create virtual player
	mmemset(c->held_keys, 0, key_count);
	c->player = xmalloc(sizeof(FE_PlayerProperties));

	c->player->moveforce = 150;
	c->player->maxspeed = 50;
	c->player->jumpforce = 180;
	c->player->jump_elapsed = 0;
	c->player->jump_started = false;

	c->player->PhysObj = FE_Physics_CreateBody(50, (SDL_Rect){c->last_location.x, c->last_location.y, 120, 100});
	FE_Physics_AddBody(c->player->PhysObj);

	FE_List_Add(&clients, c);

	// Check how many clients are connected from this IP
	size_t count = CheckClientCount(c->ip);

	if (count > 3) {
		// too many clients from this IP
		FE_StrArr_Add(ip_spam, c->ip);
		
		// check how many times this IP has spammed
		size_t spam_count = 0;
		for (size_t i = 0; i < ip_spam->items; i++) {
			if (mstrcmp(ip_spam->data[i], c->ip)) {
				spam_count++;
			}
		}
		// If they've spammed more than 3 times, ban them
		if (spam_count >= 3)
			RCON_Ban(c, "Too many clients from this IP");
		else
			RCON_Kick(c, "Too many clients from this IP");
		return;
	}

	// wait for client to send / receive state
	bool flag = AuthenticateClient(server, c, &clients, &client_count);
	if (!flag) {
		// client failed to authenticate
		FE_Physics_Remove(c->player->PhysObj);
		free(c->player);

		FE_List_Remove(&clients, c);
		free(c);
		info("[SERVER]: Client failed to authenticate");
		return;
	}

	enet_packet_destroy(event->packet);
}

// handles the first press of a key down
static void HandleKeyDown(held_keys key, client_t *c)
{
	if (key < key_count)
		c->held_keys[key] = 1;
}

// handles the first press of a key up
static void HandleKeyUp(held_keys key, client_t *c)
{
	if (key < key_count)
		c->held_keys[key] = 0;
}

/* Sends the server time to a client */
static void SendServerTime(FE_Net_RcvPacket *packet, client_t *c)
{
	FE_Net_Packet *p = FE_Net_Packet_Create(PACKET_SERVER_TIME);
	FE_Net_Packet_AddLong(p, packet->timestamp);
	FE_Net_Packet_Send(c->peer, p, true);
	enet_host_flush(server);
}

/* Handles receiving a packet from a client */
static void HandleRecieve(ENetEvent *event)
{
	client_t *c = GetClient(event->peer);
	c->packets_sent++;

	if (c->packets_sent > 60)
		RCON_Kick(c, "Too many packets sent");
	if (!c->authenticated)
		RCON_Kick(c, "Client not authenticated");

	/* Parse the packet */
	FE_Net_RcvPacket *packet = FE_Net_GetPacket(event);
	if (!packet) {
		warn("[SERVER]: Invalid packet from %s", c->username);
		return;
	}

	switch (packet->type) {
		case PACKET_CLIENT_CHAT:
			Server_ParseMessage(c, packet);
		break;

		case PACKET_CLIENT_KEYDOWN:
			HandleKeyDown(FE_Net_GetInt(packet), c);
		break;

		case PACKET_CLIENT_KEYUP:
			HandleKeyUp(FE_Net_GetInt(packet), c);
		break;

		case PACKET_CLIENT_RCON: ;
			RCON_ParseRequest(packet, c, clients);
		break;

		case PACKET_CLIENT_TIMEREQUEST: ;
			SendServerTime(packet, c);
		break;

		default:
			break;
	}

	FE_Net_DestroyRcv(packet);
}

static void HandleDisconnect(ENetEvent *event)
{
	client_t *c = GetClient(event->peer);
	if (!c) return;
	info("[SERVER]: Client %s disconnected", c->username);

	// send packet to all clients to remove the player
	FE_Net_Packet *p = FE_Net_Packet_Create(PACKET_SERVER_DESPAWN);
	FE_Net_Packet_AddString(p, c->username);
	BroadcastPacket(c->peer, p);
	FE_Net_Packet_Destroy(p);

	event->peer->data = NULL;
	FE_Physics_Remove(c->player->PhysObj);
	free(c->player);

	FE_List_Remove(&clients, c);
	free(c);
	client_count--;
}

static char *GetExternalIP()
{
	FILE *curl;
    if ((curl = popen("curl -s https://api.ipify.org", "r")) == NULL)
        return 0;

    char* ip = malloc(99);
    if (fgets(ip, 99, curl) == NULL){
        warn("[SERVER]: You are not connected to the internet");
        return 0;
    }
	
	return ip;
}

void FE_DestroyServer()
{
	if (server) {
		// free clients
		for (FE_List *l = clients; l; l = l->next) {
			client_t *c = l->data;
			FE_Physics_Remove(c->player->PhysObj);
			enet_peer_disconnect(c->peer, 0);
			c->peer = 0;
			free(c);
		}
		FE_List_Destroy(&clients);
		clients = 0;

		// host server for a few seconds to make sure clients disconnect
		ENetEvent event;
		enet_host_service(server, &event, 5000);

		free(clients);
		clients = 0;

		enet_deinitialize();
		enet_host_destroy(server);
		server = 0;

		client_count = 0;

		if (ip_spam)
			FE_StrArr_Destroy(ip_spam);
		ip_spam = 0;

		RCON_Destroy();
		DestroyServerConfig();
	}
	exit(0);
}

// initialises the server
int InitServer()
{
	RCON_Init();
	LoadServerConfig();

	// load the map
	FE_LoadedMap *m;
	if (!(m = FE_Map_Load_Basic(server_config.map))) {
		warn("[SERVER]: Failed to load map");
		return -1;
	}
    FE_Game_SetMap(m);

	if (enet_initialize () != 0) {
		warn("[SERVER]: An error occurred while initializing ENet");
		return -1;
	}

	ENetAddress address = {0};
	address.host = ENET_HOST_ANY;
	address.port = server_config.port;

	server = enet_host_create(&address, server_config.max_players, 1, 0, 0);

	if (!server) {
		warn("[SERVER]: An error occurred while trying to create an ENet server host.");
		return -1;
	}
	info("[SERVER]: Started Server");

	// show the server ip and port
    char *ip = GetExternalIP();
	if (ip) {
		printf("[SERVER] EXTERNAL IP: %s\n", ip);
		free(ip);
	}

	ip_spam = FE_StrArr_Create();

	FE_ResetDT();

	// FE_GameObject_Create_Basic((SDL_Rect){200,1000,100,100}, 100, "doge.png", "bob");

    return 0;
}

static void HostServer()
{
	ENetEvent event;
    while (enet_host_service(server, &event, 0) > 0) {
        switch (event.type)
        {
            case ENET_EVENT_TYPE_CONNECT: ;
				// check that player hasn't already connected
				char ip[32];
				enet_peer_get_ip(event.peer, ip, 32);

				if (RCON_CheckIP(ip)) {
					enet_peer_disconnect_now(event.peer, DISC_NOCON);
					break;
				}

                info("[SERVER]: A new client connected from %s:%u.",
                    ip, event.peer->address.port);
                HandleConnect(&event);
                break;
            break;

            case ENET_EVENT_TYPE_RECEIVE:
                HandleRecieve(&event);
                enet_packet_destroy(event.packet);
                break;
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

static void ResetPacketCount()
{
	// only reset packet count once per second
	static float timer = 0;
	timer += FE_DT;

	if (timer >= 1) {
		timer -= 1;
		for (FE_List *l = clients; l; l = l->next) {
			client_t *c = l->data;
			c->packets_sent = 0;
		}
	}
}

// checks each player's held keys, and applies the action associated by those keys being held down
static void UpdateHeldKeys()
{
	for (FE_List *l = clients; l; l = l->next) {
		
		client_t *c = l->data;
		FE_PlayerProperties *p = c->player;

		if (c->held_keys[0])
			FE_Player_Move(p, vec(-1, 0));
		if (c->held_keys[1])
			FE_Player_Move(p, vec(1, 0));
		if (c->held_keys[2]) {
			if (!p->jump_started)
				FE_Player_StartJump(p);
			else
				FE_Player_UpdateJump(p);
		}
	}
}

void SendSnapshot()
{
	/* Only send snapshots depending on snapshot rate */
	static uint64_t last_snapshot = 0;
	uint64_t time_passed = FE_GetTicks64() - last_snapshot;

	uint64_t snapshot_rate_ms = 1000 / server_config.snapshot_rate;
	uint64_t dif = 0;

	if (time_passed < snapshot_rate_ms)
		return;
	else
		dif = time_passed - snapshot_rate_ms;


	FE_Net_Packet *snapshot = FE_Net_Packet_Create(PACKET_SERVER_UPDATE);

	FE_Net_Array *player_snapshot = FE_Net_Array_Create(); /* Store player changes in array here */
	FE_Net_Array *object_snapshot = FE_Net_Array_Create(); /* Store object changes in array here */

	/* Check if any clients have moved, if so then send them the new position */
	for (FE_List *l = clients; l; l = l->next) {
		client_t *c = l->data;
		if (
		/* x-y any changes */	!vec2_cmp(c->last_location, vec(c->player->PhysObj->body.x, c->player->PhysObj->body.y)) || 
		/* velocity changes to 0 */ (c->last_velocity.x != 0 && c->player->PhysObj->velocity.x == 0) || (c->last_velocity.y != 0 && c->player->PhysObj->velocity.y == 0)
		) {
			
			/* Record the last sent location so we don't keep resending */
			c->last_location = vec(c->player->PhysObj->body.x, c->player->PhysObj->body.y);
			c->last_velocity = vec(c->player->PhysObj->velocity.x, c->player->PhysObj->velocity.y);

			/* Add player username */
			FE_Net_Array_Add(player_snapshot, (Value){.type = KEY_STRING, .value.str = mstrdup(c->username)});
			
			/* Attatch the player's position */
			FE_Net_Array_Add(player_snapshot, (Value){.type = KEY_INT, .value.i = (int)c->last_location.x});
			FE_Net_Array_Add(player_snapshot, (Value){.type = KEY_INT, .value.i = (int)c->last_location.y});

			/* Attatch velocity */
			FE_Net_Array_Add(player_snapshot, (Value){.type = KEY_FLOAT, .value.f = c->last_velocity.x});
			FE_Net_Array_Add(player_snapshot, (Value){.type = KEY_FLOAT, .value.f = c->last_velocity.y});
		}
	}
	FE_Net_Packet_AttatchArray(snapshot, player_snapshot); /* Add the finished array to the packet */

	for (FE_List *l = FE_GameObjects; l; l = l->next) {
		FE_GameObject *obj = l->data;
		if (mstrcmp(obj->name, "prefab") == 0) continue; // ignore prefabs

		/* Check if the object has moved */
		if (obj->last_position.x != obj->phys->body.x || obj->last_position.y != obj->phys->body.y) {

			/* Attatch the object's id to the packet */
			FE_Net_Array_Add(object_snapshot, (Value){.type = KEY_INT, .value.i = obj->id});

			/* Attatch object position */
			FE_Net_Array_Add(object_snapshot, (Value){.type = KEY_INT, .value.i = (int)obj->phys->body.x});
			FE_Net_Array_Add(object_snapshot, (Value){.type = KEY_INT, .value.i = (int)obj->phys->body.y});

			/* Record the last sent location so we don't keep resending */
			obj->last_position = vec(obj->phys->body.x, obj->phys->body.y);
		}
	}
	FE_Net_Packet_AttatchArray(snapshot, object_snapshot); /* Add the finished array to the packet */

	last_snapshot = FE_GetTicks64() - dif;

	/* Send the snapshot */
	BroadcastPacket(0, snapshot);

	/* Clean up */
	FE_Net_Array_Destroy(player_snapshot);
	FE_Net_Array_Destroy(object_snapshot);
	FE_Net_Packet_Destroy(snapshot);
}

void UpdateServer()
{
	PresentGame->Timing.UpdateTime = FE_QueryPerformanceCounter();
	
	// Reset spam counters
	ResetPacketCount();
	ResetMessageCount(&clients);

	HostServer(); // host the server

	UpdateHeldKeys(); // check which keys are being held down, and apply the action associated with those keys

	FE_Physics_Update(); // run main game loop

	FE_Timers_Update(); // update timers (if any are running)

	SendSnapshot(); // send a snapshot of the game to all clients

	PresentGame->Timing.UpdateTime = ((FE_QueryPerformanceCounter() - PresentGame->Timing.UpdateTime) / FE_QueryPerformanceFrequency()) * 1000;
}
