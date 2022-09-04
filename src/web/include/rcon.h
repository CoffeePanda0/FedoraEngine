#ifndef _H_NET_RCON
#define _H_NET_RCON

// internal use for networking

/* Checks if an IP has been banned */
bool RCON_CheckIP(enet_uint32 addr);

/* Initialises RCON */
void RCON_Init();

/* Closes RCON */
void RCON_Destroy();

/* Kicks a player */
void RCON_Kick(client_t *client, char *reason);

/* Permabans a player */
void RCON_Ban(client_t *client, char *reason);


#endif