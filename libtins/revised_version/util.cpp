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

char* getInterface(Tins::Dot11::address_type addr) {
    string address = addr.to_string();
    cout << "Address is " << address << endl;

    FILE *fp;
    char *interface_name = (char*)malloc(64);
    size_t interface_name_len = 0;
    int c;
    string command = string(GREP_FROM_IFCONFIG , strlen(GREP_FROM_IFCONFIG)) + "'" + string(HW_ADDR_KEYWORD, strlen(HW_ADDR_KEYWORD)) + address + "'";
    fp = popen(command.c_str(), "r");

    while ((c = fgetc(fp)) != EOF)
    {
        if ((char) c == ' ')
        {
            break;
        }
        interface_name[interface_name_len++] = (char)c;
    }

    interface_name[interface_name_len] = '\0';

    return interface_name;
}