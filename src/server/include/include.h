#ifndef _H_SERVER
#define _H_SERVER

/* Functions for external use to run the server */

/* Initialises a Headless server with the server.ini config file */
int InitServer();


/* Runs the server */
void RunServer();


/* Updates the server (handling and sending packets) */
void UpdateServer();


/* Cleans and shutdowns server, disconnecting all clients */
void DestroyServer();


/* Initialises FedoraEngine to run headless as a server */
void FE_Server_Init(FE_InitConfig *InitConfig);


#endif