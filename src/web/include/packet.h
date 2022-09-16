#ifndef _H_PACKET
#define _H_PACKET

#include "../../ext/enet.h"
#include "../../core/include/linkedlist.h"
#include "../../entity/include/player.h"
/* internal packet header to be used by network code only */

typedef enum packet_type {
    PACKET_TYPE_MESSAGE, // chat message
    PACKET_TYPE_KICK,
    PACKET_TYPE_SERVERMSG, // server message to one client
    PACKET_TYPE_SERVERCMD, // command from server to client
    PACKET_TYPE_UPDATE, // client position has updated
    PACKET_TYPE_KEYDOWN, // client has pressed a key
    PACKET_TYPE_KEYUP, // client has released a key
    PACKET_TYPE_RCONREQUEST, // rcon request
    PACKET_TYPE_SERVERSTATE, // server state
    PACKET_TYPE_MAP
} packet_type;

typedef struct {
    packet_type type;
    size_t properties;
    char **keys;
    char **values;
} json_packet; // the packet before serialization

typedef struct {
    char username[24];
    SDL_Rect rect;
    FE_Texture *texture;
} player;

typedef struct {
	ENetPeer *peer;
    char ip[32];
    
	bool set_username;
	char username[24];

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

typedef struct {
    packet_type type;
    char *data;
} split_packet;

typedef enum {
    KEY_LEFT,
    KEY_RIGHT,
    KEY_JUMP
} held_keys;

void SendPacket(ENetPeer* peer, packet_type type, json_packet *jsonpacket);
void JSONPacket_Destroy(json_packet *j);
void JSONPacket_Add(json_packet *j, char *key, char *value);
void JSONPacket_SendInt(ENetPeer *peer, packet_type type, char *key, int value);
json_packet *JSONPacket_Create();
char *JSONPacket_GetValue(ENetEvent *event, const char *key);
int JSONPacket_GetInt(ENetEvent *event, const char *key);
int PacketType(ENetEvent *event);
void LoadServerState(ENetEvent *event, FE_List **list);

#endif