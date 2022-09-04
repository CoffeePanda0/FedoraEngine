#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "../core/include/file.h"
#include "../core/lib/string.h"
#include "../core/include/include.h"
#include "include/internal.h"

typedef struct {
    char *addr;
    size_t count;
} kick_info; // contains how many times a person has been kicked in the current session to ban them

static FILE *ban_file = 0;
static kick_info *kicks = 0;
static size_t kick_count = 0;

void RCON_Kick(client_t *client, char *reason)
{
    // check if they already have been kicked before
    char ip[32];
    snprintf(ip, 32, "%x", client->peer->address.host);

    for (size_t i = 0; i < kick_count; i++) {
        if (strcmp(kicks[i].addr, ip) == 0) {
            kicks[i].count++;
            if (kicks[i].count >= 3) {
                RCON_Ban(client, "Kicked too many times");
                return;
            } else
                goto kick;
        }
    }

    // add them to the list of kicked people
    kicks = xrealloc(kicks, ++kick_count * sizeof(kick_info));
    kicks[kick_count - 1].addr = mstrdup(ip);
    kicks[kick_count - 1].count = 1;

kick:
    info("[SERVER]: Kicking player [%d:%i] %s (%s)", client->peer->address.host, client->peer->address.port, client->username, reason);
    
    // send client a packet so they know they have been kicked
    json_packet *p = JSONPacket_Create();
    JSONPacket_Add(p, "reason", reason);
    SendPacket(client->peer, PACKET_TYPE_KICK, p);
    JSONPacket_Destroy(p);

    enet_peer_disconnect_later(client->peer, DISC_KICK);
}

void RCON_Ban(client_t *client, char *reason)
{
    // add them to the ban list
    char ip[32];
    snprintf(ip, 32, "%x", client->peer->address.host);
    if (!ban_file)
        return;
    fprintf(ban_file, "%s\n", ip);
    fflush(ban_file);


    info("[SERVER]: Banning player [%d:%i] %s (%s)", client->peer->address.host, client->peer->address.port, client->username, reason);
    
    // send client a packet so they know they have been kicked
    json_packet *p = JSONPacket_Create();
    JSONPacket_Add(p, "reason", reason);
    SendPacket(client->peer, PACKET_TYPE_KICK, p);
    JSONPacket_Destroy(p);
    
    enet_peer_disconnect_later(client->peer, DISC_BAN);
}

bool RCON_CheckIP(enet_uint32 addr)
{
    // check if ip is banned
    if (!ban_file)
        return false;

    char ip[32];
    snprintf(ip, 32, "%x", addr);

    // Read line by line
    char line[64];
    while (fgets(line, 64, ban_file)) {
        // Remove newline
        line[mstrlen(line) - 1] = 0;

        // Check if ip matches
        if (mstrcmp(line, ip) == 0) {
            info("[SERVER]: Banned player [%d] tried to connect", addr);
            return true;
        }
    }

    // check current kick log incase they have been banned and not written to file
    for (size_t i = 0; i < kick_count; i++) {
        if (strcmp(kicks[i].addr, ip) == 0) {
            if (kicks[i].count >= 3) {
                info("[SERVER]: Banned player [%d] tried to connect", addr);
                return true;
            }
        }
    }

    return false;
}

void RCON_Init()
{
    ban_file = fopen("bans.txt", "a+");
    if (!ban_file) {
        warn("Failed to open bans.txt");
    }
}

void RCON_Destroy()
{
    if (ban_file)
        fclose(ban_file);

    // free kicks
    for (size_t i = 0; i < kick_count; i++)
        free(kicks[i].addr);
    if (kicks)
        free(kicks);
    kicks = 0;
}