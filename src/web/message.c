#include <string.h>
#include "include/internal.h"
#include "../core/lib/string.h"

void Server_ParseMessage(client_t *c, ENetEvent *event)
{
    /* Sending a message code */
    if (c->muted) {
        ServerMSG(c, "You are muted");
        return;
    }

    if ((c->messages_sent++) > 10) {
        ServerMSG(c, "Warning: Too many messages sent");
        c->limited_count++;
        // if they've sent too many messages, mute them
        if (c->limited_count > 10)
            RCON_Mute(c);
        return;
    }

    // if message is recieved from a client, send it to all clients
    if (mstrlen(JSONPacket_GetValue(event, "msg")) < 80) {
        json_packet *packet = JSONPacket_Create();
        JSONPacket_Add(packet, "username", c->username);
        JSONPacket_Add(packet, "msg", JSONPacket_GetValue(event, "msg"));
        BroadcastPacket(0, PACKET_TYPE_MESSAGE, packet);
        JSONPacket_Destroy(packet);
    }
}

void Client_SendMesage(char *msg, ENetPeer *peer)
{
    // check if message is rcon command
    if (msg[0] == '/') {
        char *command = strtok(msg, " ");
        char *arg = strtok(NULL, " ");
        if (strcmp(command, "/rcon") == 0) {
            if (arg) {
                // split arg by command:data
                char *rcon_command = strtok(arg, ";");
                char *rcon_data = strtok(NULL, ";");
                if (rcon_command && rcon_data) {
                    json_packet *p = JSONPacket_Create();
                    JSONPacket_Add(p, "cmd", rcon_command);
                    JSONPacket_Add(p, "data", rcon_data);
                    SendPacket(peer, PACKET_TYPE_RCONREQUEST, p);
                    JSONPacket_Destroy(p);
                }
            }
        }
    } else {
        // send chat message
        json_packet *p = JSONPacket_Create();
        JSONPacket_Add(p, "msg", msg);
        SendPacket(peer, PACKET_TYPE_MESSAGE, p);
        JSONPacket_Destroy(p);
    }
}

// sends a "server message" to one client
void ServerMSG(client_t *client, char *msg)
{
	json_packet *p = JSONPacket_Create();
	JSONPacket_Add(p, "msg", msg);
	SendPacket(client->peer, PACKET_TYPE_SERVERMSG, p);

	JSONPacket_Destroy(p);
}
