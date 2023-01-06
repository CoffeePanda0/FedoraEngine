#ifndef _H_SERVER_MESSAGE
#define _H_SERVER_MESSAGE

/** Sends a server message to a clients
 * \param clients The client to send the message to
 * \param msg The message to send
*/
void ServerMSG(client_t *client, char *msg);


/** Sends a packet to all clients
 * \param peer The peer to not send the packet to (NULL to send to all)
 * \param type The type of packet to send
 * \param p The packet to send
*/
void BroadcastPacket(ENetPeer *peer, packet_type type, json_packet *p);


/** Parses a chat message event
 * \client The client that sent the message
 * \event The event to parse
*/
void Server_ParseMessage(client_t *client, ENetEvent *event);


/** Resets the throttle timer for clients sending chat messages
 * \param clients The clients to reset the timer for
*/
void ResetMessageCount(FE_List **clients);


#endif