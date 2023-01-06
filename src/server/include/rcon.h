#ifndef _H_NET_RCON
#define _H_NET_RCON

// ALL FOR INTERNAL NET USE ONLY

#include "server.h"

/* Checks if an IP has been banned */
bool RCON_CheckIP(char *ip);

/* Initialises RCON */
void RCON_Init();

/* Closes RCON */
void RCON_Destroy();

/* Kicks a player */
void RCON_Kick(client_t *client, char *reason);

/* Permabans a player */
void RCON_Ban(client_t *client, char *reason);

/* Checks if an address was previously muted */
bool RCON_CheckMute(char *mute);

/* Mutes a player */
void RCON_Mute(client_t *client);

/* Unmutes a player */
void RCON_Unmute(client_t *client);

/* Handles an RCON request from a client */
void RCON_ParseRequest(FE_Net_RcvPacket *packet, client_t *c, FE_List *clients);


#endif