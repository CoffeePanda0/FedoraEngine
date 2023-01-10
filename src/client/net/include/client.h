#ifndef _H_NET_CLIENT
#define _H_NET_CLIENT

/* For internal networking use only */

#include "../../entity/include/player.h"

typedef struct Snapshot {
    uint64_t time_rcv;
    vec2 new_position;
} Snapshot;

/* the player struct for each connected player to the client */
typedef struct {
    char username[18];
    SDL_Rect rect;
    FE_Texture *texture;

    vec2 last_position;
    Snapshot s;
} player;

/* The struct for the client */
typedef struct {

    /* Connection information*/
    ENetPeer *Peer; // the Server connection peer
    ENetHost *Client; // The Client used to listen 
    char *Username; // The username given to the client by the server
    bool Connected; // Whether or not this client is connected

    /* Network connection statistics */
    uint32_t SnapshotRate; // The snapshot rate of the server
    float Ping; // The last calculated ping
    uint64_t RTT; // The last calculated rtt
    int32_t ServerTimeOffset; // The difference between our time and server time


    /* Snapshot information*/
    uint64_t LastPacket; // The last update packet that we have recieved
    uint64_t LatestPacket; // The most recent update packet that we have recieved
    uint32_t Jitter; // The server jitter from the snapshot rate

} FE_Net_Client;


/** Handles client request to join a server, along with parsing and loading server data
 * \param uname The username to join with
 * \param Client The client state
 * \param players The list of players to add
 * return true on successful connection, false otherwise
*/
bool Client_Connect(char *uname, FE_Net_Client *Client, FE_List **players);


/** Handles the recieving and awaiting of a map from the server
 * \param client The client to listen for the map on
 * \param len The length of the compressed packet
 * \param u_len The length of the uncompressed packet
*/
bool AwaitMap(ENetHost *client, size_t len, size_t u_len);


/** Loads the recieved serverstate packet
 * \param packet The packet to load
 * \param list The list of connected players to modify
 * \param Client The client state
 */
void LoadServerState(FE_Net_RcvPacket *packet, FE_List **list, FE_Net_Client *Client);


void ParseTimeResponse(FE_Net_Client *client, FE_Net_RcvPacket *packet);

void SendTimeRequest(FE_Net_Client *client);

void KeepServerTime(FE_Net_Client *client);

#endif