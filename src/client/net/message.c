#include <string.h>
#include <../lib/string.h>

#include "../../common/net/include/packet.h"

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