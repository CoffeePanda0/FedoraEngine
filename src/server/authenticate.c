#include <FE_Common.h>
#include "../common/net/include/packet.h"
#include "include/server.h"
#include "include/message.h"

/* -- INFO ON CLIENT-SERVER CONNECTION --
    1. Client connects to server with ENET
    2. Server first checks if the client is banned, then confirms the connection
    3. Client sends a login packet (containing username)
    4. Server responds confirming the set username
    5. Server sends the state, which contains the world and player data
    6. Client receives the state and renders the world
    7. Server sends a packet to confirm map size, then the map data itself
    8. Client receives the map data
*/

/* Authenticates a new user joining the server */
bool AuthenticateClient(ENetHost *server, client_t *c, FE_List **clients, size_t *client_count)
{
	/* Await username first */
	ENetEvent event;
	float timeout = 0;
	bool success = false;

	while (enet_host_service(server, &event, 0) > 0) {
		if (event.type == ENET_EVENT_TYPE_RECEIVE) {
			// Load the packet
			FE_Net_RcvPacket *packet = FE_Net_GetPacket(&event);
			if (!packet) {
				info("[SERVER]: Client sent invalid authentication");
				return false;
			}

			if (packet->type == PACKET_CLIENT_LOGIN) {

                char *username = FE_Net_GetString(packet);
                if (!username) {
                    info("[SERVER]: Client joined with invalid username");
                    return false;
                }

                // Validate username
                if (mstrlen(username) > 16) {
                    info("[SERVER]: Client joined with invalid username");
                    return false;
                }
                // Check username is not empty
                if (mstrempty(username)) {
                    info("[SERVER]: Client joined with invalid username");
                    return false;
                }

                // Strip leading and trailing whitespace
                username = mstrtrim(username);

				// check if the username is already taken
				bool set = false;
				for (FE_List *l = *clients; l; l = l->next) {
					client_t *cl = l->data;
					if (mstrcmp(cl->username, username) == 0) {
						// if so, set it to client + last_id
						snprintf(c->username, 17, "client[%ld]", *client_count);
						ServerMSG(c, "Username taken, setting to client#");
						set = true;
					}
				}
				if (!set) mstrncpy(c->username, username, 17);

				// send the client their username
				FE_Net_Packet *p = FE_Net_Packet_Create(PACKET_SERVER_LOGIN);
				FE_Net_Packet_AddString(p, c->username);
				FE_Net_Packet_Send(c->peer, p, true);

				free(username);

				success = true;
			}

			if (packet) FE_Net_DestroyRcv(packet);
		}

		timeout += FE_DT;
		if (timeout >= 3000)
			return false;
	}
	if (!success) return false;

	/* Send server state */
	size_t size = 64 + (58 * (++*client_count));
	if (server_config.has_message) size += (mstrlen(server_config.message) + 4);

	/* Generate the JSON server state */
	char *buffer = xcalloc(size, 1);
	GenerateServerState(buffer, size, *clients);

	/* Send server state as a packet */
	FE_Net_Packet *packet = FE_Net_Packet_Create(PACKET_SERVER_STATE);
	FE_Net_Packet_AddString(packet, buffer);
	FE_Net_Packet_Send(c->peer, packet, true);

	enet_host_flush(server);
	free(buffer);

	/* Send map as raw binary */
	SendMap(c->peer);

	c->authenticated = true;
	
	/* Broadcast packet telling all clients to spawn new user with their coordinates and username*/
	FE_Net_Packet *p = FE_Net_Packet_Create(PACKET_SERVER_SPAWN);
	FE_Net_Packet_AddString(p, c->username);
	FE_Net_Packet_AddInt(p, (int)c->player->PhysObj->position.x);
	FE_Net_Packet_AddInt(p, (int)c->player->PhysObj->position.y);

	BroadcastPacket(c->peer, p);
	FE_Net_Packet_Destroy(p);

	info("[SERVER] Client %s has joined the game", c->username);

	return true;
}
