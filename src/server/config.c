/* Handles loading the server config file, along with sending of server state */

#include <file.h>
#include <utils.h>
#include <mem.h>
#include <../lib/string.h>

#include "../common/ext/inih/ini.h"
#include "../common/ext/json-maker/json-maker.h"
#include "../common/ext/lz4/lz4.h"

#include "../common/net/include/packet.h"
#include "../common/entity/include/gameobject.h"
#include "include/server.h"

ServerConfig server_config;

void DestroyServerConfig()
{
    server_config.has_message = false;
    if (server_config.message != NULL) {
        free(server_config.message);
        server_config.message = NULL;
    }
    if (server_config.map != NULL) {
        free(server_config.map);
        server_config.map = NULL;
    }
    if (server_config.rcon_pass != NULL) {
        free(server_config.rcon_pass);
        server_config.rcon_pass = NULL;
    }
}

static int ConfigParser(void *user, const char *section, const char *name, const char *value)
{
    (void)user;
    if (MATCH("SERVER", "port"))
        server_config.port = atoi(value);
    else if (MATCH("SERVER", "map"))
        server_config.map = mstrdup(value);
    else if (MATCH("SERVER", "maxplayers"))
        server_config.max_players = atoi(value);
    else if (MATCH("SERVER", "rcon_password"))
        server_config.rcon_pass = mstrdup(value);

    if (MATCH("TIMING", "tickrate"))
        server_config.tickrate = atoi(value);
    if (MATCH("TIMING", "snapshot_rate"))
        server_config.snapshot_rate = atoi(value);
    
    if (MATCH("MESSAGE", "has_message"))
        server_config.has_message = (bool)atoi(value);
    else if (MATCH("MESSAGE", "message"))
        server_config.message = mstrdup(value);
    
    return 1;
}

void LoadServerConfig()
{
    // set defaults
    server_config.port = 0;
    server_config.map = 0;
    server_config.max_players = 0;
    server_config.rcon_pass = 0;
    server_config.has_message = false;
    server_config.message = 0;
    server_config.tickrate = 60;
    server_config.snapshot_rate = 20;

    if (ini_parse("server.ini", ConfigParser, NULL) < 0)
        warn("[SERVER] Can't load server.ini file");

    // load default values for fields not set
    if (server_config.map == NULL)
        server_config.map = mstrdup("test");
    if (!server_config.port)
        server_config.port = 7777;
    if (!server_config.max_players)
        server_config.max_players = 32;
}

// generates the server state
void GenerateServerState(char *buffer, size_t size, FE_List *clients)
{
	// create json packet
	buffer = json_objOpen(buffer, NULL, &size);

	// load welcome message (if exists)
	buffer = json_int(buffer, "hasmsg", server_config.has_message, &size);
	if (server_config.has_message)
		buffer = json_str(buffer, "msg", server_config.message, &size);

    // Send the server snapshot rate
    buffer = json_int(buffer, "snaprate", server_config.snapshot_rate, &size);

    // Send connected clients
	if (clients) {
		buffer = json_arrOpen(buffer, "players", &size);
		for (FE_List *l = clients; l; l = l->next) {
			client_t *c = l->data;
			if (c->authenticated) {
				buffer = json_objOpen(buffer, NULL, &size);
				buffer = json_str(buffer, "username", c->username, &size);
				buffer = json_int(buffer, "x", c->player->PhysObj->body.x, &size);
				buffer = json_int(buffer, "y", c->player->PhysObj->body.y, &size);
				buffer = json_objClose(buffer, &size);
			}
		}
		buffer = json_arrClose(buffer, &size);
	}

    if (FE_GameObjects) {
        buffer = json_arrOpen(buffer, "gameobjects", &size);
        for (FE_List *l = FE_GameObjects; l; l = l->next) {
            FE_GameObject *obj = l->data;

            if (mstrcmp(obj->name, "prefab") == 0) continue; // skip prefabs

            buffer = json_objOpen(buffer, NULL, &size);
            buffer = json_str(buffer, "name", obj->name, &size);
            buffer = json_str(buffer, "tex", obj->texture_path, &size);

            buffer = json_int(buffer, "id", obj->id, &size);
            buffer = json_int(buffer, "x", obj->phys->body.x, &size);
            buffer = json_int(buffer, "y", obj->phys->body.y, &size);
            buffer = json_int(buffer, "w", obj->phys->body.w, &size);
            buffer = json_int(buffer, "h", obj->phys->body.h, &size);
            buffer = json_int(buffer, "mass", obj->phys->mass, &size);
            
            buffer = json_objClose(buffer, &size);
        }
        buffer = json_arrClose(buffer, &size);
    }
	buffer = json_objClose(buffer, &size);
}

// send the binary map file to the server
void SendMap(ENetPeer *peer)
{
    // read map file as binary data
    char *path = mstradd("game/map/maps/", server_config.map);
	FILE *file = fopen(path, "r");
	if (!file) {
		printf("Could not open map\n");
		return;
	}

    /* Read map size and the file*/
    free(path);
	fseek(file, 0, SEEK_END);
	size_t size = ftell(file);
	fseek(file, 0, SEEK_SET);
	char *data = xmalloc(size);
	fread(data, size, 1, file);
	fclose(file);

    // compress map data using lz4
    int max_size = LZ4_compressBound(size);
    char *compressed = xmalloc(max_size);
    int compressed_size = LZ4_compress_default(data, compressed, size, size);
    if (compressed_size == 0) {
        warn("Could not compress map data\n");
        return;
    }


    // Send a packet to the client to listen for map data
    FE_Net_Packet *p = FE_Net_Packet_Create(PACKET_SERVER_MAP);
    FE_Net_Packet_AddInt(p, compressed_size);
    FE_Net_Packet_AddInt(p, size);

    FE_Net_Packet_Send(peer, p, true);

    // Send the map data
    ENetPacket *packet = enet_packet_create(compressed, compressed_size, ENET_PACKET_FLAG_RELIABLE);
    enet_peer_send(peer, 0, packet);

    free(data);
    free(compressed);
}