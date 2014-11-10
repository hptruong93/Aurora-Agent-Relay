#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include <sys/socket.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h>
#include <linux/if.h>
#include <sys/ioctl.h>

#include "ethernet_sniffer.h"

#define DEFAULT_RESERVED_SPACE     100

int ethernet_sniffer_init(ethernet_sniffer* sniffer, char* interface_name) {
    int rawsock;
    struct sockaddr_ll sll;
    struct ifreq ifr;
 
    rawsock = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (rawsock == -1) {
        /* probably a premissions error */
        return -1;
    }
 
    memset(&sll, 0, sizeof(sll));
    memset(&ifr, 0, sizeof(ifr));
     
    /* get interface index  */
    strncpy((char *)ifr.ifr_name, interface_name, IFNAMSIZ);
    if ((ioctl(rawsock, SIOCGIFINDEX, &ifr)) == -1) {
        return -1;  /* device not found */
    }
 
    /* Bind our raw socket to this interface */
    sll.sll_family = PF_PACKET;
    sll.sll_ifindex = ifr.ifr_ifindex;
    sll.sll_protocol = htons(ETH_P_ALL);
 
    if((bind(rawsock, (struct sockaddr *)&sll, sizeof(sll)))== -1) {
        return -1;  /* bind error */
    }

    sniffer->socket = rawsock;

    return 0;
}

void ethernet_sniffer_sniff(ethernet_sniffer* sniffer, void(*callback)()) {
    while (1) {
        int data_size = recvfrom(sniffer->socket, sniffer->buffer + DEFAULT_RESERVED_SPACE, SNIFFER_BUFFER_SIZE, 0, &(sniffer->saddr), &(sniffer->saddr_size));
        callback(sniffer->buffer + DEFAULT_RESERVED_SPACE, data_size);
    }
}

void print_test(unsigned char* buffer, int data_size) {
    if (data_size > 0) {
        printf("Data size is %d\n", data_size);
    }
}

int sniffer_test(int argc, char const *argv[])
{
    ethernet_sniffer sniffer;
    if (ethernet_sniffer_init(&sniffer, "eth0") == 0) {
        ethernet_sniffer_sniff(&sniffer, print_test);
    }
    return 0;
}