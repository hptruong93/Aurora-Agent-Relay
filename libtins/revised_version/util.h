#ifndef UTIL_H
#define UTIL_H

#include "tins/tins.h"

bool compare_mac(uint8_t* mac1, uint8_t* mac2);
void convert_mac(uint8_t* result_mac, Tins::HWAddress<6> mac);

#endif