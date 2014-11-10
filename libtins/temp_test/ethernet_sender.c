#include <arpa/inet.h>
#include <linux/if_packet.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/ether.h>

#include "ethernet_sender.h"
 
#define BUF_SIZ 1024

typedef unsigned char u8;

static u8 eth_dst[] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05};
static u8 eth_src[] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x04};
static u8 data[] = {65, 65, 65, 65, 65, 65};

void get_mac(int socket, char* interface_name, struct ifreq *interface_mac) {
	/* Get the MAC address of the interface to send on */
	memset(interface_mac, 0, sizeof(struct ifreq));
	strncpy(interface_mac->ifr_name, interface_name, IFNAMSIZ-1);
	if (ioctl(socket, SIOCGIFHWADDR, interface_mac) < 0) {
		perror("SIOCGIFHWADDR");
	}
}

void get_interface_index(int socket, char* interface_name, struct ifreq *interface_id) {
	/* Get the index of the interface to send on */
	memset(interface_id, 0, sizeof(struct ifreq));
	strncpy(interface_id->ifr_name, interface_name, IFNAMSIZ-1);
	if (ioctl(socket, SIOCGIFINDEX, interface_id) < 0) {
		perror("SIOCGIFINDEX");
	}
}

void prepare_ethernet_header(void* buffer, unsigned char* eth_dst, unsigned char* eth_src, unsigned short protocol) {
	struct ether_header* ethernet_header = (struct ether_header *) buffer;
	/* Construct the Ethernet header */
	/* Ethernet header */
	memcpy(ethernet_header->ether_shost, eth_src, 6);
	memcpy(ethernet_header->ether_dhost, eth_dst, 6);

	//  Ethertype field
	ethernet_header->ether_type = htons(protocol);
}

int raw_ethernet_sender_init(raw_ethernet_sender* sender, char* interface_name, unsigned short protocol) {
	memset(sender, 0, sizeof(raw_ethernet_sender));
	strcpy(sender->interface_name, interface_name);

	sender->socket = socket(PF_PACKET, SOCK_RAW, htons(protocol));
	if (sender->socket == -1) {
		printf("Error creating sender: Cannot create raw socket.\n");
		memset(sender, 0, sizeof(raw_ethernet_sender));
		return -1;
	} else {
		get_interface_index(sender->socket, sender->interface_name, &(sender->if_idx));

		//Assume that fullly prepared buffer.
		/* Index of the network device */
		sender->socket_address.sll_ifindex = sender->if_idx.ifr_ifindex;
		/* Address length*/
		sender->socket_address.sll_halen = ETH_ALEN;
		return 0;
	}
}

ssize_t raw_ethernet_sender_send(raw_ethernet_sender* sender, unsigned char* buffer, unsigned short length) {
	/* Destination MAC */
	memcpy(sender->socket_address.sll_addr, buffer, 6);

	return sendto(sender->socket, buffer, length, 0, (struct sockaddr*)&(sender->socket_address), sizeof(struct sockaddr_ll));
}

int sender_test(int argc, char *argv[]) {
	
	char ifName[IFNAMSIZ];
	/* Get interface name */
	if (argc > 1) {
		strcpy(ifName, argv[1]);
	} else {
		strcpy(ifName, "wlan0");
	}
	 
	int tx_len = 0;
	char sendbuf[BUF_SIZ];

	/* Construct the Ethernet header */
	memset(sendbuf, 0, BUF_SIZ);
	prepare_ethernet_header(sendbuf, eth_dst, eth_src, 0x8ae);
	tx_len += sizeof(struct ether_header);
	 
	/* Packet data */
	memcpy(sendbuf + tx_len, data, sizeof(data));
	tx_len += sizeof(data);
	 

	raw_ethernet_sender sender;
	if (raw_ethernet_sender_init(&sender, "wlan0", 0x8ae) == 0) {
		raw_ethernet_sender_send(&sender, sendbuf, tx_len);
	}

	return 0;
}