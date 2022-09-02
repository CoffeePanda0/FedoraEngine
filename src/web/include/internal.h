#ifndef _H_NET_INTERNAL
#define _H_NET_INTERNAL

#include "packet.h"

int InitServer(int port);
void UpdateServer();

int InitClient(char *addr, int port, char *username);
void DestroyClient();
void ClientRender();
void ClientUpdate();
void ClientEventHandle();
void DestroyServer();

#endif