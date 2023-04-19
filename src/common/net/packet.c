#include <utils.h>
#include <systime.h>
#include <../lib/string.h>
#include "../ext/msgpack/cmp.h"
#include "include/packet.h"

size_t packets_sent = 0;

typedef struct {
    char *data;
    size_t bytes_read;
} data_buffer;

FE_Net_Packet *FE_Net_Packet_Create(uint8_t type)
{
    FE_Net_Packet *p = xmalloc(sizeof(FE_Net_Packet));
    p->type = type;
    p->values = 0;
    p->properties = 0;
    p->serialised_size = 0;
    p->serialised_data = 0;
    return p;
}

void FE_Net_Packet_Destroy(FE_Net_Packet *packet)
{
    if (!packet) {
        warn("Packet is NULL (FE_Net_Packet_Destroy)");
        return;
    }
    for (size_t i = 0; i < packet->properties; i++) {
        if (packet->values[i].type == KEY_STRING)
            free(packet->values[i].value.str);
    }
    free(packet->values);

    if (packet->serialised_data)
        free(packet->serialised_data);

    free(packet);
}

static size_t buff_size = 0;

void FE_Net_Packet_AddString(FE_Net_Packet *packet, char *str)
{
    if (!packet) {
        warn("Packet is NULL (FE_Net_Packet_AddString)");
        return;
    }
    if (!str) {
        warn("String is NULL (FE_Net_Packet_AddString)");
        return;
    }

    packet->values = xrealloc(packet->values, sizeof(Value) * (packet->properties + 1));
    packet->values[packet->properties].type = KEY_STRING;
    packet->values[packet->properties++].value.str = mstrdup(str);
}

void FE_Net_Packet_AddBool(FE_Net_Packet *packet, bool b)
{
    if (!packet) {
        warn("Packet is NULL (FE_Net_Packet_AddBool)");
        return;
    }

    packet->values = xrealloc(packet->values, sizeof(Value) * (packet->properties + 1));
    packet->values[packet->properties].type = KEY_BOOL;
    packet->values[packet->properties++].value.b = b;
}

void FE_Net_Packet_AddShortInt(FE_Net_Packet *packet, uint8_t s)
{
    if (!packet) {
        warn("Packet is NULL (FE_Net_Packet_AddShortInt)");
        return;
    }

    packet->values = xrealloc(packet->values, sizeof(Value) * (packet->properties + 1));
    packet->values[packet->properties].type = KEY_SHORTINT;
    packet->values[packet->properties++].value.s = s;
}

void FE_Net_Packet_AddInt(FE_Net_Packet *packet, int i)
{
    if (!packet) {
        warn("Packet is NULL (FE_Net_Packet_AddInt)");
        return;
    }

    packet->values = xrealloc(packet->values, sizeof(Value) * (packet->properties + 1));
    packet->values[packet->properties].type = KEY_INT;
    packet->values[packet->properties++].value.i = i;
}

void FE_Net_Packet_AddFloat(FE_Net_Packet *packet, float f)
{
    if (!packet) {
        warn("Packet is NULL (FE_Net_Packet_AddFloat)");
        return;
    }

    packet->values = xrealloc(packet->values, sizeof(Value) * (packet->properties + 1));
    packet->values[packet->properties].type = KEY_FLOAT;
    packet->values[packet->properties++].value.f = f;
}

void FE_Net_Packet_AddLong(FE_Net_Packet *packet, uint64_t l)
{
        if (!packet) {
        warn("Packet is NULL (FE_Net_Packet_AddLong)");
        return;
    }

    packet->values = xrealloc(packet->values, sizeof(Value) * (packet->properties + 1));
    packet->values[packet->properties].type = KEY_LONG;
    packet->values[packet->properties++].value.l = l;
}

static size_t file_writer(cmp_ctx_t *ctx, const void *data, size_t count)
{
    /* Copy the data to the buffer */
    ctx->buf = xrealloc(ctx->buf, buff_size + count);
    memcpy((char*)ctx->buf + buff_size, data, count);
    buff_size += count;

    return count;
}

static bool file_reader(cmp_ctx_t *ctx, void *data, size_t limit) {
    /* Read from bytes read to the limit */
    data_buffer *buf = (data_buffer*)ctx->buf;

    memcpy(data, (char*)buf->data + buf->bytes_read, limit);
    buf->bytes_read += limit;
    return true;
}

static char *FE_Net_Packet_Serialise(FE_Net_Packet *packet)
{
    if (!packet) {
        warn("Packet is NULL (FE_Net_Packet_Serialise)");
        return NULL;
    }

    char *buf = 0;

    cmp_ctx_t ctx = {0};
    cmp_init(&ctx, buf, NULL, NULL, file_writer);

    /* Write the packet type */
    cmp_write_u8(&ctx, packet->type);

    /* Write the packet sent time */
    uint64_t time = FE_GetTimeUTC();
    cmp_write_u64(&ctx, time);

    /* Write values by type */
    for (size_t i = 0; i < packet->properties; i++) {
        switch (packet->values[i].type) {
            case KEY_STRING:
                cmp_write_str(&ctx, packet->values[i].value.str, mstrlen(packet->values[i].value.str));
                break;
            case KEY_FLOAT:
                cmp_write_float(&ctx, packet->values[i].value.f);
                break;
            case KEY_INT:
                cmp_write_int(&ctx, packet->values[i].value.i);
                break;
            case KEY_SHORTINT:
                cmp_write_u8(&ctx, packet->values[i].value.s);
                break;
            case KEY_BOOL:
                cmp_write_bool(&ctx, packet->values[i].value.b);
                break;
            case KEY_LONG:
                cmp_write_u64(&ctx, packet->values[i].value.l);
                break;
        }
    }

    if (packet->serialised_data)
        free(packet->serialised_data);

    /* Save the serialised data incase we want to use it again */
    packet->serialised_size = buff_size;
    packet->serialised_data = xmalloc(buff_size);
    memcpy(packet->serialised_data, ctx.buf, buff_size);

    /* Return the buffer */
    buff_size = 0;
    free(ctx.buf);

    return packet->serialised_data;
}

void FE_Net_Packet_Send(ENetPeer *peer, FE_Net_Packet *packet, bool destroy)
{
    if (!peer) {
        warn("Peer is NULL (FE_Net_Packet_Send)");
        return;
    }
    if (!packet) {
        warn("Packet is NULL (FE_Net_Packet_Send)");
        return;
    }

    packets_sent++;

    /* Serialise the packet */
    char *buf = FE_Net_Packet_Serialise(packet);
    if (!buf) {
        warn("Failed to serialise packet (FE_Net_Packet_Send)");
        return;
    }
    size_t len = packet->serialised_size;

    /* Create the ENet packet */
    ENetPacket *enet_packet = enet_packet_create(buf, len, ENET_PACKET_FLAG_RELIABLE);
    enet_peer_send(peer, 0, enet_packet);

    /* Destroy the packet */
    if (destroy)
        FE_Net_Packet_Destroy(packet);
}

FE_Net_RcvPacket *FE_Net_GetPacket(ENetEvent *event)
{
    if (!event) {
        warn("Event is NULL (FE_Net_GetPacket)");
        return NULL;
    }

    FE_Net_RcvPacket *packet = xmalloc(sizeof(FE_Net_RcvPacket));

    /* Parse data from the event */
    if (event->packet->dataLength > 1024) {
        warn("Packet is too large (FE_Net_GetPacket)");
        free(packet);
        return NULL;
    }

    packet->cmp = xmalloc(sizeof(cmp_ctx_t));
    cmp_ctx_t *ctx = packet->cmp;

    packet->data = xmalloc(event->packet->dataLength);
    memcpy(packet->data, event->packet->data, event->packet->dataLength);

    packet->len = event->packet->dataLength;

    /* Create a file reader buffer */
    data_buffer *buffer = xmalloc(sizeof(data_buffer));
    buffer->data = packet->data;
    buffer->bytes_read = 0;

    /* Parse the packet */
    cmp_init(ctx, buffer, file_reader, NULL, NULL);

    /* Read the packet type */
    if (!cmp_read_u8(ctx, &packet->type)) {
        warn("Courrupt Packet type (FE_Net_GetPacket)");
        free(packet->cmp);
        free(packet->data);
        free(packet);
        return 0;
    }
    
    /* Read the packet time */
    if (!cmp_read_u64(ctx, &packet->timestamp)) {
        warn("Courrupt Packet timestamp (FE_Net_GetPacket)");
        free(packet->cmp);
        free(packet->data);
        free(packet);
        return 0;
    }

    return packet;
}

char *FE_Net_GetString(FE_Net_RcvPacket *packet)
{
    if (!packet) {
        warn("Packet is NULL (FE_Net_GetString)");
        return NULL;
    }

    cmp_ctx_t *ctx = packet->cmp;

    /* Read the string */
    char *str = xmalloc(512);
    uint32_t size = 512;

    if (!cmp_read_str(ctx, str, &size))
        warn("Courrupt Packet string (FE_Net_GetString)");

    /* Reallocate with the correct string length */
    size_t len = mstrlen(str);
    str = xrealloc(str, len + 1);

    return str;
}

int FE_Net_GetInt(FE_Net_RcvPacket *packet)
{
    if (!packet) {
        warn("Packet is NULL (FE_Net_GetInt)");
        return 0;
    }

    cmp_ctx_t *ctx = packet->cmp;

    /* Read the int */
    int i;
    if (!cmp_read_int(ctx, &i))
        warn("Courrupt Packet int (FE_Net_GetInt)");

    return i;
}

uint64_t FE_Net_GetLong(FE_Net_RcvPacket *packet)
{
    if (!packet) {
        warn("Packet is NULL (FE_Net_GetLong)");
        return 0;
    }

    cmp_ctx_t *ctx = packet->cmp;

    /* Read the int */
    uint64_t i;
    if (!cmp_read_u64(ctx, &i))
        warn("Courrupt Packet int (FE_Net_GetLong)");

    return i;
}

float FE_Net_GetFloat(FE_Net_RcvPacket *packet)
{
    if (!packet) {
        warn("Packet is NULL (FE_Net_GetFloat)");
        return 0;
    }

    cmp_ctx_t *ctx = packet->cmp;

    /* Read the float */
    float f;
    if (!cmp_read_float(ctx, &f))
        warn("Courrupt Packet float (FE_Net_GetFloat)");

    return f;
}

bool FE_Net_GetBool(FE_Net_RcvPacket *packet)
{
    if (!packet) {
        warn("Packet is NULL (FE_Net_GetBool)");
        return false;
    }

    cmp_ctx_t *ctx = packet->cmp;

    /* Read the bool */
    bool b = false;
    if (!cmp_read_bool(ctx, &b))
        warn("Courrupt Packet bool (FE_Net_GetBool)");

    return b;
}

uint8_t FE_Net_GetShortInt(FE_Net_RcvPacket *packet)
{
    if (!packet) {
        warn("Packet is NULL (FE_Net_GetShortInt)");
        return 0;
    }

    cmp_ctx_t *ctx = packet->cmp;

    /* Read the short int */
    uint8_t s;
    if (!cmp_read_u8(ctx, &s))
        warn("Courrupt Packet shortint (FE_Net_GetShortInt)");

    return s;
}

void FE_Net_DestroyRcv(FE_Net_RcvPacket *packet)
{
    if (!packet) {
        warn("Packet is NULL (FE_Net_DestroyPacket)");
        return;
    }

    cmp_ctx_t *cmp = packet->cmp;
    if (cmp) {
        data_buffer *buf = cmp->buf;
        if (buf)
            free(buf);
        free(cmp);
    }

    if (packet->data)
        free(packet->data);

    free(packet);
}