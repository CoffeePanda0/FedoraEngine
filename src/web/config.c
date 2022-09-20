#include "include/internal.h"
#include "../ext/inih/ini.h"
#include "../ext/tiny-json/tiny-json.h"
#include "../core/include/include.h"
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

    // Send a packet to the client to listen for map data
    json_packet *p = JSONPacket_Create();
    char buff[16];
    sprintf(buff, "%zu", size);
    JSONPacket_Add(p, "len", buff);
    SendPacket(peer, PACKET_TYPE_MAP, p);
    JSONPacket_Destroy(p);

    // Send the map data
    ENetPacket *packet = enet_packet_create(data, size, ENET_PACKET_FLAG_RELIABLE);
    enet_peer_send(peer, 0, packet);

    free(data);
}