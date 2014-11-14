#ifndef UTIL_H
#define UTIL_H

#include <tins/tins.h>
#include <string>
#include <vector>

#define INC_MOD(x, mod) ((x + 1) % mod)
#define DEC_MOD(x, mod) ((x + mod - 1) % mod)
#define TRACE_MSG fprintf(stderr, "(%d) [%s:%d] here I am\n", __FUNCTION__, __FILE__, __LINE__)
#define GREP_FROM_IFCONFIG			"ifconfig | grep -E "
#define HW_ADDR_KEYWORD				"HWaddr "
#define MON_INTERFACE_KEYWORD		"mon.wlan"

bool compare_mac(uint8_t* mac1, uint8_t* mac2);
void convert_mac(uint8_t* result_mac, Tins::HWAddress<6> mac);
void print_packet(uint8_t* packet, uint32_t length);
char* getInterface(Tins::Dot11::address_type addr);
char* getInterfaceName(const std::string& addr);
void split_string(std::string& original, const std::string& delimiter, std::vector<std::string> splits);

#endif