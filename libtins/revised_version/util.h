#ifndef UTIL_H
#define UTIL_H

#include "tins/tins.h"

#define INC_MOD(x, mod) ((x + 1) % mod)
#define DEC_MOD(x, mod) ((x + mod - 1) % mod)

bool compare_mac(uint8_t* mac1, uint8_t* mac2);
void convert_mac(uint8_t* result_mac, Tins::HWAddress<6> mac);

#endif