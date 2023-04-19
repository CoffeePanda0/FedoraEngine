#include "../core/include/include.h"

#include "../../common/net/include/packet.h"
#include "include/client.h"

#include "../../common/ext/tiny-json/tiny-json.h"
#include "../../common/ext/lz4/lz4.h"

#include "../ui/include/messagebox.h"
#include "../world/include/map.h"
#include "../entity/include/gameobject.h"

/* -- SENDING AND RECEIVING MAPS  --
1. Server sends a packet with the map size to prepare the client
2. Client receives the packet and allocates the memory for the map
3. Server sends the compressed map data
4. Client decompresses the map data and loads it into the map

*/

void LoadServerState(FE_Net_RcvPacket *packet, FE_List **list, FE_Net_Client *Client)
{
    if (!packet) return;

	// first load the server state json packet
	json_t mem[64];
	
	// Load the server state JSON from the packet
	char *buff = FE_Net_GetString(packet);

    json_t const *json = json_create(buff, mem, sizeof mem / sizeof *mem);

    // Get server snapshot rate (to allow for interpolation between packets)
    Client->SnapshotRate = atoi(json_getPropertyValue(json, "snaprate"));

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
				const char* name = json_getPropertyValue(_player, "username");
				p->rect.x = atoi(json_getPropertyValue(_player, "x"));
				p->rect.y = atoi(json_getPropertyValue(_player, "y"));
				p->rect.w = 120; p->rect.h = 100;

                p->last_position = vec(p->rect.x, p->rect.y);
                p->s.new_position = vec(p->rect.x, p->rect.y);
                p->s.time_rcv = packet->timestamp;

				p->texture = FE_LoadResource(FE_RESOURCE_TYPE_TEXTURE, "game/sprites/doge.png");
				mstrcpy(p->username, name);

				// add to list
				FE_List_Add(list, p);
			}
		}
	}

    // Load GameObjects
    json_t const* gameobjects = json_getProperty(json, "gameobjects");
    if (gameobjects) {
        json_t const* _gameobject;

        for (_gameobject = json_getChild(gameobjects); _gameobject != 0; _gameobject = json_getSibling(_gameobject)) {
            if (JSON_OBJ == json_getType(_gameobject)) {
                // for each gameobject
                const char *name = json_getPropertyValue(_gameobject, "name");
                const char *texture_path = json_getPropertyValue(_gameobject, "tex");

                int x = atoi(json_getPropertyValue(_gameobject, "x"));
                int y = atoi(json_getPropertyValue(_gameobject, "y"));
                int w = atoi(json_getPropertyValue(_gameobject, "w"));
                int h = atoi(json_getPropertyValue(_gameobject, "h"));
                int mass = atoi(json_getPropertyValue(_gameobject, "mass"));

                SDL_Rect rect = {x,y,w,h};

                FE_GameObject *o = FE_GameObject_Create(rect, texture_path, mass, name);
                o->id = atoi(json_getPropertyValue(_gameobject, "id"));
                FE_GameObjectIDCounter = o->id + 1;

            }
        }
    }

	free(buff);
}

// parses and loads the map data from the server
bool AwaitMap(ENetHost *client, size_t len, size_t u_len)
{
    // close open map
    FE_LoadedMap *pres = FE_Game_GetMap();
    if (pres)
        FE_Map_Close(pres);
    
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
        warn("Could not decompress map data");
        goto fail;
    }

    // write map data to file
    FILE *f = fopen("game/map/maps/multiplayer/mapdata", "wb");
    if (!f) {
        warn("Could not create temporary mapdata file");
        goto fail;
    }
    fwrite(data, 1, u_len, f);
    fclose(f);

    // load map
    FE_LoadedMap *m = FE_Map_Load("multiplayer/mapdata");
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