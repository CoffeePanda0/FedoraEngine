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
                // split arg by command;data
                char *rcon_command = strtok(arg, ";");
                char *rcon_data = strtok(NULL, ";");

                if (rcon_command && rcon_data) {
                    /* Send the rcon command */
                    FE_Net_Packet *p = FE_Net_Packet_Create(PACKET_CLIENT_RCON);
                    FE_Net_Packet_AddString(p, rcon_command);
                    FE_Net_Packet_AddString(p, rcon_data);
                    FE_Net_Packet_Send(peer, p, true);
                }
            } // todo make this nice
        }
    } else {
        // send chat message
        FE_Net_Packet *p = FE_Net_Packet_Create(PACKET_CLIENT_CHAT);
        FE_Net_Packet_AddString(p, msg);
        FE_Net_Packet_Send(peer, p, true);
    }
}