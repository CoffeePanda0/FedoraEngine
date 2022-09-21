#ifndef _H_NET_INTERNAL
#define _H_NET_INTERNAL

#include "packet.h"
#include "rcon.h"
#include "connect.h"
#include "../net.h"

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

/* Initialises a headless server with the server.ini config file */
int InitServer();


/* Updates server (hosts network, updates game, etc) */
void UpdateServer();


/* Cleans and shutdwons server, disconnecting all clients */
void DestroyServer();


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


/** Generates the current serverstate as a JSON string
 * \param buffer The buffer to write the JSON string to
 * \param buffer_size The size of the buffer
 * \param clients The clients currently connected to the server
*/
void Server_GenerateState(char *buffer, size_t size, FE_List *clients);


/** Sends a server message to a clients
 * \param clients The client to send the message to
 * \param msg The message to send
*/
void ServerMSG(client_t *client, char *msg);


/** Sends a packet to all clients
 * \param peer The peer to not send the packet to (NULL to send to all)
 * \param type The type of packet to send
 * \param p The packet to send
*/
void BroadcastPacket(ENetPeer *peer, packet_type type, json_packet *p);


/** Parses a chat message event
 * \client The client that sent the message
 * \event The event to parse
*/
void Server_ParseMessage(client_t *client, ENetEvent *event);


/** Sends a message from a client to all other clients
 * \param msg The message to send
 * \param peer The peer sending the message
*/
void Client_SendMesage(char *msg, ENetPeer *peer);


/** Resets the throttle timer for clients sending chat messages
 * \param clients The clients to reset the timer for
*/
void ResetMessageCount(FE_List **clients);


/* Loads the server config from server.ini */
void Server_LoadConfig();


/* Frees the server config */
void Server_DestroyConfig();


/** Sends the server map to a client
  * \param peer The client to send the map to
*/
void Server_SendMap(ENetPeer *peer);


/** Handles the recieving and awaiting of a map from the server
 * \param client The client to listen for the map on
 * \param len The length of the compressed packet
 * \param u_len The length of the uncompressed packet
*/
bool Client_ReceiveMap(ENetHost *client, size_t len, size_t u_len);


#endif