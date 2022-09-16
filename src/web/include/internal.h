#ifndef _H_NET_INTERNAL
#define _H_NET_INTERNAL

#include "packet.h"
#include "rcon.h"
#include "../net.h"

int InitServer();
void UpdateServer();

int InitClient(char *addr, int port, char *username);
void DestroyClient();
void ClientRender();
void ClientUpdate();
void ClientEventHandle();
void DestroyServer();

void Server_LoadConfig();
void Server_DestroyConfig();
void Server_SendMap(ENetPeer *peer);

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