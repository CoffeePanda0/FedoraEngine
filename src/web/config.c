#include "include/internal.h"

#include "../ext/inih/ini.h"
#include "../ext/tiny-json/tiny-json.h"
#include "../ext/json-maker/json-maker.h"
#include "../ext/lz4/lz4.h"

#include "../core/include/include.h"
#include "../core/include/file.h"
#include "../core/lib/string.h"
#include "../ui/include/messagebox.h"
#include "../world/include/map.h"

ServerConfig server_config;

void Server_DestroyConfig()
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
    
    if (MATCH("MESSAGE", "has_message"))
        server_config.has_message = (bool)atoi(value);
    else if (MATCH("MESSAGE", "message"))
        server_config.message = mstrdup(value);
    
    return 1;
}

void Server_LoadConfig()
{
    // set defaults
    server_config.port = 0;
    server_config.map = 0;
    server_config.max_players = 0;
    server_config.rcon_pass = 0;
    server_config.has_message = false;
    server_config.message = 0;

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

	// load welcome message
	int has_welcome = atoi(json_getPropertyValue(json, "hasmsg"));
	if (has_welcome) {
		char *msg = (char*)json_getPropertyValue(json, "msg");
		FE_Messagebox_Show("Server Message", msg, MESSAGEBOX_TEXT);
	}

	// load each player
	json_t const* players = json_getProperty(json, "players");
	if (players) {
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

				p->texture = FE_LoadResource(FE_RESOURCE_TYPE_TEXTURE, "game/sprites/doge.png");
				mstrcpy(p->username, name);

				// add to list
				FE_List_Add(list, p);
			}
		}
	}

	free(buff);
}

// generates the server state
void Server_GenerateState(char *buffer, size_t size, FE_List *clients)
{
	// create json packet
	buffer = json_objOpen(buffer, NULL, &size);
	buffer = json_int(buffer, "type", PACKET_TYPE_SERVERSTATE, &size);

	// load welcome message (if exists)
	buffer = json_int(buffer, "hasmsg", server_config.has_message, &size);
	if (server_config.has_message)
		buffer = json_str(buffer, "msg", server_config.message, &size);

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
	buffer = json_objClose(buffer, &size);
}


/* -- SENDING AND RECEIVING MAPS  --
1. Server sends a packet with the map size to prepare the client
2. Client receives the packet and allocates the memory for the map
3. Server sends the compressed map data
4. Client decompresses the map data and loads it into the map

*/


// send the binary map file to the server
void Server_SendMap(ENetPeer *peer)
{
    // read map file as binary data
	FILE *file = fopen("game/map/maps/test", "r");
	if (!file) {
		printf("Could not open map\n");
		return;
	}
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
        printf("Could not compress map data\n");
        return;
    }

    // Send a packet to the client to listen for map data
    json_packet *p = JSONPacket_Create();

    char buff[16]; // send compressed size as string
    sprintf(buff, "%i", compressed_size);
    JSONPacket_Add(p, "len", buff);

    char buff2[16]; // send uncompressed size as string
    sprintf(buff2, "%zu", size);
    JSONPacket_Add(p, "u_len", buff2);

    SendPacket(peer, PACKET_TYPE_MAP, p);
    JSONPacket_Destroy(p);

    // Send the map data
    ENetPacket *packet = enet_packet_create(compressed, compressed_size, ENET_PACKET_FLAG_RELIABLE);
    enet_peer_send(peer, 0, packet);

    free(data);
}

// parses and loads the map data from the server
bool Client_ReceiveMap(ENetHost *client, size_t len, size_t u_len)
{
    // close open map
    if (PresentGame->MapConfig.Loaded)
        FE_CloseMap(FE_Game_GetMap());
    
    bool awaiting_map = true;

    // allocate memory for compressed map data
    char *compressed = xmalloc(len);
    char *data = xmalloc(u_len);

    size_t packet_length = 0;

    float waiting_time = 0;

    while (awaiting_map) {
        
        // wait for packet that contains map data
        ENetEvent event;
        while ((enet_host_service(client, &event, 0) > 0) && awaiting_map) {
            switch (event.type) {
                case ENET_EVENT_TYPE_RECEIVE:
                    // copy map data into buffer
                    mmemcpy(compressed, event.packet->data, event.packet->dataLength);
                    packet_length = event.packet->dataLength;
                    awaiting_map = false;
                    
                    enet_packet_destroy(event.packet);
                    break;
                default:
                    enet_packet_destroy(event.packet);
                    break;
            }
        }

        // wait for a bit
        waiting_time += FE_DT;
        if (waiting_time > 3.0f) {
            printf("Timed out waiting for map data\n");
            goto fail;
        }
    }

    // verify that the packet length is correct
    if (packet_length != len) {
        printf("Map packet length does not match expected length\n");
        return false;
    }

    // validate that the uncompressed size is correct
    if (u_len < len) {
        printf("Uncompressed map size is smaller than compressed size\n");
        goto fail;
    }

    // decompress map data
    int decompressed_size = LZ4_decompress_safe(compressed, data, len, u_len);
    if (decompressed_size == 0) {
        printf("Could not decompress map data\n");
        goto fail;
    }

    // write map data to file
    FILE *f = fopen("game/map/maps/multiplayer/mapdata", "wb");
    if (!f) {
        printf("Could not open mapdata file\n");
        goto fail;
    }
    fwrite(data, 1, u_len, f);
    fclose(f);

    // load map
    FE_LoadedMap *m = FE_LoadMap("multiplayer/mapdata");
    if (!m)
        goto fail;
    
    FE_Game_SetMap(m);

    free(data);
    free(compressed);

    // delete map data file
    remove("game/map/maps/multiplayer/mapdata");

    return true;

fail:
    if (compressed)
        free(compressed);
    if (data)
        free(data);
    if (FE_File_DirectoryExists("game/maps/map/multiplayer/mapdata"))
        remove("game/map/maps/multiplayer/mapdata");

    return false;
}