#include <FE_Common.h>

#include "../common/net/include/packet.h"
#include "include/rcon.h"

typedef struct {
    char *addr;
    size_t count;
} kick_info; // contains how many times a person has been kicked in the current session to ban them


static FILE *ban_file = 0;

// list of kicked players so that they will be kicked if they try to join again
static kick_info *kicks = 0;
static size_t kick_count = 0;

// list of muted players so that they will be muted if they rejoin
static FE_StrArr *muted;

void RCON_Kick(client_t *client, char *reason)
{
    // check if they already have been kicked before

    for (size_t i = 0; i < kick_count; i++) {
        if (strcmp(kicks[i].addr, client->ip) == 0) {
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
    kicks[kick_count - 1].addr = mstrdup(client->ip);
    kicks[kick_count - 1].count = 1;

kick:
    info("[SERVER]: Kicking player [%s:%i] %s (%s)", client->ip, client->peer->address.port, client->username, reason);
    
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
    if (!ban_file)
        return;
    fprintf(ban_file, "%s\n", client->ip);
    fflush(ban_file);

    info("[SERVER]: Banning player [%s:%i] %s (%s)", client->ip, client->peer->address.port, client->username, reason);
    
    // send client a packet so they know they have been kicked
    json_packet *p = JSONPacket_Create();
    JSONPacket_Add(p, "reason", reason);
    SendPacket(client->peer, PACKET_TYPE_KICK, p);
    JSONPacket_Destroy(p);
    
    enet_peer_disconnect_later(client->peer, DISC_BAN);
}

bool RCON_CheckIP(char *ip)
{
    // check if ip is banned
    if (!ban_file)
        return false;

    rewind(ban_file);

    // Read line by line
    char line[64];
    while (fgets(line, 64, ban_file)) {
        // Remove newline
        line[mstrlen(line) - 1] = 0;

        // Check if ip matches
        if (mstrcmp(line, ip) == 0) {
            info("[SERVER]: Banned player [%s] tried to connect", ip);
            return true;
        }
    }

    // check current kick log incase they have been banned and not written to file
    for (size_t i = 0; i < kick_count; i++) {
        if (strcmp(kicks[i].addr, ip) == 0) {
            if (kicks[i].count >= 3) {
                info("[SERVER]: Banned player [%s] tried to connect", ip);
                return true;
            }
        }
    }

    return false;
}

void RCON_Mute(client_t *client)
{
    client->muted = true;
    FE_StrArr_Add(muted, client->ip);
}

void RCON_Unmute(client_t *client)
{
    client->muted = false;
    ssize_t index = FE_StrArr_Exists(muted, client->ip);
    FE_StrArr_RemoveAt(muted, index);
}

bool RCON_CheckMute(char *ip)
{
    return (FE_StrArr_Exists(muted, ip) == -1 ? false : true);
}

void RCON_Init()
{
    ban_file = fopen("bans.txt", "a+");
    if (!ban_file) {
        warn("Failed to open bans.txt");
    }
    muted = FE_StrArr_Create();
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

    FE_StrArr_Destroy(muted);
    muted = 0;
}