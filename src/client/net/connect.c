#include <FE_Common.h>

#include "../../common/net/include/packet.h"
#include "include/client.h"


bool Client_Connect(char *uname, FE_Net_Client *Client, FE_List **players)
{
    /* Flags to ensure that we have completed all login stages correctly */
    bool setmap = false;
    bool setstate = false;
    bool setusername  = false;

    /* Send login packet with username */
    FE_Net_Packet *p = FE_Net_Packet_Create(PACKET_CLIENT_LOGIN);
    FE_Net_Packet_AddString(p, uname);
    FE_Net_Packet_Send(Client->Peer, p, true);

    ENetEvent event;
    float timeout = 0;
    while (!setmap || !setstate || !setusername) {
        while (enet_host_service(Client->Client, &event, 0) > 0) {
            if (event.type == ENET_EVENT_TYPE_RECEIVE) {

                // Load the packet
                FE_Net_RcvPacket *packet = FE_Net_GetPacket(&event);
                if (!packet) {
                    info("[CLIENT]: Server sent invalid packet");
                    return false;
                }

                switch (packet->type) {
                    /* First login stage: server sends us our username */
                    case PACKET_SERVER_LOGIN:
                        setusername = true;
                        Client->Username = FE_Net_GetString(packet);
                    break;
                    
                    /* We recieve server state (other connected players)*/
                    case PACKET_SERVER_STATE:
                        LoadServerState(packet, players, Client);
                        setstate = true;
                    break;
                    
                    case PACKET_SERVER_MAP: ;
                        /* Prepares the client to download the maps */

                        /* Read the uncompressed and compressed length of the map (using lz4)*/
                        size_t length = (size_t)FE_Net_GetInt(packet);
                        size_t u_length = (size_t)FE_Net_GetInt(packet);

                        if (length == 0 || u_length == 0) {
                            warn("Invalid map packet");
                            return false;
                        }

                        /* Recieve map as binary (client may hang while waiting for map) */
                        bool status = AwaitMap(Client->Client, length, u_length);
                        setmap = true;

                        if (!status)
                            return false;
                    break;

                }

                FE_Net_DestroyRcv(packet);

            } else if (event.type == ENET_EVENT_TYPE_DISCONNECT) {
                /* If we instantly recieve a disconnect packet, this client has been kicked */
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
        info("Successfully joined game as %s", Client->Username);
        return true;
    }
    
    return false;
}

void DisconnectClient(FE_Net_Client *client)
{
    // to be called when we want to manually disconnect from the server to let the server know we're done
    if (client->Peer) {
        ENetEvent event;
        /* Send the disconnect request */
        enet_peer_disconnect(client->Peer, 0);
        enet_host_flush(client->Client);

        /* Wait for response */
        while (enet_host_service(client->Client, &event, 3000) > 0) {
            switch (event.type) {
                case ENET_EVENT_TYPE_RECEIVE:
                    break;
                case ENET_EVENT_TYPE_DISCONNECT:
                    info("Disconnected from server");
                    client->Connected = false;
                    break;
                default:
                break;
            }
            enet_packet_destroy(event.packet);
        }
        if (client->Connected)
            info("Timed out while trying to disconnect from the server");
    }
}

void HandleDisconnect(ENetEvent *event, FE_Net_Client *Client)
{
    /* This is called if the player is disconnected from the server not by choice (e.g if they time out, or are kicked )*/
    Client->Connected = false;
    switch (event->data) {
        case DISC_SERVER:
            info("Disconnected from server (Timed out)");
            PresentGame->DisconnectInfo = (FE_DisconnectInfo) {
                .set = true,
                .type = DISC_SERVER,
                .reason = mstrdup("Disconnected from server (Timed out)"),
            };
            break;
        case DISC_KICK:
            info("You have been kicked from the server. (Reason: %s)", PresentGame->DisconnectInfo.reason);
            PresentGame->DisconnectInfo.type = DISC_KICK;
            break;
        case DISC_BAN:
            info("You have been banned from the server. (Reason: %s)", PresentGame->DisconnectInfo.reason);
            PresentGame->DisconnectInfo.type = DISC_BAN;
            break;
        case DISC_NOCON:
            PresentGame->DisconnectInfo.set = true;
            PresentGame->DisconnectInfo.reason = mstrdup("You have been banned from this server.");
            info("Failed to connect to server. (Reason: %s", PresentGame->DisconnectInfo.reason);
            PresentGame->DisconnectInfo.type = DISC_NOCON;
        break;
        default:
            info("Disconnected from server (Other)");
            break;
    }
}