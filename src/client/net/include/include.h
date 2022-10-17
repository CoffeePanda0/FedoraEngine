#ifndef _H_CLIENT_NET
#define _H_CLIENT_NET


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
void ClientEventHandle();


#endif