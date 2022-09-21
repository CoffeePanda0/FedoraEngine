#ifndef _H_NET
#define _H_NET

#define ENET_
#include <SDL.h>
#include <stdbool.h>

typedef enum {
    DISC_SERVER,
    DISC_KICK,
    DISC_BAN,
    DISC_NOCON // no connection allowed (previously banned)
} DISCON_TYPE;

typedef struct {
    bool set;
    DISCON_TYPE type;
    char *reason;
} FE_DisconnectInfo;


/** Initialises a multiplayer client
 * @param addr The address of the server to connect to.
 * @param port The port of the server to connect to.
 * @param username The username to use.
 * @return True if the client was successfully initialised.
 */
bool FE_Multiplayer_InitClient(char *addr, int port, char *username);


/** Initialises a headless server (no rendering, runs in shell) */
void FE_Multiplayer_InitServer();


/* Updates the client / server */
void FE_Multiplayer_Update();

/* Renders the client / server */
void FE_Multiplayer_Render();

/* Handles events for the client / server */
void FE_Multiplayer_EventHandle();


#endif