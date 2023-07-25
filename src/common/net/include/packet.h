#ifndef _H_NET_PACKET
#define _H_NET_PACKET

#include <stddef.h>
#include <linkedlist.h>
#include "../../ext/enet.h"

/* For keeping track of client inputs */
typedef enum {
    KEY_LEFT,
    KEY_RIGHT,
    KEY_JUMP
} held_keys;

/* -- See schema.txt for more info on networking -- */

/* Packet from a client */
typedef enum PACKET_CLIENT {
    PACKET_CLIENT_LOGIN,
    PACKET_CLIENT_KEYDOWN,
    PACKET_CLIENT_KEYUP,
    PACKET_CLIENT_CHAT,
    PACKET_CLIENT_RCON,
    PACKET_CLIENT_TIMEREQUEST
} PACKET_CLIENT;


/* Packet from a server */
typedef enum PACKET_SERVER {
    PACKET_SERVER_LOGIN,
    PACKET_SERVER_STATE,
    PACKET_SERVER_KICK,
    PACKET_SERVER_SERVERMSG,
    PACKET_SERVER_CHAT,
    PACKET_SERVER_UPDATE,
    PACKET_SERVER_SPAWN,
    PACKET_SERVER_DESPAWN,
    PACKET_SERVER_MAP,
    PACKET_SERVER_SNAPSHOTRATE,
    PACKET_SERVER_TIME
} PACKET_SERVER;


/* The packet type to use */
typedef enum PACKET_MODE {
    SERVER,
    CLIENT
} PACKET_MODE;

typedef enum KEY_TYPE {
        KEY_STRING,
        KEY_SHORTINT,
        KEY_INT,
        KEY_FLOAT,
        KEY_BOOL,
        KEY_LONG,
        KEY_ARRAY
} KEY_TYPE;

/* Allows the value to be of any type */
typedef struct {
    KEY_TYPE type;

    union {
        char *str;
        uint8_t s;
        int i;
        float f;
        bool b;
        uint64_t l;
        struct FE_Net_Array *array;
    } value;

} Value;


/* Holds an array of items */
typedef struct FE_Net_Array {
	size_t size;
	Value *values;

    size_t ptr; 
} FE_Net_Array;

/* The packet before serialisation */
typedef struct {
    uint8_t type;

    Value *values;
    size_t properties;

    size_t serialised_size; // size of the serialised packet
    char *serialised_data;
} FE_Net_Packet;


/* The parsed and received packet */
typedef struct {
    uint8_t type;
    uint64_t timestamp;

    void *cmp;

    char *data;
    size_t len;
} FE_Net_RcvPacket;

/**
 * @brief Send a packet to a peer
 * 
 * @param peer The peer to send the packet to
 * @param packet The packet to send
 * @param destroy Whether to destroy the packet after sending
 */
void FE_Net_Packet_Send(ENetPeer *peer, FE_Net_Packet *packet, bool destroy);


/**
 * @brief Adds a float to a packet
 * 
 * @param packet The packet to add the float to
 * @param f The float to add
 */
void FE_Net_Packet_AddFloat(FE_Net_Packet *packet, float f);


/**
 * @brief Adds an int to a packet
 * 
 * @param packet The packet to add the int to
 * @param i The int to add
 */
void FE_Net_Packet_AddInt(FE_Net_Packet *packet, int i);


/**
 * @brief Adds a short int to a packet
 * 
 * @param packet The packet to add the short int to
 * @param s The short int to add
 */
void FE_Net_Packet_AddShortInt(FE_Net_Packet *packet, uint8_t s);


/**
 * @brief Adds a string to a packet
 * 
 * @param packet The packet to add the string to
 * @param str The string to add
 */
void FE_Net_Packet_AddString(FE_Net_Packet *packet, char *str);


/**
 * @brief Adds a bool to a packet
 * 
 * @param packet  The packet to add the bool to
 * @param b The bool to add
 */
void FE_Net_Packet_AddBool(FE_Net_Packet *packet, bool b);


/** 
 * @brief Adds a long to a packet
 * 
 * @param packet The packet to add the long to
 * @param l The long to add
 */
void FE_Net_Packet_AddLong(FE_Net_Packet *packet, uint64_t l);


/**
 * @brief Adds an array to a packet
 * 
 * @param packet The packet to add the array to
 * @param array The array to add
 */
void FE_Net_Packet_AttatchArray(FE_Net_Packet *packet, FE_Net_Array *array);


/**
 * @brief Gets a bool from a packet
 * 
 * @param packet 
 * @return true 
 * @return false 
 */
bool FE_Net_GetBool(FE_Net_RcvPacket *packet);


/**
 * @brief Creates a new packet
 * 
 * @param type The type of the packet
 * @return FE_Net_Packet* 
 */
FE_Net_Packet *FE_Net_Packet_Create(uint8_t type);


/**
 * @brief Destroys a packet
 * 
 * @param packet The packet to destroy
 */
void FE_Net_Packet_Destroy(FE_Net_Packet *packet);


/**
 * @brief Gets a packet from an event
 * 
 * @param event The event to get the packet from
 * @return FE_Net_RcvPacket* 
 */
FE_Net_RcvPacket *FE_Net_GetPacket(ENetEvent *event);


/**
 * @brief Gets a string from a packet
 * 
 * @param packet The packet to get the string from
 * @return char* The string
 */
char *FE_Net_GetString(FE_Net_RcvPacket *packet);


/**
 * @brief Gets a float from a packet
 * 
 * @param packet The packet to get the float from
 * @return float The float
 */
float FE_Net_GetFloat(FE_Net_RcvPacket *packet);


/**
 * @brief Gets an int from a packet
 * 
 * @param packet The packet to get the int from
 * @return int The int
 */
int FE_Net_GetInt(FE_Net_RcvPacket *packet);


/**
 * @brief Gets a short int from a packet
 * 
 * @param packet The packet to get the short int from
 * @return uint8_t The short int
 */
uint8_t FE_Net_GetShortInt(FE_Net_RcvPacket *packet);


/**
 * @brief Destroys a received packet
 * 
 * @param packet The packet to destroy
 */
void FE_Net_DestroyRcv(FE_Net_RcvPacket *packet);


/**
 * @brief Gets a long unsinged int from a packet
 * 
 * @param packet The packet to get the long int from
 * @return uint64_t The long int
 */
uint64_t FE_Net_GetLong(FE_Net_RcvPacket *packet);


/**
 * @brief Creates a packet array
 * 
 * @return The packet array
 */
FE_Net_Array *FE_Net_Array_Create();


/**
 * @brief Destroys a packet array
 * 
 * @param array The array to destroy
 */
void FE_Net_Array_Destroy(FE_Net_Array *array);


/** 
 * @brief Adds a value to a packet array
 * 
 * @param array The array to add the value to
 * @param value The value to add
*/
void FE_Net_Array_Add(FE_Net_Array *array, Value value);


/**
 * @brief Gets a string from a Network Array
 * 
 * @param array 
 * @return char* 
 */
char *FE_Net_Array_GetString(FE_Net_Array *array);


/**
 * @brief Gets a network array from a packet
 * 
 * @param packet The packet to get the array from
 * @param types The data types for each value in the array
 * @param expected_len The expected number of values in the array
 * @param repeats Whether the array contains repeats (e.g for different players)
 * @return FE_Net_Array* 
 */
FE_Net_Array *FE_Net_GetArray(FE_Net_RcvPacket *packet, uint8_t *types, size_t expected_len, bool repeats);


/**
 * @brief Gets an int from a network array
 * 
 * @param array The array to get the int from
 * @return int 
 */
int FE_Net_Array_GetInt(FE_Net_Array *array);


/**
 * @brief Gets a float from a network array
 * 
 * @param array The array to get the float from
 * @return float 
 */
float FE_Net_Array_GetFloat(FE_Net_Array *array);


#endif
