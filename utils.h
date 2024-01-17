#include "protocol_structs.h"
#include "protocol_consts.h"
#include <string.h>

void serialize_packet(const packet *p,const char *serialized)
{
    memcpy(serialized, &(p->data), sizeof(p->data));
    // char *ptr = serialized + sizeof(p->data);
    // memcpy(ptr, p->payload, p->data.payload_len);
}

packet desirialize_packet(const char *serialized_packet, size_t packet_len)
{
    packet p;
    // memcpy(&(p.data), serialized_packet, PACKET_SIZE);
    return p;
}
