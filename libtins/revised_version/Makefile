CC = g++
# -pthread can be removed when c++11 is used
COMMON_FLAGS = -g -O3 -pthread -ltins -lpcap -std=c++0x
CFLAGS = $(COMMON_FLAGS) -c
LFLAGS = $(COMMON_FLAGS) -Wall

WARP_PROTOCOL_DIR = ../warp_protocol/
INC_WARP_PROTOCOL = -I$(WARP_PROTOCOL_DIR)

SEND_RECEIVE_MODULE_DIR = ../send_receive_module/
INC_SEND_MODULE = -I$(SEND_RECEIVE_MODULE_DIR)

###########################################################################################################

all : test

###################################Linking files together##################################################

test : util.o test.o
	$(CC) util.o test.o $(LFLAGS) -o test.out

mon_to_warp : util.o config.o warp_protocol.o  warp_protocol_sender.o mon_to_warp.o
	$(CC) util.o config.o warp_protocol.o  warp_protocol_sender.o mon_to_warp.o $(LFLAGS) -o mon_to_warp.out

wlan_to_warp : util.o config.o warp_protocol.o  warp_protocol_sender.o wlan_to_warp.o
	$(CC) util.o config.o warp_protocol.o  warp_protocol_sender.o wlan_to_warp.o $(LFLAGS) -o wlan_to_warp.out

warp_to_wlan : util.o config.o warp_protocol.o fragment_receiver.o warp_to_wlan.o
	$(CC) util.o config.o warp_protocol.o fragment_receiver.o warp_to_wlan.o $(LFLAGS) -o warp_to_wlan.out

###################################Compiling main files####################################################

wlan_to_warp.o : wlan_to_warp.cpp
	$(CC) wlan_to_warp.cpp $(CFLAGS) -o wlan_to_warp.o

mon_to_warp.o : mon_to_warp.cpp
	$(CC) mon_to_warp.cpp $(CFLAGS) -o mon_to_warp.o

warp_to_wlan.o : warp_to_wlan.cpp
	$(CC) warp_to_wlan.cpp $(CFLAGS) -o warp_to_wlan.o

test.o : test.cpp
	$(CC) test.cpp $(CFLAGS) -o test.o

##################################Files to link later######################################################

fragment_receiver.o : $(SEND_RECEIVE_MODULE_DIR)fragment_receiver.cpp
	$(CC) $(SEND_RECEIVE_MODULE_DIR)fragment_receiver.cpp $(CFLAGS) $(INC_SEND_MODULE) -o fragment_receiver.o

warp_protocol_sender.o : $(SEND_RECEIVE_MODULE_DIR)warp_protocol_sender.cpp
	$(CC) $(SEND_RECEIVE_MODULE_DIR)warp_protocol_sender.cpp $(CFLAGS) $(INC_SEND_MODULE) -o warp_protocol_sender.o

warp_protocol.o : $(WARP_PROTOCOL_DIR)warp_protocol.cpp
	$(CC) $(WARP_PROTOCOL_DIR)warp_protocol.cpp $(CFLAGS) $(INC_WARP_PROTOCOL) -o warp_protocol.o

util.o : util.cpp
	$(CC) util.cpp $(CFLAGS) -o util.o

config.o : config.cpp
	$(CC) config.cpp $(CFLAGS) -o config.o

clean:
	rm -rf *.o
	rm -rf *.out
	rm -rf *.output