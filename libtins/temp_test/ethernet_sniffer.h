#ifndef ETHERNET_SNIFFER_H_
#define ETHERNET_SNIFFER_H_

#define SNIFFER_BUFFER_SIZE          10000

typedef struct ethernet_sniffer {
    int socket;
    int saddr_size;
    struct ifreq ifr;
    struct sockaddr saddr;
    unsigned char buffer[SNIFFER_BUFFER_SIZE];
} ethernet_sniffer;

int ethernet_sniffer_init(ethernet_sniffer* sniffer, char* interface_name);
void ethernet_sniffer_sniff(ethernet_sniffer* sniffer, void(*callback)());

#endif