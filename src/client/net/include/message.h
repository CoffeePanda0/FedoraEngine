#ifndef _H_CLIENT_MESSAGE
#define _H_CLIENT_MESSAGE

/** Sends a message from a client to all other clients
 * \param msg The message to send
 * \param peer The peer sending the message
*/
void Client_SendMesage(char *msg, ENetPeer *peer);

#endif