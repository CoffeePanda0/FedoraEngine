#include <stdio.h>
#include <stdlib.h>

#include "net.h"

#include "include/internal.h"
#include "../include/game.h"

static const size_t key_count = 3;

static ENetHost *server;

static FE_List *clients;
static size_t client_count;

static FE_StrArr *ip_spam;

/* sends a packet to all clients other than the one who sent it
	(set to 0 to send to all clients) */
void BroadcastPacket(ENetPeer *peer, packet_type type, json_packet *p)
{
	for (FE_List *l = clients; l; l = l->next) {
		client_t *c = l->data;
		if (!peer || c->peer != peer) {
			SendPacket(c->peer, type, p);
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
	
	// create player
	mmemset(c->held_keys, 0, key_count);
	c->player = FE_Player_Create(30, 50, 180, (SDL_Rect){c->last_location.x, c->last_location.y, 120, 100});

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
	Server_AuthenticateClient(server, c, &clients, &client_count);

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

// handles receiving a packet from a client
static void HandleRecieve(ENetEvent *event)
{
	client_t *c = GetClient(event->peer);
	c->packets_sent++;

	if (c->packets_sent > 60)
		RCON_Kick(c, "Too many packets sent");
	if (!c->authenticated)
		RCON_Kick(c, "Client not authenticated");

	switch (PacketType(event)) {
		case PACKET_TYPE_MESSAGE:
			Server_ParseMessage(c, event);
		break;

		case PACKET_TYPE_KEYDOWN:
			HandleKeyDown(JSONPacket_GetInt(event, "key"), c);
		break;

		case PACKET_TYPE_KEYUP:
			HandleKeyUp(JSONPacket_GetInt(event, "key"), c);
		break;

		case PACKET_TYPE_RCONREQUEST: ;
			char *command = JSONPacket_GetValue(event, "cmd");
			char *data = JSONPacket_GetValue(event, "data");
			if (!command || !data) {
				ServerMSG(c, "Invalid RCON request");
				return;
			}

			if (c->has_rcon) {
				if (mstrcmp(command, "kick") == 0) {
					// find the player to kick by their username
					for (FE_List *l = clients; l; l = l->next) {
						client_t *cl = l->data;
						if (mstrcmp(cl->username, data) == 0) {
							ServerMSG(c, "Kicked user");
							RCON_Kick(cl, "Kicked by RCON");
							break;
						}
					}
				}
				else if (mstrcmp(command, "ban") == 0) {
					for (FE_List *l = clients; l; l = l->next) {
						client_t *cl = l->data;
						if (mstrcmp(cl->username, data) == 0) {
							ServerMSG(c, "Banned user");
							RCON_Ban(cl, "Banned by RCON");
							break;
						}
					}
				}
				else if (mstrcmp(command, "gravity") == 0) {
					PresentGame->MapConfig.Gravity = atof(data);
				}
				else if (mstrcmp(command, "shutdown") == 0) {
					info("[SERVER]: Shutting down server (RCON)");
					DestroyServer();
				}
				else if (mstrcmp(command, "mute") == 0) {
					// get the client to mute
					for (FE_List *l = clients; l; l = l->next) {
						client_t *cl = l->data;
						if (mstrcmp(cl->username, data) == 0) {
							info("[SERVER]: Muting user %s (RCON)", data);
							RCON_Mute(cl);
							break;
						}
					}
				}
				else if (mstrcmp(command, "unmute") == 0) {
					for (FE_List *l = clients; l; l = l->next) {
						client_t *cl = l->data;
						if (mstrcmp(cl->username, data) == 0) {
							info("[SERVER]: Unmuting user %s (RCON)", data);
							RCON_Unmute(cl);
							break;
						}
					}
				}
			}
			else if (mstrcmp(command, "login") == 0) {
				// if they don't have rcon, use arg to login
				if (c->rcon_attempts >= 3) { // max attempts
					ServerMSG(c, "Too many RCON attempts");
					return;	
				}
				if (mstrcmp(data, server_config.rcon_pass) == 0) {
					c->has_rcon = true;
					ServerMSG(c, "RCON granted");
				} else {
					ServerMSG(c, "Incorrect RCON password");
				}
				c->rcon_attempts++;
			} else {
				ServerMSG(c, "Invalid RCON command");
			}
		break;

		default:
			break;
	}

}

static void HandleDisconnect(ENetEvent *event)
{
	client_t *c = GetClient(event->peer);
	if (!c) return;
	info("[SERVER]: Client %s disconnected", c->username);

	// send packet to all clients to remove the player
	json_packet *p = JSONPacket_Create();
	JSONPacket_Add(p, "cmd", "removeplayer");
	JSONPacket_Add(p, "username", c->username);
	BroadcastPacket(c->peer, PACKET_TYPE_SERVERCMD, p);
	JSONPacket_Destroy(p);

	event->peer->data = NULL;
	FE_Player_Destroy(c->player);
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

void DestroyServer()
{
	if (server) {
		// free clients
		for (FE_List *l = clients; l; l = l->next) {
			client_t *c = l->data;
			FE_Player_Destroy(c->player);
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
		Server_DestroyConfig();
	}
	exit(0);
}

// initialises the server
int InitServer()
{
	RCON_Init();
	Server_LoadConfig();

	// load the map
	FE_LoadedMap *m;
	if (!(m = FE_Map_Load(server_config.map))) {
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
		FE_Player *p = c->player;

		if (c->held_keys[0])
			FE_Player_Move(p, vec(-p->moveforce, 0));
		if (c->held_keys[1])
			FE_Player_Move(p, vec(p->moveforce, 0));
		if (c->held_keys[2]) {
			p->on_ground = p->PhysObj->grounded;
			if (!p->jump_started)
				FE_Player_StartJump(p);
			else
				FE_Player_UpdateJump(p);
		}
	}
}

void UpdateServer()
{
	PresentGame->Timing.UpdateTime = SDL_GetPerformanceCounter();
	
	ResetPacketCount();
	ResetMessageCount(&clients);

	HostServer();

	UpdateHeldKeys();

	// run main game loop
	FE_Physics_Update();
	
	// check if any clients have moved, if so then send them the new position
	for (FE_List *l = clients; l; l = l->next) {
		client_t *c = l->data;
		if (!vec2_cmp(c->last_location, vec(c->player->PhysObj->body.x, c->player->PhysObj->body.y))
		|| !vec2_cmp(c->last_velocity, vec(c->player->PhysObj->velocity.x, c->player->PhysObj->velocity.y))) {
			
			c->player->on_ground = c->player->PhysObj->grounded;

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
