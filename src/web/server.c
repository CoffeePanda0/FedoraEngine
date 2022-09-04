#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include "net.h"
#include "include/internal.h"
#include "../include/game.h"
#include "../ext/json-maker/json-maker.h"

#define MAP "test"
#define rcon_password "password"

static size_t key_count = 3;

typedef struct {
	ENetPeer *peer;
	bool set_username;
	char username[24];

	FE_Player *player;
	vec2 last_location; // used to check if the player has moved
	vec2 last_velocity; // last velocity sent to client

	bool has_rcon; // if the client has rcon access
	size_t rcon_attempts; // how many times they have tried to rcon

	size_t packets_sent; // count of packets sent by client in last frame

	uint8_t held_keys[3]; // contains state of each key
} client;

static ENetHost* server;

static FE_List *clients;
static size_t client_count;

static int last_id = 0; // last client id

static FE_UI_Label *status;

// gets a client info by their peer
static client *GetClient(ENetPeer *peer)
{
	for (FE_List *l = clients; l; l = l->next) {
		client *c = l->data;
		if (c->peer == peer) {
			return c;
		}
	}
	return 0;
}

static void ServerState(char *buffer, size_t size)
{
	if (!clients)
		return;

	// create json packet
	buffer = json_objOpen(buffer, NULL, &size);
	buffer = json_int(buffer, "type", PACKET_TYPE_SERVERSTATE, &size);

	buffer = json_arrOpen(buffer, "players", &size);
	for (FE_List *l = clients; l; l = l->next) {
		client *c = l->data;
		if (c->set_username) {
			buffer = json_objOpen(buffer, NULL, &size);
			buffer = json_str(buffer, "username", c->username, &size);
			buffer = json_int(buffer, "x", c->player->PhysObj->body.x, &size);
			buffer = json_int(buffer, "y", c->player->PhysObj->body.y, &size);
			buffer = json_objClose(buffer, &size);
		}
	}
	buffer = json_arrClose(buffer, &size);
	buffer = json_objClose(buffer, &size);
}

// handles a client connecting
static void HandleConnect(ENetEvent *event)
{
	// send the current game state
	if (clients != 0) {
		size_t size = 32 * (client_count +1);
		char *buffer = xcalloc(size, 1);

		ServerState(buffer, size);

		size_t len = mstrlen(buffer);
		ENetPacket *packet = enet_packet_create(buffer, len, ENET_PACKET_FLAG_RELIABLE);
		enet_peer_send(event->peer, 0, packet);

		free(buffer);
	}
	client_count++;

	client *c = xmalloc(sizeof(client));
	c->peer = event->peer;
	c->set_username = false;
	c->has_rcon = false;
	c->rcon_attempts = 0;
	memset(c->held_keys, 0, key_count);
	c->packets_sent = 0;
	c->player = FE_CreatePlayer(40, 8, 18, (SDL_Rect){PresentGame->MapConfig.PlayerSpawn.x, PresentGame->MapConfig.PlayerSpawn.y, 120, 100});
	c->last_location = vec(PresentGame->MapConfig.PlayerSpawn.x, PresentGame->MapConfig.PlayerSpawn.y);
	c->username[0] = '\0';

	FE_List_Add(&clients, c);
}

static void UpdateStatus()
{
	char status_text[256];
	sprintf(status_text, "Connected clients: %zu", client_count);
	FE_UI_UpdateLabel(status, status_text);
}

// sends a "server message" to one client
static void ServerMSG(client *client, char *msg)
{
	json_packet *p = JSONPacket_Create();
	JSONPacket_Add(p, "msg", msg);
	SendPacket(client->peer, PACKET_TYPE_SERVERMSG, p);

	JSONPacket_Destroy(p);
}

/* sends a packet to all clients other than the one who sent it
	(set to 0 to send to all clients) */
static void BroadcastPacket(ENetPeer *peer, packet_type type, json_packet *p)
{
	for (FE_List *l = clients; l; l = l->next) {
		client *c = l->data;
		if (!peer || c->peer != peer) {
			SendPacket(c->peer, type, p);
		}
	}
}

static void KickPlayer(client *client, char *reason)
{
	info("[SERVER]: Kicking player [%s:%i] %s (%s)", client->peer->address.host, client->peer->address.port, client->username, reason);
	enet_peer_disconnect(client->peer, 0);
}

// handles the first press of a key down
static void HandleKeyDown(held_keys key, client *c)
{
	if (key < key_count)
		c->held_keys[key] = 1;
}

// handles the first press of a key up
static void HandleKeyUp(held_keys key, client *c)
{
	if (key < key_count)
		c->held_keys[key] = 0;
}

// handles receiving a message from a client
static void HandleRecieve(ENetEvent *event)
{
	client *c = GetClient(event->peer);
	c->packets_sent++;

	if (c->packets_sent > 30) // only allow 30 packets per second
		return;
	if (c->packets_sent > 60)
		KickPlayer(c, "Too many packets sent");
	if (!c->set_username && (PacketType(event) != PACKET_TYPE_MESSAGE))
		KickPlayer(c, "Username not set");

	switch (PacketType(event)) {
		case PACKET_TYPE_MESSAGE:
			// first ever packet is the username
			if (!c->set_username) {
				c->set_username = true;

				// check if the username is already taken
				bool set = false;
				for (FE_List *l = clients; l; l = l->next) {
					client *cl = l->data;
					if (strcmp(cl->username, JSONPacket_GetValue(event, "msg")) == 0) {
						// if so, set it to client + last_id
						sprintf(c->username, "client%d", last_id++);
						ServerMSG(c, "Username taken, setting to client#");
						set = true;
					}
				}
				if (!set) strncpy(c->username, JSONPacket_GetValue(event, "msg"), 23);
				info("[SERVER]: Client username set to %s", c->username);

				// send the client their id
				json_packet *p = JSONPacket_Create();
				JSONPacket_Add(p, "cmd", "username");
				JSONPacket_Add(p, "username", c->username);
				SendPacket(c->peer, PACKET_TYPE_SERVERCMD, p);
				JSONPacket_Destroy(p);

				// send packet to all clients to add the new player
				char x[16], y[16];
				sprintf(x, "%f", c->player->PhysObj->position.x);
				sprintf(y, "%f", c->player->PhysObj->position.y);

				p = JSONPacket_Create();
				JSONPacket_Add(p, "cmd", "addplayer");
				JSONPacket_Add(p, "username", c->username);
				JSONPacket_Add(p, "x", x);
				JSONPacket_Add(p, "y", y);
				BroadcastPacket(c->peer, PACKET_TYPE_SERVERCMD, p);
				JSONPacket_Destroy(p);

				UpdateStatus();
				break;
			}

			// if message is recieved from a client, send it to all clients
			json_packet *packet = JSONPacket_Create();
			JSONPacket_Add(packet, "username", c->username);
			JSONPacket_Add(packet, "msg", JSONPacket_GetValue(event, "msg"));
			BroadcastPacket(event->peer, PACKET_TYPE_MESSAGE, packet);
			JSONPacket_Destroy(packet);
			
			break;

		case PACKET_TYPE_KEYDOWN:
			HandleKeyDown(JSONPacket_GetInt(event, "key"), c);
		break;

		case PACKET_TYPE_KEYUP:
			HandleKeyUp(JSONPacket_GetInt(event, "key"), c);
		break;


		case PACKET_TYPE_RCONREQUEST:
			if (c->has_rcon) return;
			if (c->rcon_attempts >= 3) { // max attempts
				ServerMSG(c, "Too many RCON attempts");
				return;	
			}
			if (mstrcmp(JSONPacket_GetValue(event, "password"), rcon_password) == 0) {
				c->has_rcon = true;
				ServerMSG(c, "RCON granted");
			} else {
				ServerMSG(c, "Incorrect RCON password");
			}
			c->rcon_attempts++;
		break;

		default:
			break;
	}

}

static void HandleDisconnect(ENetEvent *event)
{
	client *c = GetClient(event->peer);
	info("[SERVER]: Client %s disconnected", c->username);

	// send packet to all clients to remove the player
	json_packet *p = JSONPacket_Create();
	JSONPacket_Add(p, "cmd", "removeplayer");
	JSONPacket_Add(p, "username", c->username);
	BroadcastPacket(c->peer, PACKET_TYPE_SERVERCMD, p);
	JSONPacket_Destroy(p);

	event->peer->data = NULL;
	FE_DestroyPlayer(c->player);
	FE_List_Remove(&clients, c);
	free(c);
	client_count--;

	UpdateStatus();
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

static void Exit()
{
	DestroyServer();
	FE_Menu_LoadMenu("Main");
}

void DestroyServer()
{
	if (server) {
		// free clients
		for (FE_List *l = clients; l; l = l->next) {
			client *c = l->data;
			FE_DestroyPlayer(c->player);
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
		last_id = 0;
		status = 0;

		client_count = 0;
	}
}

// initialises the server
int InitServer(int port)
{
	// load the map
	FE_LoadedMap *m;
	if (!(m = FE_LoadMap(MAP))) {
		warn("[SERVER]: Failed to load map");
		return -1;
	}
    FE_Game_SetMap(m);

	if (enet_initialize () != 0) {
		warn("[SERVER]: An error occurred while initializing ENet");
		return -1;
	}

	ENetAddress address;
	address.host = ENET_HOST_ANY;
	address.port = port;

	server = enet_host_create(&address, 32, 1, 0, 0);

	if (!server) {
		warn("[SERVER]: An error occurred while trying to create an ENet server host.");
		return -1;
	}
	info("[SERVER]: Started Server");

	// create status label
	status = FE_UI_CreateLabel(0, "Connected Clients: 0", 300, midx(256), midy(30), COLOR_BLUE);
	FE_UI_AddElement(FE_UI_LABEL, status);

	// create exit button
	FE_UI_Button *exit = FE_UI_CreateButton("Quit", 0, 28 , BUTTON_MEDIUM, &Exit, 0);
	FE_UI_AddElement(FE_UI_BUTTON, exit);

	// show the server ip and port
    char *ip = GetExternalIP();
	if (ip) {
		char text[64];
		sprintf(text, "Server IP: %s:%d", ip, server->address.port);
		FE_UI_AddElement(FE_UI_LABEL, FE_UI_CreateLabel(0, text, 400, midx(256), 100, COLOR_WHITE));
	}
	free(ip);

	FE_ResetDT();

    return 0;
}

static void HostServer()
{
	ENetEvent event;
    while (enet_host_service(server, &event, 0) > 0) {
        switch (event.type)
        {
            case ENET_EVENT_TYPE_CONNECT:
                info("[SERVER]: A new client connected from %x:%u.",
                    event.peer->address.host, event.peer->address.port);
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
			client *c = l->data;
			c->packets_sent = 0;
		}
	}
}

// checks each player's held keys, and applies the action associated by those keys being held down
static void UpdateHeldKeys()
{
	for (FE_List *l = clients; l; l = l->next) {
		
		client *c = l->data;
		FE_Player *p = c->player;

		if (c->held_keys[0])
			FE_MovePlayer(p, vec(-p->movespeed, 0));
		if (c->held_keys[1])
			FE_MovePlayer(p, vec(p->movespeed, 0));
		if (c->held_keys[2]) {
			p->on_ground = FE_PlayerOnGround(p);
			if (!p->jump_started)
				FE_StartPlayerJump(p);
			else
				FE_UpdatePlayerJump(p);
		}
	}
}

void UpdateServer()
{
	PresentGame->Timing.UpdateTime = SDL_GetPerformanceCounter();
	
	ResetPacketCount();
	HostServer();

	UpdateHeldKeys();

	// run main game loop
	FE_RunPhysics();
	
	// check if any clients have moved, if so then send them the new position
	for (FE_List *l = clients; l; l = l->next) {
		client *c = l->data;
		if (!vec2_cmp(c->last_location, vec(c->player->PhysObj->body.x, c->player->PhysObj->body.y))
		|| !vec2_cmp(c->last_velocity, vec(c->player->PhysObj->velocity.x, c->player->PhysObj->velocity.y))) {
			
			c->last_location = vec(c->player->PhysObj->body.x, c->player->PhysObj->body.y);
			c->last_velocity = vec(c->player->PhysObj->velocity.x, c->player->PhysObj->velocity.y);

			json_packet *packet = JSONPacket_Create();
			JSONPacket_Add(packet, "u", c->username);
			
			// attatch the player's position to the packet
			char x[8], y[8], velx[8], vely[8];
			sprintf(x, "%i", c->player->PhysObj->body.x);
			sprintf(y, "%i", c->player->PhysObj->body.y);
			sprintf(velx, "%.2f", c->player->PhysObj->velocity.x);
			sprintf(vely, "%.2f", c->player->PhysObj->velocity.y);
			JSONPacket_Add(packet, "x", x);
			JSONPacket_Add(packet, "y", y);
			JSONPacket_Add(packet, "vx", velx);
			JSONPacket_Add(packet, "vy", vely);

			BroadcastPacket(0, PACKET_TYPE_UPDATE, packet);
			JSONPacket_Destroy(packet);
		}
	}
	PresentGame->Timing.UpdateTime = ((SDL_GetPerformanceCounter() - PresentGame->Timing.UpdateTime) / SDL_GetPerformanceFrequency()) * 1000;
}
