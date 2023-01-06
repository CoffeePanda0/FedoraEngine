#ifndef _H_NET_CLIENT
#define _H_NET_CLIENT

/* For internal networking use only */

#include "../../entity/include/player.h"

/* the player struct for each connected player to the client */
typedef struct {
    char username[18];
    SDL_Rect rect;
    FE_Texture *texture;
} player;


/** Handles client request to join a server, along with parsing and loading server data
 * \param uname The username to join with
 * \param server The peer to the server
 * \param client The client to connect with
 * \param username The actual username to set
 * \param players The list of players to add
 * return true on successful connection, false otherwise
*/
bool Client_Connect(char *uname, ENetPeer *server, ENetHost *client, char **username, FE_List **players);


/** Handles the recieving and awaiting of a map from the server
 * \param client The client to listen for the map on
 * \param len The length of the compressed packet
 * \param u_len The length of the uncompressed packet
*/
bool AwaitMap(ENetHost *client, size_t len, size_t u_len);


/** Loads the recieved serverstate packet
 * \param packet The packet to load
 * \param list The list of connected players to modify
 */
void LoadServerState(FE_Net_RcvPacket *packet, FE_List **list);


#endif