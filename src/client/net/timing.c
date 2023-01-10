#include <systime.h>
#include <timing.h>
#include "../../common/net/include/packet.h"
#include "include/client.h"

void SendTimeRequest(FE_Net_Client *client)
{
    FE_Net_Packet *packet = FE_Net_Packet_Create(PACKET_CLIENT_TIMEREQUEST);
    FE_Net_Packet_Send(client->Peer, packet, true);
    enet_host_flush(client->Client);
}

void KeepServerTime(FE_Net_Client *client)
{
    static float now = 0;

    /* Send time request if we haven't yet */
    static bool hastime = false;
    if (!hastime) {
        hastime = true;
        SendTimeRequest(client);
        return;
    }

    /* Re-request time every three seconds*/
    now += FE_DT;
    if (now > 3.0f) {
        now -= 3.0f;
        SendTimeRequest(client);
    }
}

void ParseTimeResponse(FE_Net_Client *client, FE_Net_RcvPacket *packet)
{
    /* Calculate RTT */
    uint64_t time_sent = FE_Net_GetLong(packet);
    uint64_t time_now = FE_GetTimeUTC();

    client->RTT = (time_now - time_sent);

    /* Calculate approximate ping */
    client->Ping = client->RTT / 2.0f;

    /* Calculate approximate server time */
    uint64_t server_time = packet->timestamp + (uint64_t)client->Ping;
    client->ServerTimeOffset = server_time - time_now;
}