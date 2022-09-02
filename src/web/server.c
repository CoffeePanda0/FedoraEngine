#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include "net.h"
#include "include/internal.h"
#include "../include/game.h"

#define MAP "test"
#define rcon_password "password"

typedef struct {
	ENetPeer *peer;
	bool set_username;
	char username[24];

	FE_Player *player;
	vec2 last_location;
	vec2 last_velocity;

	bool has_rcon;
	size_t rcon_attempts;

	size_t packets_sent;
} client;

typedef struct {
    packet_type type;
    char *data;
} split_packet;

static ENetHost* server;

static client *clients;
static size_t client_count;

static int last_id = 0; // last client id

static FE_UI_Label *status;


// gets a client info by their peer
static client *GetClient(ENetPeer *peer)
{
	for (size_t i = 0; i < client_count; i++) {
		if (clients[i].peer == peer) {
			return &clients[i];
		}
	}
	return 0;
}

// handles a client connecting
static void HandleConnect(ENetEvent *event)
{
	clients = realloc(clients, sizeof(client) * (client_count + 1));
	clients[client_count].peer = event->peer;
	clients[client_count].set_username = false;
	clients[client_count].has_rcon = false;
	clients[client_count].rcon_attempts = 0;
	clients[client_count].packets_sent = 0;
	clients[client_count].player = FE_CreatePlayer(40, 8, 18, (SDL_Rect){PresentGame->MapConfig.PlayerSpawn.x, PresentGame->MapConfig.PlayerSpawn.y, 120, 100});
	clients[client_count].last_location = vec(PresentGame->MapConfig.PlayerSpawn.x, PresentGame->MapConfig.PlayerSpawn.y);
	clients[client_count++].username[0] = '\0';
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
	for (size_t i = 0; i < client_count; i++) {
		if (!peer || clients[i].peer != peer) {
			SendPacket(clients[i].peer, type, p);
		}
	}
}

static json_packet *PlayerState()
{
	// create a json packet with all the player states
	json_packet *p = JSONPacket_Create();

	

	return p;
}

static void KickPlayer(client *client, char *reason)
{
	info("[SERVER]: Kicking player [%s:%i] %s (%s)", client->peer->address.host, client->peer->address.port, client->username, reason);
	enet_peer_disconnect(client->peer, 0);
}

// handles receiving a message from a client
static void HandleRecieve(ENetEvent *event)
{
	client *c = GetClient(event->peer);
	c->packets_sent++;

	if (c->packets_sent > 180) // only allow 180 packets per second (maybe we send down/up once instead?)
		return;
	if (c->packets_sent > 360)
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
				for (size_t i = 0; i < client_count; i++) {
					if (strcmp(clients[i].username, JSONPacket_GetValue(event, "msg")) == 0) {
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
		case PACKET_TYPE_KEYDOWN: ;
			FE_Player *player = c->player;
			char *key = JSONPacket_GetValue(event, "key");

			if (mstrcmp(key, "LEFT") == 0)
            	FE_MovePlayer(player, vec(-player->movespeed, 0));
        	if (mstrcmp(key, "RIGHT") == 0)
            	FE_MovePlayer(player, vec(player->movespeed, 0));
        	if (mstrcmp(key, "JUMP") == 0) {
				player->on_ground = FE_PlayerOnGround(player);
            	if (!player->jump_started) {
                	FE_StartPlayerJump(player);
            	} else {
                	FE_UpdatePlayerJump(player);
            	}
			}
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
	for (size_t i = 0; i < client_count; i++) {
		if (clients[i].peer == event->peer) {
			event->peer->data = NULL;
			FE_DestroyPlayer(c->player);
			memmove(&clients[i], &clients[i + 1], sizeof(client) * (client_count - i - 1));
			client_count--;
			clients = realloc(clients, sizeof(client) * client_count);
			break;
		}
	}
	// send packet to all clients to remove the player
	json_packet *p = JSONPacket_Create();
	JSONPacket_Add(p, "cmd", "removeplayer");
	JSONPacket_Add(p, "username", c->username);
	BroadcastPacket(0, PACKET_TYPE_SERVERCMD, p);
	JSONPacket_Destroy(p);

	UpdateStatus();
}

static char *GetExternalIP()
{
	FILE *curl;
    if ((curl = popen("curl -s https://api.ipify.org", "r")) == NULL)
        return 0;

    char* ip = malloc(99);
    if (fgets(ip, 99, curl) == NULL){
        warn("You are not connected to the internet");
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
		for (size_t i = 0; i < client_count; i++) {
			client *c = &clients[i];
			FE_DestroyPlayer(c->player);
			enet_peer_disconnect(c->peer, 0);
			c->peer = 0;
		}

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
		warn("Failed to load map");
		return -1;
	}
    FE_Game_SetMap(m);

	if (enet_initialize () != 0) {
		warn("An error occurred while initializing ENet");
		return -1;
	}

	ENetAddress address;
	address.host = ENET_HOST_ANY;
	address.port = port;

	server = enet_host_create(&address, 32, 1, 0, 0);

	if (!server) {
		warn("An error occurred while trying to create an ENet server host.");
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
		for (size_t i = 0; i < client_count; i++) {
			clients[i].packets_sent = 0;
		}
	}
}

void UpdateServer()
{
	PresentGame->Timing.UpdateTime = SDL_GetPerformanceCounter();
	
	ResetPacketCount();
	HostServer();
	// run main game loop
	FE_RunPhysics();
	
	// check if any clients have moved, if so then send them the new position
	for (size_t i = 0; i < client_count; i++) {
		if (!vec2_cmp(clients[i].last_location, vec(clients[i].player->PhysObj->body.x, clients[i].player->PhysObj->body.y))
		|| !vec2_cmp(clients[i].last_velocity, vec(clients[i].player->PhysObj->velocity.x, clients[i].player->PhysObj->velocity.y))) {
			
			clients[i].last_location = vec(clients[i].player->PhysObj->body.x, clients[i].player->PhysObj->body.y);
			clients[i].last_velocity = vec(clients[i].player->PhysObj->velocity.x, clients[i].player->PhysObj->velocity.y);

			json_packet *packet = JSONPacket_Create();
			JSONPacket_Add(packet, "u", clients[i].username);
			
			// attatch the player's position to the packet
			char x[16], y[16], velx[16], vely[16];
			sprintf(x, "%f", clients[i].player->PhysObj->position.x);
			sprintf(y, "%f", clients[i].player->PhysObj->position.y);
			sprintf(velx, "%f", clients[i].player->PhysObj->velocity.x);
			sprintf(vely, "%f", clients[i].player->PhysObj->velocity.y);
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
