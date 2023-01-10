#ifndef _H_CLIENT_NET
#define _H_CLIENT_NET

#include "client.h"

/** Initialises a client and connects to a server
 * \param addr The address of the server to connect to
 * \param port The port of the server to connect to
 * \param name The name of the client
*/
int InitClient(char *addr, int port, char *username);


/* Destroys and cleans a client */
void DestroyClient();


/* Renders a client */
void ClientRender();


/* Updates a client (handles network, etc) */
void ClientUpdate();


/* Handles inputs from client to send to server */
void ClientEventHandle(FE_Camera *GameCamera, FE_Player *GamePlayer, FE_Net_Client *Client);


/* Disconnects the client cleanly */
void DisconnectClient(FE_Net_Client *client);

/* For if we are disconnected by the server */
void HandleDisconnect(ENetEvent *event, FE_Net_Client *Client);


#endif