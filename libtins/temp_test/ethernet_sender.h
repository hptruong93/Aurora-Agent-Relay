#ifndef ETHERNET_SENDER_H_
#define ETHERNET_SENDER_H_

#include <linux/if_packet.h>
#include <net/if.h>

typedef struct raw_ethernet_sender {
	char interface_name[IFNAMSIZ];
	struct ifreq if_idx;
	struct sockaddr_ll socket_address;
	int socket;
} raw_ethernet_sender;

void prepare_ethernet_header(void* buffer, unsigned char* eth_dst, unsigned char* eth_src, unsigned short protocol);
int raw_ethernet_sender_init(raw_ethernet_sender* sender, char* interface_name, unsigned short protocol);
ssize_t raw_ethernet_sender_send(raw_ethernet_sender* sender, unsigned char* buffer, unsigned short length);

#endif