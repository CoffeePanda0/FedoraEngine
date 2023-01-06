#include <linkedlist.h>
#include <fedoraengine.h>
#include <utils.h>
#include <../lib/string.h>
#include <timing.h>

#include "../../common/net/include/packet.h"
#include "include/client.h"


bool Client_Connect(char *uname, ENetPeer *server, ENetHost *client, char **username, FE_List **players)
{
    bool setmap = false;
    bool setstate = false;
    bool setusername  = false;

    // send login packet with username
    FE_Net_Packet *p = FE_Net_Packet_Create(PACKET_CLIENT_LOGIN);
    FE_Net_Packet_AddString(p, uname);
    FE_Net_Packet_Send(server, p, true);

    // wait to recieve username, state and map
    ENetEvent event;
    float timeout = 0;
    while (!setmap || !setstate || !setusername) {
        while (enet_host_service(client, &event, 0) > 0) {
            if (event.type == ENET_EVENT_TYPE_RECEIVE) {

                // Load the packet
                FE_Net_RcvPacket *packet = FE_Net_GetPacket(&event);
                if (!packet) {
                    info("[CLIENT]: Server sent invalid packet");
                    return false;
                }

                switch (packet->type) {

                    case PACKET_SERVER_LOGIN:
                        setusername = true;
                        *username = FE_Net_GetString(packet);
                    break;
                    
                    case PACKET_SERVER_STATE:
                        LoadServerState(packet, players);
                        setstate = true;
                    break;
                    
                    case PACKET_SERVER_MAP: ;
                        // prepare to load map and read lengths
                        size_t length = (size_t)FE_Net_GetInt(packet);
                        size_t u_length = (size_t)FE_Net_GetInt(packet);

                        if (length == 0 || u_length == 0) {
                            warn("Invalid map packet");
                            return false;
                        }

                        bool status = AwaitMap(client, length, u_length); // recieve map (client may hang while waiting for map)
                        setmap = true;

                        if (!status)
                            return false;
                    break;

                }

                FE_Net_DestroyRcv(packet);

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

    if (setstate && setusername && setmap) {
        info("Successfully joined game as %s", *username);
        return true;
    }
    
    return false;
}