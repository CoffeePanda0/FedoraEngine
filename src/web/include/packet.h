#ifndef _H_PACKET
#define _H_PACKET

#include "../../ext/enet.h"

#include "../../core/include/linkedlist.h"
#include "../../entity/include/player.h"
/* internal packet header to be used by network code only */

typedef enum packet_type {
    PACKET_TYPE_MESSAGE, // chat message
    PACKET_TYPE_LOGIN, // login request
    PACKET_TYPE_KICK, // tells a client they will be kicked
    PACKET_TYPE_SERVERMSG, // server message to one client
    PACKET_TYPE_SERVERCMD, // command from server to client
    PACKET_TYPE_UPDATE, // client position has updated
    PACKET_TYPE_KEYDOWN, // client has pressed a key
    PACKET_TYPE_KEYUP, // client has released a key
    PACKET_TYPE_RCONREQUEST, // rcon request
    PACKET_TYPE_SERVERSTATE, // server state
    PACKET_TYPE_MAP // tells client to expect a map to follow
} packet_type;


/* The packet data before serialisation */
typedef struct {
    packet_type type;
    size_t properties;
    char **keys;
    char **values;
} json_packet;


/* the player struct for each connected player to the client */
typedef struct {
    char username[18];
    GPU_Rect rect;
    FE_Texture *texture;
} player;


/* the player struct for each connected player to the server */
typedef struct {
	ENetPeer *peer;
    char ip[32];
    
	bool authenticated; // whether or not the player has logged in yet
	char username[18];

	FE_Player *player;
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


/* For keeping track of client inputs */
typedef enum {
    KEY_LEFT,
    KEY_RIGHT,
    KEY_JUMP
} held_keys;


/** Sends a JSON packet to a peer
 * \param peer The peer to send the packet to
 * \param type The type of packet to send
 * \param jsonpacket The packet to send
 */
void SendPacket(ENetPeer* peer, packet_type type, json_packet *jsonpacket);


/** Destroys and frees a JSON packet
 * \param j The packet to destroy
 */
void JSONPacket_Destroy(json_packet *j);


/** Adds a key:value pair to a JSON packet
 * \param j The packet to add to
 * \param key The key to add
 * \param value The value to add
 */
void JSONPacket_Add(json_packet *j, char *key, char *value);


/** Sends a JSON packet containing just one integer
 * \param peer The peer to send the packet to
 * \param type The type of packet to send
 * \param key The key to add
 * \param value The value to send
 */
void JSONPacket_SendInt(ENetPeer *peer, packet_type type, char *key, int value);


/** Creates and returns an empty JSON packet
 * \return The new JSON packet
 */
json_packet *JSONPacket_Create();


/** Gets the value for a given key in a recieved packet
 * \param event The packet recieve event to get the value from
 * \param key The key to get the value for
 * \return The value for the given key
 */
char *JSONPacket_GetValue(ENetEvent *event, const char *key);


/** Gets an integer value for a given key in a recieved packet
 * \param event The packet recieve event to get the value from
 * \param key The key to get the value for
 * \return The value for the given key
 */
int JSONPacket_GetInt(ENetEvent *event, const char *key);


/** Returns the type of a recieved packet
 * \param event The packet recieve event to get the type from
 * \return The type of the packet
 */
int PacketType(ENetEvent *event);


/** Loads the recieved serverstate packet
 * \param config The config file to load from
 * \param list The list of connected players to modify
 */
void LoadServerState(ENetEvent *event, FE_List **list);


#endif