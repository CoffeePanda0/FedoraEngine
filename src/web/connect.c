#include "include/internal.h"
#include "../core/include/include.h"

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
bool Server_AuthenticateClient(ENetHost *server, client_t *c, FE_List **clients, size_t *client_count)
{
	/* Await username first */
	ENetEvent event;
	float timeout = 0;
	while (enet_host_service(server, &event, 0) > 0) {
		if (event.type == ENET_EVENT_TYPE_RECEIVE) {
			// check if packet is login
			packet_type type = PacketType(&event);
			if (type == PACKET_TYPE_LOGIN) {

                char *username = JSONPacket_GetValue(&event, "username");
                if (!username) {
                    info("[SERVER]: Client joined with invalid username");
                    return false;
                }

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
				json_packet *p = JSONPacket_Create();
				JSONPacket_Add(p, "username", c->username);
				SendPacket(c->peer, PACKET_TYPE_LOGIN, p);
				JSONPacket_Destroy(p);
			}
		}

		timeout += FE_DT;
		if (timeout >= 3000)
			return false;
	}

	/* Send server state */
	size_t size = 60 + (58 * (++*client_count));
	if (server_config.has_message) size += (mstrlen(server_config.message) + 4);

	char *buffer = xcalloc(size, 1);
	Server_GenerateState(buffer, size, *clients);

	size_t len = mstrlen(buffer);
	ENetPacket *packet = enet_packet_create(buffer, len, ENET_PACKET_FLAG_RELIABLE);
	enet_peer_send(c->peer, 0, packet);
	enet_host_flush(server);
	free(buffer);

	/* Send map */
	Server_SendMap(c->peer);

	c->authenticated = true;
	
	// send packet to all clients to add the new player
	char x[16], y[16];
	sprintf(x, "%f", c->player->PhysObj->position.x);
	sprintf(y, "%f", c->player->PhysObj->position.y);

	json_packet *p = JSONPacket_Create();
	JSONPacket_Add(p, "cmd", "addplayer");
	JSONPacket_Add(p, "username", c->username);
	JSONPacket_Add(p, "x", x);
	JSONPacket_Add(p, "y", y);
	BroadcastPacket(c->peer, PACKET_TYPE_SERVERCMD, p);
	JSONPacket_Destroy(p);

	info("[SERVER] Client %s has joined the game", c->username);

	return true;
}

/* The client-side of the connection process */
bool Client_Connect(char *uname, ENetPeer *server, ENetHost *client, char **username, FE_List **players)
{
    bool setmap = false;
    bool setstate = false;
    bool setusername  = false;

    // send login packet with username
    json_packet *p = JSONPacket_Create();
    JSONPacket_Add(p, "username", uname);
    SendPacket(server, PACKET_TYPE_LOGIN, p);
    JSONPacket_Destroy(p);

    // wait to recieve username, state and map
    ENetEvent event;
    float timeout = 0;
    while (!setmap || !setstate || !setusername) {
        while (enet_host_service(client, &event, 0) > 0) {
            if (event.type == ENET_EVENT_TYPE_RECEIVE) {
                switch (PacketType(&event)) {

                    case PACKET_TYPE_LOGIN:
                        setusername = true;
                        *username = mstrdup(JSONPacket_GetValue(&event, "username"));
                    break;
                    
                    case PACKET_TYPE_SERVERSTATE:
                        LoadServerState(&event, players);
                        setstate = true;
                    break;
                    
                    case PACKET_TYPE_MAP: ;
                        // prepare to load map and read lengths
                        char *len_ = JSONPacket_GetValue(&event, "len");
                        char *u_len_ = JSONPacket_GetValue(&event, "u_len");

                        if (!len_ || !u_len_) {
                            warn("Invalid map packet");
                            return false;
                        }

                        size_t length = atoi(len_);
                        size_t u_length = atoi(u_len_);

                        bool status = Client_ReceiveMap(client, length, u_length); // recieve map (client may hang while waiting for map)
                        setmap = true;

                        if (!status)
                            return false;
                    break;

                }
            } else if (event.type == ENET_EVENT_TYPE_DISCONNECT) {
                if (event.data && event.data == DISC_NOCON) {
                    PresentGame->DisconnectInfo.set = true;
                    PresentGame->DisconnectInfo.reason = mstrdup("You have been banned from this server.");
                    info("Failed to connect to server. (Reason: %s", PresentGame->DisconnectInfo.reason);
                    PresentGame->DisconnectInfo.type = DISC_NOCON;
                    return false;
                }
            }
            if (event.packet)
                enet_packet_destroy(event.packet);
        }

        // allow 5 seconds for server to respond
        timeout += FE_DT;
        if (timeout > 5000)
            return false;

    }

    info("Successfully joined game as %s", *username);

    if (setstate && setusername && setmap)
        return true;
    else
        return false;

}