#ifndef _H_NET_INTERNAL
#define _H_NET_INTERNAL

#include "packet.h"
#include "rcon.h"
#include "connect.h"
#include "../net.h"

/* Initialises a headless server with the server.ini config file */
int InitServer();

/* Updates server (hosts network, updates game, etc) */
void UpdateServer();

int InitClient(char *addr, int port, char *username);
void DestroyClient();
void ClientRender();
void ClientUpdate();
void ClientEventHandle();
void DestroyServer();
void Server_GenerateState(char *buffer, size_t size, FE_List *clients);
void ServerMSG(client_t *client, char *msg);
void BroadcastPacket(ENetPeer *peer, packet_type type, json_packet *p);
void Server_ParseMessage(client_t *client, ENetEvent *event);
void Client_SendMesage(char *msg, ENetPeer *peer);
void Server_LoadConfig();
void Server_DestroyConfig();
void Server_SendMap(ENetPeer *peer);
bool Client_ReceiveMap(ENetHost *client, size_t len, size_t u_len);


/* The parsed server config file */
typedef struct {
    char *map;
    int max_players;
    char *rcon_pass;
    int port;

    bool has_message;
    char *message;
} ServerConfig;
extern ServerConfig server_config;


#endif