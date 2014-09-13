#include "tins/tins.h"
#include <cstdlib>

#include "util.h"

using namespace std;
using namespace Tins;

bool compare_mac(uint8_t* mac1, uint8_t* mac2) {
	uint8_t i = 0;
	for (; i < 6; i++) {
		if (mac1[i] != mac2[i]) {
			return false;
		}
	}
	return true;
}

void convert_mac(uint8_t* result_mac, Tins::HWAddress<6> mac) {
	auto it = mac.begin();
	uint8_t i = 0;
	for (; it != mac.end(); it++) {
		result_mac[i] = *it;
		i++;
	}
}

void print_packet(uint8_t* packet, uint32_t length) {
	uint32_t i = 0;
	printf("Length is %d\n", length);

	if (length > 500) {
		length = 100;
	}
	
	for (i = 0; i < length; i++) {
		printf("%d-", packet[i]);
	}
	printf("\n");
}