#ifndef _H_NET_CONNECT
#define _H_NET_CONNECT


/** Handles authentication for new client joining a server, along with sending server config
 * \param server The server to authenticate
 * \param c The client to authenticate
 * \param clients The list of existing connected clients
 * \param client_count The amount of clients already connected
 * \return true on successful connection, false otherwise
*/
bool Server_AuthenticateClient(ENetHost *server, client_t *c, FE_List **clients, size_t *client_count);


/** Handles client request to join a server, along with parsing and loading server data
 * \param uname The username to join with
 * \param server The peer to the server
 * \param client The client to connect with
 * \param username The actual username to set
 * \param players The list of players to add
 * return true on successful connection, false otherwise
*/
bool Client_Connect(char *uname, ENetPeer *server, ENetHost *client, char **username, FE_List **players);


#endif