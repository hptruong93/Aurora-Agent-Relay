CC = gcc
PP = g++
COMMON_FLAGS = -g -O3 -lpcap -lpthread
COMMON_CPP_FLAGS = -g -O3 -lpcap -lpthread -ltins -std=c++0x

CFLAGS = $(COMMON_FLAGS) -c
CPP_FLAGS = $(COMMON_CPP_FLAGS) -c
LFLAGS = $(COMMON_FLAGS) -Wall
LCPP_FLAGS = $(COMMON_CPP_FLAGS) -Wall

WARP_PROTOCOL_DIR = ../warp_protocol/
INC_WARP_PROTOCOL = -I$(WARP_PROTOCOL_DIR)

SEND_RECEIVE_MODULE_DIR = ../send_receive_module/
INC_SEND_MODULE = -I$(SEND_RECEIVE_MODULE_DIR)

UTIL_MODULE_DIR = ../revised_version/
INC_UTIL_MODULE = -I$(UTIL_MODULE_DIR)

###########################################################################################################

all : libtin_test

###################################Linking files together##################################################

# integrated : ethernet_sender.o ethernet_sniffer.o receive.o
# 	$(CC) ethernet_sniffer.o ethernet_sender.o receive.o $(LFLAGS) -o integrated.out

# test_sender : ethernet_sender.o
# 	$(CC) ethernet_sender.o $(LFLAGS) -o sender.out

# test_sniffer : ethernet_sniffer.o
# 	$(CC) ethernet_sniffer.o $(LFLAGS) -o sniffer.out

# send_wlan_c : ethernet_sender.o ethernet_sniffer.o send_wlan_c.o
	# $(CC) ethernet_sniffer.o ethernet_sender.o send_wlan_c.o $(LFLAGS) -o send_wlan_c.out

libtin_test : send_mon.o warp_protocol.o util.o
	# $(PP) send_wlan.o $(LCPP_FLAGS) -o send_wlan.out
	$(PP) send_mon.o warp_protocol.o util.o $(LCPP_FLAGS) -o send_mon.out

###################################Compiling main files####################################################

receive.o : receive.c
	$(CC) receive.c $(CFLAGS) -o receive.o

ethernet_sender.o : ethernet_sender.c
	$(CC) ethernet_sender.c $(CFLAGS) -o ethernet_sender.o

ethernet_sniffer.o : ethernet_sniffer.c
	$(CC) ethernet_sniffer.c $(CFLAGS) -o ethernet_sniffer.o

send_wlan_c.o : send_wlan_c.c
	$(CC) send_wlan_c.c $(CFLAGS) -o send_wlan_c.o

send_wlan.o : send_wlan.cpp
	$(PP) send_wlan.cpp $(CPP_FLAGS) -o send_wlan.o

send_mon.o : send_mon.cpp
	$(PP) send_mon.cpp $(CPP_FLAGS) -o send_mon.o

warp_protocol.o : $(WARP_PROTOCOL_DIR)warp_protocol.cpp
	$(PP) $(WARP_PROTOCOL_DIR)warp_protocol.cpp $(CPP_FLAGS) $(INC_WARP_PROTOCOL) -o warp_protocol.o

util.o : $(UTIL_MODULE_DIR)util.cpp
	$(PP) $(UTIL_MODULE_DIR)util.cpp $(CPP_FLAGS) $(INC_UTIL_MODULE) -o util.o

clean:
	rm -rf *.o
	rm -rf *.out
	rm -rf *.output