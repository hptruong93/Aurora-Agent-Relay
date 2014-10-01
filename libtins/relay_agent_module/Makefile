CC = g++
# -pthread can be removed when c++11 is used
COMMON_FLAGS = -g -O3 -pthread -ltins -lpcap -lzmq -std=c++0x
CFLAGS = $(COMMON_FLAGS) -c
LFLAGS = $(COMMON_FLAGS) -Wall

WARP_PROTOCOL_DIR = ../warp_protocol/
INC_WARP_PROTOCOL = -I$(WARP_PROTOCOL_DIR)

SEND_RECEIVE_MODULE_DIR = ../send_receive_module/
INC_SEND_MODULE = -I$(SEND_RECEIVE_MODULE_DIR)

UTILITY_DIR = ../revised_version/
INC_UTILITY = -I$(UTILITY_DIR)

###########################################################################################################

all: agents

########################################  Linking  ########################################################

agents : util.o config.o warp_protocol.o fragment_receiver.o warp_protocol_sender.o relay_agent.o wlan_to_warp_agent.o agents.o
	$(CC) util.o config.o warp_protocol.o fragment_receiver.o warp_protocol_sender.o relay_agent.o wlan_to_warp_agent.o agents.o $(LFLAGS) -o agents.out

###################################Compiling main files####################################################


wlan_to_warp_agent.o : wlan_to_warp_agent.cpp
	$(CC) wlan_to_warp_agent.cpp $(CFLAGS) -o wlan_to_warp_agent.o

mon_to_warp_agent.o : mon_to_warp_agent.cpp
	$(CC) mon_to_warp_agent.cpp $(CFLAGS) -o mon_to_warp_agent.o

warp_to_wlan_agent.o : warp_to_wlan_agent.cpp
	$(CC) warp_to_wlan_agent.cpp $(CFLAGS) -o warp_to_wlan_agent.o

relay_agent.o : relay_agent.cpp
	$(CC) relay_agent.cpp $(CFLAGS) -o relay_agent.o

agents.o : agents.cpp
	$(CC) agents.cpp $(CFLAGS) -o agents.o

##################################Files to link later######################################################

fragment_receiver.o : $(SEND_RECEIVE_MODULE_DIR)fragment_receiver.cpp
	$(CC) $(SEND_RECEIVE_MODULE_DIR)fragment_receiver.cpp $(CFLAGS) $(INC_SEND_MODULE) -o fragment_receiver.o

warp_protocol_sender.o : $(SEND_RECEIVE_MODULE_DIR)warp_protocol_sender.cpp
	$(CC) $(SEND_RECEIVE_MODULE_DIR)warp_protocol_sender.cpp $(CFLAGS) $(INC_SEND_MODULE) -o warp_protocol_sender.o

warp_protocol.o : $(WARP_PROTOCOL_DIR)warp_protocol.cpp
	$(CC) $(WARP_PROTOCOL_DIR)warp_protocol.cpp $(CFLAGS) $(INC_WARP_PROTOCOL) -o warp_protocol.o

util.o : $(UTILITY_DIR)util.cpp
	$(CC) $(UTILITY_DIR)util.cpp $(CFLAGS) $(INC_UTILITY) -o util.o

config.o : $(UTILITY_DIR)config.cpp
	$(CC) $(UTILITY_DIR)config.cpp $(CFLAGS) $(INC_UTILITY) -o config.o

###########################################################################################################

clean:
	rm -rf *.o
	rm -rf *.out
	rm -rf *.output