#ifndef HELPER_FUNCTIONS_H
#define HELPER_FUNCTIONS_H

inline void xor_swap(int &a, int &b)
{
    a ^= b;
    b ^= a;
    a ^= b;
}

inline void zero_packet(packet_data &p)
{
    p.flags = 0;
    p.ack = 0;
    p.seq = 0;
    p.checksum = 0;
    p.payload_len = 0;
    p.payload = nullptr;
}

#endif