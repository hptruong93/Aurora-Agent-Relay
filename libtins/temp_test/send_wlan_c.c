#include <netinet/ether.h>

#include <stdio.h>
#include <stdlib.h>
#include "ethernet_sender.h"
#include "ethernet_sniffer.h"

typedef unsigned char u8;
typedef unsigned short u16;

ethernet_sniffer sniffer;
raw_ethernet_sender eth_sender;

#define MON_TYPE    0x8cf
#define MON_TO_WARP_TYPE 0x8de
#define WARP_IP_TYPE 0x8bf
#define WARP_ARP_TYPE 0x8af

#define MSB(x) ((x >> 8) & 0xff)
#define LSB(x) (x & 0xff)

void forward(u8* buffer, int data_size) {
	static u16 count = 0;
	u16 type = buffer[13] + (buffer[12] << 8);
	if (type == 0x0800 || type == 0x0806) {
		raw_ethernet_sender_send(&eth_sender, buffer, data_size);
		// printf("Sent 1 type is %02x - %02x with len %d\n", buffer[12], buffer[13], data_size);
	}
	// else if (type != 0x8de && type != 0x8cf) {
	// 	printf("Type is %02x and %02x\n", buffer[12], buffer[13]);
	// }
}

int main(int argc, char const *argv[])
{
	raw_ethernet_sender_init(&eth_sender, "eth1", ETH_P_ALL);

    ethernet_sniffer_init(&sniffer, "wlan0");
    ethernet_sniffer_sniff(&sniffer, forward);
	return 0;
}