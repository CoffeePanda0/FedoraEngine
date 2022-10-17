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

                        bool status = AwaitMap(client, length, u_length); // recieve map (client may hang while waiting for map)
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