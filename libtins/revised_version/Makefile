CC = g++
CFLAGS = -O3 -lpthread -ltins -lpcap -c -std=c++0x 
LFLAGS = -O3 -lpthread -ltins -lpcap -Wall -std=c++0x

WARP_PROTOCOL_DIR = ../warp_protocol/
INC = -I$(WARP_PROTOCOL_DIR)

all : mon_to_warp wlan_to_warp warp_to_mon warp_to_wlan

wlan_to_warp : warp_protocol.o wlan_to_warp.o util.o
	$(CC) warp_protocol.o wlan_to_warp.o util.o $(LFLAGS) -o wlan_to_warp.out

mon_to_warp : warp_protocol.o mon_to_warp.o
	$(CC) warp_protocol.o mon_to_warp.o $(LFLAGS) -o mon_to_warp.out


warp_to_mon : warp_protocol.o warp_to_mon.o 
	$(CC) warp_to_mon.o warp_protocol.o $(LFLAGS) -o warp_to_mon.out

warp_to_wlan : warp_protocol.o warp_to_wlan.o
	$(CC) warp_to_wlan.o warp_protocol.o $(LFLAGS) -o warp_to_wlan.out




wlan_to_warp.o : wlan_to_warp.cpp
	$(CC) wlan_to_warp.cpp $(CFLAGS) -o wlan_to_warp.o

mon_to_warp.o : mon_to_warp.cpp
	$(CC) mon_to_warp.cpp $(CFLAGS) -o mon_to_warp.o


warp_to_mon.o : warp_to_mon.cpp
	$(CC) warp_to_mon.cpp $(CFLAGS) -o warp_to_mon.o

warp_to_wlan.o : warp_to_wlan.cpp
	$(CC) warp_to_wlan.cpp $(CFLAGS) -o warp_to_wlan.o


warp_protocol.o : $(WARP_PROTOCOL_DIR)warp_protocol.cpp
	$(CC) $(WARP_PROTOCOL_DIR)warp_protocol.cpp $(CFLAGS) $(INC) -o warp_protocol.o

util.o : util.cpp
	$(CC) util.cpp $(CFLAGS) -o util.o

clean:
	rm -rf *.o
	rm -rf *.out
	rm -rf *.output
