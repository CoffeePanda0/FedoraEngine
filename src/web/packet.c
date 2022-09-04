#include <stdlib.h>
#include <stdio.h>
#include <enet/enet.h>
#include "../ext/tiny-json.h"
#include "../ext/json-maker/json-maker.h"
#include "../core/lib/string.h"
#include "../core/include/include.h"
#include "include/packet.h"

#define MAX_FIELDS 12

char *JSONPacket_GetValue(ENetEvent *event, const char *key)
{
	// use buffer so we don't change the original packet
	json_t pool[MAX_FIELDS];

	static char buff[1024];
	mmemset(buff, 0, sizeof(buff));
	mmemcpy(buff, event->packet->data, event->packet->dataLength < 1024 ? event->packet->dataLength : 1024);

	json_t const *parent = json_create(buff, pool, MAX_FIELDS);
	if (!parent) {
		warn("Failed to parse JSON");
		return 0;
	}

	return (char*)json_getPropertyValue(parent, key);	
}

json_packet *JSONPacket_Create()
{
	json_packet *packet = xmalloc(sizeof(json_packet));
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
	j->keys = xrealloc(j->keys, sizeof(char*) * (j->properties + 1));
	j->values = xrealloc(j->values, sizeof(char*) * (j->properties + 1));

	j->keys[j->properties] = mstrdup(key);
	j->values[j->properties++] = mstrdup(value);
}

static char *sendint(char *data, size_t *len, packet_type type, char *key, int value)
{
	data = json_objOpen(data, NULL, len);
	data = json_int(data, "type", type, len);
	data = json_int(data, key, value, len);
	data = json_objClose(data, len);
	return data;
}

void JSONPacket_SendInt(ENetPeer *peer, packet_type type, char *key, int value)
{
	char data[64];
	size_t len = 64;

	sendint(data, &len, type, key, value);

	ENetPacket *packet = enet_packet_create(data, mstrlen(data), ENET_PACKET_FLAG_RELIABLE);
	enet_peer_send(peer, 0, packet);
}

int JSONPacket_GetInt(ENetEvent *event, const char *key)
{
	// use buffer so we don't change the original packet
	json_t pool[8];

	static char buff[256];
	mmemset(buff, 0, sizeof(buff));
	mmemcpy(buff, event->packet->data, event->packet->dataLength < 256 ? event->packet->dataLength : 256);

	json_t const *parent = json_create(buff, pool, 8);
	if (!parent) {
		warn("Failed to parse JSON");
		return 0;
	}

	json_t const *property = json_getProperty(parent, key);
	if (!property || JSON_INTEGER != json_getType(property)) {
		warn("Failed to get property");
		return 0;
	}
	return json_getInteger(property);
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
	char *t = JSONPacket_GetValue(event, "type");
	if (!t) {
		warn("Invalid packet JSON. Packet data: %s", event->packet->data);
		return 0;
	}
	return atoi(t);
}

char *JSONPacket_ToString(json_packet *j, char *data)
{
	if (!j)
		return 0;
	
	size_t size = 1024;
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

	char buffer[1024];
	JSONPacket_ToString(jsonpacket, buffer);

	size_t len = mstrlen(buffer);

	ENetPacket *packet = enet_packet_create(buffer, len, ENET_PACKET_FLAG_RELIABLE);
	enet_peer_send(peer, 0, packet);
}

void LoadServerState(ENetEvent *event, FE_List **list)
{
	// first load the server state json packet
	json_t mem[32];
	
	// use a buffer so we don't change the original packet
	char *buff = xmalloc(event->packet->dataLength + 1);
	mmemcpy(buff, event->packet->data, event->packet->dataLength);

    json_t const *json = json_create(buff, mem, sizeof mem / sizeof *mem);

	int type = atoi(json_getPropertyValue(json, "type"));
	if (type != PACKET_TYPE_SERVERSTATE)
		return;

	// load each player
	json_t const* players = json_getProperty(json, "players");
    json_t const* _player;

    for (_player = json_getChild(players); _player != 0; _player = json_getSibling(_player)) {
        if (JSON_OBJ == json_getType(_player)) {
			// for each player
			player *p = xmalloc(sizeof(player));

			// load vars
            char const* name = json_getPropertyValue(_player, "username");
			p->rect.x = atoi(json_getPropertyValue(_player, "x"));
			p->rect.y = atoi(json_getPropertyValue(_player, "y"));
			p->rect.w = 120; p->rect.h = 100;

            p->texture = FE_LoadResource(FE_RESOURCE_TYPE_TEXTURE, "game/sprites/doge.png"); // todo sprite isnt working
			mstrcpy(p->username, name);

			// add to list
			FE_List_Add(list, p);
        }
    }

	free(buff);
}