#ifndef HELPER_FUNCTIONS_H
#define HELPER_FUNCTIONS_H

inline void xor_swap(int &a, int &b)
{
    a ^= b;
    b ^= a;
    a ^= b;
}

#endif