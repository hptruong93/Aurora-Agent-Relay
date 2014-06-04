# Kevin Han
# McGill University Broadband Communications Research Lab
# Smart Applications on Virtual Infrastructure Team
# May 2014

CC = g++
CFLAGS = -O3 -std=c++11 -lpthread -ltins -lpcap -Wall -c
LFLAGS = -O3 -std=c++11 -lpthread -ltins -lpcap -Wall

all : ethToWifi wifiToEth pprocess pcrafter

ethToWifi : ethToWifi.o WARPControl_pdu.o
	$(CC) ethToWifi.o WARPControl_pdu.o -o ethToWifi $(LFLAGS)

wifiToEth : wifiToEth.o WARPControl_pdu.o
	$(CC) wifiToEth.o WARPControl_pdu.o -o wifiToEth $(LFLAGS)

pprocess : process_packets.o WARPControl_pdu.o
	$(CC) process_packets.o WARPControl_pdu.o -o pprocess $(LFLAGS)

pcrafter : packet_crafter.o WARPControl_pdu.o
	$(CC) packet_crafter.o WARPControl_pdu.o -o pcrafter $(LFLAGS)

ethToWifi.o : ethToWifi.h ethToWifi.cpp
	$(CC) ethToWifi.cpp $(CFLAGS)

wifiToEth.o : wifiToEth.h wifiToEth.cpp
	$(CC) wifiToEth.cpp $(CFLAGS)

process_packets.o : process_packets.cpp
	$(CC) process_packets.cpp $(CFLAGS)

packet_crafter.o : packet_crafter.cpp
	$(CC) packet_crafter.cpp $(CFLAGS)

WARPControl_pdu.o : WARPControl_pdu.h WARPControl_pdu.cpp
	$(CC) WARPControl_pdu.cpp $(CFLAGS)

clean:
	rm -rf *.o *~ ethToWifi wifiToEth pprocess pcrafter
