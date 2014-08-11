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