#include <stdlib.h>
#include <stdio.h>
#include <enet/enet.h>
#include "../ext/tiny-json.h"
#include "../ext/json-maker/json-maker.h"
#include "../core/lib/string.h"
#include "../core/include/include.h"
#include "include/packet.h"

#define MAX_FIELDS 8
static json_t pool[MAX_FIELDS]; // allow for up to 8 fields

char *JSONPacket_GetValue(ENetEvent *event, const char *key)
{
	// use buffer so we don't change the original packet
	static char buff[256];
	memcpy(buff, event->packet->data, event->packet->dataLength < 256 ? event->packet->dataLength : 256);

	json_t const *parent = json_create(buff, pool, MAX_FIELDS);
	if (!parent) {
		warn("Failed to parse JSON");
		return 0;
	}

	return (char*)json_getPropertyValue(parent, key);	
}

json_packet *JSONPacket_Create()
{
	json_packet *packet = malloc(sizeof(json_packet));
	packet->properties = 0;
	packet->keys = 0;
	packet->values = 0;
	packet->type = 0;
	return packet;
}

void JSONPacket_Add(json_packet *j, char *key, char *value)
{
	if (!j)
		return;
	j->keys = realloc(j->keys, sizeof(char*) * (j->properties + 1));
	j->values = realloc(j->values, sizeof(char*) * (j->properties + 1));

	j->keys[j->properties] = mstrdup(key);
	j->values[j->properties++] = mstrdup(value);
}


void JSONPacket_Destroy(json_packet *j)
{
	if (!j)
		return;

	for (size_t i = 0; i < j->properties; i++) {
		free(j->keys[i]);
		free(j->values[i]);
	}
	free(j->keys);
	free(j->values);
	free(j);
}

int PacketType(ENetEvent *event)
{
	return atoi(JSONPacket_GetValue(event, "type"));
}

char *JSONPacket_ToString(json_packet *j, char *data)
{
	if (!j)
		return 0;
	
	size_t size = 512;
	data = json_objOpen(data, NULL, &size);
	
	// add type
	data = json_int(data, "type", j->type, &size);

	// add properties
	for (size_t i = 0; i < j->properties; i++)
		data = json_str(data, j->keys[i], j->values[i], &size);

	data = json_objClose(data, &size);

	return data;
}

void SendPacket(ENetPeer* peer, packet_type type, json_packet *jsonpacket)
{
	jsonpacket->type = type;

	char buffer[512];
	JSONPacket_ToString(jsonpacket, buffer);

	size_t len = mstrlen(buffer);

	ENetPacket *packet = enet_packet_create(buffer, len, ENET_PACKET_FLAG_RELIABLE);
	enet_peer_send(peer, 0, packet);
}
