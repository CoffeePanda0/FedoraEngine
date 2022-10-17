#include <utils.h>
#include <timing.h>
#include <../lib/string.h>

#include "../common/net/include/packet.h"
#include "include/server.h"
#include "include/rcon.h"
#include "include/message.h"

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

    char *msg = JSONPacket_GetValue(event, "msg");
    if (!msg) {
        warn("[SERVER]: Illegal message sent by %s", c->username);
        return;
    }

    // if message is recieved from a client, send it to all clients
    if (mstrlen(msg) < 80 && mstrlen(msg) > 1 && !mstrempty(msg)) {
        json_packet *packet = JSONPacket_Create();
        JSONPacket_Add(packet, "username", c->username);
        JSONPacket_Add(packet, "msg", mstrtrim(msg));
        BroadcastPacket(0, PACKET_TYPE_MESSAGE, packet);
        JSONPacket_Destroy(packet);
    }
}

void ResetMessageCount(FE_List **clients)
{
	static float timer = 0;
	timer += FE_DT;

	if (timer >= 3) {
		timer -= 3;
		for (FE_List *l = *clients; l; l = l->next) {
			client_t *c = l->data;
			c->messages_sent = 0;
		}
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
