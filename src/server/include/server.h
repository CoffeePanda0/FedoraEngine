#ifndef _H_NET_SERVER
#define _H_NET_SERVER

#include "../../../common/physics/include/physics.h"
#include "../../../common/entity/include/player.h"

// ALL FOR INTERNAL NET USE ONLY

/* The parsed server config file */
typedef struct {
    char *map;
    int max_players;
    char *rcon_pass;
    int port;

    bool has_message;
    char *message;

    int tickrate;
    int snapshot_rate;

} ServerConfig;
extern ServerConfig server_config;


/* the player struct for each connected player to the server */
typedef struct {
	ENetPeer *peer;
    char ip[32];
    
	bool authenticated; // whether or not the player has logged in yet
	char username[18];

	FE_PlayerProperties *player;
	
	vec2 last_location; // used to check if the player has moved
	vec2 last_velocity; // last velocity sent to client

	bool has_rcon; // if the client has rcon access
	size_t rcon_attempts; // how many times they have tried to rcon

	size_t packets_sent; // count of packets sent by client in last frame

    size_t messages_sent; // amount of messages sent in last 3 seconds
    size_t limited_count; // how many times they have been limited
    bool muted; // if the client is muted

	uint8_t held_keys[3]; // contains state of each key
} client_t;


/* Cleans and shutdowns server, disconnecting all clients */
void FE_DestroyServer();


/** Handles authentication for new client joining a server, along with sending server config
 * \param server The server to authenticate
 * \param c The client to authenticate
 * \param clients The list of existing connected clients
 * \param client_count The amount of clients already connected
 * \return true on successful connection, false otherwise
*/
bool AuthenticateClient(ENetHost *server, client_t *c, FE_List **clients, size_t *client_count);


/** Generates the current serverstate as a JSON string
 * \param buffer The buffer to write the JSON string to
 * \param buffer_size The size of the buffer
 * \param clients The clients currently connected to the server
*/
void GenerateServerState(char *buffer, size_t size, FE_List *clients);


/** Sends the server map to a client
  * \param peer The client to send the map to
*/
void SendMap(ENetPeer *peer);


/* Loads the server config from server.ini */
void LoadServerConfig();


/* Frees the server config */
void DestroyServerConfig();


#endif