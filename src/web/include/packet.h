#ifndef _H_PACKET
#define _H_PACKET

#include <enet/enet.h>

/* internal packet header to be used by network code only */

typedef enum packet_type {
    PACKET_TYPE_MESSAGE,
    PACKET_TYPE_SERVERMSG, // server message to one client
    PACKET_TYPE_SERVERCMD, // command from server to client
    PACKET_TYPE_CONNECT, // client has connected
    PACKET_TYPE_DISCONNECT, // client has disconnected
    PACKET_TYPE_UPDATE, // client position has updated
    PACKET_TYPE_KEYDOWN, // client has pressed a key
    PACKET_TYPE_RCONREQUEST, // rcon request
} packet_type;

typedef struct {
    packet_type type;
    size_t properties;
    char **keys;
    char **values;
} json_packet; // the packet before serialization

void SendPacket(ENetPeer* peer, packet_type type, json_packet *jsonpacket);
void JSONPacket_Destroy(json_packet *j);
void JSONPacket_Add(json_packet *j, char *key, char *value);
json_packet *JSONPacket_Create();
char *JSONPacket_GetValue(ENetEvent *event, const char *key);
int PacketType(ENetEvent *event);

#endif