#include <netinet/ether.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ethernet_sender.h"
#include "ethernet_sniffer.h"

typedef unsigned char u8;
typedef unsigned short u16;

ethernet_sniffer sniffer;
raw_ethernet_sender wlan_sender, mon_sender;

#define MON_TYPE    0x8cf
#define MON_TO_WARP_TYPE 0x8de
#define WARP_IP_TYPE 0x8bf
#define WARP_ARP_TYPE 0x8af

#define MSB(x) ((x >> 8) & 0xff)
#define LSB(x) (x & 0xff)

void forward(u8* buffer, int data_size) {
	static u16 count = 0;
	u16 type = buffer[13] + (buffer[12] << 8);
	if (type == WARP_IP_TYPE || type == WARP_ARP_TYPE) {
		if (type == WARP_IP_TYPE) {
			buffer[12] = 0x08;
			buffer[13] = 0x00;
		} else if (type == WARP_ARP_TYPE) {
			buffer[12] = 0x08;
			buffer[13] = 0x06;
		} else {
			return;
		}
		raw_ethernet_sender_send(&wlan_sender, buffer, data_size);
		// printf("Sent 1 type is %02x - %02x with len %d\n", buffer[12], buffer[13], data_size);
		printf("Sent wlan\n");
	} else if (type == MON_TYPE) {
		const u8 default_radio_tap_buffer[14] = {0x00, 0x00, 0x0e, 0x00, 0x0e, 0x00, 0x00, 0x00, 0x00, 0x02, 0x71, 0x09, 0xa0, 0x00};
		memcpy(buffer + 14 - sizeof(default_radio_tap_buffer), default_radio_tap_buffer, sizeof(default_radio_tap_buffer));
		raw_ethernet_sender_send(&mon_sender, buffer + 14 - sizeof(default_radio_tap_buffer), data_size - 14 + sizeof(default_radio_tap_buffer));
		printf("Sent mon\n");
	}
}

int main(int argc, char const *argv[])
{
	if (raw_ethernet_sender_init(&wlan_sender, "wlan0", ETH_P_ALL) != -1
		&& raw_ethernet_sender_init(&mon_sender, "mon.wlan0", ETH_P_ALL) != -1) {

    	if (ethernet_sniffer_init(&sniffer, "eth1") != -1) {
    		printf("Starting...\n");
    		ethernet_sniffer_sniff(&sniffer, forward);
    	}
	}
	return 0;
}