#include <utils.h>
#include <timing.h>
#include <../lib/string.h>

#include "../common/net/include/packet.h"
#include "include/server.h"
#include "include/rcon.h"
#include "include/message.h"

void Server_ParseMessage(client_t *c, FE_Net_RcvPacket *packet)
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

    char *msg = FE_Net_GetString(packet);
    if (!msg) {
        warn("[SERVER]: Illegal message sent by %s", c->username);
        return;
    }

    // if message is recieved from a client, send it to all clients
    if (mstrlen(msg) < 80 && mstrlen(msg) > 1 && !mstrempty(msg)) {

        FE_Net_Packet *packet = FE_Net_Packet_Create(PACKET_SERVER_CHAT);
        FE_Net_Packet_AddString(packet, c->username);
        FE_Net_Packet_AddString(packet, mstrtrim(msg));

        BroadcastPacket(0, packet);
        FE_Net_Packet_Destroy(packet);
    }

    free(msg);
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

/* Sends a "server message" to one client */
void ServerMSG(client_t *client, char *msg)
{
	FE_Net_Packet *p = FE_Net_Packet_Create(PACKET_SERVER_SERVERMSG);
	FE_Net_Packet_AddString(p, msg);
    FE_Net_Packet_Send(client->peer, p, true);
}
